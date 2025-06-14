#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"

#include <bitset>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
using namespace std;

#define bDEBUG false 

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
void jbnu_daq_quickQA(int RunNo = 2080, int nEvtToRead = 10000, const char* inPath = "data_hodoscope_raw")
{

	//gStyle->SetPalette(kRainBow);

	TCanvas *ctmp;
	if ( bDEBUG ){
		ctmp = new TCanvas("ctmp", "ctmp", 600, 500);
	}

	const char* inFile = Form("%s/jbnu_daq_21_%i.dat", inPath, RunNo);
	cout <<Form("Start quick QA by directly decoding %s...\n", inFile);


	// Cuts, Containers
	//----------------------------------------------------------

	const int nCh = 32;
	const int cutPed = 50; //Select pulses has at least an entry larger than this value
	const float xCVS = 1.0; //Adjust canvas size
	std::map<int, int> chMap = GetChMap();

	int nHit[nCh] = {0};

	gStyle->SetOptStat(0);
	gStyle->SetTitleSize(0.04);
	TH1F* H1_pulse = new TH1F("H1_pulse", "", 2500, 0, 2500);
	TCanvas* c1 = new TCanvas("c1", Form("RUN %d, Hodoscope Channel QA", RunNo), -1, 0, 200*8*xCVS, 150*4*xCVS); c1->Divide(8, 4);
	gPad->SetMargin(0,0,0,0);
	for (int a=0; a<nCh; a++)
	{
		c1->cd(a+1);
		gPad->SetMargin(0.17,0.01,0.16,0.01);
		gPad->SetTicks();
		TH1F* H1Temp = new TH1F(Form("RUN %d, Hodoscope CH%d", RunNo, a+1), "", 2500, 0, 2500);
		H1Temp->GetXaxis()->SetRangeUser(0, 124);
		H1Temp->GetYaxis()->SetRangeUser(-100, 10000);
		H1Temp->GetXaxis()->SetTitle("Time bin");
		H1Temp->GetXaxis()->SetTitleSize(0.05*1.6);
		H1Temp->GetXaxis()->SetLabelSize(0.045*1.6);
		H1Temp->GetYaxis()->SetTitle("ADC");
		H1Temp->GetYaxis()->SetTitleSize(0.05*1.6);
		H1Temp->GetYaxis()->SetLabelSize(0.045*1.6);
		H1Temp->GetYaxis()->SetTitleOffset(1.2);
		H1Temp->DrawCopy();
	}

	unsigned long trigN = -1;
	unsigned long long trigT = -1;
	TH2F* H2 = new TH2F("trig_nVSt", Form("RUN %d, JBNU DAQ Trigger QA; Trigger Number; Trigger Time", RunNo), 10000,0,10000, 200,0,0.5*1.E10);

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
	char data[2048];
	int data_read = 0;

	//Open data file and read event by event
	fp = fopen(inFile, "rb");
	int nPacketProcessed=0;
	while (data_read < file_size)
	{
		//Read header
		//++++++++++++++++++++++++++++++++++++++++++++

		fread(header, 1, 32, fp);

		unsigned int data_length = 0;
		for (int a=0; a<4; a++) data_length += ((unsigned int)(header[a] & 0xFF) << 8*a);

		/*
		int run_number = 0;
		for (int a=0; a<2; a++) run_number += ((int)(header[a+4] & 0xFF) << 8*a);
		int trigger_type = ((int)header[6] & 0xFF);
		*/

		unsigned int tcb_trigger_number = 0;
		for (int a=0; a<4; a++) tcb_trigger_number += ((unsigned int)(header[a+7] & 0xFF) << 8*a);

		unsigned int tcb_trigger_fine_time = ((unsigned int)header[11] & 0xFF);
		unsigned int tcb_trigger_coarse_time = 0;
		for (int a=0; a<3; a++) tcb_trigger_coarse_time += ((unsigned int)(header[a+12] & 0xFF) << 8*a);
		unsigned long long tcb_trigger_time = (tcb_trigger_fine_time * 8) + (tcb_trigger_coarse_time * 1000);

		//int mid = ((int)header[15] & 0xFF);
		unsigned int channel = ((unsigned int)header[16] & 0xFF);

		//Fill trig # vs. trigger time
		if ((trigN != tcb_trigger_number) && fabs(trigT - tcb_trigger_time) > 5.E7)
		{
			trigN = tcb_trigger_number;
			trigT = tcb_trigger_time;
			H2->Fill(trigN, trigT);
		}

		/*
		int local_trigger_number = 0;
		for (int a=0; a<4; a++) local_trigger_number += ((int)(header[a+17] & 0xFF) << 8*a);
		int local_trigger_pattern = 0;
		for (int a=0; a<4; a++) local_trigger_pattern += ((int)(header[a+21] & 0xFF) << 8*a);

		long long local_gate_fine_time = ((long long)header[25] & 0xFF);
		long long local_gate_coarse_time = 0;
		for (int a=0; a<6; a++) local_gate_coarse_time += ((long long)(header[a+26] & 0xFF) << 8*a);
		long long local_gate_time = (local_gate_fine_time * 8) + (local_gate_coarse_time * 1000);
		*/

		int wave_length = (data_length - 32) / 2;

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
			for (int a=0; a<wave_length; a++)
			{
				int t_adc1 = (data[2 * a + 0] & 0xFF);
				int t_adc2 = (data[2 * a + 1] & 0xFF) << 8;
				adc[a] = (short)(t_adc1 + t_adc2);

				if (adc[a] > cutPed) validPulse = true;
				H1_pulse->SetBinContent(a+1, adc[a]);
			}//a

			if (validPulse)
			{
				const int ch = chMap[channel];
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
	}//while
	#endif

	//Draw

	TH1D *H1_rate = new TH1D("", Form("RUN %d, Hodoscope QA", RunNo), nCh, 0, nCh);
	for (int a=0; a<nCh; a++){
		c1->cd(a+1);

		TLegend *leg = new TLegend(0.2, 0.95-0.08*2, 0.5, 0.95);
		leg->SetFillStyle(0);
		leg->SetBorderSize(0);
		leg->SetTextSize(0.045*1.6);
		leg->AddEntry("",Form("RUN %d, CH%d", RunNo, a),"h"); 
		leg->AddEntry("",Form("Hit rate %d / %ld = %1.4f",nHit[a],trigN,nHit[a]*1.0/trigN),"h");

		H1_rate->SetBinContent(a+1, nHit[a]*1.0/trigN);
		leg->Draw();
	}

	TH1D *H2_projx = (TH1D*)H2->ProjectionX("H2_projx");
	if ( bDEBUG ){
		ctmp->cd();
		H2_projx->Draw();
	}

	TCanvas* c2 = new TCanvas("c2", Form("RUN %d, JBNU DAQ Trigger QA", RunNo), -1, 150*4*xCVS+55, 400*1.2*2*xCVS, 400*xCVS); 
	c2->Divide(2,1);
	c2->cd(1);
	gPad->SetMargin(0.15,0.12,0.12,0.12);
	gPad->SetTicks();
	H2->GetXaxis()->SetRangeUser(0, trigN);
	H2->GetXaxis()->SetTitleSize(0.05);;
	H2->GetXaxis()->SetLabelSize(0.045);;
	H2->GetYaxis()->SetTitleSize(0.05);;
	H2->GetYaxis()->SetLabelSize(0.045);;
	H2->GetZaxis()->SetTitleSize(0.05);;
	H2->GetZaxis()->SetLabelSize(0.045);;
	H2->DrawCopy("colz");

	long trigN_GOOD = 0;
	for (int ii=0; ii<trigN*0.9; ii++){
		if ( H2_projx->GetBinContent(ii+1)==(nCh+1) ){
			trigN_GOOD++;
		}
	}

	TLegend *leg = new TLegend(0.15, 0.85-0.06*2, 0.5, 0.85);
	leg->SetFillStyle(0);
	leg->SetBorderSize(0);
	leg->SetTextSize(0.045);
	leg->AddEntry("",Form("Good events %ld / %ld = %4.1f%%",trigN_GOOD,long(trigN*0.9+1),trigN_GOOD*100/(trigN*0.9+1)),"h");
	if ( trigN_GOOD/(trigN*0.9+1)<0.9 ){
		leg->AddEntry("","Bad RUN!","h");
	}else{
		leg->AddEntry("","Good RUN!","h");
	}
	leg->Draw();

	c2->cd(2);
	gPad->SetTicks();
	gPad->SetMargin(0.14,0.0,0.12,0.12);
	H1_rate->GetYaxis()->SetRangeUser(0, 0.15);
	H1_rate->GetXaxis()->SetTitle("CH Index");
	H1_rate->GetXaxis()->SetTitleSize(0.05);
	H1_rate->GetXaxis()->SetLabelSize(0.045);
	H1_rate->GetYaxis()->SetTitle("HIT Rate");
	H1_rate->GetYaxis()->SetTitleSize(0.05);
	H1_rate->GetYaxis()->SetLabelSize(0.045);
	H1_rate->GetZaxis()->SetTitleSize(0.05);
	H1_rate->GetZaxis()->SetLabelSize(0.045);
	H1_rate->DrawCopy("HIST");

	return;
}//Main
