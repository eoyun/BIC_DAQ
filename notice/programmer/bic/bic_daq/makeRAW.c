#include <stdio.h>

int main(void)
{
  FILE *fp;
  char wdat;
  int ch;
  int i;

  fp = fopen("tdc_raw.lut", "wb");
  for (ch = 0; ch < 32; ch++) {
    for (i = 0; i < 4096; i++) {
      wdat = (i >> 2) & 0xFF;
      fprintf(fp, "%c", wdat);
      wdat = (i >> 10) & 0x03;
      fprintf(fp, "%c", wdat);
    }
  }
  fclose(fp);   

  return 0;
}
