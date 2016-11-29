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

void distance()
{
  TString hist = "genN1decayr_zoom";

  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // Signals
  TFile * file100 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau100/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau100 = (TH1F*)file100->Get(hist.Data()); 
  ctau100->Scale(1.0/ctau100->Integral());
  ctau100->SetLineColor(kViolet-1);

  TFile * file2000 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau2000/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau2000 = (TH1F*)file2000->Get(hist.Data()); 
  ctau2000->Scale(1.0/ctau2000->Integral());
  ctau2000->SetLineColor(kRed+1);

  TFile * file6000 = TFile::Open("output/MC/signal/withReReco/nocuts/ctau6000/cuts_jetpt35.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F  * ctau6000 = (TH1F*)file6000->Get(hist.Data()); 
  ctau6000->Scale(1.0/ctau6000->Integral());
  ctau6000->SetLineColor(kBlue+1);

  // Draw it all
  TCanvas * canv = new TCanvas("canv","canv");
  canv->cd();
  canv->SetLogy();
  //  ctau100 ->GetYaxis()->SetRangeUser(5e-5,3);
  ctau100 ->GetYaxis()->SetTitle("Events");
  ctau100 ->Draw("HIST");
  ctau100 ->Draw("HIST same");
  ctau2000->Draw("HIST same");
  ctau6000->Draw("HIST same");

  TLegend * leg = new TLegend(0.6,0.75,0.8,0.9);
  leg->AddEntry(ctau100 ,"c#tau = 36.5 mm" ,"l");
  leg->AddEntry(ctau2000,"c#tau = 730.5 mm","l");
  leg->AddEntry(ctau6000,"c#tau = 2192 mm","l");
  leg->Draw("same");

  CMSLumi(canv,"Simulation");
  canv->SaveAs(Form("%s.png",hist.Data()));

  // overflow 
  std::cout << "ctau100  " << 100.f*ctau100 ->GetBinContent(ctau100->GetNbinsX()+1) << "%" << std::endl;
  std::cout << "ctau2000 " << 100.f*ctau2000->GetBinContent(ctau100->GetNbinsX()+1) << "%" << std::endl;
  std::cout << "ctau6000 " << 100.f*ctau6000->GetBinContent(ctau100->GetNbinsX()+1) << "%" << std::endl;
}
