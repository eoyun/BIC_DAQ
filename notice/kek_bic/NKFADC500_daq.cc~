#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <unistd.h>

#include "TROOT.h"
#include "TString.h"
#include "NoticeNKFADC500ROOT.h"
#include "usb3comroot.h"
using namespace std;

#define DRAM_SIZE       (10)                  // available PC DRAM size in Mbyte
#define CHUNK_SIZE      (DRAM_SIZE*1024)      // array size in kilobyte
#define DATA_ARRAY_SIZE (DRAM_SIZE*1024*1024) // array size in byte

int evt_calculator(int RL, long long data_size){
	return data_size * 1024 / RL / 128 / 4;
}

bool bSTOP = false;
void sigint_handler(int sig)
{
	bSTOP = true;
	return;
}

int main(int argc, char** argv)
{
	std::signal (SIGINT, sigint_handler);
	char gateW_file[100];

	//Local variables
	int runNo; //Run ID
	const int nCh       = 4; //# of channels per FADC module
	const int nMID      = 1; //# of FADC modules
	int mid; //FADC modules ID
	int data_written;

	//-------------------------------------------

	mid = atoi(argv[1]);
	runNo = atoi(argv[2]);

	sprintf(gateW_file,"gatewidth%d.txt",mid);
	FILE* fp_GW = fopen(gateW_file,"rt");
	int gate_width;
	fscanf(fp_GW,"%d",&gate_width);
	fclose(fp_GW);
	char rm_GW[100];
	sprintf(rm_GW,"rm gatewidth%d.txt",mid);
	system(rm_GW);
	//Output file
	string fName[nMID];
	ofstream ofStr[nMID];
	char mkdir_name[200];
	sprintf(mkdir_name,"mkdir -p /home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/",runNo,runNo,mid);
	system(mkdir_name);
	for (int a=0; a<nMID; a++)
	{
		fName[a] = Form("/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/FADCData_%i_%i.dat",runNo,runNo,mid, mid, runNo);
		ofStr[a].open(fName[a].c_str(), std::ios::binary);
	}

	//Raw data array
	char* dArray = (char*)malloc(nCh * DATA_ARRAY_SIZE);

	//Define VME class
	usb3comroot *usb = new usb3comroot;
	usb->USB3Init(0);

	//Define KFADC500 class
	NKNKFADC500* nkfadc = new NKNKFADC500;

	int status = 0;

	for (int a=0; a<nMID; a++)
	{
		status = status + nkfadc->NKFADC500open(mid, 0);
	}

	data_written = 0;
	

	//-------------------------------------------

	bool bCountNonzero = false;
	unsigned long bCount[nMID] = {0};
	if (status > 0){
		char flag_name[100];
		sprintf(flag_name,"touch KILLTCB%d",mid);
		system(flag_name);
		
		while (bSTOP == false) //Take data continually if nEvent = 0
		{
			bCountNonzero = false;
			for (int a=0; a<nMID; a++) //Check buffer count
			{
				bCount[a] = nkfadc->NKFADC500read_BCOUNT(mid);
				if (bCount[a] > 0) bCountNonzero = true;
			}
			if (bCountNonzero == false) continue; //usleep(10);
		
			for (int a=0; a<nMID; a++) //Loop over FADC modules
			{
				if (bCount[a] > 0)
				{
					if (bCount[a] > CHUNK_SIZE)
					{
						cout <<Form("WARNING! BCOUNT of mID_%i > CHUNK: truncate it...\n", a);
						bCount[a] = CHUNK_SIZE;
					}
					nkfadc->NKFADC500read_DATA(mid, bCount[a], dArray);
					ofStr[a].write(dArray, bCount[a] * 1024);
					data_written = data_written + bCount[a];
					int nevts_now = evt_calculator(gate_width, data_written);
					if (nevts_now % 1000 > 0 && nevts_now % 1000 < 100)printf("NKFADC500[%d] %d events are taken\n", mid, nevts_now);
				}
			}
			if (access("KILLME",F_OK)==0)
				break;
			
			//if (FILE *file = fopen("stop.txt", "r")) break;
		}//While
		
		for (int a=0; a<nMID; a++) //Loop over FADC modules
		{
			if (bCount[a] > 0)
			{
				if (bCount[a] > CHUNK_SIZE)
				{
					cout <<Form("WARNING! BCOUNT of mID_%i > CHUNK: truncate it...\n", a);
					bCount[a] = CHUNK_SIZE;
				}
				nkfadc->NKFADC500read_DATA(mid, bCount[a], dArray);
				ofStr[a].write(dArray, bCount[a] * 1024);
				data_written = data_written + bCount[a];
				printf("NKFADC500[%d] %d kbytes are taken\n", mid, data_written);
			}
		}
	}
	free(dArray); //Free data array
	for (int a=0; a<nMID; a++)
	{
		nkfadc->NKFADC500close(mid); //Close KFADC500
		ofStr[a].close(); //Close data file
	}
	usb->USB3Exit(0);

	printf("NKFADC stopped.\n");

	return 0;
}//Main
