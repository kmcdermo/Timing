void copy()
{
  TFile * file1 = TFile::Open("data.root");
  file1->cd();
  TH1F * hist = (TH1F*)file1->Get("td_nvtx_EEEE_sigma_gaus2fm");

  TFile * file2 = TFile::Open("~/Timing/TimingAnalyzer/work/gaus2fm/DATA/doubleeg/plots.root","UPDATE");
  file2->cd();
  hist->Write(hist->GetName(),TObject::kWriteDelete);
}
