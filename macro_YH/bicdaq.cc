#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
using namespace std;
#define SAMPLE_CLOCK_PERIOD_NS 5

int bicdaq(int RunNo = 60051, int nEvtToRead=10000)
{
	bool fWrite = true;
	
	const int Len = 64; //kbytes
	int MID = 31;

	const char* inFile;
	if ( MID==31 ){
		inFile = Form("/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/jbnu_daq_%d_%i.dat", RunNo, RunNo, MID, MID, RunNo);

	}else{
		inFile = Form("/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/bic_daq_%d_%i.dat", RunNo, RunNo, MID, MID, RunNo);
	}
	cout <<Form("Start quick QA by directly decoding %s...\n", inFile);


	const int nCh = 32;
	int nHit[nCh] = {0};
	int trigN = -1;
	unsigned long long trigT = -1;
	//Get data file size (the size should be < 2 GB)
	FILE *fp = fopen(inFile, "rb");
	fseek(fp, 0L, SEEK_END);
	int file_size = ftell(fp);
	fclose(fp);

	//Variables
	char header[32];
	char data[10000];
	int data_read = 0;
	
	//Open data file and read event by event
	fp = fopen(inFile, "rb");
	int nPacketProcessed=0;

	TFile* F;
	if(fWrite){
		F = new TFile(Form("%i_%i.root",RunNo, MID), "recreate");
	}
	TTree* T = new TTree(Form("MID%i", MID), Form("MID%i", MID));

	UInt_t fFineTime;
	ULong_t fCoarseTime;
	int Cycle = 0;
	ULong_t tCoarseTimeBefore;

	T->Branch("finetime", 	&fFineTime,	"finetime/i");
	T->Branch("coarsetime", &fCoarseTime, "coarsetime/l");

	while (data_read < file_size)
	{
		//Read header
		//++++++++++++++++++++++++++++++++++++++++++++

		fread(header, 1, 32, fp);

		int data_length = 0;
		for (int a=0; a<4; a++) data_length += ((int)(header[a] & 0xFF) << 8*a);

		/*
		int run_number = 0;
		for (int a=0; a<2; a++) run_number += ((int)(header[a+4] & 0xFF) << 8*a);
		int trigger_type = ((int)header[6] & 0xFF);
		*/

		int tcb_trigger_number = 0;
		for (int a=0; a<4; a++) tcb_trigger_number += ((int)(header[a+7] & 0xFF) << 8*a);

		int tcb_trigger_fine_time = ((int)header[11] & 0xFF);
		int tcb_trigger_coarse_time = 0;
		for (int a=0; a<3; a++) tcb_trigger_coarse_time += ((int)(header[a+12] & 0xFF) << 8*a);
		if(tCoarseTimeBefore - tCoarseTime > 16e6) Cycle++;
		fCoarseTime = tcb_trigger_coarse_time * Cycle;
		fFineTime = tcb_trigger_fine_time;


		//int MID = ((int)header[15] & 0xFF);
		int channel = ((int)header[16] & 0xFF);

		if ( channel>nCh ){
			cout << "WARNNING! suspicious channel number! MID: " << MID << ", CH:" << channel << endl; 
			fread(data, 1, 128*4 - 32, fp);
			nPacketProcessed++;
			continue;
		}

		if ( fabs(tcb_trigger_number-trigN)>10000 ){
			cout << "WARNNING! suspicious tcb trigger number! MID: " << MID <<", TCB TrigN: " << tcb_trigger_number << " " << trigN 
				<< ", CH: " << channel << endl;
			fread(data, 1, 128*4 - 32, fp);
			nPacketProcessed++;
			continue;
		}
		if ( !(data_length==256 || data_length==128*4) ){
			if ( channel==0 ){
				fread(data, 1, 256 - 32, fp);
			}else{
				fread(data, 1, 128*4 - 32, fp);
			}
			cout << "WARNNING! suspicious data length! MID: " << MID << ", DLength: " << data_length << ", CH: " << channel << endl;
			nPacketProcessed++;
			continue;
		}

		int wave_length = (MID==31) ? (data_length - 32) / 2 : (data_length - 32) / 4;

		//Read body, data_length - 32 bytes (header)
		//++++++++++++++++++++++++++++++++++++++++++++

		fread(data, 1, data_length - 32, fp);

		data_read = data_read + data_length;

		//++++++++++++++++++++++++++++++++++++++++++++

		nPacketProcessed++;
		if (nPacketProcessed%10000 == 0) cout << "Processed eventNum = " << nPacketProcessed/(nCh+1) << endl;
		if ((nPacketProcessed/(nCh+1)) == nEvtToRead) break;
		
		T->Fill();
	}//While

	fclose(fp);
	if(fWrite){
		F->Write();
		F->Close();
	}
	return 0;
}


