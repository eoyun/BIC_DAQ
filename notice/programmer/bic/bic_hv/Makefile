########################################################################
#                                                                      #
#   Notice Korea    6UVME C Libary                                     #
#                                                                      #
#   Created: Sept. 2006                                                #
#                                                                      #
#   Auth: H.J. Kim hongjoo@knu.ac.kr                                   #
#                                                                      #
########################################################################
#   NKHOME should be defined

LIBUSB_INC = /usr/include/libusb-1.0
LIBUSB_LIB = /usr/lib64

FILENAME = set_DAC_ID

EXE	=	$(FILENAME).exe

CC	=	gcc

COPTS	=	-fPIC -Wall

FLAGS	=	-shared -Wall,-soname -nostartfiles -s

LDFLAGS =        -O2 -Wall 

OutPutOpt = -o 

LIBS	=	-L$(NKHOME)/lib -lNoticeBIC_HV -lm

INCLUDEDIR =	-I$(NKHOME)/include -I$(LIBUSB_INC)

INSTALLDIR     =	$(NKHOME)/bin
INSTALLINCLUDE =	$(NKHOME)/include

OBJS	=	$(FILENAME).o

INCLUDES =	 


#########################################################################

all	:	$(EXE)
clean	:
		/bin/rm -f $(OBJS) $(EXE)

install	:	$(EXE)
		#/bin/rm -f $(INSTALLDIR)/$(EXE)
		install $(EXE) $(INSTALLDIR)

$(EXE):      $(OBJS)
		$(CC) $^ $(LIBS) $(OutPutOpt)$@ $(LDFLAGS)
		@echo "$@ done"

$(OBJS)	:	$(INCLUDES) 

%.o	:	%.c
		$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<

