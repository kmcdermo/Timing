void showmean()
{
  TFile * file = TFile::Open("config/ecalmaps2015.root");
  TH1F * hist = (TH1F*)file->Get("hist1D_total_x12_eem"); 

  std::cout << hist->GetMean() << std::endl;
  
  hist->Draw();
}
