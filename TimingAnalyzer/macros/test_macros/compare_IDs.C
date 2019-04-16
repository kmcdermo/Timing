void quick()
{
  auto file1 = new TFile("sig_eff.root");
  auto file2 = new TFile("sig_eff_nolepveto.root");

  auto hist10_1  = (TH1F*)file1->Get("GMSB_CTau10cm_Eff");
  auto hist200_1 = (TH1F*)file1->Get("GMSB_CTau200cm_Eff");

  auto hist10_2  = (TH1F*)file2->Get("GMSB_CTau10cm_Eff");
  hist10_2->SetLineColor(kAzure+10);
  hist10_2->SetMarkerColor(kAzure+10);

  auto hist200_2 = (TH1F*)file2->Get("GMSB_CTau200cm_Eff");
  hist200_2->SetLineColor(kMagenta);
  hist200_2->SetMarkerColor(kMagenta);
  
  auto leg = new TLegend(0.5,0.5,0.8,0.8);
  leg->AddEntry(hist10_1,"c#tau=10cm [W/ Lep Veto]","epl");
  leg->AddEntry(hist10_2,"c#tau=10cm [W/O Lep Veto]","epl");
  leg->AddEntry(hist200_1,"c#tau=200cm [W/ Lep Veto]","epl");
  leg->AddEntry(hist200_2,"c#tau=200cm [W/O Lep Veto]","epl");

  auto canv = new TCanvas();
  canv->cd();
  
  hist10_1->Draw("ep");
  hist10_2->Draw("ep same");
  hist200_1->Draw("ep same");
  hist200_2->Draw("ep same");
  leg->Draw("same");
}
