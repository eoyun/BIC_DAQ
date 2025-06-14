#include <stdio.h>

void plot_data(void)
{
  char filename[100];
  FILE *fp;
  int file_size;
  TH1F **wave;
  TH1F **tdc;
  TH1F **spect;
  char plot_name[10];
  char plot_title[10];
  int first_waveform;
  int data_read;
  char header[32];
  char data[2048];
  int data_length;
  int run_number;
  int trigger_type;
  int tcb_trigger_number;
  long long tcb_trigger_fine_time;
  long long tcb_trigger_coarse_time;
  long long tcb_trigger_time;
  int mid;
  int channel;
  int local_trigger_number;
  int local_trigger_pattern;
  long long local_gate_fine_time;
  long long local_gate_coarse_time;
  long long local_gate_time;
  int itmp;
  long long ltmp;
  int ch;
  int charge[32];
  int timing[32];
  int hit_flag[32];
  int wave_length;
  int tdc_length;
  short adc;
  short tdc_val;
  int i;
  int CH;
  TH1F **plot;
  printf("channel?\n");
  scanf("%d",&CH);  
  // filename here
  sprintf(filename, "../data/bic_daq_41_1065.dat");
  
  // get data file size, size should be less than 2 Gbytes
  fp = fopen(filename, "rb");
  fseek(fp, 0L, SEEK_END);
  file_size = ftell(fp);
  fclose(fp);
  
//file_size = 10000;
  
  // define some histograms
  TCanvas *c1 = new TCanvas("c1", "Waveform", 1600, 800);
  TCanvas *c2 = new TCanvas("c2", "Spectrum", 1600, 800);
  //c1->Divide(8, 8);
  c1->Divide(2,1);
  c2->Divide(8, 4);
  wave = new TH1F *[32];
  tdc = new TH1F *[32];
  spect = new TH1F *[32];
  for (ch = 0; ch < 32; ch++) {
    sprintf(plot_name, "spect%d", ch);
    sprintf(plot_title, "ch %d", ch + 1);
    spect[ch] = new TH1F(plot_name, plot_title, 4096, 0, 4096 * 16); // adjsut range by peak sum width
    spect[ch]->Reset();
  }
  
  // open data file  
  fp = fopen(filename, "rb");
  
  data_read = 0;
  first_waveform = 1;
  
  while (data_read < file_size) {
    // read header
    fread(header, 1, 32, fp);
    
    // get header information
    data_length = header[0] & 0xFF;
    itmp = header[1] & 0xFF;
    itmp = itmp << 8;
    data_length = data_length + itmp;
    itmp = header[2] & 0xFF;
    itmp = itmp << 16;
    data_length = data_length + itmp;
    itmp = header[3] & 0xFF;
    itmp = itmp << 24;
    data_length = data_length + itmp;

    run_number = header[4] & 0xFF;
    itmp = header[5] & 0xFF;
    itmp = itmp << 8;
    run_number = run_number + itmp;

    trigger_type = header[6] & 0xFF;

    tcb_trigger_number = header[7] & 0xFF;
    itmp = header[8] & 0xFF;
    itmp = itmp << 8;
    tcb_trigger_number = tcb_trigger_number + itmp;
    itmp = header[9] & 0xFF;
    itmp = itmp << 16;
    tcb_trigger_number = tcb_trigger_number + itmp;
    itmp = header[10] & 0xFF;
    itmp = itmp << 24;
    tcb_trigger_number = tcb_trigger_number + itmp;

    tcb_trigger_fine_time = header[11] & 0xFF;
    
    tcb_trigger_coarse_time = header[12] & 0xFF;
    ltmp = header[13] & 0xFF;
    ltmp = ltmp << 8;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + itmp;
    ltmp = header[14] & 0xFF;
    ltmp = ltmp << 16;
    tcb_trigger_coarse_time = tcb_trigger_coarse_time + itmp;
    
    tcb_trigger_time = tcb_trigger_coarse_time * 1000 + tcb_trigger_fine_time * 8;

    mid = header[15] & 0xFF;

    channel = header[16] & 0xFF;

    local_trigger_number = header[17] & 0xFF;
    itmp = header[18] & 0xFF;
    itmp = itmp << 8;
    local_trigger_number = local_trigger_number + itmp;
    itmp = header[19] & 0xFF;
    itmp = itmp << 16;
    local_trigger_number = local_trigger_number + itmp;
    itmp = header[20] & 0xFF;
    itmp = itmp << 24;
    local_trigger_number = local_trigger_number + itmp;

    local_trigger_pattern = header[21] & 0xFF;
    itmp = header[22] & 0xFF;
    itmp = itmp << 8;
    local_trigger_pattern = local_trigger_pattern + itmp;
    itmp = header[23] & 0xFF;
    itmp = itmp << 16;
    local_trigger_pattern = local_trigger_pattern + itmp;
    itmp = header[24] & 0xFF;
    itmp = itmp << 24;
    local_trigger_pattern = local_trigger_pattern + itmp;

    local_gate_fine_time = header[25] & 0xFF;
    
    local_gate_coarse_time = header[26] & 0xFF;
    ltmp = header[27] & 0xFF;
    ltmp = ltmp << 8;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[28] & 0xFF;
    ltmp = ltmp << 16;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[29] & 0xFF;
    ltmp = ltmp << 24;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[30] & 0xFF;
    ltmp = ltmp << 32;
    local_gate_coarse_time = local_gate_coarse_time + itmp;
    ltmp = header[31] & 0xFF;
    ltmp = ltmp << 40;
    local_gate_coarse_time = local_gate_coarse_time + itmp;

    local_gate_time = local_gate_coarse_time * 1000 + local_gate_fine_time * 8;

    // read body, data_length - 32(header) bytes
    if (channel == 0) fread(data, 1, data_length - 32, fp);
    else fread(data, 1, data_length - 32, fp);
    // for fast(spectrum) data
    if (channel == 0) {
      // get fast data
      for (ch = 1; ch <= 32; ch++) {
        charge[ch - 1] = data[6 * (ch - 1)] & 0xFF;
        itmp = data[6 * (ch - 1) + 1] & 0xFF;
        itmp = itmp << 8;
        charge[ch - 1] = charge[ch - 1] + itmp;
        itmp = data[6 * (ch - 1) + 2] & 0xFF;
        itmp = itmp << 16;
        charge[ch - 1] = charge[ch - 1] + itmp;
        
        timing[ch - 1] = data[6 * (ch - 1) + 3] & 0xFF;
        itmp = data[6 * (ch - 1) + 4] & 0xFF;
        itmp = itmp << 8;
        timing[ch - 1] = timing[ch - 1] + itmp;
        
        hit_flag[ch - 1] = data[6 * (ch - 1) + 5] & 0xFF;
        
        // fill charge to spectrum
        spect[ch - 1]->Fill(charge[ch - 1]);
      }
    }
    if (channel != CH) continue;
    // for waveform data
    else {
      // for the first waveform data, define histogram range
      if (first_waveform) {
        wave_length = (data_length - 32) / 4;
        tdc_length = (data_length - 32) / 4;
        for (ch = 0; ch < 32; ch++) {
          sprintf(plot_name, "wave%d", ch);
          sprintf(plot_title, "ch %d", ch + 1);
          wave[ch] = new TH1F(plot_name, plot_title, wave_length, 0, wave_length * 16); 
          wave[ch]->Reset();
          wave[ch]->SetStats(0);
          tdc[ch] = new TH1F(plot_name, plot_title, wave_length, 0, wave_length * 16); 
          tdc[ch]->Reset();
          tdc[ch]->SetStats(0);
        }
        first_waveform = 0;
      }

      // reset waveform
      wave[channel - 1]->Reset();
      tdc[channel - 1]->Reset();
      
      // get waveform data
      for (i = 0; i < wave_length; i++) {
        adc = data[4 * i] & 0xFF;
        itmp = data[4 * i + 1] & 0xFF;
        itmp = itmp << 8;
        adc = adc + itmp;
	tdc_val = data[4 * i + 2] & 0xFF;
        itmp = data[4 * i + 3] & 0xFF;
        itmp = itmp << 8;
        tdc_val = tdc_val + itmp;
        // fill waveform
        wave[channel - 1]->Fill(i * 16, adc);
        tdc[channel - 1]->Fill(i * 16, tdc_val);
      }

      // show waveform
      //c1->cd(channel * 2 - 1);
      c1->cd(1);
      wave[channel - 1]->Draw("hist");
      //c1->cd(channel * 2);
      c1->cd(2);
      tdc[channel - 1]->Draw("hist");
      c1->Modified();
      c1->Update();
    }
        
    data_read = data_read + data_length;
    
    printf("continue?");
    scanf("%d", &i);
    if (i==0) break;    
  }
  
  // plot spectrum
  for (ch = 1; ch <= 32; ch++) {
    c2->cd(ch); 
    spect[ch - 1]->Draw();      
  }  
  c2->Modified();
  c2->Update();
  
  fclose(fp);
}


