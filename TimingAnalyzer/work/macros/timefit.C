#include "TVirtualFitter.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TF1.h"
#include "TFormula.h"
#include "TGaxis.h"
#include "TPaveText.h"
#include "common/common.C"

static const Double_t sigma_nEB = 0.0513466; // 51 MeV
static const Double_t sigma_nEE = 0.126938; // 127 MeV

void timefit()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  Bool_t isMC = false;
  Bool_t isEB = false;
  TString outdir = "plots";

  TFile * file = TFile::Open(Form("%s/%s/plots.root",outdir.Data(),isMC?"MC/dyll":"DATA/doubleeg"));
  TH1F  * hist = (TH1F*)file->Get(Form("td_effseedE_%s_sigma_gaus2fm",isEB?"EBEB":"EEEE"));
  hist->GetXaxis()->SetTitle("E_{eff}/#sigma_{n}");
  hist->GetYaxis()->SetTitle("#sigma(t_{1}-t_{2}) [ns]");

  TVirtualFitter::SetDefaultFitter("Minuit2");
  TString formname = "timefit";
  TFormula form(formname.Data(),Form("sqrt((([0]*[0])/(x*x))+(2*[1]*[1]))"));
  
  Double_t xlow  = hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetFirst());
  Double_t xhigh = hist->GetXaxis()->GetBinUpEdge (hist->GetXaxis()->GetLast());

  TF1 * fit  = new TF1(Form("%s_fit",formname.Data()),formname.Data(),xlow,xhigh);
  fit->SetParameters(35.0,0.150);
  fit->SetParLimits(0,0,100.0);
  fit->SetParLimits(1,0,1.0);
  fit->SetLineColor(kBlue);

  TCanvas * canv = new TCanvas();
  canv->cd();
 
  hist->Fit(Form("%s_fit",formname.Data()));
  
  canv->SetLogx(1);
  canv->SetLogy(1);
  canv->SetGridx(1);
  canv->SetGridy(1);

  hist->SetMaximum(2.1);
  hist->SetMinimum(0.08);
  hist->Draw();
  
  fit->Draw("same");

  Double_t xp = 0.05;
  Double_t yp = 0.05;

  TPaveText * text = new TPaveText(0.5+xp,0.6+yp,0.8+xp,0.8+yp,"NDC");
  text->AddText("#sigma(t)=#frac{N}{E_{eff}/#sigma_{n}} #oplus #sqrt{2}C");
  text->AddText(Form("N = %4.1f #pm %3.1f ns",fit->GetParameter(0),fit->GetParError(0)));
  text->AddText(Form("C = %6.4f #pm %6.4f ns",fit->GetParameter(1),fit->GetParError(1)));
  text->SetTextAlign(11);
  text->SetFillColorAlpha(text->GetFillColor(),0);
  text->Draw("same");

  Double_t elow  = xlow  * (isEB ? sigma_nEB : sigma_nEE);
  Double_t ehigh = xhigh * (isEB ? sigma_nEB : sigma_nEE);

  Double_t height = hist->GetMinimum()*1.5;
  Double_t fudge  = 0.15;

  Double_t elowf  = elow  * (1.0 + fudge);
  Double_t ehighf = ehigh * (1.0 - fudge);

  Double_t xlowf  = xlow  * (1.0 + fudge);
  Double_t xhighf = xhigh * (1.0 - fudge);

  TGaxis * axis = new TGaxis(xlowf,height,xhighf,height,elowf,ehighf,10,"G");
  axis->SetMoreLogLabels(1);
  axis->SetNoExponent(1);
  axis->SetTitle(Form("E in %s [GeV]",isEB?"EB":"EE"));
  axis->Draw("same");
  
  CMSLumi(canv,"Preliminary");

  canv->SaveAs(Form("%s/timeresfit/%s_%s.png",outdir.Data(),isMC?"mc":"data",isEB?"eb":"ee"));
  canv->SaveAs(Form("%s/timeresfit/%s_%s.pdf",outdir.Data(),isMC?"mc":"data",isEB?"eb":"ee"));
}
