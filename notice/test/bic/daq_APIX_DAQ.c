#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_DAQ.h"

int main(int argc, char *argv[])
{
  int sid;
  char filename_ch1[100];
  char filename_ch2[100];
  FILE *fp_ch1;
  FILE *fp_ch2;
  int run;
  int data_size;
  int data_written_ch1;
  int data_written_ch2;
  char *data;
  int nevt;
  
  if (argc > 1) 
    sid = atoi(argv[1]);
  else {
    printf("Enter DAQ mid : ");
    scanf("%d", &sid);
  }

  // init usb
  USB3Init();
  
  // open DAQ
  DAQopen(sid);

  // open file
  sprintf(filename_ch1, "apix_1.dat");
  sprintf(filename_ch2, "apix_2.dat");
  fp_ch1 = fopen(filename_ch1, "wb"); 
  fp_ch2 = fopen(filename_ch2, "wb"); 
  data_written_ch1 = 0;
  data_written_ch2 = 0;
  
  // assign data array
  data = (char *)malloc(65536);
  
  run = 1;
  while (run) {
    // check data size ch1
    data_size = DAQread_APIX_DATASIZE(sid, 1);
   
    // read data ch1
    if (data_size) {
      DAQread_APIX_DATA(sid, 1, data_size, data);
      fwrite(data, 1, data_size * 1024, fp_ch1);
      data_written_ch1 = data_written_ch1 + data_size;
      nevt = data_written_ch1 * 16;
      printf("APIX1 : %d events are taken\n", nevt);
    }

    // check data size ch2
    data_size = DAQread_APIX_DATASIZE(sid, 2);
   
    // read data ch2
    if (data_size) {
      DAQread_APIX_DATA(sid, 2, data_size, data);
      fwrite(data, 1, data_size * 1024, fp_ch2);
      data_written_ch2 = data_written_ch2 + data_size;
      nevt = data_written_ch2 * 16;
      printf("APIX2 : %d events are taken\n", nevt);
    }

    run = DAQread_RUN(sid);
  }
  
  printf("DAQ is stopped and read remaining data\n");

  // read remaining data  
  data_size = 16;
  while (data_size) {
    // check data size ch1
    data_size = DAQread_APIX_DATASIZE(sid, 1);
   
    // read data ch1
    if (data_size) {
      DAQread_APIX_DATA(sid, 1, data_size, data);
      fwrite(data, 1, data_size * 1024, fp_ch1);
      data_written_ch1 = data_written_ch1 + data_size;
      nevt = data_written_ch1 * 16;
      printf("APIX1 : %d events are taken\n", nevt);
    }

    // check data size ch2
    data_size = DAQread_APIX_DATASIZE(sid, 2);
   
    // read data ch2
    if (data_size) {
      DAQread_APIX_DATA(sid, 2, data_size, data);
      fwrite(data, 1, data_size * 1024, fp_ch2);
      data_written_ch2 = data_written_ch2 + data_size;
      nevt = data_written_ch2 * 16;
      printf("APIX2 : %d events are taken\n", nevt);
    }
  }    
  
  // release data array
  free(data);

  // close file
  fclose(fp_ch1);
  fclose(fp_ch2);

  // close DAQ
  DAQclose(sid);
  
  // close usb
  USB3Exit();
  
  return 0;
}


