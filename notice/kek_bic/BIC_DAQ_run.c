#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "NoticeBIC_TCB.h"

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define BLUE "\033[1;34m"
#define RESET "\033[0m"

int main(int argc, char *argv[])
{
//	signal(SIGINT, sigint_handler);

	int sid = 0;
	int mid[40];
	int daq_mid[40]; //Given sid to each DAQ
	int link_status[2];
	int daq_exist;
	int run_number;
	int acq_time;
	char cmd[100];
	int sec;
	int nbytes;

	if (argc > 2)
	{
		run_number = atoi(argv[1]);
		nbytes   = atoi(argv[2]);
	}
	else
	{
		printf("Arguments (runnumber and runtime) are missing! Stop.\n");
		return 0;
	}

	system("rm stop.txt");

	// init USB
	USB3Init();

	// open TCB
	TCBopen(sid);

	// reset TCB
	TCBreset(sid);
	acq_time = 2147483647;
	//------------------------------------------------
	//Initialize daq_mid, ckim
	for (int a=0; a<40; a++) daq_mid[a] = -999;

	// get link status and ADC mid
	int num_of_daq = 0;
	TCBread_LINK_STATUS(sid, link_status);
	TCBread_MID(sid, daq_mid);
	printf("link status = %X %X\n", link_status[1], link_status[0]);
	for (int daq = 0; daq < 32; daq++)
	{
		daq_exist = link_status[0] & (1 << daq);
		if (daq_exist)
		{
			mid[num_of_daq] = daq_mid[daq]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq);
			num_of_daq = num_of_daq + 1;
		}
	}
	for (int daq = 0; daq < 8; daq++)
	{
		daq_exist = link_status[1] & (1 << daq);
		if (daq_exist)
		{
			mid[num_of_daq] = daq_mid[daq + 32]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 32);
			num_of_daq = num_of_daq + 1;
		}
	}

	// read temperature and pedestal
	for (int daq = 0; daq < num_of_daq; daq++)
	{
		for (int ch = 1; ch <= 32; ch++)
		{
			printf("DAQ[%d] ch%d pedestal = %d\n", mid[daq], ch, TCBread_PED(sid, mid[daq], ch));
		}
	}

	// set run number
	TCBwrite_RUN_NUMBER(sid, run_number);
	printf("run_number = %d\n", TCBread_RUN_NUMBER(sid));

	// set acquisition time
	TCBwrite_ACQ_TIME(sid, acq_time );
	printf("acquisition time = %d s\n", TCBread_ACQ_TIME(sid));

	// reset timer if necessary
	// TCBreset_TIMER(sid);  

	//------------------------------------------------

	// start DAQ
	TCBstart_DAQ(sid);
	printf("Run = %d\n", TCBread_RUN(sid));

	// close DAQ
	TCBclose(sid);

	// close usb
	USB3Exit();
	
	//sprintf(cmd, "./INIT.sh \n sleep 1");
	//system(cmd);
	int flag[num_of_daq];
	for (int a=0; a<num_of_daq; a++)
	{
		flag[a] = 0;
		if (mid[a] < 20){
		  //sprintf(cmd, "./bin/NKFADC500_daq %d %d &", mid[a], run_number);
		  sprintf(cmd, "./bin/NKFADC500_daq %d %d &", mid[a], run_number);
		}else if (mid[a] < 40){
		  //sprintf(cmd, "./bin/JBNU_DAQ_daq.exe %d %d %d &", mid[a], run_number, nbytes);
		  sprintf(cmd, "./JBNU_DAQ_daq.exe %d %d %d &", mid[a], run_number, nbytes);

		}else if ( mid[a] < 50){
		  //sprintf(cmd, "./bin/BIC_DAQ_daq.exe %d %d %d &", mid[a], run_number, nbytes);
		  sprintf(cmd, "./BIC_DAQ_daq.exe %d %d %d >> ./log/bic_%d.log &", mid[a], run_number, nbytes, run_number);
		}else{
		  sprintf(cmd, "./APIX_DAQ_daq.exe %d %d &", mid[a], run_number);
		}

		printf(cmd);
		printf("\n");
		
		int rtn =system(cmd);
		printf("Returns : %d",rtn);
					     
		sprintf(cmd,"sleep 1");
		system(cmd);
	}
	int flag_daq = 1;
	for (sec = 0; sec < acq_time; sec++) {
		usleep(1000000);
		printf("%d sec\n", sec + 1);
		if (access("KILLME",F_OK)==0){
		  usleep(1000000);
		  
		  system("rm KILLME");
		  //printf("###############\nrun is ended\n\n\n###############");
		  printf(RED "\n=================================================\n" RESET);
		  printf(YELLOW "      ⚠️  Received termination (%d) ⚠️\n" RESET, run_number);
		  printf(GREEN "         Ending the run gracefully... \n" RESET);
		  printf(RED "=================================================\n" RESET);
		  
		  // ASCII Art for exit message
		  printf("\n");
		  printf(BLUE "  ██████  ██   ██████     ██   ██ ███████ ██   ██\n");
		  printf(BLUE "  ██   ██ ██  ██          ██ ██   ██      ██ ██  \n");
		  printf(BLUE "  ██████  ██ ██           ███     █████   ███    \n");
		  printf(BLUE "  ██   ██ ██  ██          ██ ██   ██      ██ ██  \n");
		  printf(BLUE "  ██████  ██   ██████     ██   ██ ███████ ██   ██\n");
		  
		  break;
		}
		
		for (int a = 0; a < num_of_daq; a++){
			char flag_name[100];
			char rm_flag[100];
			sprintf(flag_name,"KILLTCB%d",mid[a]);
			sprintf(rm_flag,"rm KILLTCB%d",mid[a]);
			if (access(flag_name,F_OK)==0 && flag[a] == 0){
				system(rm_flag);
				flag_daq ++;
				flag[a] = 1;
				printf("%d daqs are done!\n",flag_daq-1);
			}
			if(flag_daq == num_of_daq ) system("touch KILLME");
		}
	}
	
//	system("touch stop.txt");

	return 0;
}


