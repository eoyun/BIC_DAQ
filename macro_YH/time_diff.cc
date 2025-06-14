#include "TCanvas.h"
#include "TFile.h"
#include "TGraph.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TStyle.h"
#include "TTree.h"

#include <iostream>
#include <fstream>
#include <numeric>
#include <vector>
using namespace std;

void time_diff(const int RunNo=60051)
{
    const long dt_max = 5e8;
    const long n_dtbin = 400;

	TFile* FAstroPix  = new TFile( Form( "./apix_2_41_%i.root", RunNo) );
	TTree* TAstroPix = (TTree*)FAstroPix -> Get("apix2");
	const long nAEvents = (long)TAstroPix->GetEntries();
 	const long nAEvents = 0;
	cout << "Number of AstroPix events: " << nAEvents << endl;

    //variables of tree
	UInt_t AFineTime; //FADC Data starting time (ns)
	TAstroPix -> SetBranchAddress("finetime",    &AFineTime);
	ULong64_t ACoarseTime; //FADC Trigger time (ns)
	TAstroPix -> SetBranchAddress("coarsetime",    &ACoarseTime);

	TFile* FCalor  = new TFile( Form( "./%i_31.root", RunNo) );
	TTree* TCalor = (TTree*)FCalor -> Get("MID31");
 	const long nCEvents = (long)TCalor->GetEntries();
	cout << "Number of Calorimeter events: " << nCEvents << endl;

	UInt_t CFineTime; //FADC Data starting time (ns)
	TCalor -> SetBranchAddress("finetime",    &CFineTime);
	ULong64_t CCoarseTime; //FADC Trigger time (ns)
	TCalor -> SetBranchAddress("coarsetime",    &CCoarseTime);

    //Define histogram & graphs

    TGraph* Atime = new TGraph( nAEvents );
    TGraph* Adt = new TGraph( nAEvents );
	TH2L* H2index_time = new TH2L( "H2index_time",  ";Trigger Number; Trigger Time", nAEvents, 0, nAEvents, 200, 0, 0.5*1.E10);
    TH1L* hist_dt = new TH1L( "dt", ";dt [ns];Counts", 100, 0, dt_max);

    TGraph* GCtime = new TGraph( nCEvents );
    TGraph* GCdt = new TGraph( nCEvents );
	
    TAstroPix -> GetEntry(0);
    ULong64_t Time_Start = (ULong64_t)AFineTime*8 + ACoarseTime*1000;
	ULong64_t Time_before = Time_Start;

    for( int iEv = 1; iEv < nAEvents; iEv++ ){
        TAstroPix -> GetEntry( iEv );

        ULong64_t Time = (ULong64_t)AFineTime*8 + ACoarseTime*1000 - Time_Start;
		ULong64_t dt = Time - Time_before;
		H2index_time -> Fill( iEv, Time);
        Atime -> SetPoint( iEv, iEv, Time);
		Adt -> SetPoint( iEv, iEv, dt);
        hist_dt-> Fill( dt );

		Time_before = Time;
    }


    //trigger times of first event are defined as 0
    TCalor -> GetEntry(0);
    ULong64_t CTime_Start = (ULong64_t)CFineTime*8 + CCoarseTime*1000;
	ULong64_t CTime_before = CTime_Start;

    for( int iEv = 1; iEv < nCEvents; iEv++ ){
        TCalor -> GetEntry( iEv );

        ULong64_t CTime = (ULong64_t)CFineTime*8 + CCoarseTime*1000 - CTime_Start;
		ULong64_t Cdt = CTime - CTime_before;
        GCtime -> SetPoint( iEv, iEv, CTime);
		GCdt -> SetPoint( iEv, iEv, Cdt);

		CTime_before = CTime;
	}

    //Drawing
    TCanvas* c1 = new TCanvas();
	c1 -> cd();
    Atime -> SetMarkerStyle( 2 );
    Atime -> SetMarkerColor( 4 );
	Atime -> GetYaxis() -> SetRangeUser( 0, 2e10 );
    //Atime -> GetXaxis() -> SetRangeUser( 0, 10000 );
	Atime -> Draw("AP");

    GCtime -> SetMarkerStyle( 2 );
    GCtime -> SetMarkerColor( 2 );
	GCtime -> GetYaxis() -> SetRangeUser( 0, 2e10 );
    //GCtime -> GetXaxis() -> SetRangeUser( 0, 10000 );
	GCtime -> Draw("SAME P");

	//H2index_time -> DrawCopy("COLZ");

    TCanvas* c2 = new TCanvas();
	c2 -> cd();
    Adt -> SetMarkerStyle( 2 );
    Adt -> SetMarkerColor( 4 );
	//Adt -> GetXaxis() -> SetRangeUser( 0, 1e8 );
    Adt -> GetYaxis() -> SetTitle( "dt [ns]" );
    Adt -> GetXaxis() -> SetTitle( "iEvent" );
	Adt -> GetYaxis() -> SetRangeUser( 0, dt_max );
	Adt -> Draw("AP");
	//Atime -> Draw( "AP" );

    TLegend* Legend = new TLegend( 0.74, 0.65, 0.99, 0.87 );
    Legend -> SetFillStyle(0);
    Legend -> SetBorderSize(0);
    Legend -> SetTextSize(0.04);
    Legend -> Draw();

    TCanvas* canvas_dt = new TCanvas("cdt", "cdt", 600, 600);
    canvas_dt -> cd();

    hist_dt -> SetLineColor(2);
    hist_dt -> Draw("SAME HIST");

    return;
}
