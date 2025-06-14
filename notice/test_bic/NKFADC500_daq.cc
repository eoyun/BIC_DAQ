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


bool bSTOP = false;
void sigint_handler(int sig)
{
	bSTOP = true;
	return;
}

int main(int argc, char** argv)
{
	std::signal (SIGINT, sigint_handler);

	//Local variables
	int runNo; //Run ID
	const int nCh       = 4; //# of channels per FADC module
	const int nMID      = 1; //# of FADC modules
	int mid; //FADC modules ID
	int data_written;

	//-------------------------------------------

	mid = atoi(argv[1]);
	runNo = atoi(argv[2]);

	//Output file
	string fName[nMID];
	ofstream ofStr[nMID];
	for (int a=0; a<nMID; a++)
	{
		fName[a] = Form("FADCData_%i_%i.dat", mid, runNo);
		ofStr[a].open(fName[a].c_str(), std::ios::binary);
	}

	//Raw data array
	char* dArray = (char*)malloc(nCh * DATA_ARRAY_SIZE);

	//Define VME class
	usb3comroot *usb = new usb3comroot;
	usb->USB3Init(0);

	//Define KFADC500 class
	NKNKFADC500* nkfadc = new NKNKFADC500;
	for (int a=0; a<nMID; a++)
	{
		nkfadc->NKFADC500open(mid, 0);
	}

	data_written = 0;
	

	//-------------------------------------------

	bool bCountNonzero = false;
	unsigned long bCount[nMID] = {0};
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
				printf("NKFADC500[%d] %d kbytes are taken\n", mid, data_written);
			}
		}
		
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
