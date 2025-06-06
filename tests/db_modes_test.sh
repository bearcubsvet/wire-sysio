#!/usr/bin/env bash

# This test is intended to verify that switching between DB modes "just works". Additionally
# it tries to make sure the dirty bit behaves as expected even in heap and mapped_private mode.

set -euo pipefail

VERBOSE=0
TEST_LOCKED_MODE=0

while getopts ":lv" opt; do
   case ${opt} in
      l)
         TEST_LOCKED_MODE=1
         ;;
      v)
         VERBOSE=1
         set -o xtrace
         ;;
      \?)
         echo "Use -v for verbose; -l to enable test of locked mode"
         exit 1;
         ;;
      :)
         echo "Invalid option"
         exit 1;
         ;;
   esac
done

SYSIO_STUFF_DIR=$(mktemp -d)
trap "rm -rf $SYSIO_STUFF_DIR" EXIT
NODEOP_LAUNCH_PARAMS="./programs/nodeop/nodeop --resource-monitor-not-shutdown-on-threshold-exceeded -d $SYSIO_STUFF_DIR --config-dir $SYSIO_STUFF_DIR \
--chain-state-db-size-mb 8 --chain-state-db-guard-size-mb 0 \
-e -psysio"

run_nodeop() {
   if (( $VERBOSE == 0 )); then
      $NODEOP_LAUNCH_PARAMS --http-server-address '' --p2p-listen-endpoint '' "$@" 2>/dev/null &
   else
      $NODEOP_LAUNCH_PARAMS --http-server-address '' --p2p-listen-endpoint '' "$@" &
   fi
}

run_expect_success() {
   run_nodeop "$@"
   local NODEOP_PID=$!
   sleep 10
   kill $NODEOP_PID
   rc=0
   wait $NODEOP_PID && rc=$? || rc=$?
   if [[ $rc -eq  127  || $rc -eq  $NODEOP_PID ]]; then
      rc=0
   fi
   return $rc
}

run_and_kill() {
   run_nodeop "$@"
   local NODEOP_PID=$!
   sleep 10
   kill -KILL $NODEOP_PID
   ! wait $NODEOP_PID
}

run_expect_failure() {
   run_nodeop "$@"
   local NODEOP_PID=$!
   MYPID=$$
   (sleep 20; kill -ALRM $MYPID) & local TIMER_PID=$!
   trap "kill $NODEOP_PID; wait $NODEOP_PID; exit 1" ALRM
   sleep 10
   if wait $NODEOP_PID; then exit 1; fi
   kill $TIMER_PID
   trap ALRM
}

#new chain with mapped mode
run_expect_success --delete-all-blocks
#use previous DB with heap mode
run_expect_success --database-map-mode heap
#use previous DB with mapped_private mode
run_expect_success --database-map-mode mapped_private
#use previous DB with heap mode
run_expect_success --database-map-mode heap
#test lock mode if enabled
if (( $TEST_LOCKED_MODE == 1 )); then
   run_expect_success --database-map-mode locked
fi
#locked mode should fail when it's not possible to lock anything
ulimit -l 0
run_expect_failure --database-map-mode locked
#But shouldn't result in the dirty flag staying set; so next launch should run
run_expect_success
#Try killing with KILL
run_and_kill
#should be dirty now
run_expect_failure
#should also still be dirty in heap mode
run_expect_failure --database-map-mode heap
#should also still be dirty in mapped_private mode
run_expect_failure --database-map-mode mapped_private

#start over again! but this time start with heap mode
run_expect_success --delete-all-blocks  --database-map-mode heap
#Then switch back to mapped
run_expect_success
#Then switch back to mapped_private
run_expect_success --database-map-mode mapped_private
#try killing it while in heap mode
run_and_kill --database-map-mode heap
#should be dirty if we run in either mode node
run_expect_failure --database-map-mode heap
run_expect_failure
