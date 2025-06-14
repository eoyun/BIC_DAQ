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

int GetBoardNumber(const char* inFile){
	ifstream in;
	in.open(inFile, std::ios::binary);
	if (!in.is_open()) { cout <<"GetDataLength - cannot open the file! Stop.\n"; return 1; }

	char data;
	in.read(&data, 1);
	int bid = data & 0xFF;

	return bid;
}

// Function to convert a hex string to a binary string
std::string hexToBinary(const std::string& hex) {
    std::string binary;
    for (char c : hex) {
        int value;
        if (c >= '0' && c <= '9') value = c - '0';
        else if (c >= 'a' && c <= 'f') value = 10 + c - 'a';
        else if (c >= 'A' && c <= 'F') value = 10 + c - 'A';
        else continue; // Skip invalid characters

        std::bitset<4> bits(value);
        binary += bits.to_string();
    }
    return binary;
}

void extractInfo(const std::string& binary, bool& isCol, UChar_t& index, UChar_t& timestamp, float& ToT) {
    //std::cout << binary << std::endl;

    isCol = binary[7] == '1';

    index = 0;
    for(UChar_t i=0; i<6; i++){
        UChar_t p = 1 << i;
        if(binary[i]=='1') index += p;
    }

    timestamp = 0;
    for(UChar_t i=8; i<16; i++){
        UChar_t p = 1 << (i-8);
        if(binary[i]=='1') timestamp += p;
    }

    int ToT_MSB = 0;
    for(int i=16; i<20; i++){
        int p = 1 << (i-16);
        if(binary[i]=='1') ToT_MSB += p;
    }

    int ToT_LSB = 0;
    for(int i=24; i<32; i++){
        int p = 1 << (i-24);
        if(binary[i]=='1') ToT_LSB += p;
    }

    long ToT_total  = (ToT_MSB << 8) + ToT_LSB;
    ToT = (ToT_total * SAMPLE_CLOCK_PERIOD_NS) / 1000.0;

}

int bicdaq_apix(int RunNo = 60051)
{
	int argc = 1;
	char** argv;
	//bool fWrite = false;
	bool fWrite = true;
	
	const int nChip = 2;
	const int Len = 64; //kbytes
	int MID = 41;
	///home/kobic/25KEKDATA/Run_1659/Run_1659_MID_41/apix_1_41_1659.dat

	TFile* F;
	TString FPath; FPath.Form("/home/kobic/25KEKDATA/Run_%d/Run_%d_MID_%d/", RunNo, RunNo, MID);

	//ROOT Tree
	//-------------------------------------------
	ifstream in;
	char data[Len];
	for(int i=0; i<nChip; i++){
		TString FName; FName.Form("apix_%d_%d_%d", i+1, MID, RunNo);
		cout << endl << endl << "AstroPix #" << i+1 << endl;
		if(fWrite){
			F = new TFile(FName+".root", "recreate");
		}
		TTree* T = new TTree(Form("apix%i", i+1), Form("apix%i", i+1));
		int BID = GetBoardNumber( FPath+FName+".dat" );

		UInt_t fBoardID;
		UInt_t fFineTime;
		ULong_t fCoarseTime;
		char fHexData[81]; //40 bytes
		int Cycle = 0;
		ULong_t tCoarseTimeBefore;

		UChar_t fHit;
		UChar_t fRow[20];
		UChar_t fCol[20];
		float fToT[20];


		T->Branch("bid",      	&fBoardID, 	"bid/i");
		T->Branch("finetime", 	&fFineTime,	"finetime/i");
		T->Branch("coarsetime", &fCoarseTime, "coarsetime/l");
		T->Branch("hexdata",	fHexData, "hexdata[81]/C");

		T->Branch("nhit", &fHit, "nhit/b");
		T->Branch("irow", fRow, "irow[nhit]/b");
		T->Branch("icol", fCol, "icol[nhit]/b");
		T->Branch("tot", fToT, "tot[nhit]/F");

		/*
		T->Branch("nhit",		&fHit,	"ihit/b");
		T->Branch("irow",		fRow,	"irow[20]/b");
		T->Branch("icol",		fCol,	"icol[20]/b");
		T->Branch("tot",		fToT,	"tot[20]/F");
		*/

		in.open( FPath+FName+".dat", std::ios::binary );
		if (!in.is_open()) { cout << FPath+FName << endl;  cout <<"Cannot open the file! Stop.\n"; return 1; }

		unsigned long nLine = 0;
		while (in.peek() != EOF)
		{
			in.read(data, Len);
			if (!in.good())	{ cout <<"Data file is currupted! Stop.\n"; break; }

			fBoardID = data[0] & 0xFF;
			if (fBoardID != BID){ cout <<"Data file is currupted! Stop.\n"; break; }

			fFineTime = data[1] & 0xFF;

			int tCoarseTime = 0;
			for (int a=0; a<3; a++) tCoarseTime += ( (int)(data[2+a] & 0xFF) << 8*a );
			if(tCoarseTimeBefore - tCoarseTime > 16e6) Cycle++;
			fCoarseTime = tCoarseTime * Cycle;
			tCoarseTimeBefore = tCoarseTime;

			stringstream hexStream;
			int offset = 8;
            for (int j = 0; j < Len-8; j++) {

				hexStream << hex << setw(2) << setfill('0') << (int)(data[j+8] & 0xFF);

            }
            string hexString = hexStream.str();
			string::size_type sPos;
			/*
			while ((sPos = hexString.find("bc")) != string::npos) {
    			hexString.erase(sPos, 2);
			}
			while ((sPos = hexString.find("0000")) != string::npos) {
    			hexString.erase(sPos, 2);
			}
			*/
				
			if(nLine < 30){
				//cout << (UInt_t)data[2] << ',' << (UInt_t)data[3] << ',' << (UInt_t)data[4] << endl;
				cout << "Ev #" << nLine <<", Fine = " << fFineTime << ", Coarse = " << tCoarseTime
					 << ", AstroPix" << i+1 <<" Data = " << hexString<< endl;
			}

			long dec_order=0;
			size_t pos = 0;

			vector<UChar_t> RowIndex;
			vector<UChar_t> RowTimestamp;
			vector<float>	RowToT;

			vector<UChar_t> ColIndex;
			vector<UChar_t> ColTimestamp;
			vector<float>	ColToT;
			while (pos < hexString.length()) {
				// Search for '20'
				if (pos + 2 <= hexString.length() && hexString.substr(pos, 2) == "20") {
					// Ensure there are at least 8 characters after '20'
					if (pos + 10 <= hexString.length()) {
						// Extract the 8 characters after '20'
						std::string hex = hexString.substr(pos + 2, 8);
						//cout << hex << endl;

						// Convert hex to binary
						std::string binary = hexToBinary(hex);

						// Extract isCol and index
						bool isCol;
						UChar_t index;
						UChar_t timestamp;
						float ToT;
						extractInfo(binary, isCol, index, timestamp, ToT);

						if (nLine < 30){

						// Output the results
						char ColRow = isCol ? 'c' : 'r';
						std::cout << "\tFound pattern at position " << pos
						        << ": location " << ColRow << (int)index << ", timestamp = " << (int)timestamp
						        << ", ToT = " << ToT << std::endl;
						}
						dec_order++;

						if(isCol){
							ColIndex.push_back( index );
							ColTimestamp.push_back( timestamp );
							ColToT.push_back( ToT );
						}else{
							RowIndex.push_back( index );
							RowTimestamp.push_back( timestamp );
							RowToT.push_back( ToT );
						}
					}
					pos += 8; // Move past '20'
				} else {
					pos++; // Move to the next character
				}
			}//inhexString loop

			fHit = 0;
			for(int c=0; c<ColIndex.size(); c++){
				for(int r=0; r<RowIndex.size(); r++){
					if(ColIndex[c] > 34) continue;
					if(RowIndex[r] > 34) continue;
					if(abs((int)ColTimestamp[c] - (int)RowTimestamp[r]) > 1) continue;
					if(ColToT[c] <= 0 or RowToT[r] <= 0) continue;
					if(fabs(ColToT[c] - RowToT[r]) / ColToT[c] > 0.1) continue;

					fRow[fHit] = RowIndex[r];
					fCol[fHit] = ColIndex[c];
					fToT[fHit] = (ColToT[c] + RowToT[r])/2.;
					fHit++;

				}
			}//Matching

            // 16진수 문자열을 fHexData에 복사
            strncpy(fHexData, hexString.c_str(), 80);
            fHexData[80] = '\0'; // 널 종료 문자 추가

			
			T->Fill();
			//if (nLine%100==0 and hexString.substr(0, 6) == "bcbc20")
			//if (nLine%100==0)
			if (nLine<30)
			{
		
				if( fHit == 0) {
					cout << "No matching hits" << endl << endl;
				}else{
					cout << "\t\tFound " << (int)fHit << " Hits" << endl;;
					for(int hit=0; hit<(int)fHit; hit++) cout << "\t\tr" << (int)fRow[hit] << "c" << (int)fCol[hit] << ", ToT = " << fToT[hit] << endl;
					cout << endl << endl;
				}
			}

			nLine++;
		}//while
		in.close();
	}//MID

	if(fWrite){
		F->Write();
		F->Close();
	}
	return 0;

}//Main



