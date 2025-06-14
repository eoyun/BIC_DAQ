#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_TCB.h"

int main(int argc, char *argv[])
{
  int sid;
  int link_status[2];
  int daq_mid[40];
  int num_of_daq;
  int daq;
  int daq_exist;
  int mid[40];
  FILE *fp;
  int coin_width;
  int multi_thr_tcb;
  int multi_thr_daq;
  int ptrig_interval;
  int disc_thr;
  int gate_width;
  int trig_enable;
  int idelay;
  int peak_sum_width;
  int zero_sup;
  int prescale;
  int disc_enable[32];
  int ch;
  
  if (argc > 1) 
    sid = atoi(argv[1]);
  else 
    sid = 0;

  // init USB
  USB3Init();
  
  // open TCB
  TCBopen(sid);

  // reset TCB
  TCBreset(sid);

  // get link status and ADC mid
  TCBread_LINK_STATUS(sid, link_status);
  TCBread_MID(sid, daq_mid);
  printf("link status = %X %X\n", link_status[1], link_status[0]);
  num_of_daq = 0;
  for (daq = 0; daq < 32; daq++) {
    daq_exist = link_status[0] & (1 << daq);
    if (daq_exist) {
      mid[num_of_daq] = daq_mid[daq]; 
      printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 1);
      num_of_daq = num_of_daq + 1;
    }
  }
  for (daq = 0; daq < 8; daq++) {
    daq_exist = link_status[1] & (1 << daq);
    if (daq_exist) {
      mid[num_of_daq] = daq_mid[daq + 32]; 
      printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 33);
      num_of_daq = num_of_daq + 1;
    }
  }

  // read settings
  fp = fopen("setup.txt", "rt");
  fscanf(fp, "%d", &coin_width);
  fscanf(fp, "%d", &multi_thr_tcb);
  fscanf(fp, "%d", &multi_thr_daq);
  fscanf(fp, "%d", &ptrig_interval);
  fscanf(fp, "%d", &disc_thr);
  fscanf(fp, "%d", &gate_width);
  fscanf(fp, "%d", &trig_enable);
  fscanf(fp, "%d", &idelay);
  fscanf(fp, "%d", &peak_sum_width);
  fscanf(fp, "%d", &zero_sup);
  fscanf(fp, "%d", &prescale);
  for (ch = 0; ch < 32; ch++)
    fscanf(fp, "%d", disc_enable + ch);
  fclose(fp);

  // write setting
  TCBwrite_COIN_WIDTH(sid, 0, coin_width);
  TCBwrite_MULTI_THR(sid, 0, multi_thr_tcb);
  TCBwrite_PEDESTAL_TRIGGER_INTERVAL(sid, ptrig_interval);
  TCBwrite_TRIGGER_ENABLE(sid, trig_enable);
  for (daq = 0; daq < num_of_daq; daq++) {
    TCBinit_BIC_ADC(sid, mid[daq]);
    TCBinit_DRAM(sid, mid[daq]);
    TCBwrite_COIN_WIDTH(sid, mid[daq], coin_width);
    TCBwrite_MULTI_THR(sid, mid[daq], multi_thr_daq);
    for (ch = 1; ch <= 32; ch++)
      TCBwrite_DISC_THR(sid, mid[daq], ch, disc_thr);
    TCBwrite_GATE_WIDTH(sid, mid[daq], gate_width);
    TCBwrite_DELAY(sid, mid[daq], idelay);
    TCBwrite_PEAK_SUM_WIDTH(sid, mid[daq], peak_sum_width);
    TCBwrite_ZERO_SUP(sid, mid[daq], zero_sup);
    TCBwrite_PRESCALE(sid, mid[daq], prescale);
    TCBwrite_DISC_ENABLE(sid, mid[daq], disc_enable);
    TCBmeasure_PED(sid, mid[daq]);
  }
  
  // read back settings
  printf("TCB coincidence width = %d ns\n", TCBread_COIN_WIDTH(sid, 0));
  printf("TCB multiplicity threshold = %d\n", TCBread_MULTI_THR(sid, 0));
  printf("TCB pedestal trigger interval = %d ms\n", TCBread_PEDESTAL_TRIGGER_INTERVAL(sid));
  printf("TCB trigger enable = %d\n", TCBread_TRIGGER_ENABLE(sid));
  for (daq = 0; daq < num_of_daq; daq++) {
    printf("DAQ[%d] coincidence width = %d ns\n", mid[daq], TCBread_COIN_WIDTH(sid, mid[daq]));
    printf("DAQ[%d] multiplicity threshold = %d\n", mid[daq], TCBread_MULTI_THR(sid, mid[daq]));
    printf("DAQ[%d] discriminator threshold = %d\n", mid[daq], TCBread_DISC_THR(sid, mid[daq], 1));
    printf("DAQ[%d] gate width = %d us\n", mid[daq], TCBread_GATE_WIDTH(sid, mid[daq]));
    printf("DAQ[%d] input delay = %d ns\n", mid[daq], TCBread_DELAY(sid, mid[daq]));
    printf("DAQ[%d] peak sum width = %d ns\n", mid[daq], TCBread_PEAK_SUM_WIDTH(sid, mid[daq]));
    printf("DAQ[%d] zero suppression = %d\n", mid[daq], TCBread_ZERO_SUP(sid, mid[daq]));
    printf("DAQ[%d] waveform prescale = %d\n", mid[daq], TCBread_PRESCALE(sid, mid[daq]));
    printf("DAQ[%d] disc enable =", mid[daq]);
    TCBread_DISC_ENABLE(sid, mid[daq], disc_enable);
    for (ch = 1; ch <= 32; ch++)
      printf(" %d", disc_enable[ch]);
    printf("\n");
    for (ch = 1; ch <= 32; ch++)
      printf("DAQ[%d] ch%d pedestal = %d\n", mid[daq], ch, TCBread_PED(sid, mid[daq], ch));
  }
    
  // only if necessary, reset timer
  TCBreset_TIMER(sid);
  
  // reset DAQ
  TCBreset(sid);

  // close TCB
  TCBclose(sid);
  
  // close USB
  USB3Exit();
  
  return 0;
}


