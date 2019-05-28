void dumpInfo(TH2F * hist, const Float_t xlow, const Float_t xhigh)
{
  hist->GetXaxis()->SetRangeUser(xlow,xhigh);
  
  std::cout << xlow << " : " << xhigh << " nev: " << hist->Integral() << " CF: " << hist->GetCorrelationFactor() << std::endl;
}

void printCorrFactor()
{
  const TString filename = "met_vs_time_gjets_DEG.root";
  auto file = TFile::Open(filename.Data());
  auto hist = (TH2F*)file->Get("Data_Hist");
  
  dumpInfo(hist,-2.0,25);
  dumpInfo(hist, 1.0,25);
  dumpInfo(hist, 1.5,25);
}
