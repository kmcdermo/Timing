#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TVirtualFitter.h"
#include "TF1.h"
#include "TFormula.h"
#include "TStyle.h"

Double_t rad2(Double_t x, Double_t y){return x*x + y*y;}
void prepFit(TH1F *& hist, TF1 *& fit, TString formname, Double_t fitrange, Double_t g1, Double_t g2);
void drawSubComp(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2, TString formname, Double_t fitrange); 
void getMeanSigma(TF1 *& fit, Double_t & mean, Double_t & emean, Double_t & sigma, Double_t & esigma, TString formname);

void refitcanv(TString tisMC, TString fitdir, TString name, TString bins, TString tbin, TString tg1, TString tg2)
{
  Bool_t  isMC = tisMC.Atoi();
  Int_t    bin = tbin.Atoi();
  Double_t  g1 = tg1.Atof();
  Double_t  g2 = tg2.Atof();
  TString indir    = "output";
  TString formname = "gaus2fm";

  TString outdir = Form("%s/%s",indir.Data(), (isMC?"MC/dyll":"DATA/doubleeg") );
  TFile   * file = TFile::Open(Form("%s/plots.root",outdir.Data()),"UPDATE");
  TCanvas * canv = (TCanvas*)file->Get(Form("%s_%s_%s_fit",name.Data(),bins.Data(),formname.Data()));
  TH1F    * hist = (TH1F*)   canv->GetPrimitive(Form("%s_%s",name.Data(),bins.Data()));
  TH1F   * hmean = (TH1F*)   file->Get(Form("%s_mean_%s",name.Data(),formname.Data()));
  TH1F  * hsigma = (TH1F*)   file->Get(Form("%s_sigma_%s",name.Data(),formname.Data()));

  // first prep the fit
  TVirtualFitter::SetDefaultFitter("Minuit2");
  TF1 * fit;
  Double_t fitrange = 5.0; 
  prepFit(hist,fit,formname,fitrange,g1,g2);
  
  // do the fit
  hist->Fit(Form("%s_fit",formname.Data()));
  
  // draw the new fit
  TCanvas * outcanv = new TCanvas("outcanv","outcanv");
  outcanv->cd();
  
  hist->Draw();
  
  TF1 * sub1; TF1 * sub2;
  drawSubComp(fit,outcanv,sub1,sub2,formname,fitrange);

  // now save the new fit as a png
  outcanv->SaveAs(Form("%s/%s_refit.png",fitdir.Data(),outcanv->GetName()));
  
  // now compute mean + sigma of new fit, print out and save it in appropriate histogram
  Double_t mean, sigma, emean, esigma;
  getMeanSigma(fit,mean,emean,sigma,esigma,formname);

  std::cout << "New mean: " << mean << "+/-" << emean << " and sigma: " << sigma << "+/-" << esigma << std::endl;
  
  hmean ->SetBinContent(bin,mean);
  hmean ->SetBinError(bin,emean);
  hsigma->SetBinContent(bin,sigma);
  hsigma->SetBinError(bin,esigma);

  hmean->Write(hmean->GetName(),TObject::kWriteDelete);
  hsigma->Write(hsigma->GetName(),TObject::kWriteDelete);

  TCanvas * canv = new TCanvas();
  canv->cd();

  //  canv->SetLogy(1);
  //  canv->SetLogx(1);
  //  canv->SetGridy(1);
  //  canv->SetGridx(1);
  gStyle->SetOptStat(0);
  hsigma->Draw();
  //hmean->Draw();

}

void prepFit(TH1F *& hist, TF1 *& fit, TString formname, Double_t fitrange, Double_t g1, Double_t g2)
{
  TF1 * tempfit = new TF1("temp","gaus(0)",-fitrange,fitrange);
  tempfit->SetParLimits(2,0,10);
  hist->Fit("temp","RQ0B");
  const Double_t tempp0 = tempfit->GetParameter(0); // constant
  const Double_t tempp1 = tempfit->GetParameter(1); // mean
  const Double_t tempp2 = tempfit->GetParameter(2); // sigma
  delete tempfit;

  if (formname.EqualTo("gaus1",TString::kExact)) {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(Form("%s_fit",formname.Data()),formname.Data(),-fitrange,fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2);
    fit->SetParLimits(2,0,10);
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
//     fit->SetParameters(tempp0,tempp1,tempp2,tempp0/g1,tempp2*g2);
//     fit->SetParLimits(2,0,10);
//     fit->SetParLimits(4,0,10);

     fit->SetParameters(tempp0,tempp1,tempp2,0,1);
     fit->SetParLimits(3,0,5);
     fit->SetParLimits(4,0,5);
  }

  fit->SetLineColor(kMagenta-3); 
}

void drawSubComp(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2, TString formname, Double_t fitrange) 
{
  if (formname.EqualTo("gaus2",TString::kExact)) {
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
  else { // do not do anything in this function
    return;
  }

  canv->cd();

  sub1->SetLineColor(kRed) ;  // kgreen-3
  sub1->SetLineWidth(2);
  sub1->SetLineStyle(7);
  sub1->Draw("same");

  sub2->SetLineColor(kBlue); // kViolet-3
  sub2->SetLineWidth(2);
  sub2->SetLineStyle(7);
  sub2->Draw("same");
}

void getMeanSigma(TF1 *& fit, Double_t & mean, Double_t & emean, Double_t & sigma, Double_t & esigma, TString formname) 
{
  if (formname.EqualTo("gaus1",TString::kExact)) {
    mean   = fit->GetParameter(1);
    emean  = fit->GetParError (1);
    sigma  = fit->GetParameter(2);
    esigma = fit->GetParError (2);
  }
  else if (formname.EqualTo("gaus2",TString::kExact)) {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t denom =  const1 + const2;

    mean   = (const1*fit->GetParameter(1) + const2*fit->GetParameter(4))/denom;
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(5))/denom;

    emean  = rad2(const1*fit->GetParError(1),const2*fit->GetParError(4));
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(5));

    emean  = std::sqrt(emean) /denom;
    esigma = std::sqrt(esigma)/denom;
  }
  else if (formname.EqualTo("gaus2fm",TString::kExact)) {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t denom =  const1 + const2;

    mean   = fit->GetParameter(1);
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4))/denom;

    emean  = fit->GetParError(1);
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4));

    esigma = std::sqrt(esigma)/denom;
  }
}
