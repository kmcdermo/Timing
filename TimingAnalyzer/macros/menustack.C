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

void dostack(TString & input, TFile *& file, std::vector<TString> & histnames, TString & output)
{
  gStyle->SetOptStat(0);

  std::vector<TString> labels;
  
  std::vector<Color_t> colors;
  colors.push_back(kBlack);
  colors.push_back(kBlue);
  colors.push_back(kViolet-1);
  colors.push_back(kRed);
  colors.push_back(kOrange+7);
  colors.push_back(kYellow-1);
  colors.push_back(kGreen+1);
  colors.push_back(kAzure+10);
  colors.push_back(kMagenta);
  colors.push_back(kPink-4);

  std::vector<Marker_t> markers;
  markers.push_back(kFullCircle);
  markers.push_back(kFullSquare);
  markers.push_back(kFullTriangleUp);
  markers.push_back(kFullTriangleDown);
  markers.push_back(kFullDiamond);
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

  // delete everything
  delete leg;
  delete legcanv;
  delete histcanv;
  
  for (UInt_t ihist = 0; ihist < hists.size(); ihist++) {delete hists[ihist];}
}

void ptstack()
{
  TString input  = "output/gmsb6000/test_hlt2/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/";
  TFile * file = TFile::Open(input+"plots.root");

  std::vector<TString> histnames;
  histnames.push_back("phdelayseedtime_hltdispho45_eff");
  histnames.push_back("phdelayseedtime_hltdispho50_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_eff");
  histnames.push_back("phdelayseedtime_hltdispho80_eff");
  histnames.push_back("phdelayseedtime_hltdispho100_eff");

  TString output = "ptstack";

  dostack(input,file,histnames,output);
  delete file;
}

void cmsmenustack()
{
  TString input  = "output/gmsb6000/test_hlt2/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/";
  TFile * file = TFile::Open(input+"plots.root");

  std::vector<TString> histnames;
  histnames.push_back("phdelayseedtime_hltdispho45_eff");
  histnames.push_back("phdelayseedtime_hltpho175_eff");
  histnames.push_back("phdelayseedtime_hltpho120_met40_eff");
  histnames.push_back("phdelayseedtime_hltddoublepho60_eff");

  TString output = "cmsmenustack";

  dostack(input,file,histnames,output);
  delete file;
}

void dispho45stack()
{
  TString input  = "output/gmsb6000/test_hlt2/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/";
  TFile * file = TFile::Open(input+"plots.root");

  std::vector<TString> histnames;
  histnames.push_back("phdelayseedtime_hltdispho45_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_jet50_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_dijet50_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_dijet35_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_trijet35_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_el100veto_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_notkveto_eff");

  TString output = "dispho45stack";

  dostack(input,file,histnames,output);
  delete file;
}

void dispho60stack()
{
  TString input  = "output/gmsb6000/test_hlt2/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/";
  TFile * file = TFile::Open(input+"plots.root");

  std::vector<TString> histnames;
  histnames.push_back("phdelayseedtime_hltdispho60_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_jet50_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_dijet50_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_dijet35_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_trijet35_eff");
  histnames.push_back("phdelayseedtime_hltdispho60_notkveto_eff");

  TString output = "dispho60stack";

  dostack(input,file,histnames,output);
  delete file;
}

void breakdownstack()
{
  TString input  = "output/gmsb6000/test_hlt2/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_ecalaccept/";
  TFile * file = TFile::Open(input+"plots.root");

  std::vector<TString> histnames;
  histnames.push_back("phdelayseedtime_hltdispho45_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_notkveto_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_notime_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_nosmaj_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_nosmin_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_nosieie_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_nor9_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_nohoe_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_noet_eff");
  histnames.push_back("phdelayseedtime_hltdispho45_nol1match_eff");

  TString output = "breakdownstack";

  dostack(input,file,histnames,output);
  delete file;
}

void menustacks()
{
  cmsmenustack();
  ptstack();
  dispho45stack();
  dispho60stack();
  breakdownstack();
}

