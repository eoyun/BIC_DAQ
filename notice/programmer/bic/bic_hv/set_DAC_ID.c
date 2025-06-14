#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include "NoticeBIC_HV.h"

// Followings are executable routine
int main(void)
{
  char ip[256];
  int tcp_Handle;                // TCP/IP handler
  int com;
  int ch;
  int addr;
  int dac_id;

//  printf("enter IP address : ");
//  scanf("%s", ip);
sprintf(ip, "192.168.0.2");

  // open MINITCB_V2
  tcp_Handle = HVopen(ip);

  com = 99;

  while(com) {
    printf("\n");
    printf("1. Set DAC id\n");
    printf("0. quit\n");
    printf("enter command : ");
    scanf("%d", &com);

    if (com == 1) {
      for (ch = 0; ch < 32; ch++) {
        for (addr = 0; addr < 4; addr++) {
          dac_id = addr;
          HVwrite_DAC_ID(tcp_Handle, ch, addr, dac_id);
        }
      }
    }
  }

  // close MINITCB_V2
  HVclose(tcp_Handle);

  return 0;
}
  
 







