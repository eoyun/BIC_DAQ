#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_DAQ.h"

int main(int argc, char *argv[])
{
	int sid;
	int run_number;
	char filename[100];
	FILE *fp;
	int run;
	int data_size;
	int data_written;
	char *data;

	sid = atoi(argv[1]);
	run_number = atoi(argv[2]);

	// init usb
	USB3Init();

	// open DAQ
	DAQopen(sid);

	// open file
	sprintf(filename, "jbnu_daq_%d_%d.dat", sid, run_number);
	fp = fopen(filename, "wb"); 
	data_written = 0;

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
			DAQread_DATA(sid, data_size, data);
			fwrite(data, 1, data_size * 1024, fp);
			data_written = data_written + data_size;
			printf("JBNU_DAQ[%d] %d kbytes are taken\n", sid, data_written);
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
			printf("JBNU_DAQ[%d] %d kbytes are taken\n", sid, data_written);
		}
	}    

	// release data array
	free(data);

	// close file
	fclose(fp);

	// close DAQ
	DAQclose(sid);

	// close usb
	USB3Exit();

	return 0;
}


