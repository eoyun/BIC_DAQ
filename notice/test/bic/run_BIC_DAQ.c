#include <unistd.h>
#include <stdio.h>
#include "NoticeJBNU_TCB.h"

int main(int argc, char *argv[])
{
  int sid;
  int link_status[2];
  int daq_mid[40];
  int num_of_daq;
  int daq;
  int daq_exist;
  int mid[40];
  int ch;
  int run_number;
  int acq_time;
  
  if (argc > 3) {
    sid = atoi(argv[1]);
    run_number = atoi(argv[2]);
    acq_time = atoi(argv[3]);
  }
  else {
    printf("Enter TCB sid : ");
    scanf("%d", &sid);
    printf("Enter run number(0~65535) : ");
    scanf("%d", &run_number);
    printf("Enter acquisition time(0~1,000,000s, 0 for infinite) : ");
    scanf("%d", &acq_time);
  }

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
      printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq);
      num_of_daq = num_of_daq + 1;
    }
  }
  for (daq = 0; daq < 8; daq++) {
    daq_exist = link_status[1] & (1 << daq);
    if (daq_exist) {
      mid[num_of_daq] = daq_mid[daq + 32]; 
      printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 32);
      num_of_daq = num_of_daq + 1;
    }
  }

  // read temperature and pedestal
  for (daq = 0; daq < num_of_daq; daq++) {
   for (ch = 1; ch <= 32; ch++)
      printf("DAQ[%d] ch%d pedestal = %d\n", mid[daq], ch, TCBread_PED(sid, mid[daq], ch));
  }

  // set run number
  TCBwrite_RUN_NUMBER(sid, run_number);
  printf("run_number = %d\n", TCBread_RUN_NUMBER(sid));
  
  // set acquisition time
  TCBwrite_ACQ_TIME(sid, acq_time);
  printf("acquisition time = %d s\n", TCBread_ACQ_TIME(sid));
  
  // reset timer if necessary
//  TCBreset_TIMER(sid);  

  // start DAQ
  TCBstart_DAQ(sid);
  printf("Run = %d\n", TCBread_RUN(sid));
  
  // close DAQ
  TCBclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}


