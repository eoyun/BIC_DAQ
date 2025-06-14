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

void time_diff_hodo(const int RunNo=60051)
{
    const long dt_max = 5e8;
    const long n_dtbin = 400;

	TFile* FCalor  = new TFile( Form( "./%i_31.root", RunNo) );
	TTree* TCalor = (TTree*)FCalor -> Get("MID31");
 	const long nCEvents = (long)TCalor->GetEntries();
	cout << "Number of Calorimeter events: " << nCEvents << endl;

	UInt_t CFineTime; //FADC Data starting time (ns)
	TCalor -> SetBranchAddress("finetime",    &CFineTime);
	ULong64_t CCoarseTime; //FADC Trigger time (ns)
	TCalor -> SetBranchAddress("coarsetime",    &CCoarseTime);

    //Define histogram & graphs
    TGraph* GCtime = new TGraph( nCEvents );
    TGraph* GCdt = new TGraph( nCEvents );
	

    //trigger times of first event are defined as 0
    TCalor -> GetEntry(0);
    ULong64_t CTime_Start = (ULong64_t)CFineTime*8 + CCoarseTime*1000;
	ULong64_t CTime_before = CTime_Start;

    for( int iEv = 1; iEv < nCEvents; iEv++ ){
        TCalor -> GetEntry( iEv );

        ULong64_t CTime = (ULong64_t)CFineTime*8 + CCoarseTime*1000 ;
		ULong64_t Cdt = CTime - CTime_before;
        GCtime -> SetPoint( iEv, iEv, CTime);
		GCdt -> SetPoint( iEv, iEv, Cdt);

		CTime_before = CTime;
	}

    //Drawing
    TCanvas* c1 = new TCanvas();
	c1 -> cd();
    GCtime -> SetMarkerStyle( 2 );
    GCtime -> SetMarkerColor( 2 );
	GCtime -> GetYaxis() -> SetRangeUser( 0, 2e10 );
    //GCtime -> GetXaxis() -> SetRangeUser( 0, 10000 );
	GCtime -> Draw("AP");
    return;
}
