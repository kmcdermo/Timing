#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"
#include "TString.h"

#include <vector>

void overplot()
{
  gStyle->SetOptStat(0);

  std::vector<TFile*> files(2);
  files[0] = TFile::Open("output/test2000/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/plots.root");
  files[1] = TFile::Open("output/testhvds/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/plots.root");

  TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
  TLegend * leg  = new TLegend(0.6,0.7,0.9,0.9);

  std::vector<TH1F*>   hists(files.size()); 
  std::vector<Color_t> colors = {kBlack,kRed+1,kViolet-1,kBlue+1,kGreen+2};
  std::vector<TString> labels = {"GMSB c#tau = 730.5 mm","HVDS c#tau = 200 mm"};

  for (UInt_t i = 0; i < hists.size(); i++)
  { 
    //    hists[i] = (TH1F*)files[i]->Get("ph1seedtime");
    hists[i] = (TH1F*)files[i]->Get(i==0?"genN1gamma":"genvPiongamma");
    if (i == 0) hists[i]->Add((TH1F*)files[i]->Get("genN2gamma"));

    hists[i] ->SetTitle("Neutralinos or Dark Pions #gamma factor");
    hists[i] ->GetYaxis()->SetTitle("Neutralinos or Dark Pions");
    hists[i] ->Scale(1.f/hists[i]->Integral());
    hists[i] ->SetLineColor(colors[i]);
    hists[i] ->SetMarkerColor(colors[i]);

    hists[i] ->Draw(i>0?"same hist":"hist");
    leg->AddEntry(hists[i],labels[i].Data(),"l");
  }

  leg->Draw("same");
  canv->SaveAs("SignalMC_gamma.png");
}
