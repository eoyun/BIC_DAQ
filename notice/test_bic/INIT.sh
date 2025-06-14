#!/usr/bin/env bash

export TCBINIT=./bin/BIC_DAQ_set.exe
export RUNNO=`cat RUNNUMBER`

date > log_${RUNNO}.txt; echo "" >> log_${RUNNO}.txt
$TCBINIT >> log_${RUNNO}.txt;
cat log_${RUNNO}.txt
