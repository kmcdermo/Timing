#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TColor.h"
#include "TAttMarker.h"

#include <vector>

void ptstack()
{
  TString output = "ptstack";
  TString input  = "output/gmsb6000/test_hlt2/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/";

  gStyle->SetOptStat(0);

  TFile * file = TFile::Open(input+"plots.root");

  std::vector<TString> histnames;
  histnames.push_back("phdelayseedtime_hltdispho45_eff");
  histnames.push_back("phdelayseedtime_hltdispho50_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_eff");
  histnames.push_back("phdelayseedtime_hltdispho80_eff");
  histnames.push_back("phdelayseedtime_hltdispho100_eff");

  std::vector<TString> labels;
  
  std::vector<Color_t> colors;
  colors.push_back(kBlack);
  colors.push_back(kGreen+1);
  colors.push_back(kRed);
  colors.push_back(kBlue);
  colors.push_back(kViolet-1);

  std::vector<Marker_t> markers;
  markers.push_back(kFullCircle);
  markers.push_back(kFullSquare);
  markers.push_back(kFullTriangleUp);
  markers.push_back(kFullTriangleDown);
  markers.push_back(kFullDiamond);

  TCanvas * histcanv = new TCanvas(); histcanv->cd();
  TLegend * leg = new TLegend(0.0,0.6,0.9,0.9);

  std::vector<TH1F*> hists(histnames.size());
  for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
  {
    hists[ihist] = (TH1F*)file->Get(histnames[ihist].Data());
    labels.push_back(hists[ihist]->GetTitle());
    hists[ihist]->SetTitle("Efficiency vs Most Delayed Photon Seed Time [ns]");
    hists[ihist]->GetXaxis()->SetTitle("Most Delayed Photon Seed Time [ns]");
    hists[ihist]->GetYaxis()->SetTitle("Efficiency");
    hists[ihist]->SetLineColor(colors[ihist]);
    hists[ihist]->SetMarkerColor(colors[ihist]);
    hists[ihist]->SetMarkerStyle(markers[ihist]);
    hists[ihist]->SetMaximum(1.1);
    hists[ihist]->SetMinimum(0.f);
    hists[ihist]->Draw(ihist>0?"ep":"ep same");

    Ssiz_t  start = labels[ihist].Index("HLT_");
    Ssiz_t  end   = labels[ihist].Index(" Efficiency");
    TString label(labels[ihist](start,end-start));
    leg->AddEntry(hists[ihist],label.Data(),"pl");
  }
  histcanv->SaveAs("hist_"+output+".png");
  
  TCanvas * legcanv = new TCanvas(); legcanv->cd();
  leg->Draw();
  legcanv->SaveAs("leg_"+output+".png");
}
