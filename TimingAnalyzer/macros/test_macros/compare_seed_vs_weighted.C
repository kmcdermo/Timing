#include "Common.cpp+"

void compare_seed_vs_weighted()
{
  auto tdrStyle = new TStyle();
  Common::SetTDRStyle(tdrStyle);
  
  Common::SetupEras();

  auto seedfile = TFile::Open("seedtime.root");  
  auto wgtdfile = TFile::Open("weightedtime.root");

  auto dataseed = (TH1F*)seedfile->Get("Data_Hist_Plotted");
  auto bkgdseed = (TH1F*)seedfile->Get("Bkgd_Hist");

  dataseed->SetLineColor(kRed);
  dataseed->SetMarkerColor(kRed);
  bkgdseed->SetLineColor(kBlue);
  bkgdseed->SetMarkerColor(kBlue);

  dataseed->Scale(1.f/dataseed->Integral());
  bkgdseed->Scale(1.f/bkgdseed->Integral());

  auto dataseedf = new TF1("dsf","gaus(0)",dataseed->GetMean()-2*dataseed->GetStdDev(),dataseed->GetMean()+2*dataseed->GetStdDev());
  dataseed->Fit(dataseedf->GetName());
  
  auto bkgdseedf = new TF1("dsf","gaus(0)",bkgdseed->GetMean()-2*bkgdseed->GetStdDev(),bkgdseed->GetMean()+2*bkgdseed->GetStdDev());
  bkgdseedf->SetLineColor(kBlue);
  bkgdseed->Fit(bkgdseedf->GetName());

  auto datawgtd = (TH1F*)wgtdfile->Get("Data_Hist_Plotted");
  auto bkgdwgtd = (TH1F*)wgtdfile->Get("Bkgd_Hist");

  datawgtd->SetLineColor(kRed);
  datawgtd->SetMarkerColor(kRed);
  bkgdwgtd->SetLineColor(kBlue);
  bkgdwgtd->SetMarkerColor(kBlue);

  datawgtd->SetLineStyle(9);
  bkgdwgtd->SetLineStyle(9);

  datawgtd->Scale(1.f/datawgtd->Integral());
  bkgdwgtd->Scale(1.f/bkgdwgtd->Integral());

  auto datawgtdf = new TF1("dsf","gaus(0)",datawgtd->GetMean()-2*datawgtd->GetStdDev(),datawgtd->GetMean()+2*datawgtd->GetStdDev());
  datawgtdf->SetLineStyle(9);
  datawgtd->Fit(datawgtdf->GetName());

  auto bkgdwgtdf = new TF1("dsf","gaus(0)",bkgdwgtd->GetMean()-2*bkgdwgtd->GetStdDev(),bkgdwgtd->GetMean()+2*bkgdwgtd->GetStdDev());
  bkgdwgtdf->SetLineColor(kBlue);
  bkgdwgtdf->SetLineStyle(9);
  bkgdwgtd->Fit(bkgdwgtdf->GetName());

  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  
  auto leg = new TLegend(0.6,0.6,0.8,0.8);
  leg->AddEntry(dataseed,"Data Seed Time","epl");
  leg->AddEntry(bkgdseed,"MC Seed Time","epl");
  leg->AddEntry(datawgtd,"Data Weighted Time","epl");
  leg->AddEntry(bkgdwgtd,"MC Weighted Time","epl");

  dataseed->GetYaxis()->SetRangeUser(1e-3,0.8);
  dataseed->Draw("ep");
  bkgdseed->Draw("ep same");
  datawgtd->Draw("ep same");
  bkgdwgtd->Draw("ep same");

  leg->Draw("same");

  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,"overplot");
}
