#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_TCB.h"

int main(int argc, char *argv[])
{
	int sid;
	if (argc > 1) sid = atoi(argv[1]);
	else
	{
		sid = 0;
		//printf("sid is missing! Stop.\n"); return 0;
	}

	// init USB
	USB3Init();

	// open TCB
	TCBopen(sid);

	// reset TCB
	TCBreset(sid);

	// Check link status of DAQs
	//-------------------------------------------

	int link_status[2] = {-999, -999};
	int daq_exist = -999;
	int num_of_daq = 0;
	int daq_mid[40] = {0};
	int mid[40] = {0};

	// get link status and ADC mid
	TCBread_LINK_STATUS(sid, link_status);
	TCBread_MID(sid, daq_mid);
	printf("link status = %X %X\n", link_status[1], link_status[0]);
	for (int daq = 0; daq < 32; daq++)
	{
		daq_exist = link_status[0] & (1 << daq);
		if (daq_exist)
		{
			mid[num_of_daq] = daq_mid[daq]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 1);
			num_of_daq = num_of_daq + 1;
		}
	}
	for (int daq = 0; daq < 8; daq++)
	{
		daq_exist = link_status[1] & (1 << daq);
		if (daq_exist)
		{
			mid[num_of_daq] = daq_mid[daq + 32]; 
			printf("DAQ[%d] is found @%d\n", mid[num_of_daq], daq + 33);
			num_of_daq = num_of_daq + 1;
		}
	}
	if (num_of_daq == 0) { printf("No daq is found! Stop.\n"); return 0; }

	// Read settings, write on each DAQ, and read them back
	//-----------------------------------------------------
	char filenameTCB[100];
	sprintf(filenameTCB,"setup/setup_TCB.txt");
	FILE *fp = fopen(filenameTCB, "rt");
	int coin_width;
	int multi_thr_tcb;
	int ptrig_interval;
	int trig_enable;
	fscanf(fp, "%d", &coin_width); //1
	fscanf(fp, "%d", &multi_thr_tcb); //2
	fscanf(fp, "%d", &ptrig_interval); //4
	fscanf(fp, "%d", &trig_enable); //7
	TCBwrite_COIN_WIDTH(sid, 0, coin_width);
	TCBwrite_MULTI_THR(sid, 0, multi_thr_tcb);
	TCBwrite_PEDESTAL_TRIGGER_INTERVAL(sid, ptrig_interval);
	TCBwrite_TRIGGER_ENABLE(sid, trig_enable);
	printf("TCB coincidence width = %d ns\n", TCBread_COIN_WIDTH(sid, 0));
	printf("TCB multiplicity threshold = %d\n", TCBread_MULTI_THR(sid, 0));
	printf("TCB pedestal trigger interval = %d ms\n", TCBread_PEDESTAL_TRIGGER_INTERVAL(sid));
	printf("TCB trigger enable = %d\n", TCBread_TRIGGER_ENABLE(sid));



	for (int a=0; a<num_of_daq; a++)
	{
		char filename[100];
		sprintf(filename, "setup/setup_%d.txt", mid[a]);
		FILE *fp = fopen(filename, "rt");

		if (mid[a] < 20) // sid < 20, nkfadc500
		{
			//Read
			int fRL;      // Recoding length
			int fDLY;     // Delay
			int fTHR;     // Threshold
			int fAMODE;   // ADC mode: 0 = raw, 1 = filtered
			int fDACOFF;  // ADC offset value: 0 ~ 4095
			int fPOL;     // Input pulse polarity: 0 = negative, 1 = positive
			fscanf(fp, "%d", &fRL); //1
			fscanf(fp, "%d", &fDLY); //2
			fscanf(fp, "%d", &fTHR); //3
			fscanf(fp, "%d", &fAMODE); //4
			fscanf(fp, "%d", &fDACOFF); //5
			fscanf(fp, "%d", &fPOL); //6

			//Write
			MINITCB_V2write_RL(sid, mid[a], fRL);
			for (int ch=0; ch<4; ch++)
			{
				MINITCB_V2write_DLY   (sid, mid[a], ch+1, fDLY);
				MINITCB_V2write_THR   (sid, mid[a], ch+1, fTHR);
				MINITCB_V2write_AMODE (sid, mid[a], ch+1, fAMODE);
				MINITCB_V2write_DACOFF(sid, mid[a], ch+1, fDACOFF);
				MINITCB_V2write_POL   (sid, mid[a], ch+1, fPOL);
				MINITCB_V2measure_PED (sid, mid[a], ch+1);
			}
			MINITCB_V2write_POL     (sid, mid[a], 5, fPOL); //5 -> TDC
			MINITCB_V2write_DRAMON  (sid, mid[a], 1);
			MINITCB_V2_ADCALIGN     (sid, mid[a]);
			MINITCB_V2_ADCALIGN_DRAM(sid, mid[a]);

			//Read back settings
			printf("\nNKFADC500: MID%d\n", mid[a]);
            printf("- %-25s: %d", "Recording length", MINITCB_V2read_RL(sid, mid[a]));
            printf("\n- %-25s:", "Delay");
            for (int ch=0; ch<4; ch++) printf("%5d", MINITCB_V2read_DLY(sid, mid[a], ch+1));
            printf("\n- %-25s:", "Threshold");
            for (int ch=0; ch<4; ch++) printf("%5d", MINITCB_V2read_THR(sid, mid[a], ch+1));
            printf("\n- %-25s:", "ADC mode");
            for (int ch=0; ch<4; ch++) printf("%5d", MINITCB_V2read_AMODE(sid, mid[a], ch+1));
            printf("\n- %-25s:", "DAC offset");
            for (int ch=0; ch<4; ch++) printf("%5d", MINITCB_V2read_DACOFF(sid, mid[a], ch+1));
            printf("\n- %-25s:", "Input pulse polarity");
            for (int ch=0; ch<4; ch++) printf("%5d", MINITCB_V2read_POL(sid, mid[a], ch+1));
            printf("\n- %-25s: %d\n", "DRAM_ON", MINITCB_V2read_DRAMON(sid, mid[a]));
		}
		else if (mid[a] < 40) // 20 <= sid < 40, jbnu_daq
		{
			//Read
			float hv;
			int multi_thr_daq;
			int disc_thr;
			int gate_width;
			int idelay;
			int peak_sum_width;
			int zero_sup;
			int prescale;
			//float temp[2] = {0};
			fscanf(fp, "%f", &hv); //1
			fscanf(fp, "%d", &multi_thr_daq); //4
			fscanf(fp, "%d", &disc_thr); //6
			fscanf(fp, "%d", &gate_width); //7
			fscanf(fp, "%d", &idelay); //9
			fscanf(fp, "%d", &peak_sum_width); //10
			fscanf(fp, "%d", &zero_sup); //11
			fscanf(fp, "%d", &prescale); //12

			// Wwrite
			TCBinit_JBNU_ADC(sid, mid[a]);
			TCBinit_DRAM(sid, mid[a]);
			TCBwrite_HV(sid, mid[a], 1, hv);
			TCBwrite_HV(sid, mid[a], 2, hv);
			TCBwrite_COIN_WIDTH(sid, mid[a], coin_width);
			TCBwrite_MULTI_THR(sid, mid[a], multi_thr_daq);
			TCBwrite_GATE_WIDTH(sid, mid[a], gate_width);
			TCBwrite_DELAY(sid, mid[a], idelay);
			TCBwrite_PEAK_SUM_WIDTH(sid, mid[a], peak_sum_width);
			TCBwrite_ZERO_SUP(sid, mid[a], zero_sup);
			TCBwrite_PRESCALE(sid, mid[a], prescale);
			TCBmeasure_PED(sid, mid[a]);
			for (int ch = 1; ch <= 32; ch++) TCBwrite_DISC_THR(sid, mid[a], ch, disc_thr);

			// Read back settings
			printf("\nJBNU_DAQ: MID%d\n", mid[a]);
			printf("DAQ[%d] high voltage ch1 = %f V\n", mid[a], TCBread_HV(sid, mid[a], 1));
			printf("DAQ[%d] high voltage ch2 = %f V\n", mid[a], TCBread_HV(sid, mid[a], 2));
			printf("DAQ[%d] coincidence width = %d ns\n", mid[a], TCBread_COIN_WIDTH(sid, mid[a]));
			printf("DAQ[%d] multiplicity threshold = %d\n", mid[a], TCBread_MULTI_THR(sid, mid[a]));
			printf("DAQ[%d] discriminator threshold = %d\n", mid[a], TCBread_DISC_THR(sid, mid[a], 1));
			printf("DAQ[%d] gate width = %d us\n", mid[a], TCBread_GATE_WIDTH(sid, mid[a]));
			printf("DAQ[%d] input delay = %d ns\n", mid[a], TCBread_DELAY(sid, mid[a]));
			printf("DAQ[%d] peak sum width = %d ns\n", mid[a], TCBread_PEAK_SUM_WIDTH(sid, mid[a]));
			printf("DAQ[%d] zero suppression = %d\n", mid[a], TCBread_ZERO_SUP(sid, mid[a]));
			printf("DAQ[%d] waveform prescale = %d\n", mid[a], TCBread_PRESCALE(sid, mid[a]));
			//TCBread_TEMP(sid, mid[a], temp); !!!!!
			//printf("DAQ[%d] temperature ch1 = %f\n", mid[a], temp[0]);
			//printf("DAQ[%d] temperature ch2 = %f\n", mid[a], temp[1]);
			for (int ch = 1; ch <= 32; ch++)
			{
				if (ch == 1) printf("DAQ[%d] pedestal:\n", mid[a]);
				printf("%d ", TCBread_PED(sid, mid[a], ch));
				if (ch%8 == 0) printf("\n");
			}
		}// 20 <= sid < 40, jbnu_daq
		else // sid >= 40, bic_daq
		{
			//Read
			int multi_thr_daq;
			int disc_thr;
			int gate_width;
			int idelay;
			int peak_sum_width;
			int zero_sup;
			int prescale;
			int disc_enable[32];
			fscanf(fp, "%d", &multi_thr_daq); //3
			fscanf(fp, "%d", &disc_thr); //5
			fscanf(fp, "%d", &gate_width); //6
			fscanf(fp, "%d", &idelay); //8
			fscanf(fp, "%d", &peak_sum_width); //9
			fscanf(fp, "%d", &zero_sup); //10
			fscanf(fp, "%d", &prescale); //11
			for (int ch = 1; ch <= 32; ch++) fscanf(fp, "%d", disc_enable + ch); //12+

			//Write
			TCBinit_BIC_ADC(sid, mid[a]);
			TCBinit_DRAM(sid, mid[a]);
			TCBwrite_COIN_WIDTH(sid, mid[a], coin_width);
			TCBwrite_MULTI_THR(sid, mid[a], multi_thr_daq);
			TCBwrite_GATE_WIDTH(sid, mid[a], gate_width);
			TCBwrite_DELAY(sid, mid[a], idelay);
			TCBwrite_PEAK_SUM_WIDTH(sid, mid[a], peak_sum_width);
			TCBwrite_ZERO_SUP(sid, mid[a], zero_sup);
			TCBwrite_PRESCALE(sid, mid[a], prescale);
			TCBwrite_DISC_ENABLE(sid, mid[a], disc_enable);
			TCBmeasure_PED(sid, mid[a]);
			for (int ch = 1; ch <= 32; ch++) TCBwrite_DISC_THR(sid, mid[a], ch, disc_thr);

			//Read back
			printf("\nBIC_DAQ: MID%d\n", mid[a]);
			printf("DAQ[%d] coincidence width = %d ns\n", mid[a], TCBread_COIN_WIDTH(sid, mid[a]));
			printf("DAQ[%d] multiplicity threshold = %d\n", mid[a], TCBread_MULTI_THR(sid, mid[a]));
			printf("DAQ[%d] discriminator threshold = %d\n", mid[a], TCBread_DISC_THR(sid, mid[a], 1));
			printf("DAQ[%d] gate width = %d us\n", mid[a], TCBread_GATE_WIDTH(sid, mid[a]));
			printf("DAQ[%d] input delay = %d ns\n", mid[a], TCBread_DELAY(sid, mid[a]));
			printf("DAQ[%d] peak sum width = %d ns\n", mid[a], TCBread_PEAK_SUM_WIDTH(sid, mid[a]));
			printf("DAQ[%d] zero suppression = %d\n", mid[a], TCBread_ZERO_SUP(sid, mid[a]));
			printf("DAQ[%d] waveform prescale = %d\n", mid[a], TCBread_PRESCALE(sid, mid[a]));
			printf("DAQ[%d] disc enable \n", mid[a]);
			TCBread_DISC_ENABLE(sid, mid[a], disc_enable);
			for (int ch = 1; ch <= 32; ch++)
			{
				if (ch == 1) printf("DAQ[%d] discriminator enabled status:\n", mid[a]);
				printf("%d ", disc_enable[ch]);
				if (ch%8 == 0) printf("\n");
			}
			for (int ch = 1; ch <= 32; ch++)
			{
				if (ch == 1) printf("DAQ[%d] pedestal:\n", mid[a]);
				printf("%d ", TCBread_PED(sid, mid[a], ch));
				if (ch%8 == 0) printf("\n");
			}
		}//bic_daq

		fclose(fp);
	}//a, num_of_daq

	//-------------------------------------------

	// only if necessary, reset timer
	TCBreset_TIMER(sid);

	// reset DAQ
	TCBreset(sid);

	// close TCB
	TCBclose(sid);

	// close USB
	USB3Exit();

	return 0;
}


