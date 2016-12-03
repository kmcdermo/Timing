void eraoverplot()
{
  gStyle->SetOptStat(0);
  TVirtualFitter::SetDefaultFitter("Minuit2");
  
  TString var  = "td_effseedE";
  TString part = "EEEE";

  TString labelb = "runB";
  TString labelc = "runC";
  TString labeld = "runD";
  TString labele = "runE";
  TString labelf = "runF";
  TString labelg = "runG";
  TString labelh = "runH";

  Bool_t  isLogX = false;
  Bool_t  isLogY = false;
  
  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogX); 
  canv->SetLogy(isLogY);

  TFile * fileb = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labelb.Data()));
  TH1F * hb = (TH1F*)fileb->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  hb->SetMaximum(0.8);
  hb->SetMinimum(0.3);
  hb->SetLineColor(kRed);
  hb->SetMarkerColor(kRed);
  hb->SetMarkerStyle(1);

  TFile * filec = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labelc.Data()));
  TH1F * hc = (TH1F*)filec->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  hc->SetLineColor(kBlue);
  hc->SetMarkerColor(kBlue);
  hc->SetMarkerStyle(1);

  TFile * filed = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labeld.Data()));
  TH1F * hd = (TH1F*)filed->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  hd->SetLineColor(kGreen+1);
  hd->SetMarkerColor(kGreen+1);
  hd->SetMarkerStyle(1);

  TFile * filee = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labele.Data()));
  TH1F * he = (TH1F*)filee->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  he->SetLineColor(kViolet);
  he->SetMarkerColor(kViolet);
  he->SetMarkerStyle(1);

  TFile * filef = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labelf.Data()));
  TH1F * hf = (TH1F*)filef->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  hf->SetLineColor(kOrange);
  hf->SetMarkerColor(kOrange);
  hf->SetMarkerStyle(1);

  TFile * fileg = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labelg.Data()));
  TH1F * hg = (TH1F*)fileg->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  hg->SetLineColor(kCyan);
  hg->SetMarkerColor(kCyan);
  hg->SetMarkerStyle(1);

  TFile * fileh = TFile::Open(Form("%s/DATA/doubleeg/plots.root",labelh.Data()));
  TH1F * hh = (TH1F*)fileh->Get(Form("%s_%s_sigma_gaus1core",var.Data(),part.Data()));
  hh->SetLineColor(kPink-9);
  hh->SetMarkerColor(kPink-9);
  hh->SetMarkerStyle(1);
		     
  hb->Draw("ep");
  hc->Draw("ep SAME");
  hd->Draw("ep SAME");
  he->Draw("ep SAME");
  hf->Draw("ep SAME");
  hg->Draw("ep SAME");
  hh->Draw("ep SAME");

  TLegend * leg = new TLegend(0.15,0.7,0.35,0.85);
  leg->AddEntry(hb,labelb.Data(),"epl");
  leg->AddEntry(hc,labelc.Data(),"epl");
  leg->AddEntry(hd,labeld.Data(),"epl");
  leg->AddEntry(he,labele.Data(),"epl");
  leg->AddEntry(hf,labelf.Data(),"epl");
  leg->AddEntry(hg,labelg.Data(),"epl");
  leg->AddEntry(hh,labelh.Data(),"epl");
  leg->Draw("same");

  canv->SaveAs(Form("runeras_%s_%s.pdf", var.Data(), part.Data()));
}
