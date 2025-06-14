ROOTCINT = rootcling
NOTICE_LIB = $(NKHOME)/lib
NOTICE_INC = $(NKHOME)/include

INSTALL_LIB = $(NKHOME)/lib
INSTALL_INC = $(NKHOME)/include

RC     := root-config
ifeq ($(shell which $(RC) 2>&1 | sed -ne "s@.*/$(RC)@$(RC)@p"),$(RC))
MKARCH := $(wildcard $(shell $(RC) --etcdir)/Makefile.arch)
endif
ifneq ($(MKARCH),)
include $(MKARCH)
else
ROOTSYS = ..
include $(ROOTSYS)/etc/Makefile.arch
endif
-include ../MyConfig.mk

#------------------------------------------------------------------------------

SrcSuf = cc
HdrSuf = h
ObjSuf = o
DllSuf = so
OutPutOpt = -o #keep the space after the option "-o"

CXXFLAGS += -I. -I$(LIBUSB_INC) -I$(NOTICE_INC)
ifneq ($(DEBUG),)
CXXFLAGS += -g
endif

#LDFLAGS += -L$(LIB_nkfx3) -lnnkusb -shared -Wall,-soname
SOFLAGS += -Wall
LDFLAGS = $(SOFLAGS)
LDLIBS = -L$(NOTICE_LIB) -lusb3com

SOURCE = usb3comroot.$(SrcSuf) usb3comrootDict.$(SrcSuf)
INCLUDE = usb3comroot.$(HdrSuf) LinkDef.h
OBJECT = $(SOURCE:.cc=.o)
LIBRARY = libusb3comroot.$(DllSuf)

all: $(LIBRARY)
clean:
	@rm -rf $(LIBRARY) $(OBJECT) *Dict.$(SrcSuf) *Dict.$(HdrSuf)

install: $(LIBRARY) usb3comroot.h
	#/bin/rm -f $(INSTALL_LIB)/$<
	#/bin/rm -f $(INSTALL_INC)/usb3comroot.h
	install *.pcm $(INSTALL_LIB)
	install $< $(INSTALL_LIB)
	install usb3comroot.h $(INSTALL_INC)

$(LIBRARY): $(OBJECT)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) $(OutPutOpt) $@

usb3comrootDict.$(SrcSuf): $(INCLUDE) LinkDef.h
	@echo "Generating dictionary $@..."
	$(ROOTCINT) -f $@ -c -I$(NOTICE_INC) $^

#dependicies
$(OBJECT):$(INCLUDE)
