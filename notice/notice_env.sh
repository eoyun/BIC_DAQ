#/bin/bash -f

export NKHOME=/home/kobic/bic_daq/notice
export ROOTHOME=$ROOTSYS; . $ROOTHOME/bin/thisroot.sh;
export LIBUSB_INC=/usr/include/libusb-1.0
export LIBUSB_LIB=/usr/lib/x86_64-linux-gnu
echo "NKHOME:    " $NKHOME
echo "ROOTHOME:  " $ROOTHOME
echo "LIBUSB_INC:" $LIBUSB_INC
echo "LIBUSB_LIB:" $LIBUSB_LIB

#------------------------------------------------

if [ -z "${PATH}" ]; then
   PATH=$NKHOME/bin; export PATH
   echo hello
else
   PATH=$NKHOME/bin:$PATH; export PATH
fi

if [ -z "${LD_LIBRARY_PATH}" ]; then
   LD_LIBRARY_PATH=$NKHOME/lib:/usr/local/lib; export LD_LIBRARY_PATH
   echo hello2
else
   LD_LIBRARY_PATH=$NKHOME/lib:/usr/local/lib:$LD_LIBRARY_PATH; export LD_LIBRARY_PATH
fi

#------------------------------------------------

export TERM='xterm-256color'
export LS_COLORS="di=01;34":"fi=0;37":"ex=0;32":"ln=0;36":"so=0;33"

alias cl='clear'
alias cls='rm li*.d *.so *.pcm'
alias dfh='df -Th -x "squashfs" | grep ^/'
alias greps='grep -rl --color'
alias l='ls --color'
alias ll='ls -ahlvG --color'
alias psg='ps wax | grep -v grep | grep'
alias rl='root -l'
alias rlbq='root -l -b -q'
alias vi='vim'
alias notice='cd $NKHOME'
alias work='cd /home/kobic/bic_daq/notice'
