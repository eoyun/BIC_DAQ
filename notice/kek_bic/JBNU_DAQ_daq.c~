#include <unistd.h>
#include <stdio.h>
#include "NoticeJBNU_DAQ.h"

long long data_size_calculator(int gate_width, int prescale, int event){
	return ((gate_width * 128 * 32 + 256 * prescale) * event) / 1024;
}

int evt_calculator(int gate_width,int prescale, long long data_size){
	return data_size * 1024 / (gate_width * 128 * 32 + 256 * prescale );
}
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
	long long flag_data_size;
	int flag=0;
	int status;
	int nevts_flag;
	sid = atoi(argv[1]);
	run_number = atoi(argv[2]);
	nevts_flag = atoi(argv[3]);
	char gateW_file[100];
	sprintf(gateW_file,"gatewidth%d.txt",sid);
	FILE* fp_GW = fopen(gateW_file,"rt");
	int prescale;
	int gate_width;
	fscanf(fp_GW,"%d %d",&gate_width, &prescale);
	flag_data_size = data_size_calculator(gate_width,prescale,nevts_flag);
	fclose(fp_GW);
	char rm_GW[100];
	sprintf(rm_GW,"rm gatewidth%d.txt",sid);
	system(rm_GW);
	// init usb
	USB3Init();

	// open DAQ
	status = DAQopen(sid);
	printf("hello\n");
	char mkdir_name[200];
	sprintf(mkdir_name,"mkdir -p /home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/",run_number,run_number,sid);
	system(mkdir_name);
	sprintf(filename, "/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/jbnu_daq_%d_%d.dat",run_number,run_number,sid, sid, run_number);
	fp = fopen(filename, "wb"); 
	data_written = 0;
	printf("status = %d\n",status);
    if (status > 0){
		
		// open file
		
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
				int nevts_now = evt_calculator(gate_width,prescale, data_written);
				printf("JBNU_DAQ[%d] %d events are taken\n", sid, nevts_now);
			}
		
			run = DAQread_RUN(sid);
			if (access("KILLME",F_OK)==0)
				break;
			if (data_written > flag_data_size && flag ==0){
				char flag_name[100];
				sprintf(flag_name,"touch KILLTCB%d",sid);
				system(flag_name);
				flag = 1;
			}
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
		free(data);
	}
	// release data array

	// close file
	fclose(fp);

	// close DAQ
	DAQclose(sid);

	// close usb
	USB3Exit();

	return 0;
}


