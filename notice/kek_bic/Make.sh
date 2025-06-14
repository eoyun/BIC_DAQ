#!/usr/bin/env bash

mkdir -p bin

echo ""
echo COMPILE "Makefile_daq_bic"
echo "##########################################################"
make -f Makefile_daq_bic

echo ""
echo COMPILE "Makefile_daq_jbnu"
echo "##########################################################"
make -f Makefile_daq_jbnu

echo ""
echo COMPILE "Makefile_run"
echo "##########################################################"
make -f Makefile_run

echo ""
echo COMPILE "Makefile_set"
echo "##########################################################"
make -f Makefile_set

make

mv *.o *.exe ./bin
