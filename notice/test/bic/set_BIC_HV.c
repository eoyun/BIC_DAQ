#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_HV.h"

int main(void)
{
  char ip[256];
  int tcp_Handle;               
  FILE *fp;
  float hv_base;
  float hv_ch[32][16];
  int ch;
  int sipm;
  float value;
  
//  printf("enter IP address : ");
//  scanf("%s", ip);
sprintf(ip, "192.168.0.2");

  // open BIC HV
  tcp_Handle = HVopen(ip);

  // read settings
  fp = fopen("sipm_hv.txt", "rt");
  fscanf(fp, "%f", &hv_base);
  for (ch = 0; ch < 32; ch++) {
    for (sipm = 0; sipm < 16; sipm++) {
      fscanf(fp, "%f", &value);
      hv_ch[ch][sipm] = value;
    }
  }
  fclose(fp);

  // write setting
  HVwrite_BASE(tcp_Handle, hv_base);

  for (ch = 0; ch < 32; ch++) {
    for (sipm = 0; sipm < 16; sipm++) 
      HVwrite_CH(tcp_Handle, ch, sipm, hv_ch[ch][sipm]) ;
  }
  
  // read back settings
  printf("HV base = %f V\n", HVread_BASE(tcp_Handle));
  for (ch = 0; ch < 32; ch++) {
    for (sipm = 0; sipm < 16; sipm++) 
      printf("HV channel[%d][%d] = %f V\n", ch + 1, sipm + 1, HVread_CH(tcp_Handle, ch, sipm));
  }

  // close BIC HV
  HVclose(tcp_Handle);
  
  return 0;
}


