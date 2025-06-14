#!/usr/bin/env bash

ps aux | grep "NKFADC500_daq" | grep -v grep
ps aux | grep "BIC_DAQ_daq"   | grep -v grep
ps aux | grep "APIX_DAQ_daq"  | grep -v grep
ps aux | grep "JBNU_DAQ_daq"  | grep -v grep

