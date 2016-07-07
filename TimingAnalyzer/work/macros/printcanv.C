#include "TString.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TVirtualFitter.h"
#include "TF1.h"
#include "TFormula.h"

void prepFit(TH1F *& hist, TF1 *& fit, TString formname, Double_t fitrange);
void drawSubComp(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2, TString formname, Double_t fitrange); 

void printcanv()
{
  Bool_t  isMC     = false;
  TString indir    = "output";
  TString fitdir   = "timing/effective/E/inclusive";
  TString name     = "td_effE_inclusive_22_24";
  TString formname = "gaus2fm";

  TString outdir = Form("%s/%s",indir.Data(), (isMC?"MC/dyll":"DATA/doubleeg") );
  TFile   * file = TFile::Open(Form("%s/plots.root",outdir.Data()));
  TCanvas * canv = (TCanvas*)file->Get(Form("%s_%s_fit",name.Data(),formname.Data()));
  TH1F    * hist = (TH1F*)canv->GetPrimitive(name.Data());

  // first prep the fit
  TVirtualFitter::SetDefaultFitter("Minuit2");
  TF1 * fit;
  Double_t fitrange = 5.0; 
  prepFit(hist,fit,formname,fitrange);
  
  // do the fit
  hist->Fit(Form("%s_fit",formname.Data()));
  
  // draw the new fit
  TCanvas * outcanv = new TCanvas();
  outcanv->cd();
  
  hist->Draw();
  
  TF1 * sub1; TF1 * sub2;
  drawSubComp(fit,outcanv,sub1,sub2,formname,fitrange);
}

void prepFit(TH1F *& hist, TF1 *& fit, TString formname, Double_t fitrange)
{
  TF1 * tempfit = new TF1("temp","gaus(0)",-fitrange,fitrange);
  tempfit->SetParLimits(2,0,10);
  hist->Fit("temp","RQ0B");
  const Float_t tempp0 = tempfit->GetParameter(0); // constant
  const Float_t tempp1 = tempfit->GetParameter(1); // mean
  const Float_t tempp2 = tempfit->GetParameter(2); // sigma
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
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,tempp2*4);
    fit->SetParLimits(2,0,10);
    fit->SetParLimits(4,0,10);
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
