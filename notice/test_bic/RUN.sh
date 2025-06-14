#!/usr/bin/env bash

#export FADCEXE=./bin/nkfadc500_run
export BICEXE=./bin/BIC_DAQ_run.exe
export RUNNO=`cat RUNNUMBER`
export RUNTIME=${1:-0}

echo "==================="
echo "RUNNUMBER: ${RUNNO}"
echo "RUNTIME: ${RUNTIME} sec"
echo "==================="
echo ""

#$FADCEXE $RUNNO &
#sleep 1
$BICEXE $RUNNO $RUNTIME &

if [ $RUNTIME -ne 0 ]; then
	sleep $RUNTIME
	source ./STOP.sh
fi
