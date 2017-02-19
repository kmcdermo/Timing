#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"
#include "TString.h"

#include <vector>

void stack5()
{
  gStyle->SetOptStat(0);

  std::vector<TFile*> files(5);
  files[0] = TFile::Open("output/test6000/cuts_jetpt35.0_phpt0.0_phVIDnone_rhE1.0_ecalaccept/plots.root");
  files[1] = TFile::Open("output/test6000/cuts_jetpt35.0_phpt50.0_phVIDnone_rhE1.0_ecalaccept/plots.root");
  files[2] = TFile::Open("output/test6000/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/plots.root");
  files[3] = TFile::Open("output/test6000-gmsbMatched/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/plots.root");
  files[4] = TFile::Open("output/test6000-phMatched/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/plots.root");

  TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
  TLegend * leg  = new TLegend(0.55,0.6,0.9,0.9);

  std::vector<TH1F*> hists(files.size()); 
  std::vector<Color_t> colors = {kBlack,kRed+1,kViolet-1,kBlue+1,kGreen+2};
  std::vector<TString> labels = {"No Cuts","p_{T}>50","p_{T}>50, Loose","p_{T}>50, Loose, GMSB Match","p_{T}>50, Loose, Ph Match"};

  for (UInt_t i = 0; i < hists.size(); i++)
  { 
    hists[i] = (TH1F*)files[i]->Get("ph1seedtime");
    hists[i] ->Scale(1.f/hists[i]->Integral());
    hists[i] ->SetLineColor(colors[i]);
    hists[i] ->SetMarkerColor(colors[i]);

    hists[i] ->Draw(i>0?"same hist":"hist");
    leg->AddEntry(hists[i],labels[i].Data(),"l");
  }

  leg->Draw("same");
  canv->SaveAs("GMSB_2192mm_cutflow_seed1time.png");

}
