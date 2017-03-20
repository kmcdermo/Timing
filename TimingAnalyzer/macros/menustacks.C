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

  if (output.Contains("breakdownstack",TString::kExact))
  {
    std::vector<TString> extralabels = {"Only L1","Add L1 Regional HLT EG Matching","Add EG E_{T} Filter","Add EG HoverE Filter","Add EG R9 Filter","Add EG #sigma_{i#eta i#eta} Filter","Add EG S_{Major} Filter","Add EG S_{Minor} Filter","Add EG Time Filter","Add EG Track Veto (Full Path)"};
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

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "ptstack";

  dostack(file,dir,histnames,title,output);
}

void cmsmenustack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltpho175_eff","phdelayseedtime_hltpho120_met40_eff","phdelayseedtime_hltdoublepho60_eff"};

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "cmsmenustack";

  dostack(file,dir,histnames,title,output);
}

void dispho45stack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltdispho45_jet50_eff","phdelayseedtime_hltdispho45_dijet50_eff","phdelayseedtime_hltdispho45_dijet35_eff","phdelayseedtime_hltdispho45_trijet35_eff"};

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "dispho45stack";

  dostack(file,dir,histnames,title,output);
}

void dispho60stack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho60_eff","phdelayseedtime_hltdispho60_jet50_eff","phdelayseedtime_hltdispho60_dijet50_eff","phdelayseedtime_hltdispho60_dijet35_eff","phdelayseedtime_hltdispho60_trijet35_eff"};

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "dispho60stack";

  dostack(file,dir,histnames,title,output);
}

void seedtimebreakdownstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayseedtime_hltdispho45_eff","phdelayseedtime_hltdispho45_notkveto_eff","phdelayseedtime_hltdispho45_notime_eff","phdelayseedtime_hltdispho45_nosmaj_eff","phdelayseedtime_hltdispho45_nosmin_eff","phdelayseedtime_hltdispho45_nosieie_eff","phdelayseedtime_hltdispho45_nor9_eff","phdelayseedtime_hltdispho45_nohoe_eff","phdelayseedtime_hltdispho45_noet_eff","phdelayseedtime_hltdispho45_nol1match_eff"};
  std::reverse(histnames.begin(),histnames.end());

  TString title  = "Most Delayed Photon Seed RecHit Time [ns]";
  TString output = "seedtimebreakdownstack";

  dostack(file,dir,histnames,title,output);
}

void etabreakdownstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayeta_hltdispho45_eff","phdelayeta_hltdispho45_notkveto_eff","phdelayeta_hltdispho45_notime_eff","phdelayeta_hltdispho45_nosmaj_eff","phdelayeta_hltdispho45_nosmin_eff","phdelayeta_hltdispho45_nosieie_eff","phdelayeta_hltdispho45_nor9_eff","phdelayeta_hltdispho45_nohoe_eff","phdelayeta_hltdispho45_noet_eff","phdelayeta_hltdispho45_nol1match_eff"};
  std::reverse(histnames.begin(),histnames.end());

  TString title  = "Most Delayed Photon #eta";
  TString output = "etabreakdownstack";

  dostack(file,dir,histnames,title,output);
}

void phibreakdownstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelayphi_hltdispho45_eff","phdelayphi_hltdispho45_notkveto_eff","phdelayphi_hltdispho45_notime_eff","phdelayphi_hltdispho45_nosmaj_eff","phdelayphi_hltdispho45_nosmin_eff","phdelayphi_hltdispho45_nosieie_eff","phdelayphi_hltdispho45_nor9_eff","phdelayphi_hltdispho45_nohoe_eff","phdelayphi_hltdispho45_noet_eff","phdelayphi_hltdispho45_nol1match_eff"};
  std::reverse(histnames.begin(),histnames.end());

  TString title  = "Most Delayed Photon #phi";
  TString output = "phibreakdownstack";

  dostack(file,dir,histnames,title,output);
}

void ptbreakdownstack(TFile *& file, TString dir)
{
  std::vector<TString> histnames = {"phdelaypt_hltdispho45_eff","phdelaypt_hltdispho45_notkveto_eff","phdelaypt_hltdispho45_notime_eff","phdelaypt_hltdispho45_nosmaj_eff","phdelaypt_hltdispho45_nosmin_eff","phdelaypt_hltdispho45_nosieie_eff","phdelaypt_hltdispho45_nor9_eff","phdelaypt_hltdispho45_nohoe_eff","phdelaypt_hltdispho45_noet_eff","phdelaypt_hltdispho45_nol1match_eff"};
  std::reverse(histnames.begin(),histnames.end());

  TString title  = "Most Delayed Photon p_{T}";
  TString output = "ptbreakdownstack";

  dostack(file,dir,histnames,title,output);
}

void menustacks()
{
  TString dir  = "output/trigger/MC/signal/HVDS/ctau1000/cuts_jetpt35.0_njets3_ph1pt50.0_ph1VIDmedium_phanypt10.0_phanyVIDloose_rhE1.0_ecalaccept/";
  //TString dir  = "output/trigger/MC/signal/GMSB/ctau6000/cuts_jetpt35.0_njets3_ph1pt50.0_ph1VIDmedium_phanypt10.0_phanyVIDloose_rhE1.0_EEOnly/";
  //TString dir  = "output/trigger/MC/signal/GMSB/ctau100/cuts_jetpt35.0_njets3_ph1pt50.0_ph1VIDmedium_phanypt10.0_phanyVIDloose_rhE1.0_ecalaccept/";
  TFile * file = TFile::Open(dir+"plots.root");

  //  cmsmenustack(file,dir);
  //  ptstack(file,dir);
  //  dispho45stack(file,dir);
  //  dispho60stack(file,dir);
  seedtimebreakdownstack(file,dir);
  etabreakdownstack(file,dir);
  phibreakdownstack(file,dir);
  ptbreakdownstack(file,dir);

  delete file;
}

