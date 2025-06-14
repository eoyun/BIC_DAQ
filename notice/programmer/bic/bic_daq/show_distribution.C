{
  FILE *fp;
  int ch;
  int i;
  int data;

  TCanvas *c1 = new TCanvas("c1", "KIMS", 800, 400);
  c1->Divide(2, 2);
  TH1F *tdc = new TH1F("tdc", "TDC calibration", 4096, 0, 4096);
  tdc->Reset();
  tdc->SetStats(0);

  fp = fopen("check_calib.txt", "rt");

  for (i = 0; i < 4096; i++) {
    fscanf(fp, "%d", &data);
    tdc->Fill(i, data);
  }

  tdc->Draw("hist");
  c1->Modified();
  c1->Update();
}
    
