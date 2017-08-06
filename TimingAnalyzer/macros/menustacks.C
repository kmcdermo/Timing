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
#include <algorithm>

void dostack(TFile *& file, TString dir, std::vector<TString> & histnames, TString title, TString output)
{
  gStyle->SetOptStat(0);

  std::vector<TString>  labels;
  std::vector<Color_t>  colors  = {kBlack,kBlue,kViolet-1,kRed+1,kOrange+1,kYellow-7,kGreen+1,kAzure+10,kMagenta,kYellow+3};
  std::vector<Marker_t> markers = {kFullCircle,kFullSquare,kFullTriangleUp,kFullTriangleDown,kFullDiamond,kFullCircle,kFullSquare,kFullTriangleUp,kFullTriangleDown,kFullDiamond};

  TCanvas * histcanv = new TCanvas(); histcanv->cd();
  TLegend * leg = new TLegend(0.0,0.0,1.0,1.0);

  std::vector<TH1F*> hists(histnames.size());
  for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
  {
    hists[ihist] = (TH1F*)file->Get(histnames[ihist].Data());
    hists[ihist]->SetDirectory(0);
    labels.push_back(hists[ihist]->GetTitle());
    hists[ihist]->SetTitle(Form("Efficiency vs %s",title.Data()));
    hists[ihist]->GetXaxis()->SetTitle(Form("%s",title.Data()));
    hists[ihist]->GetYaxis()->SetTitle("Efficiency");
    hists[ihist]->SetLineColor(colors[ihist]);
    hists[ihist]->SetMarkerColor(colors[ihist]);
    hists[ihist]->SetMarkerStyle(markers[ihist]);
    hists[ihist]->SetMaximum(1.1);
    hists[ihist]->SetMinimum(0.f);
    hists[ihist]->Draw(ihist>0?"ep same":"ep");

    Ssiz_t  start = labels[ihist].Index("HLT_");
    Ssiz_t  end   = labels[ihist].Index(" Efficiency");
    TString label(labels[ihist](start,end-start));
    leg->AddEntry(hists[ihist],label.Data(),"pl");
  }
  histcanv->SaveAs(dir+"hist_"+output+".png");
  
  TCanvas * legcanv = new TCanvas(); legcanv->cd();
  leg->Draw();
  legcanv->SaveAs(dir+"leg_"+output+".png");

  // delete everything
  delete leg;
  delete legcanv;
  delete histcanv;
  
  for (UInt_t ihist = 0; ihist < hists.size(); ihist++) {delete hists[ihist];}
}

void cmsmenustack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho60_ht350_eff","phdelayseedtime_hltpho175_eff","phdelayseedtime_hltpho120_met40_eff","phdelayseedtime_hltdoublepho60_eff","phdelayseedtime_hltdoublepho42_25_m15_eff","phdelayseedtime_hltpho90_ht600_eff"};

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "cmsmenustack";

  dostack(file,dir,histnames,title,output);
}

void etstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho50_ht350_eff","phdelayseedtime_hltdispho60_ht350_eff","phdelayseedtime_hltdispho70_ht350_eff"};

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "etstack";

  dostack(file,dir,histnames,title,output);
}

void menustacks()
{
  Bool_t isFull = false;
  Bool_t isGMSB = true;

  TString cuts = (isFull) ? "cuts_jetht350.0_minjetpt15.0_ph1pt60.0_ph1VIDmedium_ph1r90.9_phanypt10.0_phanyVIDloose_phanyr90.9_rhE1.0_ecalaccept" : "cuts_jetht350.0_minjetpt15.0_phanypt10.0_rhE1.0_ecalaccept";
  TString dir  = (isGMSB) ? Form("output/trigger/MC/signal/GMSB/ctau6000/%s/",cuts.Data()) : Form("output/trigger/MC/signal/HVDS/ctau1000/%s/",cuts.Data());
  TFile * file = TFile::Open(dir+"plots.root");

  cmsmenustack(file,dir);
  etstack(file,dir);

  delete file;
}

