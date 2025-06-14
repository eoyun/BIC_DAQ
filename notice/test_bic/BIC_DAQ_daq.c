#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_DAQ.h"

int main(int argc, char *argv[])
{
	int sid;
	int run_number;
	char filename[100];
	char filename_apix1[100];
	char filename_apix2[100];
	FILE *fp;
	FILE *fp_apix1;
	FILE *fp_apix2;
	int run;
	int data_size;
	int data_size_apix1;
	int data_size_apix2;
	int data_written;
	int data_written_apix1;
	int data_written_apix2;
	char *data;

    int status;

	sid = atoi(argv[1]);
	run_number = atoi(argv[2]);

	// init usb
	USB3Init();

	// open DAQ
	status = DAQopen(sid);
    if (status > 0){
		// open file
		sprintf(filename, "bic_daq_%d_%d.dat", sid, run_number);
		sprintf(filename_apix1, "apix_1_%d_%d.dat", sid, run_number);
		sprintf(filename_apix2, "apix_2_%d_%d.dat", sid, run_number);
		fp = fopen(filename, "wb"); 
		fp_apix1 = fopen(filename_apix1, "wb"); 
		fp_apix2 = fopen(filename_apix2, "wb"); 
		data_written = 0;
		data_written_apix1 = 0;
		data_written_apix2 = 0;
		
		// assign data array
		data = (char *)malloc(65536);
		
		run = 1;
		while (run) {
			// check data size
			data_size = DAQread_DATASIZE(sid);
		
			// read data
			if (data_size) {
				//printf("data_size = %d\n", data_size);    
				//if (data_size > 16) data_size = 16;
				//printf("BIC data dbg\n");
				DAQread_DATA(sid, data_size, data);
				fwrite(data, 1, data_size * 1024, fp);
				data_written = data_written + data_size;
				printf("BIC_DAQ[%d] %d kbytes are taken\n", sid, data_written);
			}
		
			// check apix1 data size
			data_size_apix1 = DAQread_APIX_DATASIZE(sid, 1);
		
			// read apix1 data
			if (data_size_apix1) {
				//printf("apix1 data dbg\n");
				DAQread_APIX_DATA(sid, 1, data_size_apix1, data);
				fwrite(data, 1, data_size_apix1 * 1024, fp_apix1);
				data_written_apix1 = data_written_apix1 + data_size_apix1;
				printf("APIX1[%d] %d kbytes are taken\n", sid, data_written_apix1);
			}
		
			// check apix2 data size
			data_size_apix2 = DAQread_APIX_DATASIZE(sid, 2);
		
			// read apix2 data
			if (data_size_apix2) {
				//printf("apix2 data dbg\n");
				DAQread_APIX_DATA(sid, 2, data_size_apix2, data);
				fwrite(data, 1, data_size_apix2 * 1024, fp_apix2);
				data_written_apix2 = data_written_apix2 + data_size_apix2;
				printf("APIX2[%d] %d kbytes are taken\n", sid, data_written_apix2);
			}
		
			run = DAQread_RUN(sid);
		}
		
		printf("DAQ is stopped and read remaining data\n");
		
		// read remaining data  
		data_size = 16;
		while (data_size) {
			// check data size
			data_size = DAQread_DATASIZE(sid);
		
			// read data
			if (data_size) {
				//printf("data_size = %d\n", data_size);    
				//if (data_size > 16) data_size = 16;
				DAQread_DATA(sid, data_size, data);
				fwrite(data, 1, data_size * 1024, fp);
				data_written = data_written + data_size;
				printf("BIC_DAQ[%d] %d kbytes are taken\n", sid, data_written);
			}
		}
		
		data_size_apix1 = 16;
		while (data_size_apix1) {
			// check apix1 data size
			data_size_apix1 = DAQread_APIX_DATASIZE(sid, 1);
		
			// read apix1 data
			if (data_size_apix1) {
				DAQread_APIX_DATA(sid, 1, data_size_apix1, data);
				fwrite(data, 1, data_size_apix1 * 1024, fp_apix1);
				data_written_apix1 = data_written_apix1 + data_size_apix1;
				printf("APIX1[%d] %d kbytes are taken\n", sid, data_written_apix1);
			}
		}
		
		data_size_apix2 = 16;
		while (data_size_apix2) {
			// check apix2 data size
			data_size_apix2 = DAQread_APIX_DATASIZE(sid, 2);
		
			// read apix2 data
			if (data_size_apix2) {
				DAQread_APIX_DATA(sid, 2, data_size_apix2, data);
				fwrite(data, 1, data_size_apix2 * 1024, fp_apix2);
				data_written_apix2 = data_written_apix2 + data_size_apix2;
				printf("APIX2[%d] %d kbytes are taken\n", sid, data_written_apix2);
			}
		}    
    }
	// release data array
	free(data);

	// close file
	fclose(fp);
	fclose(fp_apix1);
	fclose(fp_apix2);

	// close DAQ
	DAQclose(sid);

	// close usb
	USB3Exit();

	return 0;
}


