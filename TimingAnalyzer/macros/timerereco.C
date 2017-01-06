#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "THStack.h"
#include "TCanvas.h"
#include "TLegend.h"

#include "common/common.C"

#include <vector>

static const Float_t lumi = 36.46 * 1000; // pb

void timerereco()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  TFile * wReReco = TFile::Open("output/ctau6000_wTiming-test/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ph1seedtime_wReReco = (TH1F*)wReReco->Get("ph1seedtime"); 

  TFile * noReReco = TFile::Open("output/ctau6000-test/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ph1seedtime_noReReco = (TH1F*)noReReco->Get("ph1seedtime"); 

  ph1seedtime_wReReco ->Scale(1.0/ph1seedtime_wReReco ->Integral());
  ph1seedtime_noReReco->Scale(1.0/ph1seedtime_noReReco->Integral());
  ph1seedtime_wReReco ->SetLineColor(kRed);
  ph1seedtime_noReReco->SetLineColor(kBlue);

  // Draw it all
  TCanvas * ph1seedtimecanv = new TCanvas("ph1seedtimecanv","ph1seedtimecanv");
  ph1seedtimecanv->cd();
  ph1seedtimecanv->SetLogy();

  ph1seedtime_wReReco ->GetXaxis()->SetTitle("Leading Photon Seed RecHit Time [ns]");
  ph1seedtime_wReReco ->GetYaxis()->SetTitle("Events");
  ph1seedtime_wReReco ->Draw("HIST");
  ph1seedtime_noReReco->Draw("HIST same");

  TLegend * ph1seedtimeleg = new TLegend(0.6,0.8,0.8,0.9);
  ph1seedtimeleg->AddEntry(ph1seedtime_wReReco ,"With ReReco","l");
  ph1seedtimeleg->AddEntry(ph1seedtime_noReReco,"No ReReco" ,"l");
  ph1seedtimeleg->Draw("same");

  CMSLumi(ph1seedtimecanv,"Simulation");
  ph1seedtimecanv->SaveAs("comptimes-2000.png");
}
