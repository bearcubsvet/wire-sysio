#!/usr/bin/env python3

import random
import time
import signal
import threading
import os
import platform
import traceback

from TestHarness import Account, Cluster, ReturnType, TestHelper, Utils, WalletMgr
from TestHarness.TestHelper import AppArgs

###############################################################
# send_read_only_transaction_tests
#
# Tests to exercise the send_read_only_transaction RPC endpoint functionality.
# More internal tests are in unittest/read_only_trx_tests.cpp
#
###############################################################

Print=Utils.Print
errorExit=Utils.errorExit

appArgs=AppArgs()
appArgs.add(flag="--read-only-threads", type=int, help="number of read-only threads", default=0)
appArgs.add(flag="--num-test-runs", type=int, help="number of times to run the tests", default=1)
appArgs.add(flag="--sys-vm-oc-enable", type=str, help="specify sys-vm-oc-enable option", default="auto")
appArgs.add(flag="--wasm-runtime", type=str, help="if set to sys-vm-oc, must compile with SYSIO_SYS_VM_OC_DEVELOPER", default="sys-vm-jit")

args=TestHelper.parse_args({"-p","-n","-d","-s","--nodes-file","--seed"
                            ,"--dump-error-details","-v","--leave-running"
                            ,"--keep-logs","--unshared"}, applicationSpecificArgs=appArgs)

pnodes=args.p
topo=args.s
delay=args.d
total_nodes = pnodes if args.n < pnodes else args.n
# For this test, we need at least 1 non-producer
if total_nodes <= pnodes:
    Print ("non-producing nodes %d must be greater than 0. Force it to %d. producing nodes: %d," % (total_nodes - pnodes, pnodes + 1, pnodes))
    total_nodes = pnodes + 1
debug=args.v
nodesFile=args.nodes_file
dontLaunch=nodesFile is not None
seed=args.seed
dumpErrorDetails=args.dump_error_details
numTestRuns=args.num_test_runs

Utils.Debug=debug
testSuccessful=False
errorInThread=False
noOC = args.sys_vm_oc_enable == "none"
allOC = args.sys_vm_oc_enable == "all"

random.seed(seed) # Use a fixed seed for repeatability.
# all debuglevel so that "executing ${h} with sys vm oc" is logged
cluster=Cluster(loggingLevel="all", unshared=args.unshared, keepRunning=True if nodesFile is not None else args.leave_running, keepLogs=args.keep_logs)

walletMgr=WalletMgr(True)
SYSIO_ACCT_PRIVATE_DEFAULT_KEY = "5KQwrPbwdL6PhXujxW37FSSQZ1JiwsST4cqQzDeyXtP79zkvFD3"
SYSIO_ACCT_PUBLIC_DEFAULT_KEY = "SYS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"

producerNode = None
apiNode = None
testAccountName = "test"
userAccountName = "user"
payloadlessAccountName = "payloadless"

def getCodeHash(node, account):
    # Example get code result: code hash: 67d0598c72e2521a1d588161dad20bbe9f8547beb5ce6d14f3abd550ab27d3dc
    cmd = f"get code {account}"
    codeHash = node.processClioCmd(cmd, cmd, silentErrors=False, returnType=ReturnType.raw)
    if codeHash is None: errorExit(f"Unable to get code {account} from node {node.nodeId}")
    else: codeHash = codeHash.split(' ')[2].strip()
    if Utils.Debug: Utils.Print(f"{account} code hash: {codeHash}")
    return codeHash

def startCluster():
    global total_nodes
    global producerNode
    global apiNode

    TestHelper.printSystemInfo("BEGIN")
    cluster.setWalletMgr(walletMgr)

    if dontLaunch: # run test against remote cluster
        jsonStr=None
        with open(nodesFile, "r") as f:
            jsonStr=f.read()
        if not cluster.initializeNodesFromJson(jsonStr):
            errorExit("Failed to initilize nodes from Json string.")
        total_nodes=len(cluster.getNodes())

        print("Stand up walletd")
        if walletMgr.launch() is False:
            errorExit("Failed to stand up kiod.")

    Print ("producing nodes: %d, non-producing nodes: %d, topology: %s, delay between nodes launch(seconds): %d" % (pnodes, total_nodes-pnodes, topo, delay))

    Print("Stand up cluster")
    # set up read-only options for API node
    specificExtraNodeopArgs={}
    # producer nodes will be mapped to 0 through pnodes-1, so the number pnodes is the no-producing API node
    specificExtraNodeopArgs[pnodes]=" --plugin sysio::net_api_plugin"
    specificExtraNodeopArgs[pnodes]+=" --contracts-console "
    specificExtraNodeopArgs[pnodes]+=" --read-only-write-window-time-us "
    specificExtraNodeopArgs[pnodes]+=" 10000 "
    specificExtraNodeopArgs[pnodes]+=" --read-only-read-window-time-us "
    specificExtraNodeopArgs[pnodes]+=" 490000 "
    specificExtraNodeopArgs[pnodes]+=" --sys-vm-oc-cache-size-mb "
    specificExtraNodeopArgs[pnodes]+=" 1 " # set small so there is churn
    specificExtraNodeopArgs[pnodes]+=" --read-only-threads "
    specificExtraNodeopArgs[pnodes]+=str(args.read_only_threads)
    if args.sys_vm_oc_enable:
        if platform.system() != "Linux":
            Print("OC not run on Linux. Skip the test")
            exit(True) # Do not fail the test
        specificExtraNodeopArgs[pnodes]+=" --sys-vm-oc-enable "
        specificExtraNodeopArgs[pnodes]+=args.sys_vm_oc_enable
    if args.wasm_runtime:
        specificExtraNodeopArgs[pnodes]+=" --wasm-runtime "
        specificExtraNodeopArgs[pnodes]+=args.wasm_runtime
    extraNodeopArgs=" --http-max-response-time-ms 990000 --disable-subjective-api-billing false "
    if cluster.launch(pnodes=pnodes, totalNodes=total_nodes, topo=topo, delay=delay, specificExtraNodeopArgs=specificExtraNodeopArgs, extraNodeopArgs=extraNodeopArgs ) is False:
        errorExit("Failed to stand up sys cluster.")

    Print ("Wait for Cluster stabilization")
    # wait for cluster to start producing blocks
    if not cluster.waitOnClusterBlockNumSync(3):
        errorExit("Cluster never stabilized")

    producerNode = cluster.getNode()
    apiNode = cluster.nodes[-1]

    sysioCodeHash = getCodeHash(producerNode, "sysio.token")
    # sysio.* should be using oc unless oc tierup disabled
    Utils.Print(f"search: executing {sysioCodeHash} with sys vm oc")
    found = producerNode.findInLog(f"executing {sysioCodeHash} with sys vm oc")
    assert( found or (noOC and not found) )

    if args.sys_vm_oc_enable:
        verifyOcVirtualMemory()

def verifyOcVirtualMemory():
    try:
        with open(f"/proc/{apiNode.pid}/statm") as f:
            data = f.read().split()
            vmPages = int(data[0])
            pageSize = os.sysconf("SC_PAGESIZE")
            actualVmSize = vmPages * pageSize

            # In OC tierup a memory slice is 8GB;
            # The main thread uses 529 slices; each read-only thread uses 11 slices.
            # Total virtual memory is around:
            # 529 slices * 8GB (for main thread) + numReadOnlyThreads * 11 slices * 8GB
            # This test verifies virtual memory does not grow by the number
            # of read-only thread in the order of TB.
            memoryByOthersGB = 1000 # add 1TB for virtual memory used by others
            expectedVmSize = ((529 * 8) + (args.read_only_threads * 88) + memoryByOthersGB) * 1024 * 1024 * 1024
            Utils.Print(f"pid: {apiNode.pid}, actualVmSize: {actualVmSize}, expectedVmSize: {expectedVmSize}")
            assert(actualVmSize < expectedVmSize)
    except FileNotFoundError:
        Utils.Print(f"/proc/{apiNode.pid}/statm not found")
        assert(False)

def deployTestContracts():
    Utils.Print("create test accounts")
    testAccount = Account(testAccountName)
    testAccount.ownerPublicKey = SYSIO_ACCT_PUBLIC_DEFAULT_KEY
    testAccount.activePublicKey = SYSIO_ACCT_PUBLIC_DEFAULT_KEY
    cluster.createAccountAndVerify(testAccount, cluster.sysioAccount, nodeOwner=cluster.carlAccount, buyRAM=500000) # 95632 bytes required for test contract

    userAccount = Account(userAccountName)
    userAccount.ownerPublicKey = SYSIO_ACCT_PUBLIC_DEFAULT_KEY
    userAccount.activePublicKey = SYSIO_ACCT_PUBLIC_DEFAULT_KEY
    cluster.createAccountAndVerify(userAccount, cluster.sysioAccount, nodeOwner=cluster.carlAccount, stakeCPU=2000)

    noAuthTableContractDir="unittests/test-contracts/no_auth_table"
    noAuthTableWasmFile="no_auth_table.wasm"
    noAuthTableAbiFile="no_auth_table.abi"
    Utils.Print("Publish no_auth_table contract")
    producerNode.publishContract(testAccount, noAuthTableContractDir,noAuthTableWasmFile, noAuthTableAbiFile, waitForTransBlock=True)

    Utils.Print("Create payloadless account and deploy payloadless contract")
    payloadlessAccount = Account(payloadlessAccountName)
    payloadlessAccount.ownerPublicKey = SYSIO_ACCT_PUBLIC_DEFAULT_KEY
    payloadlessAccount.activePublicKey = SYSIO_ACCT_PUBLIC_DEFAULT_KEY
    cluster.createAccountAndVerify(payloadlessAccount, cluster.sysioAccount, nodeOwner=cluster.carlAccount, buyRAM=100000)
    payloadlessContractDir="unittests/test-contracts/payloadless"
    payloadlessWasmFile="payloadless.wasm"
    payloadlessAbiFile="payloadless.abi"
    producerNode.publishContract(payloadlessAccount, payloadlessContractDir, payloadlessWasmFile, payloadlessAbiFile, waitForTransBlock=True)

def sendTransaction(account, action, data, auth=[], opts=None):
    trx = {
       "actions": [{
          "account": account,
          "name": action,
          "authorization": auth,
          "data": data
      }]
    }
    return apiNode.pushTransaction(trx, opts)

def sendReadOnlyPayloadless():
    return sendTransaction('payloadless', action='doit', data={}, auth=[], opts='--read')

def sendReadOnlySlowPayloadless():
    return sendTransaction('payloadless', action='doitslow', data={}, auth=[], opts='--read')

# Send read-only trxs from mutltiple threads to bump load
def sendReadOnlyTrxOnThread(startId, numTrxs):
    Print("start sendReadOnlyTrxOnThread")

    global errorInThread
    errorInThread = False
    try:
       for i in range(numTrxs):
           results = sendTransaction(testAccountName, 'age', {"user": userAccountName, "id": startId + i}, opts='--read')
           assert(results[0])
           assert(results[1]['processed']['action_traces'][0]['return_value_data'] == 25)

           results = sendReadOnlyPayloadless()
           assert(results[0])
           assert(results[1]['processed']['action_traces'][0]['console'] == "Im a payloadless action")

           results = sendReadOnlySlowPayloadless()
           assert(results[0])
           assert(results[1]['processed']['action_traces'][0]['console'] == "Im a payloadless slow action")
           assert(int(results[1]['processed']['elapsed']) > 100)
    except Exception as e:
        Print("Exception in sendReadOnlyTrxOnThread: ", repr(e))
        traceback.print_exc()
        errorInThread = True

# Send regular trxs from mutltiple threads to bump load
def sendTrxsOnThread(startId, numTrxs, opts=None):
    Print("sendTrxsOnThread: ", startId, numTrxs, opts)

    global errorInThread
    errorInThread = False
    try:
        for i in range(numTrxs):
            results = sendTransaction(testAccountName, 'age', {"user": userAccountName, "id": startId + i}, auth=[{"actor": userAccountName, "permission":"active"}], opts=opts)
            assert(results[0])
    except Exception as e:
        Print("Exception in sendTrxsOnThread: ", repr(e))
        traceback.print_exc()
        errorInThread = True

def doRpc(resource, command, numRuns, fieldIn, expectedValue, code, payload={}):
    global errorInThread
    errorInThread = False
    try:
        for i in range(numRuns):
            ret_json = apiNode.processUrllibRequest(resource, command, payload)
            if code is None:
                assert(ret_json["payload"][fieldIn] is not None)
                if expectedValue is not None:
                    assert(ret_json["payload"][fieldIn] == expectedValue)
            else:
                assert(ret_json["code"] == code)
    except Exception as e:
        Print("Exception in doRpc: ", repr(e))
        traceback.print_exc()
        errorInThread = True

def runReadOnlyTrxAndRpcInParallel(resource, command, fieldIn=None, expectedValue=None, code=None, payload={}):
    Print("runReadOnlyTrxAndRpcInParallel: ", command)

    numRuns = 10
    trxThread = threading.Thread(target = sendReadOnlyTrxOnThread, args = (0, numRuns ))
    rpcThread = threading.Thread(target = doRpc, args = (resource, command, numRuns, fieldIn, expectedValue, code, payload))
    trxThread.start()
    rpcThread.start()

    trxThread.join()
    rpcThread.join()
    assert(not errorInThread)

def mixedOpsTest(opt=None):
    Print("mixedOpsTest -- opt = ", opt)

    numRuns = 200
    readOnlyThread = threading.Thread(target = sendReadOnlyTrxOnThread, args = (0, numRuns ))
    readOnlyThread.start()
    sendTrxThread = threading.Thread(target = sendTrxsOnThread, args = (numRuns, numRuns, opt))
    sendTrxThread.start()
    pushBlockThread = threading.Thread(target = doRpc, args = ("chain", "push_block", numRuns, None, None, 202, {"block":"signed_block"}))
    pushBlockThread.start()

    readOnlyThread.join()
    sendTrxThread.join()
    pushBlockThread.join()
    assert(not errorInThread)

def sendMulReadOnlyTrx(numThreads):
    threadList = []
    num_trxs_per_thread = 200
    for i in range(numThreads):
        thr = threading.Thread(target = sendReadOnlyTrxOnThread, args = (i * num_trxs_per_thread, num_trxs_per_thread ))
        thr.start()
        threadList.append(thr)
    for thr in threadList:
        thr.join()

def basicTests():
    Print("Insert a user")
    results = sendTransaction(testAccountName, 'insert', {"user": userAccountName, "id": 1, "age": 10}, auth=[{"actor": userAccountName, "permission":"active"}])
    assert(results[0])
    apiNode.waitForTransactionInBlock(results[1]['transaction_id'])

    testAccountCodeHash = getCodeHash(producerNode, testAccountName)
    found = producerNode.findInLog(f"executing {testAccountCodeHash} with sys vm oc")
    assert( (allOC and found) or not found )

    # verify the return value (age) from read-only is the same as created.
    Print("Send a read-only Get transaction to verify previous Insert")
    results = sendTransaction(testAccountName, 'getage', {"user": userAccountName}, opts='--read')
    assert(results[0])
    assert(results[1]['processed']['action_traces'][0]['return_value_data'] == 10)

    # verify non-read-only modification works
    Print("Send a non-read-only Modify transaction")
    results = sendTransaction(testAccountName, 'modify', {"user": userAccountName, "age": 25}, auth=[{"actor": userAccountName, "permission": "active"}])
    assert(results[0])
    apiNode.waitForTransactionInBlock(results[1]['transaction_id'])

def multiReadOnlyTests():
    sendMulReadOnlyTrx(numThreads=5)

def chainApiTests():
    # verify chain APIs can run in parallel with read-ony transactions
    runReadOnlyTrxAndRpcInParallel("chain", "get_info", "server_version")
    runReadOnlyTrxAndRpcInParallel("chain", "get_consensus_parameters", "chain_config")
    runReadOnlyTrxAndRpcInParallel("chain", "get_activated_protocol_features", "activated_protocol_features")
    runReadOnlyTrxAndRpcInParallel("chain", "get_block", "block_num", expectedValue=1, payload={"block_num_or_id":1})
    runReadOnlyTrxAndRpcInParallel("chain", "get_block_info", "block_num", expectedValue=1, payload={"block_num":1})
    runReadOnlyTrxAndRpcInParallel("chain", "get_account", "account_name", expectedValue=userAccountName, payload = {"account_name":userAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_code", "account_name", expectedValue=testAccountName, payload = {"account_name":testAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_code_hash", "account_name", expectedValue=testAccountName, payload = {"account_name":testAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_abi", "account_name", expectedValue=testAccountName, payload = {"account_name":testAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_raw_code_and_abi", "account_name", expectedValue=testAccountName, payload = {"account_name":testAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_raw_abi", "account_name", expectedValue=testAccountName, payload = {"account_name":testAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_producers", "rows", payload = {"json":"true","lower_bound":""})
    runReadOnlyTrxAndRpcInParallel("chain", "get_table_rows", "rows", payload = {"json":"true","code":"sysio","scope":"sysio","table":"global"})
    runReadOnlyTrxAndRpcInParallel("chain", "get_table_by_scope", fieldIn="rows", payload = {"json":"true","table":"noauth"})
    runReadOnlyTrxAndRpcInParallel("chain", "get_currency_balance", code=200, payload = {"code":"sysio.token", "account":testAccountName})
    runReadOnlyTrxAndRpcInParallel("chain", "get_currency_stats", fieldIn="SYS", payload = {"code":"sysio.token", "symbol":"SYS"})
    runReadOnlyTrxAndRpcInParallel("chain", "get_required_keys", code=400)
    runReadOnlyTrxAndRpcInParallel("chain", "get_transaction_id", code=400, payload = {"ref_block_num":"1"})
    runReadOnlyTrxAndRpcInParallel("chain", "push_block", code=202, payload = {"block":"signed_block"})
    runReadOnlyTrxAndRpcInParallel("chain", "get_producer_schedule", "active")
    runReadOnlyTrxAndRpcInParallel("chain", "get_scheduled_transactions", "transactions", payload = {"json":"true","lower_bound":""})

def netApiTests():
    # NET APIs
    runReadOnlyTrxAndRpcInParallel("net", "status", code=201, payload = "localhost")
    runReadOnlyTrxAndRpcInParallel("net", "connections", code=201)
    runReadOnlyTrxAndRpcInParallel("net", "connect", code=201, payload = "localhost")
    runReadOnlyTrxAndRpcInParallel("net", "disconnect", code=201, payload = "localhost")

def runEverythingParallel():
    threadList = []
    threadList.append(threading.Thread(target = multiReadOnlyTests))
    threadList.append(threading.Thread(target = chainApiTests))
    threadList.append(threading.Thread(target = netApiTests))
    threadList.append(threading.Thread(target = mixedOpsTest))
    for thr in threadList:
        thr.start()
    for thr in threadList:
        thr.join()

try:
    startCluster()
    deployTestContracts()

    basicTests()

    if args.read_only_threads > 0: # Save test time. No need to run other tests if multi-threaded is not enabled
        for i in range(numTestRuns):
            multiReadOnlyTests()
            chainApiTests()
            netApiTests()
            mixedOpsTest()
            runEverythingParallel()

    testSuccessful = True
finally:
    TestHelper.shutdown(cluster, walletMgr, testSuccessful, dumpErrorDetails)

errorCode = 0 if testSuccessful else 1
exit(errorCode)
