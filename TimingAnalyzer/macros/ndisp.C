#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>

static const Float_t lumi = 36.56 * 1000; // pb

void ndisp()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // Signals
  TFile * file100 = TFile::Open("output/withReReco/nocuts/ctau100/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau100nNeutoPhGr = (TH1F*)file100->Get("nNeutoPhGr"); 

  ctau100nNeutoPhGr->Scale(1.0/ctau100nNeutoPhGr->Integral());
  ctau100nNeutoPhGr->SetLineColor(kViolet-1);

  TFile * file2000 = TFile::Open("output/withReReco/nocuts/ctau2000/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau2000nNeutoPhGr = (TH1F*)file2000->Get("nNeutoPhGr"); 

  ctau2000nNeutoPhGr->Scale(1.0/ctau2000nNeutoPhGr->Integral());
  ctau2000nNeutoPhGr->SetLineColor(kRed+1);

  TFile * file6000 = TFile::Open("output/withReReco/nocuts/ctau6000/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau6000nNeutoPhGr = (TH1F*)file6000->Get("nNeutoPhGr"); 

  ctau6000nNeutoPhGr->Scale(1.0/ctau6000nNeutoPhGr->Integral());
  ctau6000nNeutoPhGr->SetLineColor(kBlue+1);

  // Draw it all
  TCanvas * nNeutoPhGrcanv = new TCanvas("nNeutoPhGrcanv","nNeutoPhGrcanv");
  nNeutoPhGrcanv->cd();
  nNeutoPhGrcanv->SetLogy();
  ctau100nNeutoPhGr ->GetXaxis()->SetTitle("n(#tilde{#chi}^{0}_{1} #rightarrow #gamma + #tilde{G}) [Max 2 per Event]");
  ctau100nNeutoPhGr ->Draw("HIST");
  ctau2000nNeutoPhGr->Draw("HIST same");
  ctau6000nNeutoPhGr->Draw("HIST same");

  TLegend * nNeutoPhGrleg = new TLegend(0.4,0.75,0.6,0.9);
  nNeutoPhGrleg->AddEntry(ctau100nNeutoPhGr ,"c#tau = 36.5 mm" ,"l");
  nNeutoPhGrleg->AddEntry(ctau2000nNeutoPhGr,"c#tau = 730.5 mm","l");
  nNeutoPhGrleg->AddEntry(ctau6000nNeutoPhGr,"c#tau = 2192 mm","l");
  nNeutoPhGrleg->Draw("same");

  CMSLumi(nNeutoPhGrcanv,"Simulation");
  nNeutoPhGrcanv->SaveAs("nNeutoPhGr.png");
}

