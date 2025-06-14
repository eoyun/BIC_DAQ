#!/usr/bin/env bash

export FADCPROC=NKFADC500_daq
export BICPROC=BIC_DAQ_run.exe
export BICDAQPROC=BIC_DAQ_daq.exe
export RUNNO=`cat RUNNUMBER`

#: '
# Stop FADC
PIDFADC=`pidof $FADCPROC`
for i in $(echo $PIDFADC)
do
    kill -s SIGINT $i
done
#'

# Stop JBNU_DAQ and BIC_DAQ
PIDBIC=`pidof $BICPROC`
for i in $(echo $PIDBIC)
do
    kill -s SIGINT $i
done

PIDBIC=`pidof $BICDAQPROC`
for i in $(echo $PIDBIC)
do
    kill -s SIGINT $i
done

echo "Move dat files to ./data in 5 sec..."
sleep 5

# Mmove dat files to ./data
if compgen -G ./*_${RUNNO}*.dat > /dev/null; then
	mkdir -p ./data
	mv ./log*.txt ./*.dat ./data
fi

# Increase Run number by 1 for next run
RUNNO=`expr $RUNNO + 1`
echo "$RUNNO" > RUNNUMBER
