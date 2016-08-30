static const Float_t fitrange = 3.0;
static const TString formname = "gaus2fm";

inline Float_t rad2 (Float_t x, Float_t y){return x*x + y*y;}

void doFit(TH1F *& hist, TF1 *& fit)
{
  TF1 * tempfit = new TF1("temp","gaus(0)",-fitrange,fitrange);
  tempfit->SetParLimits(2,0,10);
  hist->Fit("temp","R0B");
  const Float_t tempp0 = tempfit->GetParameter(0); // constant
  const Float_t tempp1 = tempfit->GetParameter(1); // mean
  const Float_t tempp2 = tempfit->GetParameter(2); // sigma
  delete tempfit;

  if (formname.EqualTo("gaus1",TString::kExact)) {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(Form("%s_fit",formname.Data()),formname.Data(),-fitrange,fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2);
  }
  else if (formname.EqualTo("gaus2",TString::kExact)) {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[4])/[5])**2)");
    fit  = new TF1(Form("%s_fit",formname.Data()),formname.Data(),-fitrange,fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,0,tempp2*4);
    fit->SetParLimits(2,0,10);
    fit->SetParLimits(5,0,10);
  }
  else if (formname.EqualTo("gaus2fm",TString::kExact)) {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(Form("%s_fit",formname.Data()),formname.Data(),-fitrange,fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,tempp2*4);
    fit->SetParLimits(2,0,10);
    fit->SetParLimits(4,0,10);
  }
  else if (formname.EqualTo("gaus3fm",TString::kExact)) {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)+[5]*exp(-0.5*((x-[1])/[6])**2)");
    fit  = new TF1(Form("%s_fit",formname.Data()),formname.Data(),-fitrange,fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/2,tempp2*5,tempp0/2,tempp2*5);
    fit->SetParLimits(0,0.05,0.1);
    fit->SetParLimits(3,0.02,0.05);
    fit->SetParLimits(5,0.02,0.05);
  }

  Int_t status = hist->Fit(fit->GetName(),"RB0");
}

void meansigma(TF1 *& fit, Float_t & mean, Float_t & emean, Float_t & sigma, Float_t & esigma) 
{
  if (formname.EqualTo("gaus1",TString::kExact)) {
    mean   = fit->GetParameter(1);
    sigma  = fit->GetParameter(2);

    emean  = fit->GetParError(1);
    esigma = fit->GetParError(2);
  }
  else  if (formname.EqualTo("gaus2",TString::kExact)) {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom =  const1 + const2;

    mean   = (const1*fit->GetParameter(1) + const2*fit->GetParameter(4))/denom;
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(5))/denom;

    emean  = rad2(const1*fit->GetParError(1),const2*fit->GetParError(4));
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(5));

    emean  = std::sqrt(emean) /denom;
    esigma = std::sqrt(esigma)/denom;
  }
  else if (formname.EqualTo("gaus2fm",TString::kExact)) {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom =  const1 + const2;

    mean   = fit->GetParameter(1);
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4))/denom;

    emean  = fit->GetParError(1);
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4));

    esigma = std::sqrt(esigma)/denom;
  }
  else if (formname.EqualTo("gaus3fm",TString::kExact)) {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t const3 = fit->GetParameter(5);
    const Float_t denom  =  const1 + const2 + const3;

    mean   = fit->GetParameter(1);
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4) + const3*fit->GetParameter(6))/denom;

    emean  = fit->GetParError(1);
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4))+std::pow(const3*fit->GetParError(6),2);

    esigma = std::sqrt(esigma)/denom;
  }
}

void subcomps(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2, TF1 *& sub3) 
{
  canv->cd();

  if (formname.EqualTo("gaus1",TString::kExact)) {
    return;
  }
  else if (formname.EqualTo("gaus2",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-fitrange,fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-fitrange,fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(4),fit->GetParameter(5));
  } 
  else if (formname.EqualTo("gaus2fm",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-fitrange,fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-fitrange,fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(1),fit->GetParameter(4));
  } 
  else if (formname.EqualTo("gaus3fm",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-fitrange,fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-fitrange,fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(1),fit->GetParameter(4));

    sub3 = new TF1("sub3","gaus(0)",-fitrange,fitrange);
    sub3->SetParameters(fit->GetParameter(5),fit->GetParameter(1),fit->GetParameter(6));

    sub3->SetLineColor(kGreen-3); // kViolet-3
    sub3->SetLineWidth(2);
    sub3->SetLineStyle(7);
    sub3->Draw("same");
  } 
  
  sub1->SetLineColor(kRed) ;  // kgreen-3
  sub1->SetLineWidth(2);
  sub1->SetLineStyle(7);
  sub1->Draw("same");

  sub2->SetLineColor(kBlue); // kViolet-3
  sub2->SetLineWidth(2);
  sub2->SetLineStyle(7);
  sub2->Draw("same");
}

void drawsubcomp() 
{
  gStyle->SetOptStat(0);
  TVirtualFitter::SetDefaultFitter("Minuit2");

  TString label = "seedtime";
  TFile * file = TFile::Open("outtimes.root");

  TCanvas * canv = new TCanvas();
  canv->cd();

  TH1F * hist = (TH1F*)file->Get("h_seedtimeTOF");
  hist->Scale(1.0/hist->Integral());
  hist->SetMarkerStyle(1);
  hist->SetMarkerColor(kBlack);
  hist->SetLineColor(kBlack);
  hist->SetTitle(label.Data());
  hist->GetXaxis()->SetTitle("Time [ns]");
  hist->GetYaxis()->SetTitle("Events");
  hist->Draw("ep");

  
  // use directly the functionin ROOT::MATH note that the parameters definition is different is (alpha, n sigma, mu)
//   TF1 * fit = new TF1("fit","crystalball",-fitrange,fitrange);
//   // N, mean, sigma, alpha, n
//   fit->SetParameters(0.08, 0.0, 1.0, 1.0, 0.5);
//   hist->Fit(fit->GetName(),"RB0");
//   fit->Draw("same");

   Float_t mean, emean, sigma, esigma;
   TF1 * fit;

  doFit(hist,fit);
  meansigma(fit,mean,emean,sigma,esigma);
//   fit->SetLineWidth(3);
//   fit->SetLineColor(kMagenta-3); //kViolet-6
//   fit->Draw("same");

  TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
  TF1 * fitfull = new TF1(Form("%s_fit_full",formname.Data()),formname.Data(),-3.0,3.0);
  fitfull->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  fitfull->SetLineWidth(3);
  fitfull->SetLineColor(kMagenta-3); //kViolet-6
  fitfull->Draw("same");
  
  TF1 * sub1; TF1 * sub2; TF1 * sub3;
  subcomps(fit,canv,sub1,sub2,sub3);

  TPaveText * text = new TPaveText(0.6,0.68,0.9,0.78,"NDC");
  text->AddText(Form("#mu = %f #pm %f, #sigma = %f #pm %f",mean,emean,sigma,esigma));
  text->AddText(Form("#chi^{2} = %f",fit->GetChisquare()));
  text->SetTextSize(0.03);
  text->Draw("same");
}
