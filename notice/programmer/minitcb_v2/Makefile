FILENAME = programmer_minitcb_v2

EXE	=	$(FILENAME).exe

CC	=	gcc

COPTS	=	-fPIC -DLINUX -Wall -DNKC

FLAGS	=	-shared -Wall,-soname -nostartfiles -s

LDFLAGS =        -O2 -Wall 

OutPutOpt = -o 

LIBS	=	

INCLUDEDIR =	

INSTALLDIR     =	
INSTALLINCLUDE =	

OBJS	=	$(FILENAME).o

INCLUDES =	$(FILENAME).h 


#########################################################################

all	:	$(EXE)
clean	:
		/bin/rm -f $(OBJS) $(EXE)

install	:	$(EXE)
		/bin/rm -f $(INSTALLDIR)/$(EXE)
		install $(EXE) $(INSTALLDIR)

$(EXE):      $(OBJS)
		$(CC) $(LDFLAGS) $(INCLUDEDIR) $^ $(LIBS) $(OutPutOpt)$@
		@echo "$@ done"

$(OBJS)	:	$(INCLUDES) 

%.o	:	%.c
		$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<

