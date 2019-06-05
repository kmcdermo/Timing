#include "Common.cpp+"

void time_comp(const TString & corr, const Double_t min, const Double_t max);
TH1F * GetHist(TFile * file, const TString & name, const TString & histname, const Color_t color, 
	       const Double_t min, const Double_t max, TLegend * leg, const TString & label, const TString & opt);

void makeSingleTimeComp()
{
  auto tdrStyle = new TStyle();
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  time_comp("nocorrs",0,0.5);
  time_comp("shift",0,0.5);
  time_comp("smear",0,0.25);

  delete tdrStyle;
}

void time_comp(const TString & corr, const Double_t min, const Double_t max)
{
  // input files
  const auto zee_filename = "time_comp/zee_"+corr+".root";
  auto zee_file = TFile::Open(zee_filename.Data());
  Common::CheckValidFile(zee_file,zee_filename);

  const auto gjets_filename = "time_comp/gjets_"+corr+".root";
  auto gjets_file = TFile::Open(gjets_filename.Data());
  Common::CheckValidFile(gjets_file,gjets_filename);

  // add to legend
  auto leg = new TLegend(0.6,0.7,0.82,0.93);

  // get hists
  auto zee_data_hist = GetHist(zee_file,"Data_Hist","Data_Zee",kBlue,min,max,leg,"Z#rightarrowll Data","epl");
  auto zee_mc_hist   = GetHist(zee_file,"Bkgd_Hist","MC_Zee"  ,kBlue,min,max,leg,"Z#rightarrowll MC","f");

  auto gjets_data_hist = GetHist(gjets_file,"Data_Hist","Data_GJets",kRed,min,max,leg,"#gamma+jets Data","epl");
  auto gjets_mc_hist   = GetHist(gjets_file,"Bkgd_Hist","MC_GJets"  ,kRed,min,max,leg,"#gamma+jets MC","f");

  auto zee_mc_errs_hist   = (TH1F*)zee_mc_hist  ->Clone(Form("%s_Errs",zee_mc_hist  ->GetName()));
  auto gjets_mc_errs_hist = (TH1F*)gjets_mc_hist->Clone(Form("%s_Errs",gjets_mc_hist->GetName()));

  // make canvas
  auto canv = new TCanvas();
  canv->cd();
  
  // draw
  zee_data_hist  ->Draw("ep");
  gjets_data_hist->Draw("ep same");

  zee_mc_hist  ->Draw("hist same");
  gjets_mc_hist->Draw("hist same");

  // draw errors
  zee_mc_errs_hist  ->SetFillStyle(3254); 
  gjets_mc_errs_hist->SetFillStyle(3254); 

  zee_mc_errs_hist  ->Draw("E2 same");
  gjets_mc_errs_hist->Draw("E2 same");

  leg->Draw("same");

  // make canv
  Common::CMSLumi(canv,0,"Full");

  // save
  Common::SaveAs(canv,corr);

  // delete it all
  delete canv;
  delete gjets_mc_errs_hist;
  delete zee_mc_errs_hist;
  delete gjets_mc_hist;
  delete gjets_data_hist;
  delete zee_mc_hist;
  delete zee_data_hist;
  delete leg;
  delete gjets_file;
  delete zee_file;
}

TH1F * GetHist(TFile * file, const TString & name, const TString & histname, const Color_t color, 
	       const Double_t min, const Double_t max, TLegend * leg, const TString & label, const TString & opt)
{
  auto hist = (TH1F*)file->Get(name.Data());
  Common::CheckValidHist(hist,name,file->GetName());
  
  hist->SetName(histname.Data());
  hist->SetTitle("Photon Weighted Time [ns]");
  hist->GetXaxis()->SetTitle("Photon Weighted Time [ns]");
  hist->GetYaxis()->SetTitle("Fraction of Events");

  hist->Scale(1.f/hist->Integral());
  hist->GetYaxis()->SetRangeUser(min,max);
  hist->SetLineColor(color);

  if (name.Contains("Data"))
  {
    hist->SetMarkerColor(color);
  }
  else
  {
    hist->SetMarkerSize(0);
    hist->SetFillColorAlpha(color,0.35);
    hist->SetFillStyle(1001);
  }
  
  leg->AddEntry(hist,label.Data(),opt.Data());

  return hist;
}
