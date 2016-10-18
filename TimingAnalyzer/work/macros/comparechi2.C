void comparechi2()
{
  gStyle->SetOptStat(0);

  TString label1 = "gaus1core";
  TString label2 = "gaus2fm";

  TString plotname = "td_effseedE_EEEE";

  Bool_t isMC = false;

  TFile * data1 = TFile::Open(Form("%s/%s/plots.root",label1.Data(),(isMC?"MC/dyll":"DATA/doubleeg")));
  TFile * data2 = TFile::Open(Form("%s/%s/plots.root",label2.Data(),(isMC?"MC/dyll":"DATA/doubleeg")));

  /////////
  // NDF //
  /////////

  TH1F * ndf1 = (TH1F*)data1->Get(Form("%s_chi2ndf_%s",plotname.Data(),label1.Data()));
  TH1F * ndf2 = (TH1F*)data2->Get(Form("%s_chi2ndf_%s",plotname.Data(),label2.Data()));
  
  TH1F * hndf1 = new TH1F("hndf1","Chi2/ndf",20,0.0,20.0);
  hndf1->SetLineColor(kRed);
  TH1F * hndf2 = new TH1F("hndf2","Chi2/ndf",20,0.0,20.0);
  hndf2->SetLineColor(kBlue);  

  for (UInt_t ibin = 1; ibin <= ndf1->GetNbinsX(); ibin++) { hndf1->Fill(ndf1->GetBinContent(ibin)); }
  for (UInt_t ibin = 1; ibin <= ndf2->GetNbinsX(); ibin++) { hndf2->Fill(ndf2->GetBinContent(ibin)); }

  TCanvas * cndf = new TCanvas();
  cndf->cd();

  Double_t maxndf = (hndf1->GetMaximum()>hndf2->GetMaximum()?hndf1->GetMaximum():hndf2->GetMaximum());
  hndf1->SetMaximum(maxndf+1);

  hndf1->Draw();
  hndf2->Draw("same");
  
  TLegend * lndf = new TLegend(0.75,0.85,1.0,1.0);
  lndf->AddEntry(hndf1,Form("%s - OF: %2.0f/%2.0f",label1.Data(),hndf1->GetBinContent(hndf1->GetNbinsX()+1),hndf1->Integral(1,hndf1->GetNbinsX()+1)),"l");
  lndf->AddEntry(hndf2,Form("%s - OF: %2.0f/%2.0f",label2.Data(),hndf2->GetBinContent(hndf2->GetNbinsX()+1),hndf2->Integral(1,hndf2->GetNbinsX()+1)),"l");
  lndf->Draw("same");

  cndf->SaveAs(Form("chi2/ndf_%s_%s_vs_%s.png",plotname.Data(),label1.Data(),label2.Data()));

  //////////
  // Prob //
  //////////

  TH1F * prob1 = (TH1F*)data1->Get(Form("%s_chi2prob_%s",plotname.Data(),label1.Data()));
  TH1F * prob2 = (TH1F*)data2->Get(Form("%s_chi2prob_%s",plotname.Data(),label2.Data()));
  
  TH1F * hprob1 = new TH1F("hprob1","Chi2prob",20,0.0,1.0);
  hprob1->SetLineColor(kRed);
  TH1F * hprob2 = new TH1F("hprob2","Chi2prob",20,0.0,1.0);
  hprob2->SetLineColor(kBlue);  

  for (UInt_t ibin = 1; ibin <= prob1->GetNbinsX(); ibin++) { hprob1->Fill(prob1->GetBinContent(ibin)); }
  for (UInt_t ibin = 1; ibin <= prob2->GetNbinsX(); ibin++) { hprob2->Fill(prob2->GetBinContent(ibin)); }

  TCanvas * cprob = new TCanvas();
  cprob->cd();

  Double_t maxprob = (hprob1->GetMaximum()>hprob2->GetMaximum()?hprob1->GetMaximum():hprob2->GetMaximum());
  hprob1->SetMaximum(maxprob+1);

  hprob1->Draw();
  hprob2->Draw("same");

  TLegend * lprob = new TLegend(0.75,0.85,1.0,1.0);
  lprob->AddEntry(hprob1,Form("%s - OF: %2.0f/%2.0f",label1.Data(),hprob1->GetBinContent(hprob1->GetNbinsX()+1),hprob1->Integral(1,hprob1->GetNbinsX()+1)),"l");
  lprob->AddEntry(hprob2,Form("%s - OF: %2.0f/%2.0f",label2.Data(),hprob2->GetBinContent(hprob2->GetNbinsX()+1),hprob2->Integral(1,hprob2->GetNbinsX()+1)),"l");
  lprob->Draw("same");

  cprob->SaveAs(Form("chi2/prob_%s_%s_vs_%s.png",plotname.Data(),label1.Data(),label2.Data()));
}
