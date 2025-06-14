#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"

#include <bitset>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <vector>
using namespace std;

#define bDEBUG true 

//for drawing
map<int, int> wform_factor = {{1, 1000}, {2, 1000}, {21, 1000}, {41, 500}, {42, 500}};
map<int, int> cutPed = {{21, 50}, {41, 50}, {42, 50}};

const float xCVS = 1.0; //Adjust canvas size

TCanvas *ctmp;
TCanvas *c2;

void bic_daq_quickQA(int RunNo, int nEvtToRead, const char* inPath, int mid);
void nkfadc_daq_quickQA(int RunNo, int nEvtToRead, const char* inPath, int mid);

void quickQA(int RunNo = 2080, int nEvtToRead = 10000, const char* inPath = "data_hodoscope_raw"){

	gStyle->SetOptStat(0);
	gStyle->SetTitleSize(0.04);

	vector<int> mid = {41, 42};

	if ( bDEBUG ){
		ctmp = new TCanvas("ctmp", "ctmp", 600, 500);
	}

	c2 = new TCanvas("c2", Form("RUN %d, DAQ Trigger QA", RunNo), -1, 0, 200*1.2*5*xCVS, 200*2*xCVS); 
	gPad->SetMargin(0,0,0,0);
	c2->Divide(5,2);

	if ( find(mid.begin(), mid.end(), 1)!=mid.end() ){
		nkfadc_daq_quickQA(RunNo, nEvtToRead, inPath, 1);
	}
	if ( find(mid.begin(), mid.end(), 2)!=mid.end() ){
		nkfadc_daq_quickQA(RunNo, nEvtToRead, inPath, 2);
	}
	if ( find(mid.begin(), mid.end(), 21)!=mid.end() ){
		bic_daq_quickQA(RunNo, nEvtToRead, inPath, 21);
	}
	if ( find(mid.begin(), mid.end(), 42)!=mid.end() ){
		bic_daq_quickQA(RunNo, nEvtToRead, inPath, 42);
	}
	if ( find(mid.begin(), mid.end(), 41)!=mid.end() ){
		bic_daq_quickQA(RunNo, nEvtToRead, inPath, 41);
	}

	return;
}


int GetDataLength(const char* inFile)
{
	//Reading 1st event, 1st header (32 bytes fixed), 1st channel's very first four bits will be enough
	ifstream in;
	in.open(inFile, std::ios::binary);
	if (!in.is_open()) { cout <<"GetDataLength - cannot open the file! Stop.\n"; return 1; }

	char data[32];
	in.read(data, 32);
	unsigned long dataLength = 0;
	for (int i=0; i<4; i++) dataLength += ((ULong_t)(data[4*i] & 0xFF) << 8*i);

	in.close();
	return (int)dataLength;
}//GetDataLength

map<int, int> GetChMap(void)
{
    std::map<int, int> chMap;

    chMap.insert( std::pair<int, int> ( 1,  1) );
    chMap.insert( std::pair<int, int> ( 2,  2) );
    chMap.insert( std::pair<int, int> ( 3,  3) );
    chMap.insert( std::pair<int, int> ( 4,  4) );
    chMap.insert( std::pair<int, int> ( 5,  8) );
    chMap.insert( std::pair<int, int> ( 6,  7) );
    chMap.insert( std::pair<int, int> ( 7,  6) );
    chMap.insert( std::pair<int, int> ( 8,  5) );
    chMap.insert( std::pair<int, int> ( 9, 12) );
    chMap.insert( std::pair<int, int> (10, 11) );
    chMap.insert( std::pair<int, int> (11, 10) );
    chMap.insert( std::pair<int, int> (12,  9) );
    chMap.insert( std::pair<int, int> (13, 13) );
    chMap.insert( std::pair<int, int> (14, 14) );
    chMap.insert( std::pair<int, int> (15, 15) );
    chMap.insert( std::pair<int, int> (16, 16) );

    chMap.insert( std::pair<int, int> ( 1+16,  1+16) );
    chMap.insert( std::pair<int, int> ( 2+16,  2+16) );
    chMap.insert( std::pair<int, int> ( 3+16,  3+16) );
    chMap.insert( std::pair<int, int> ( 4+16,  4+16) );
    chMap.insert( std::pair<int, int> ( 5+16,  8+16) );
    chMap.insert( std::pair<int, int> ( 6+16,  7+16) );
    chMap.insert( std::pair<int, int> ( 7+16,  6+16) );
    chMap.insert( std::pair<int, int> ( 8+16,  5+16) );
    chMap.insert( std::pair<int, int> ( 9+16, 12+16) );
    chMap.insert( std::pair<int, int> (10+16, 11+16) );
    chMap.insert( std::pair<int, int> (11+16, 10+16) );
    chMap.insert( std::pair<int, int> (12+16,  9+16) );
    chMap.insert( std::pair<int, int> (13+16, 13+16) );
    chMap.insert( std::pair<int, int> (14+16, 14+16) );
    chMap.insert( std::pair<int, int> (15+16, 15+16) );
    chMap.insert( std::pair<int, int> (16+16, 16+16) );

    return chMap;
}//map

//--------------------------------------------------------------------------------------------------------
void nkfadc_daq_quickQA(int RunNo = 2080, int nEvtToRead = 10000, const char* inPath = "data_hodoscope_raw", int mid = 1)
{

	const char* inFile = Form("%s/FADCData_%d_%i.dat", inPath, mid, RunNo);
	cout <<Form("Start quick QA by directly decoding %s...\n", inFile);

	const int nCh  = 4;
	const bool FlipADC = false; //If true flip the ADC to '4095 - ADC'

	const int DLen = GetDataLength(Form("%s/FADCData_%d_%i.dat", inPath, mid, RunNo)); //header (32) + body (vary), per ch
	if (DLen > 16384) { cout <<"WARNING! Irregular data length detected: stop.\n"; return; }
	const int TLen = DLen * nCh; //Total length
	const int nADC = (DLen - 32)/2; //# of ADC samples per event
	const int nTDC = nADC/4;

	UInt_t  fADC[nCh][nADC]; //ADC
	UInt_t  fTDC[nCh][nTDC]; //TDC

	if ( bDEBUG ){
		cout << "Total length: " << TLen << ", nADC: " << nADC << ", nTDC: " << nTDC << endl;
	}

	//Read binary and Convert it to ROOT
	//-------------------------------------------

	TH1F* H1_pulse = new TH1F(Form("H1_pulse_%d",mid), "", 2500, 0, 2500);
	TCanvas* c1 = new TCanvas(Form("c1_%d",mid), Form("RUN %d, MID %d Channel QA", RunNo, mid), -1, 200*2*xCVS, 120*1.2*4*xCVS, 120*1*xCVS); c1->Divide(4, 1);
	gPad->SetMargin(0,0,0,0);
	for (int a=0; a<nCh; a++)
	{
		c1->cd(a+1);
		gPad->SetMargin(0.17,0.01,0.16,0.01);
		gPad->SetTicks();
		TH1F* H1Temp = new TH1F(Form("RUN %d, MID %d, CH%d", RunNo, mid, a+1), "", 2500, 0, 2500);
		H1Temp->GetXaxis()->SetRangeUser(0, nADC);
		H1Temp->GetYaxis()->SetRangeUser(-100, 5500);
		H1Temp->GetXaxis()->SetTitle("Time bin");
		H1Temp->GetXaxis()->SetTitleSize(0.05*1.6);
		H1Temp->GetXaxis()->SetLabelSize(0.045*1.6);
		H1Temp->GetYaxis()->SetTitle("ADC");
		H1Temp->GetYaxis()->SetTitleSize(0.05*1.6);
		H1Temp->GetYaxis()->SetLabelSize(0.045*1.6);
		H1Temp->GetYaxis()->SetTitleOffset(1.2);
		H1Temp->DrawCopy();
	}

	int trigN = -1;
	unsigned long long trigT = -1;
	TH2F* H2 = new TH2F(Form("trig_nVSt_%d",mid), Form("RUN %d, MID %d; Trigger Number; Trigger Time", RunNo, mid), 10000,0,10000, 200,0,0.5*1.E10);

	ifstream in;
	int nPacketProcessed=0;
	char data[TLen];
	char dataChop[nCh][DLen];
	in.open(Form("%s/FADCData_%d_%d.dat", inPath, mid, RunNo), std::ios::binary);
	if (!in.is_open()) { cout <<"Cannot open the file! Stop.\n"; return; }

	unsigned long nLine = 0;
	while (in.peek() != EOF)
	{
		in.read(data, TLen);
		if (!in.good())	{ cout <<"Data file is currupted! Stop.\n"; break; }

		for (int j=0; j<nCh; j++)
		{
			for (int k=0; k<DLen; k++) dataChop[j][k] = data[j + 4*k];

				/*
				+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				Header
				Byte: (item #) Contents

				  0-3:  (1) Data length: total data length in bytes including header & body,
				  4-5:  (2) Run number: run number received from TCB
				    6:  (3) Trigger type: trigger type received from TCB
			     7-10:  (4) Trigger number: trigger number received from TCB
				   11:  (5) Trigger fine time: trigger time received from TCB, 0~124, 8 ns per unit
			    12-14:  (6) Trigger coarse time: trigger time received from TCB, 0~16,777,215 (us), 1 us per unit
			       15:  (7) ADC module ID
				   16:  (8) ADC channel number
			    17-20:  (9) Local trigger number
			    21-22: (10) Local trigger pattern
			    23-24: (11) ADC pedestal
			       25: (12) Data starting fine time, unit is 8 ns
			    26-31: (13) Data starting coarse time. unit is us

				* 8 bits per bytes: e.g., 0-3 means [7:0], [15:8], [23:16], and [31:24]
				+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				*/

				ULong_t tDataLen = 0;
				for (int a=0; a<4; a++) tDataLen += ((ULong_t)(dataChop[j][a] & 0xFF) << 8*a);

				UInt_t tTrigType = dataChop[j][6] & 0x0F;

				ULong_t tTrigNum = 0;
				for (int a=0; a<4; a++) tTrigNum += ((ULong_t)(dataChop[j][7+a] & 0xFF) << 8*a);

				int tTrigFineTime = (int)(dataChop[j][11] & 0xFF);
				int tTrigCoarseTime = 0;
				for (int a=0; a<3; a++) tTrigCoarseTime += ((int)(dataChop[j][a+12] & 0xFF) << 8*a);
				unsigned long long tTrigTime = (tTrigFineTime * 8) + (tTrigCoarseTime * 1000); 

					/*
				unsigned long long tTrigTime = ( (int)(dataChop[j][11] & 0xFF)        ) * 8    + //Fine time: 8 ns/unit
					                ( (int)(dataChop[j][12] & 0xFF)        ) * 1000 + //Coarse time: 1 us/unit
									( (ULong_t)(dataChop[j][13] & 0xFF) << 8   ) * 1000 +
									( (ULong_t)(dataChop[j][14] & 0xFF) << 8*2 ) * 1000;
									*/

				UInt_t tModuleID = dataChop[j][15] & 0xFF;
				UInt_t tChannel  = dataChop[j][16] & 0xFF;
				if (tChannel<1 || tChannel>4) cout <<Form("WARNING: irregular ch ID found: %i\n", tChannel);

				ULong_t tLocTNum = 0;
				for (int a=0; a<4; a++) tLocTNum += ( (ULong_t)(dataChop[j][17+a] & 0xFF) << 8*a );

				UInt_t tLocTPat = 0;
				for (int a=0; a<2; a++) tLocTPat += ( (dataChop[j][21+a] & 0xFF) << 8*a );

				UInt_t tPed = 0;
				for (int a=0; a<2; a++) tPed += ( (dataChop[j][23+a] & 0xFF) << 8*a );

				//unsigned int: 16 bit, unsigned long: 32 bit, unsigned long long = 64 bit
				unsigned long long tDatTime = ( (unsigned long long)(dataChop[j][25] & 0xFF)        ) * 8    +
					                          ( (unsigned long long)(dataChop[j][26] & 0xFF)        ) * 1000 +
											  ( (unsigned long long)(dataChop[j][27] & 0xFF) << 8   ) * 1000 +
											  ( (unsigned long long)(dataChop[j][28] & 0xFF) << 8*2 ) * 1000 +
											  ( (unsigned long long)(dataChop[j][29] & 0xFF) << 8*3 ) * 1000 +
											  ( (unsigned long long)(dataChop[j][30] & 0xFF) << 8*4 ) * 1000 +
											  ( (unsigned long long)(dataChop[j][31] & 0xFF) << 8*5 ) * 1000;

				if ( bDEBUG ){
					cout <<Form("%i %i %i %i | %4li %4i | %i %5li %5li %i | %lli %lli \n",
							mid,tModuleID,j+1,tChannel, tDataLen,tPed,
							tTrigType,tTrigNum,tLocTNum,tLocTPat, tTrigTime,tDatTime);
				}


				if ( trigN!=tTrigNum ){
					if ( trigT!=tTrigTime ){
						trigN = tTrigNum;
						trigT = tTrigTime;
					}else{
						cout << "WARNNING! different trigger number but same trigger time!" << endl;
					}
				}

				H2->Fill(tTrigNum, tTrigTime);

				/*
				++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				Body
				Byte: [Bits], Contents

				0: [7~0], ADC data[0][ 7:0]
				1: [3~0], ADC data[0][11:8]
				1: [7~4], TDC data[0][ 3:0]
				2: [7~0], ADC data[1][ 7:0]
				3: [3~0], ADC data[1][11:8]
				3: [7~4], TDC data[0][ 7:4]
				4: [7~0], ADC data[2][ 7:0]
				5: [3~0], ADC data[2][11:8]
				5: [7~4], TDC data[0][11:8]
				6: [7~0], ADC data[3][ 7:0]
				7: [3~0], ADC data[3][11:8]
				7: [7~4], 0 (empty)
				---------------------------
				8: [7~0], ADC data[0][ 7:0]
				...

				* 4 ADC samples and 1 TDC sample per 8 bytes, then repeat for 'N = (data_length - 32)/2'
				* 8 ps per TDC channel
				++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
				*/

				H1_pulse->Reset();
				bool validPulse = false;
				for (int a=0; a<nADC; a++)
				{
					const int iSmp = 32 + 2*a;
					UInt_t tADC = (dataChop[j][iSmp] & 0xFF) + ((dataChop[j][iSmp + 1] & 0xF) << 8);

					//if (j==0 || j==2) fADC[j][a] = (4095 - tADC); //TEMPORARY! Aug. 23, 2022
					if (FlipADC == true) fADC[j][a] = (4095 - tADC);
					else fADC[j][a] = tADC;

					H1_pulse->SetBinContent(a+1, fADC[j][a]);
				}

				/*
				for (int a=0; a<nTDC; a++)
				{
					const int iSmp = 32 + 8*a;
					UInt_t tTDC = ((dataChop[j][iSmp + 1] & 0xF0) >> 4) + 
						          ((dataChop[j][iSmp + 3] & 0xF0)) +
						          ((dataChop[j][iSmp + 5] & 0xF0) << 4);

					fTDC[j][a] = tTDC;
				}
				*/

				//if (validPulse)
				if (1)
				{
					//nHit[ch-1]++;
					c1->cd(j+1);
					H1_pulse->DrawCopy("same");
				}

				nPacketProcessed++;
				if (nPacketProcessed%10000 == 0) cout << "Processed eventNum = " << nPacketProcessed/(nCh) << endl;

		}//j

		if ((nPacketProcessed/(nCh)) >= nEvtToRead) break;

	}//while
	in.close();

	if ( mid==1 ){
		c2->cd(1);
	}else{
		c2->cd(2);
	}
	gPad->SetMargin(0.15,0.12,0.12,0.12);
	gPad->SetTicks();
	H2->GetXaxis()->SetRangeUser(0, trigN);
	H2->GetXaxis()->SetTitleSize(0.05);
	H2->GetXaxis()->SetLabelSize(0.045);
	H2->GetYaxis()->SetTitleSize(0.05);
	H2->GetYaxis()->SetLabelSize(0.045);
	H2->GetZaxis()->SetTitleSize(0.05);
	H2->GetZaxis()->SetLabelSize(0.045);
	H2->GetZaxis()->SetRangeUser(0, 1.5*nCh);
	H2->DrawCopy("colz");


}


//--------------------------------------------------------------------------------------------------------
void bic_daq_quickQA(int RunNo = 2080, int nEvtToRead = 10000, const char* inPath = "data_hodoscope_raw", int mid = 42)
{

	//gStyle->SetPalette(kRainBow);

	const char* inFile = Form("%s/bic_daq_%d_%i.dat", inPath, mid, RunNo);
	cout <<Form("Start quick QA by directly decoding %s...\n", inFile);

	// Cuts, Containers
	//----------------------------------------------------------
	const int nCh = 32;
	std::map<int, int> chMap = GetChMap();

	int nHit[nCh] = {0};

	TH1F* H1_pulse = new TH1F(Form("H1_pulse_%d",mid), "", 2500, 0, 2500);
	TCanvas* c1 = new TCanvas(Form("c1_%d",mid), Form("RUN %d, MID %d Channel QA", RunNo, mid), -1, 200*2*xCVS, 120*1.2*8*xCVS, 120*4*xCVS); c1->Divide(8, 4);
	gPad->SetMargin(0,0,0,0);
	for (int a=0; a<nCh; a++)
	{
		c1->cd(a+1);
		gPad->SetMargin(0.17,0.01,0.16,0.01);
		gPad->SetTicks();
		TH1F* H1Temp = new TH1F(Form("RUN %d, MID %d, CH%d", RunNo, mid, a+1), "", 2500, 0, 2500);
		H1Temp->GetXaxis()->SetRangeUser(0, 124);
		H1Temp->GetYaxis()->SetRangeUser(-10*wform_factor[mid], 5*wform_factor[mid]);
		H1Temp->GetXaxis()->SetTitle("Time bin");
		H1Temp->GetXaxis()->SetTitleSize(0.05*1.6);
		H1Temp->GetXaxis()->SetLabelSize(0.045*1.6);
		H1Temp->GetYaxis()->SetTitle("ADC");
		H1Temp->GetYaxis()->SetTitleSize(0.05*1.6);
		H1Temp->GetYaxis()->SetLabelSize(0.045*1.6);
		H1Temp->GetYaxis()->SetTitleOffset(1.2);
		H1Temp->DrawCopy();
	}

	int trigN = -1;
	unsigned long long trigT = -1;
	TH2F* H2 = new TH2F(Form("trig_nVSt_%d",mid), Form("RUN %d, MID %d; Trigger Number; Trigger Time", RunNo, mid), 10000,0,10000, 200,0,0.5*1.E10);

	#if 1 
	// Processing
	//----------------------------------------------------------

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
		unsigned long long tcb_trigger_time = (tcb_trigger_fine_time * 8) + (tcb_trigger_coarse_time * 1000);

		//int mid = ((int)header[15] & 0xFF);
		int channel = ((int)header[16] & 0xFF);

		//cout << tcb_trigger_number << " " << tcb_trigger_time << " "  << channel << " " << data_length << endl;

		if ( data_length > 1024 ){
			if ( channel==0 ){
				fread(data, 1, 256 - 32, fp);
			}else{
				fread(data, 1, 512 - 32, fp);
			}
			cout << "WARNNING! suspicious data length: " << data_length << ", ch: " << channel << endl;
			nPacketProcessed++;
			continue;
		}

		if ( fabs(tcb_trigger_number-trigN)>100 ){
			cout << "WARNNING! suspicious tcb trigger number: " << tcb_trigger_number << " " << trigN 
				<< ", ch: " << channel << endl;
			fread(data, 1, 512 - 32, fp);
			nPacketProcessed++;
			continue;
		}

		if ( trigN!=tcb_trigger_number ){
			if ( trigT!=tcb_trigger_time ){
				trigN = tcb_trigger_number;
				trigT = tcb_trigger_time;
			}else{
				cout << "WARNNING! different trigger number but same trigger time!" << endl;
			}
		}

		H2->Fill(tcb_trigger_number, tcb_trigger_time);

		int wave_length = (mid==21) ? (data_length - 32) / 2 : (data_length - 32) / 4;

		//Read body, data_length - 32 bytes (header)
		//++++++++++++++++++++++++++++++++++++++++++++

		fread(data, 1, data_length - 32, fp);

		int charge[32] = {0};
		int timing[32] = {0};
		int hitFlag[32] = {0};
		int adc[wave_length];
		for (int a=0; a<wave_length; a++) adc[a] = 0;

		if (channel == 0) //Spectrum data
		{
			/*
			for (int ch=0; ch<32; ch++)
			{
				for (int a=0; a<3; a++) charge[ch] += ((int)(data[6 * ch + a + 0] & 0xFF) << 8*a);
				for (int a=0; a<2; a++) timing[ch] += ((int)(data[6 * ch + a + 3] & 0xFF) << 8*a);
				hitFlag[ch] = data[6 * ch + 5] & 0xFF;
			}//a
			*/
		}
		else //Waveform data
		{
			H1_pulse->Reset();
			bool validPulse = false;
			if ( mid==21 ){
				for (int a=0; a<wave_length; a++)
				{
					int t_adc1 = (data[2 * a + 0] & 0xFF);
					int t_adc2 = (data[2 * a + 1] & 0xFF) << 8;
					adc[a] = (short)(t_adc1 + t_adc2);

					if (fabs(adc[a]) > cutPed[mid]) validPulse = true;
					H1_pulse->SetBinContent(a+1, adc[a]);
				}//a
			}else{
				for (int a=0; a<wave_length; a++)
				{
					int t_adc1 = (data[4 * a + 0] & 0xFF);
					int t_adc2 = (data[4 * a + 1] & 0xFF) << 8;
					adc[a] = (short)(t_adc1 + t_adc2);

					if (fabs(adc[a]) > cutPed[mid]) validPulse = true;
					H1_pulse->SetBinContent(a+1, adc[a]);
				}
			}

			if (validPulse)
			{
				const int ch = (mid==21) ? chMap[channel] : channel;
				nHit[ch-1]++;
				c1->cd(ch);
				H1_pulse->DrawCopy("same");
			}
		}

		data_read = data_read + data_length;

		//++++++++++++++++++++++++++++++++++++++++++++

		nPacketProcessed++;
		if (nPacketProcessed%10000 == 0) cout << "Processed eventNum = " << nPacketProcessed/(nCh+1) << endl;
		if ((nPacketProcessed/(nCh+1)) == nEvtToRead) break;

	}
	#endif

	fclose(fp);

	//Draw
	TH1D *H1_rate = new TH1D("", Form("RUN %d, MID %d QA", RunNo, mid), nCh, 0, nCh);
	for (int a=0; a<nCh; a++){
		c1->cd(a+1);

		TLegend *leg = new TLegend(0.2, 0.95-0.08*2, 0.5, 0.95);
		leg->SetFillStyle(0);
		leg->SetBorderSize(0);
		leg->SetTextSize(0.045*1.6);
		leg->AddEntry("",Form("RUN %d, CH%d", RunNo, a),"h"); 
		leg->AddEntry("",Form("Hit rate %d / %d = %1.4f",nHit[a],trigN,nHit[a]*1.0/trigN),"h");

		H1_rate->SetBinContent(a+1, nHit[a]*1.0/trigN);
		leg->Draw();
	}

	TH1D *H2_projx = (TH1D*)H2->ProjectionX("H2_projx");
	if ( bDEBUG ){
		ctmp->cd();
		H2_projx->Draw();
	}

	if ( mid==42 ){
		c2->cd(4);
	}else{
		c2->cd(3);
	}
	gPad->SetMargin(0.15,0.12,0.12,0.12);
	gPad->SetTicks();
	H2->GetXaxis()->SetRangeUser(0, trigN);
	H2->GetXaxis()->SetTitleSize(0.05);
	H2->GetXaxis()->SetLabelSize(0.045);
	H2->GetYaxis()->SetTitleSize(0.05);
	H2->GetYaxis()->SetLabelSize(0.045);
	H2->GetZaxis()->SetTitleSize(0.05);
	H2->GetZaxis()->SetLabelSize(0.045);
	H2->GetZaxis()->SetRangeUser(0, 1.5*nCh);
	H2->DrawCopy("colz");

	int trigN_GOOD = 0;
	for (int ii=0; ii<trigN*0.9; ii++){
		if ( H2_projx->GetBinContent(ii+1)==(nCh+1) ){
			trigN_GOOD++;
		}
	}

	TLegend *leg = new TLegend(0.15, 0.85-0.06*2, 0.5, 0.85);
	leg->SetFillStyle(0);
	leg->SetBorderSize(0);
	leg->SetTextSize(0.045);
	leg->AddEntry("",Form("Good events %d / %d = %4.1f%%",trigN_GOOD,int(trigN*0.9+1),trigN_GOOD*100/(trigN*0.9+1)),"h");
	if ( trigN_GOOD/(trigN*0.9+1)<0.9 ){
		leg->AddEntry("","Bad RUN!","h");
	}else{
		leg->AddEntry("","Good RUN!","h");
	}
	leg->Draw();

	if ( mid==42 ){
		c2->cd(5+4);
	}else{
		c2->cd(5+3);
	}
	gPad->SetTicks();
	gPad->SetMargin(0.14,0.01,0.12,0.12);
	H1_rate->GetYaxis()->SetRangeUser(0, 1.5*H1_rate->GetMaximum());
	H1_rate->GetXaxis()->SetTitle("CH Index");
	H1_rate->GetXaxis()->SetTitleSize(0.05);
	H1_rate->GetXaxis()->SetLabelSize(0.045);
	H1_rate->GetYaxis()->SetTitle("HIT Rate");
	H1_rate->GetYaxis()->SetTitleSize(0.05);
	H1_rate->GetYaxis()->SetLabelSize(0.045);
	H1_rate->GetZaxis()->SetTitleSize(0.05);
	H1_rate->GetZaxis()->SetLabelSize(0.045);
	H1_rate->DrawCopy("HIST");

}

