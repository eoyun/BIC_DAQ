#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JBNU_DAQ_VID  (0x0547)
#define JBNU_DAQ_PID (0x2209)

extern void USB3Init(void);
extern void USB3Exit(void);
extern int DAQopen(int sid);
extern void DAQclose(int sid);
extern int DAQread_DATASIZE(int sid);
extern int DAQread_RUN(int sid);
extern void DAQread_DATA(int sid, int data_size, char *data);

#ifdef __cplusplus
}
#endif




