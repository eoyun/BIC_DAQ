#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libusb.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JBNU_TCB_VID (0x0547)
#define JBNU_TCB_PID (0x1501)

extern void USB3Init(void);
extern void USB3Exit(void);
extern int  TCBopen(int sid);
extern void TCBclose(int sid);
extern void TCBreset_TIMER(int sid);
extern void TCBreset(int sid);
extern void TCBstart_DAQ(int sid);
extern int  TCBread_RUN(int sid);
extern void TCBwrite_RUN_NUMBER(int sid, int data);
extern int  TCBread_RUN_NUMBER(int sid);
extern void TCBwrite_ACQ_TIME(int sid, int data);
extern int  TCBread_ACQ_TIME(int sid);
extern void TCBwrite_DISC_ENABLE(int sid, int mid, int *data);
extern void TCBread_DISC_ENABLE(int sid, int mid, int *data);
extern void TCBwrite_COIN_WIDTH(int sid, int mid, int data);
extern int  TCBread_COIN_WIDTH(int sid, int mid);
extern void TCBwrite_MULTI_THR(int sid, int mid, int data);
extern int  TCBread_MULTI_THR(int sid, int mid);
extern void TCBwrite_PEDESTAL_TRIGGER_INTERVAL(int sid, int data);
extern int  TCBread_PEDESTAL_TRIGGER_INTERVAL(int sid);
extern void TCBwrite_DISC_THR(int sid, int mid, int ch, int data);
extern int  TCBread_DISC_THR(int sid, int mid, int ch);
extern void TCBsend_TRIG(int sid);
extern void TCBwrite_GATE_WIDTH(int sid, int mid, int data);
extern int  TCBread_GATE_WIDTH(int sid, int mid);
extern void TCBwrite_TRIGGER_ENABLE(int sid, int data);
extern int  TCBread_TRIGGER_ENABLE(int sid);
extern void TCBwrite_DELAY(int sid, int mid, int data);
extern int  TCBread_DELAY(int sid, int mid);
extern void TCBmeasure_PED(int sid, int mid);
extern int  TCBread_PED(int sid, int mid, int ch);
extern void TCBwrite_PEAK_SUM_WIDTH(int sid, int mid, int data);
extern int  TCBread_PEAK_SUM_WIDTH(int sid, int mid);
extern void TCBwrite_ZERO_SUP(int sid, int mid, int data);
extern int  TCBread_ZERO_SUP(int sid, int mid);
extern void TCBwrite_PRESCALE(int sid, int mid, int data);
extern int  TCBread_PRESCALE(int sid, int mid);
extern void TCBwrite_TDC_CAL_CH(int sid, int mid, int data);
extern int  TCBread_TDC_CAL_CH(int sid, int mid);
extern void TCBstart_TDC_CAL(int sid, int mid);
extern void TCBstop_TDC_CAL(int sid, int mid);
extern int  TCBread_TDC_CAL_STATUS(int sid, int mid);
extern void TCBread_LINK_STATUS(int sid, int *data);
extern void TCBread_MID(int sid, int *data);
extern void TCBinit_BIC_ADC(int sid, int mid);
extern void TCBinit_JBNU_ADC(int sid, int mid);
extern void TCBinit_DRAM(int sid, int mid);
extern void TCBwrite_HV(int sid, int mid, int ch, float data);
extern float TCBread_HV(int sid, int mid, int ch);

//NKFADC
extern void MINITCB_V2write_RL         (int sid, int mid, int data);
extern int  MINITCB_V2read_RL          (int sid, int mid);
extern void MINITCB_V2write_DRAMON     (int sid, int mid, int data);
extern int  MINITCB_V2read_DRAMON      (int sid, int mid);
extern void MINITCB_V2write_DACOFF     (int sid, int mid, int ch, int data);
extern int  MINITCB_V2read_DACOFF      (int sid, int mid, int ch);
extern void MINITCB_V2measure_PED      (int sid, int mid, int ch);
extern int  MINITCB_V2read_PED         (int sid, int mid, int ch);
extern void MINITCB_V2write_DLY        (int sid, int mid, int ch, int data);
extern int  MINITCB_V2read_DLY         (int sid, int mid, int ch);
extern void MINITCB_V2write_THR        (int sid, int mid, int ch, int data);
extern int  MINITCB_V2read_THR         (int sid, int mid, int ch);
extern void MINITCB_V2write_POL        (int sid, int mid, int ch, int data);
extern int  MINITCB_V2read_POL         (int sid, int mid, int ch);
extern void MINITCB_V2write_AMODE      (int sid, int mid, int ch, int data);
extern int  MINITCB_V2read_AMODE       (int sid, int mid, int ch);
extern void MINITCB_V2send_ADCRST      (int sid, int mid);
extern void MINITCB_V2send_ADCCAL      (int sid, int mid);
extern void MINITCB_V2write_ADCDLY     (int sid, int mid, int ch, int data);
extern void MINITCB_V2write_ADCALIGN   (int sid, int mid, int data);
extern int  MINITCB_V2read_ADCSTAT     (int sid, int mid);
extern void MINITCB_V2_ADCALIGN        (int sid, int mid);
extern void MINITCB_V2write_DRAMDLY    (int sid, int mid, int ch, int data);
extern void MINITCB_V2write_DRAMBITSLIP(int sid, int mid, int ch);
extern void MINITCB_V2write_DRAMTEST   (int sid, int mid, int data);
extern int  MINITCB_V2read_DRAMTEST    (int sid, int mid, int ch);
extern void MINITCB_V2_ADCALIGN_DRAM   (int sid, int mid);

#ifdef __cplusplus
}
#endif
