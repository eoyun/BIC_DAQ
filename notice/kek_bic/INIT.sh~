#!/usr/bin/env bash

export TCBINIT=./bin/BIC_DAQ_set.exe
export RUNNO=`cat RUNNUMBER`

date > ./log/log_${RUNNO}.txt; echo "" >> ./log/log_${RUNNO}.txt
$TCBINIT >> ./log/log_${RUNNO}.txt;
cat ./log/log_${RUNNO}.txt
