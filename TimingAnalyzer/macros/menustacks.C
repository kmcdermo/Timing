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

void dostack(TFile *& file, TString dir, std::vector<TString> & histnames, TString & output)
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
    hists[ihist]->SetTitle("Efficiency vs Most Delayed Photon Seed Time [ns]");
    hists[ihist]->GetXaxis()->SetTitle("Most Delayed Photon Seed Time [ns]");
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

  if (output.Contains("breakdownstack",TString::kExact))
  {
    std::vector<TString> extralabels = {"Only L1","Add L1 Regional HLT EG Matching","Add EG E_{T} Filter","Add EG HoverE Filter","Add EG R9 Filter","Add EG #sigma_{i#eta i#eta} Filter","Add EG S_{Major} Filter","Add EG S_{Minor} Filter","Add EG Time Filter","Add EG Track Veto (Full Path)"}
    TLegend * extraleg = new TLegend(0.0,0.0,1.0,1.0);
    for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
    {
      extraleg->AddEntry(hists[ihist],extralabels[ihist].Data(),"pl");
    }

    TCanvas * extralegcanv = new TCanvas(); extralegcanv->cd();
    extraleg->Draw();
    extralegcanv->SaveAs(dir+"extraleg_"+output+".png");

    delete extralegcanv;
    delete extraleg;
  }

  // delete everything
  delete leg;
  delete legcanv;
  delete histcanv;
  
  for (UInt_t ihist = 0; ihist < hists.size(); ihist++) {delete hists[ihist];}
}

void ptstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltdispho50_eff","phdelayseedtime_hltdispho60_eff","phdelayseedtime_hltdispho80_eff","phdelayseedtime_hltdispho100_eff"};

  TString output = "ptstack";

  dostack(file,dir,histnames,output);
}

void cmsmenustack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltpho175_eff","phdelayseedtime_hltpho120_met40_eff","phdelayseedtime_hltdoublepho60_eff"};

  TString output = "cmsmenustack";

  dostack(file,dir,histnames,output);
}

void dispho45stack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltdispho45_jet50_eff","phdelayseedtime_hltdispho45_dijet50_eff","phdelayseedtime_hltdispho45_dijet35_eff","phdelayseedtime_hltdispho45_trijet35_eff","phdelayseedtime_hltdispho45_el100veto_eff","phdelayseedtime_hltdispho45_notkveto_eff"};

  TString output = "dispho45stack";

  dostack(file,dir,histnames,output);
}

void dispho60stack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho60_eff","phdelayseedtime_hltdispho60_jet50_eff","phdelayseedtime_hltdispho60_dijet50_eff","phdelayseedtime_hltdispho60_dijet35_eff","phdelayseedtime_hltdispho60_trijet35_eff","phdelayseedtime_hltdispho60_notkveto_eff"};

  TString output = "dispho60stack";

  dostack(file,dir,histnames,output);
}

void breakdownstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltdispho45_notkveto_eff","phdelayseedtime_hltdispho45_notime_eff","phdelayseedtime_hltdispho45_nosmaj_eff","phdelayseedtime_hltdispho45_nosmin_eff","phdelayseedtime_hltdispho45_nosieie_eff","phdelayseedtime_hltdispho45_nor9_eff","phdelayseedtime_hltdispho45_nohoe_eff","phdelayseedtime_hltdispho45_noet_eff","phdelayseedtime_hltdispho45_nol1match_eff"};
  std::reverse(histnames.begin(),histnames.end());

  TString output = "breakdownstack";

  dostack(file,dir,histnames,output);
}

void menustacks()
{
  TString dir  = "output/MC/signal/HVDS/ctau1000/cuts_jetpt35.0_phpt50.0_phVIDloose_rhE1.0_EEOnly/";
  TFile * file = TFile::Open(dir+"plots.root");

  cmsmenustack(file,dir);
  ptstack(file,dir);
  dispho45stack(file,dir);
  dispho60stack(file,dir);
  //  breakdownstack(file,dir);

  delete file;
}

