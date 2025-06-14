make clean -f Makefile
make clean -f Makefile_daq_bic 
make clean -f Makefile_daq_jbnu 
make clean -f Makefile_daq_nkfadc
make clean -f Makefile_run 
make clean -f Makefile_set
make clean -f Makefile_daq_stop

make -f Makefile
make -f Makefile_daq_bic 
make -f Makefile_daq_jbnu 
make -f Makefile_daq_nkfadc
make -f Makefile_run 
make -f Makefile_set
make -f Makefile_stop

make install -f Makefile
make install -f Makefile_daq_bic 
make install -f Makefile_daq_jbnu 
make install -f Makefile_daq_nkfadc
make install -f Makefile_run 
make install -f Makefile_set
make install -f Makefile_daq_stop