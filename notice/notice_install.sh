#/bin/bash -f

. notice_env.sh

mkdir -p include; rm include/*
mkdir -p lib; rm lib/*

echo ""
echo "Installing... NKHOME/src/nkusb/nkusb" 
cd $NKHOME/src/nkusb/nkusb; make clean; make install;

echo ""
echo "Installing... NKHOME/src/nkusb/nkusbroot" 
cd $NKHOME/src/nkusb/nkusbroot; make clean; make install;

echo ""
echo "Installing... NKHOME/src/usb3com/usb3com" 
cd $NKHOME/src/usb3com/usb3com; make clean; make install;

echo ""
echo "Installing... NKHOME/src/usb3com/usb3comroot" 
cd $NKHOME/src/usb3com/usb3comroot; make clean; make install;

echo ""
echo "Installing... NKHOME/src/bic_tcb" 
cd $NKHOME/src/bic_tcb; make clean; make install;

echo ""
echo "Installing... NKHOME/src/bic_daq" 
cd $NKHOME/src/bic_daq; make clean; make install;

echo ""
echo "Installing... NKHOME/src/bic_hv" 
cd $NKHOME/src/bic_hv; make clean; make install;

echo ""
echo "Installing... NKHOME/src/jbnu_tcb"
cd $NKHOME/src/jbnu_tcb/; make clean; make install

echo ""
echo "Installing... NKHOME/src/jbnu_daq"
cd $NKHOME/src/jbnu_daq/; make clean; make install

echo ""
echo "Installing... NKHOME/src/nkfadc500/nkfadc500"
cd $NKHOME/src/nkfadc500/nkfadc500; make clean; make install;

echo ""
echo "Installing... NKHOME/src/nkfadc500/nkfadc500root"
cd $NKHOME/src/nkfadc500/nkfadc500root; make clean; make install;

cd $NKHOME
