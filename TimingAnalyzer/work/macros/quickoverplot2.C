inline Float_t rad2 (Float_t x, Float_t y){return x*x + y*y;}

void doFit(TH1F *& hist, TF1 *& fit, Float_t & mean, Float_t & emean, Float_t & sigma, Float_t & esigma) 
{
  Float_t fitrange = 3.0;
  TF1 * tempfit = new TF1("temp","gaus(0)",-fitrange,fitrange);
  tempfit->SetParLimits(2,0,10);
  hist->Fit("temp","RQ0B");
  const Float_t tempp0 = tempfit->GetParameter(0); // constant
  const Float_t tempp1 = tempfit->GetParameter(1); // mean
  const Float_t tempp2 = tempfit->GetParameter(2); // sigma
  delete tempfit;

  TString formname = "gaus2fm";
  TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
  fit = new TF1(Form("%s_fit",hist->GetName()),formname.Data(),-fitrange,fitrange);
  fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,tempp2*4);
  fit->SetParLimits(2,0,10);
  fit->SetParLimits(4,0,10);
  
  Int_t status = hist->Fit(fit->GetName(),"RBQ0");
  
  const Float_t const1 = fit->GetParameter(0); 
  const Float_t const2 = fit->GetParameter(3);
  const Float_t denom =  const1 + const2;
  
  mean   = fit->GetParameter(1);
  sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4))/denom;
  
  emean  = fit->GetParError(1);
  esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4));
  
  esigma = std::sqrt(esigma)/denom;
}

void quickoverplot2() {
  gStyle->SetOptStat(0);
  //  gStyle->SetOptFit(0);
  TVirtualFitter::SetDefaultFitter("Minuit2");

  TString label1 = "seedtime";
  TString label2 = "weighted";

  Bool_t  isLogY = false;
  
  TFile * file = TFile::Open("outtimes.root");

  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogY);

  TH1F * h1 = (TH1F*)file->Get("h_seedtimeTOF");
  h1->SetLineColor(kRed);
  h1->SetMarkerColor(kRed);
  h1->SetMarkerStyle(1);
  h1->SetTitle("Seed Time (TOF) vs Weighted Time");
  h1->GetXaxis()->SetTitle("Time [ns]");
  h1->GetYaxis()->SetTitle("Events");

  Float_t m1, em1, s1, es1;
  TF1 * f1;
  doFit(h1,f1,m1,em1,s1,es1);
  f1->SetLineColor(kRed);
  f1->SetLineWidth(0.5);

  TH1F * h2 = (TH1F*)file->Get("h_weighttime");
  h2->SetLineColor(kBlue);
  h2->SetMarkerColor(kBlue);
  h2->SetMarkerStyle(1);

  Float_t m2, em2, s2, es2;
  TF1 * f2;
  doFit(h2,f2,m2,em2,s2,es2);
  f2->SetLineColor(kBlue);
  f2->SetLineWidth(0.5);

  h1->Draw("epl");
  f1->Draw("same");
  h2->Draw("epl SAME");
  f2->Draw("same");

  TLegend * leg = new TLegend(0.8,0.8,0.95,0.95);
  leg->AddEntry(h1,label1.Data(),"epl");
  leg->AddEntry(h2,label2.Data(),"epl");
  leg->Draw("same");

  TPaveText * text = new TPaveText(0.6,0.68,0.9,0.78,"NDC");
  text->AddText(Form("#mu_{s} = %f #pm %f, #sigma_{s} = %f #pm %f",m1,em1,s1,es1));
  text->AddText(Form("#mu_{w} = %f #pm %f, #sigma_{w} = %f #pm %f",m2,em2,s2,es2));
  text->SetTextSize(0.03);
  text->Draw("same");
  
  canv->SaveAs(Form("%s_vs_%s_%s.png", label1.Data(), label2.Data(), (isLogY?"log":"lin") ));
}
