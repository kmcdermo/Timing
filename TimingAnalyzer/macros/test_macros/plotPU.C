TH1F * prepHist(TLegend * leg, const TString & histname, const TString & title, const TString & xtitle, const TString & ytitle,
		const Bool_t doScale, TFile * file, const Color_t color, const TString & label)
{
  auto hist = (TH1F*)file->Get(histname.Data());

  hist->SetLineColor(color);
  hist->SetMarkerColor(color);
  hist->SetName(label.Data());
  
  if (doScale) hist->Scale(1.0/hist->Integral());

  hist->SetTitle(title.Data());
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());

//   const TString mean = Form("#mu: %5.2f#pm%4.2f",hist->GetMean(),hist->GetMeanError());
//   const TString stddev = Form("#sigma: %5.2f#pm%4.2f",hist->GetStdDev(),hist->GetStdDevError());
  const TString mean = Form("#mu: %5.2f",hist->GetMean());
  const TString stddev = Form("#sigma: %5.2f",hist->GetStdDev());
  leg->AddEntry(hist,label+", "+mean+", "+stddev,"epl");

  return hist;
}

void plotPU()
{
  gStyle->SetOptStat(0);

  const TString histname = "h_genputrue_sum";
  const TString title    = "Number of Gen PU [True]";
  const TString xtitle   = "Number of Gen PU [True]";
  const TString ytitle   = "Fraction of Events";
  const auto doScale = true;

//   const TString histname = "h_genputrue_PUWeightsHist";
//   const TString title    = "Gen PU [True] Weight";
//   const TString xtitle   = "Number of Gen PU [True]";
//   const TString ytitle   = "PU Weight";
//   const auto doScale = false;

  auto file_ttjets = TFile::Open("ttjets_pu.root");
  auto file_dyll   = TFile::Open("dyll_pu.root");

  auto canv = new TCanvas();
  canv->cd();
  canv->SetTickx();
  canv->SetTicky();
  canv->SetLogy();

  auto leg = new TLegend(0.14,0.14,0.6,0.24);
  leg->SetLineColor(0);

  auto hist_ttjets = prepHist(leg,histname,title,xtitle,ytitle,doScale,file_ttjets,kBlue,"TTJets");
  auto hist_dyll   = prepHist(leg,histname,title,xtitle,ytitle,doScale,file_dyll,  kRed, "DY#rightarrowLL");

  canv->cd();
  hist_ttjets->Draw("ep");
  hist_dyll  ->Draw("ep same");
  leg->Draw("same");

  canv->SaveAs(histname+".pdf");
}
