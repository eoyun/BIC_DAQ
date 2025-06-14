#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "NoticeBIC_TCB.h"

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

	if (argc > 2)
	{
		run_number = atoi(argv[1]);
		acq_time   = atoi(argv[2]);
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
	
	sprintf(cmd, "./INIT.sh \n sleep 1");
	system(cmd);
	for (int a=0; a<num_of_daq; a++)
	{
		if (mid[a] < 20)
			sprintf(cmd, "./bin/NKFADC500_daq %d %d &", mid[a], run_number);
		else if (mid[a] < 40)
			sprintf(cmd, "./bin/JBNU_DAQ_daq.exe %d %d &", mid[a], run_number);
		else
			sprintf(cmd, "./bin/BIC_DAQ_daq.exe %d %d &", mid[a], run_number);
		
		system(cmd); 
		sprintf(cmd,"sleep 1");
		system(cmd);
	}
	
	for (sec = 0; sec < acq_time; sec++) {
		usleep(1000000);
		printf("%d/%d sec\n", sec + 1, acq_time);
	}
	
//	system("touch stop.txt");

	return 0;
}


