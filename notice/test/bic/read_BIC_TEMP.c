#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_HV.h"

int main(void)
{
  char ip[256];
  int tcp_Handle;               
  float temp[32];
  int ch;
  
//  printf("enter IP address : ");
//  scanf("%s", ip);
sprintf(ip, "192.168.0.2");

  // open BIC HV
  tcp_Handle = HVopen(ip);

  HVread_TEMP(tcp_Handle, temp);
  
  for (ch = 0; ch < 32; ch++)
    printf("Ch%d = %f\n", ch + 1, temp[ch]);

  // close BIC HV
  HVclose(tcp_Handle);
  
  return 0;
}


