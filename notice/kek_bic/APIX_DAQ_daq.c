#include <unistd.h>
#include <stdio.h>
#include "NoticeAPIX_DAQ.h"

long long data_size_calculator(int gate_width,int prescale, int event){
	return ((gate_width * 256 * 32 + 256 * prescale) * event) / 1024;
}

int evt_calculator(int gate_width,int prescale, long long data_size){
	return data_size * 1024 / (gate_width * 256 * 32 + 256 * prescale);
}

int main(int argc, char *argv[])
{
	int sid;
	int run_number;
	char filename[100];
	//char filename_apix1[100];
	//char filename_apix2[100];
	FILE *fp[3];
	//FILE *fp_apix1;
	//FILE *fp_apix2;
	int run;
	int data_size;
	//int data_size_apix1;
	//int data_size_apix2;
	int data_written[3]={0};
	//int data_written_apix1;
	//int data_written_apix2;
	char *data;

	int flag_data_size;
    int status;

	int flag =0;
	sid = atoi(argv[1]);
	run_number = atoi(argv[2]);

	int gate_width;
	int prescale;
	char rm_GW[100];
	char gateW_file[100];
	/*
	sprintf(gateW_file,"gatewidth%d.txt",sid);
	FILE *fp_GW = fopen(gateW_file,"rt");
	fscanf(fp_GW,"%d %d",&gate_width,&prescale);
	flag_data_size = data_size_calculator(gate_width,prescale,nevts_flag);
	// init usb
	fclose(fp_GW);
	sprintf(rm_GW,"rm gatewidth%d.txt",sid);
	system(rm_GW);
	*/

	USB3Init();

	// open DAQ
	status = DAQopen(sid);
	char mkdir_name[200];
	sprintf(mkdir_name,"mkdir -p /home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/",run_number,run_number,sid);
	system(mkdir_name);

	for( int i = 0; i<3; i++){
		sprintf(filename, "/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/APIX_daq_%d_%d.dat",run_number,run_number,sid, i, run_number);
		fp[i] = fopen(filename, "wb"); 
	}
	unsigned long iloop=0;
    if (status > 0){
		// open file
		
		// assign data array
		data = (char *)malloc(65536);
		
		run = 1;
		while (run) {

			for( int ich = 0; ich < 3; ich++){
				
				data_size = DAQread_DATASIZE( sid, ich+1);
				//printf( "APIX_DataSize %d %d %ld \n",sid, ich+1, data_size);
				if( data_size > 0 ){
					DAQread_DATA( sid, ich+1, data_size, data);
					//printf("Read ch %d\n",ich);
					fwrite( data, 1, data_size*1024, fp[ich]);
					data_written[ich] = data_written[ich] + data_size;
					//printf("APIX_DAQ[%d] : %d : %d\n", sid, ich+1, data_written[ich]);
				}else{
					usleep(10);
				}
			}

			run = DAQread_RUN(sid);
			//printf("RUN %d : %d\n", iloop, run);
			if (access("KILLME",F_OK)==0)
				break;
			/*
			if (data_written > flag_data_size && flag ==0){
				char flag_name[100];
				sprintf(flag_name,"touch KILLTCB%d",sid);
				system(flag_name);
				flag = 1;
			}
			*/
			if( iloop % 100000 ==0 ){ printf("APIX RUN(%d) %d %d %d\n",run, data_written[0],data_written[1], data_written[2]);}
			iloop++;
		}
		
		printf("DAQ is stopped and read remaining data\n");
		
		// read remaining data  

		for( int ich = 0; ich < 3; ich++){
			data_size = DAQread_DATASIZE( sid, ich+1);
			if( data_size > 0 ){
				DAQread_DATA( sid, ich+1, data_size, data);
				fwrite( data, 1, data_size*1024, fp[ich]);
				data_written[ich] = data_written[ich] + data_size;
				//printf("APIX_DAQ[%d] : %d : %d\n", sid, ich+1, data_written[ich]);
			}	
		}

    }
	// release data array
	free(data);

	// close file
	for( int i = 0; i< 3; i++){
		fclose(fp[i]);
	}
	//fclose(fp_apix1);
	//fclose(fp_apix2);

	// close DAQ
	DAQclose(sid);

	// close usb
	USB3Exit();

	return 0;
}


