import atexit
import subprocess
import time
import shutil
import signal
import os
from collections import namedtuple
import re
import sys

from .testUtils import Utils

Wallet=namedtuple("Wallet", "name password host port")
# pylint: disable=too-many-instance-attributes
class WalletMgr(object):
    __walletDataDir=f"{Utils.DataPath}/test_wallet_0"
    __walletLogOutFile=f"{__walletDataDir}/test_kiod_out.log"
    __walletLogErrFile=f"{__walletDataDir}/test_kiod_err.log"
    __MaxPort=9999

    # pylint: disable=too-many-arguments
    # walletd [True|False] True=Launch wallet(kiod) process; False=Manage launch process externally.
    def __init__(self, walletd, nodeopPort=8888, nodeopHost="localhost", port=9899, host="localhost", keepRunning=False, keepLogs=False):
        atexit.register(self.shutdown)
        self.walletd=walletd
        self.nodeopPort=nodeopPort
        self.nodeopHost=nodeopHost
        self.port=port
        self.host=host
        self.keepRunning=keepRunning
        self.keepLogs=keepLogs or keepRunning
        self.testFailed=False
        self.wallets={}
        self.popenProc=None
        self.walletPid=None

    def getWalletEndpointArgs(self):
        if not self.walletd or not self.isLaunched():
            return ""

        return " --wallet-url http://%s:%d" % (self.host, self.port)

    def getArgs(self):
        return " --url http://%s:%d%s %s" % (self.nodeopHost, self.nodeopPort, self.getWalletEndpointArgs(), Utils.MiscSysClientArgs)

    def isLaunched(self):
        return self.walletPid is not None

    def isLocal(self):
        return self.host=="localhost" or self.host=="127.0.0.1"

    def findAvailablePort(self):
        for i in range(WalletMgr.__MaxPort):
            port=self.port+i
            if port > WalletMgr.__MaxPort:
                port-=WalletMgr.__MaxPort
            if Utils.arePortsAvailable(port):
                return port
            if Utils.Debug: Utils.Print("Port %d not available for %s" % (port, Utils.SysWalletPath))

        Utils.errorExit("Failed to find free port to use for %s" % (Utils.SysWalletPath))

    def launch(self):
        if not self.walletd:
            Utils.Print("ERROR: Wallet Manager wasn't configured to launch kiod")
            return False

        if self.isLaunched():
            return True

        if self.isLocal():
            self.port=self.findAvailablePort()

        pgrepCmd=Utils.pgrepCmd(Utils.SysWalletName)
        if Utils.Debug:
            portTaken=False
            if self.isLocal():
                if not Utils.arePortsAvailable(self.port):
                    portTaken=True
            psOut=Utils.checkOutput(pgrepCmd.split(), ignoreError=True)
            if psOut or portTaken:
                statusMsg=""
                if psOut:
                    statusMsg+=" %s - {%s}." % (pgrepCmd, psOut)
                if portTaken:
                    statusMsg+=" port %d is NOT available." % (self.port)
                Utils.Print("Launching %s, note similar processes running. %s" % (Utils.SysWalletName, statusMsg))

        cmd="%s --data-dir %s --config-dir %s --unlock-timeout=999999 --http-server-address=%s:%d --http-max-response-time-ms 99999 --verbose-http-errors" % (
            Utils.SysWalletPath, WalletMgr.__walletDataDir, WalletMgr.__walletDataDir, self.host, self.port)
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        if not os.path.isdir(WalletMgr.__walletDataDir):
            if Utils.Debug: Utils.Print(f"Creating dir {WalletMgr.__walletDataDir} in dir: {os.getcwd()}")
            os.makedirs(WalletMgr.__walletDataDir)
        with open(WalletMgr.__walletLogOutFile, 'w') as sout, open(WalletMgr.__walletLogErrFile, 'w') as serr:
            self.popenProc=subprocess.Popen(cmd.split(), stdout=sout, stderr=serr)
            self.walletPid=self.popenProc.pid

        # Give kiod time to warm up
        time.sleep(2)

        try:
            if Utils.Debug: Utils.Print("Checking if %s launched. %s" % (Utils.SysWalletName, pgrepCmd))
            psOut=Utils.checkOutput(pgrepCmd.split())
            if Utils.Debug: Utils.Print("Launched %s. {%s}" % (Utils.SysWalletName, psOut))
        except subprocess.CalledProcessError as ex:
            Utils.Print(f"STDOUT: {ex.output}")
            Utils.Print(f"STDERR: {ex.stderr}")
            Utils.errorExit("Failed to launch the wallet manager")

        return True

    def create(self, name, accounts=None, exitOnError=True):
        wallet=self.wallets.get(name)
        if wallet is not None:
            if Utils.Debug: Utils.Print("Wallet \"%s\" already exists. Returning same." % name)
            return wallet
        p = re.compile(r'\n\"(\w+)\"\n', re.MULTILINE)
        cmdDesc="wallet create"
        cmd="%s %s %s --name %s --to-console" % (Utils.SysClientPath, self.getArgs(), cmdDesc, name)
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        retStr=None
        maxRetryCount=4
        retryCount=0
        while True:
            try:
                retStr=Utils.checkOutput(cmd.split())
                break
            except subprocess.CalledProcessError as ex:
                retryCount+=1
                if retryCount<maxRetryCount:
                    delay=10
                    pgrepCmd=Utils.pgrepCmd(Utils.SysWalletName)
                    psOut=Utils.checkOutput(pgrepCmd.split())
                    portStatus="N/A"
                    if self.isLocal():
                        if Utils.arePortsAvailable(self.port):
                            portStatus="AVAILABLE"
                        else:
                            portStatus="NOT AVAILABLE"
                    if Utils.Debug: Utils.Print("%s was not accepted, delaying for %d seconds and trying again. port %d is %s. %s - {%s}" % (cmdDesc, delay, self.port, pgrepCmd, psOut, portStatus))
                    time.sleep(delay)
                    continue

                msg=ex.stderr.decode("utf-8")
                errorMsg="ERROR: Failed to create wallet - %s. %s" % (name, msg)
                if exitOnError:
                    Utils.errorExit("%s" % (errorMsg))
                Utils.Print("%s" % (errorMsg))
                return None

        m=p.search(retStr)
        if m is None:
            if exitOnError:
                Utils.cmdError("could not create wallet %s" % (name))
                Utils.errorExit("Failed  to create wallet %s" % (name))

            Utils.Print("ERROR: wallet password parser failure")
            return None
        p=m.group(1)
        wallet=Wallet(name, p, self.host, self.port)
        self.wallets[name] = wallet

        if accounts:
            self.importKeys(accounts,wallet)

        return wallet

    def importKeys(self, accounts, wallet, ignoreDupKeyWarning=False):
        for account in accounts:
            Utils.Print("Importing keys for account %s into wallet %s." % (account.name, wallet.name))
            if not self.importKey(account, wallet, ignoreDupKeyWarning):
                Utils.Print("ERROR: Failed to import key for account %s" % (account.name))
                return False

    def importKey(self, account, wallet, ignoreDupKeyWarning=False):
        warningMsg="Key already in wallet"
        cmd="%s %s wallet import --name %s --private-key %s" % (
            Utils.SysClientPath, self.getArgs(), wallet.name, account.ownerPrivateKey)
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        try:
            Utils.checkOutput(cmd.split())
        except subprocess.CalledProcessError as ex:
            msg=ex.stderr.decode("utf-8")
            if warningMsg in msg:
                if not ignoreDupKeyWarning:
                    Utils.Print("WARNING: This key is already imported into the wallet.")
            else:
                Utils.Print("ERROR: Failed to import account owner key %s. %s" % (account.ownerPrivateKey, msg))
                return False

        if account.activePrivateKey is None:
            Utils.Print("WARNING: Active private key is not defined for account \"%s\"" % (account.name))
        else:
            cmd="%s %s wallet import --name %s  --private-key %s" % (
                Utils.SysClientPath, self.getArgs(), wallet.name, account.activePrivateKey)
            if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
            try:
                Utils.checkOutput(cmd.split())
            except subprocess.CalledProcessError as ex:
                msg=ex.stderr.decode("utf-8")
                if warningMsg in msg:
                    if not ignoreDupKeyWarning:
                        Utils.Print("WARNING: This key is already imported into the wallet.")
                else:
                    Utils.Print("ERROR: Failed to import account active key %s. %s" %
                                (account.activePrivateKey, msg))
                    return False

        return True

    def lockWallet(self, wallet):
        cmd="%s %s wallet lock --name %s" % (Utils.SysClientPath, self.getArgs(), wallet.name)
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        if 0 != subprocess.call(cmd.split(), stdout=Utils.FNull):
            Utils.Print("ERROR: Failed to lock wallet %s." % (wallet.name))
            return False

        return True

    def unlockWallet(self, wallet):
        cmd="%s %s wallet unlock --name %s" % (Utils.SysClientPath, self.getArgs(), wallet.name)
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        popen=subprocess.Popen(cmd.split(), stdout=Utils.FNull, stdin=subprocess.PIPE)
        _, errs = popen.communicate(input=wallet.password.encode("utf-8"))
        if 0 != popen.wait():
            Utils.Print("ERROR: Failed to unlock wallet %s: %s" % (wallet.name, errs.decode("utf-8")))
            return False

        return True

    def lockAllWallets(self):
        cmd="%s %s wallet lock_all" % (Utils.SysClientPath, self.getArgs())
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        if 0 != subprocess.call(cmd.split(), stdout=Utils.FNull):
            Utils.Print("ERROR: Failed to lock all wallets.")
            return False

        return True

    def getOpenWallets(self):
        wallets=[]

        p = re.compile(r'\s+\"(\w+)\s\*\",?\n', re.MULTILINE)
        cmd="%s %s wallet list" % (Utils.SysClientPath, self.getArgs())
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        retStr=None
        try:
            retStr=Utils.checkOutput(cmd.split())
        except subprocess.CalledProcessError as ex:
            msg=ex.stderr.decode("utf-8")
            Utils.Print("ERROR: Failed to open wallets. %s" % (msg))
            return False

        m=p.findall(retStr)
        if m is None:
            Utils.Print("ERROR: wallet list parser failure")
            return None
        wallets=m

        return wallets

    def getKeys(self, wallet):
        keys=[]

        p = re.compile(r'\n\s+\"(\w+)\"\n', re.MULTILINE)
        cmd="%s %s wallet private_keys --name %s --password %s " % (Utils.SysClientPath, self.getArgs(), wallet.name, wallet.password)
        if Utils.Debug: Utils.Print("cmd: %s" % (cmd))
        retStr=None
        try:
            retStr=Utils.checkOutput(cmd.split())
        except subprocess.CalledProcessError as ex:
            msg=ex.stderr.decode("utf-8")
            Utils.Print("ERROR: Failed to get keys. %s" % (msg))
            return False
        m=p.findall(retStr)
        if m is None:
            Utils.Print("ERROR: wallet private_keys parser failure")
            return None
        keys=m

        return keys


    def dumpErrorDetails(self):
        Utils.Print("=================================================================")
        if self.walletPid is not None:
            Utils.Print("Contents of %s:" % (WalletMgr.__walletLogOutFile))
            Utils.Print("=================================================================")
            with open(WalletMgr.__walletLogOutFile, "r") as f:
                shutil.copyfileobj(f, sys.stdout)
            Utils.Print("Contents of %s:" % (WalletMgr.__walletLogErrFile))
            Utils.Print("=================================================================")
            with open(WalletMgr.__walletLogErrFile, "r") as f:
                shutil.copyfileobj(f, sys.stdout)

    def shutdown(self):
        '''Shutdown the managed kiod instance unless keepRunning was set.'''
        if self.keepRunning:
            return
        if self.popenProc:
            Utils.Print(f"Shutting down wallet manager process {self.walletPid}")
            self.popenProc.send_signal(signal.SIGTERM)
            self.popenProc.wait()
        elif self.walletPid:
            Utils.Print("Killing wallet manager process %d" % (self.walletPid))
            os.kill(self.walletPid, signal.SIGKILL)
        self.cleanup()

    def cleanup(self):
        if self.keepLogs or self.keepRunning or self.testFailed:
            return
        if os.path.isdir(WalletMgr.__walletDataDir) and os.path.exists(WalletMgr.__walletDataDir):
            shutil.rmtree(WalletMgr.__walletDataDir)
