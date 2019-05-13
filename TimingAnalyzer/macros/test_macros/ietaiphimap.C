#include "Common.cpp+"

#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TStyle.h"

#include <iostream>
#include <vector>
#include <map>

void drawHists(TH2F * h_T, TH2F * h_E, TFile * outfile, const TString & outtext, const TString & option)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetGridx();
  canv->SetGridy();

  // draw time
  h_T->Draw(option.Data());
  Common::CMSLumi(canv);
  Common::SaveAs(canv,outtext+"_T_"+option);
  Common::Write(outfile,h_T,outtext+"_h_T_"+option);
  Common::Write(outfile,canv,outtext+"_canv_T_"+option);

  // draw energy
  if (option == "colz") 
  {
    canv->SetLogz();
    h_E->GetZaxis()->SetRangeUser(1,500); // 100
  }
  else h_E->GetZaxis()->SetRangeUser(0.5,500); // 100
  h_E->Draw(option.Data());
  Common::CMSLumi(canv);
  Common::SaveAs(canv,outtext+"_E_"+option);
  Common::Write(outfile,h_E,outtext+"_h_E_"+option);
  Common::Write(outfile,canv,outtext+"_canv_E_"+option);

  // delete it all
  delete canv;
}

void ietaiphimap(const TString & outtext = "prompt")
{
  // always set style
  auto tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // setup eras
  Common::SetupEras();

  // initialize ecal map
  Common::SetupDetIDs();

  // initialize tree and file + options
  const auto filename = Common::eosPreFix+"/"+Common::eosDir+"/unskimmed/GMSB_L-200TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/gmsb_GMSB_L-200TeV_Ctau-200cm_TuneCP5_13TeV-pythia8/190305_061616/0000/test/dispho.root";
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);

  const TString treename = "tree/disphotree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // initialize branches
  std::vector<Float_t> * rhtime_t = 0; tree->SetBranchAddress("rhtime", &rhtime_t);
  std::vector<Float_t> * rhE_t    = 0; tree->SetBranchAddress("rhE", &rhE_t);
  std::vector<UInt_t>  * rhID_t   = 0; tree->SetBranchAddress("rhID", &rhID_t);

  // photon stuff
  std::vector<Int_t> * phorecHits_0_t = 0; tree->SetBranchAddress("phorecHits_0", &phorecHits_0_t);
  Int_t phoseed_0 = 0; tree->SetBranchAddress("phoseed_0", &phoseed_0);

  // get entry
  UInt_t entry = 10558; // v1 good: delayed (ev = 88424, row = 74423), prompt (ev = 558, row = 10558) // v2 bad: delayed (ev = 96140, row = 134), prompt (ev = 96082, row = 91)
  tree->GetEntry(entry);

  // outfile
  auto outfile = TFile::Open(outtext+".root","UPDATE");
  outfile->cd();

  // make vecs
  const auto rhtime = *rhtime_t;
  const auto rhE = *rhE_t;
  const auto rhID = *rhID_t;
  const auto phorecHits_0 = *phorecHits_0_t;

  // tmp vars
  auto miniphi =  10000;
  auto maxiphi = -10000;

  auto minieta =  10000;
  auto maxieta = -10000;

  auto minT =  10000.f;
  auto maxT = -10000.f;

  auto minE =  10000.f;
  auto maxE = -10000.f;

  // get range of iphi, ieta
  for (const auto irh : phorecHits_0)
  {
    const auto tmpID   = rhID[irh];
    const auto tmpiphi = Common::DetIDMap[tmpID].i1;
    const auto tmpieta = Common::DetIDMap[tmpID].i2;

    const auto tmpT = rhtime[irh];
    const auto tmpE = rhE[irh];

    if (tmpiphi < miniphi) miniphi = tmpiphi;
    if (tmpiphi > maxiphi) maxiphi = tmpiphi;
    
    if (tmpieta < minieta) minieta = tmpieta;
    if (tmpieta > maxieta) maxieta = tmpieta;

    if (tmpT < minT) minT = tmpT;
    if (tmpT > maxT) maxT = tmpT;

    if (tmpE < minE) minE = tmpE;
    if (tmpE > maxE) maxE = tmpE;
  }

  // create buffer of 1 cell all around
  miniphi-=1; minieta-=1;
  maxiphi+=1; maxieta+=1;

  // make hists
  const auto MakeHist = [=](const TString & name, const TString & title, const TString & ztitle, const Float_t minz, const Float_t maxz)
  {
    auto hist = new TH2F(name.Data(),title.Data(),maxieta-minieta+1,minieta,maxieta+1,maxiphi-miniphi+1,miniphi,maxiphi+1); 
    hist->SetXTitle("i_{#eta}");
    hist->SetYTitle("i_{#phi}");
    hist->SetZTitle(ztitle.Data());
    
    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      Int_t label = hist->GetXaxis()->GetBinLowEdge(ibinX);
      hist->GetXaxis()->SetBinLabel(ibinX,Form("%i",label));
    }
    for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
    {
      Int_t label = hist->GetYaxis()->GetBinLowEdge(ibinY);
      hist->GetYaxis()->SetBinLabel(ibinY,Form("%i",label));
    }
    
    hist->SetMinimum(minz);
    hist->SetMaximum(maxz);

    return hist;
  };

  auto h_T = MakeHist("h_T","RecHit Time in SC","RecHit Time [ns]",minT,maxT);
  auto h_E = MakeHist("h_E","RecHit Energy in SC","RecHit Energy [GeV]",minE,maxE);

  for (const auto irh : phorecHits_0)
  {
    const auto tmpID   = rhID[irh];
    const auto tmpiphi = Common::DetIDMap[tmpID].i1;
    const auto tmpieta = Common::DetIDMap[tmpID].i2;

    const auto tmpT = rhtime[irh];
    const auto tmpE = rhE[irh];

    h_T->Fill(tmpieta,tmpiphi,tmpT);
    h_E->Fill(tmpieta,tmpiphi,tmpE);
  }
  
  // set zero content bins to -1e9.f;
  const auto FixHist = [](TH2F *& hist)
  {
    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      for (auto ibinY = 1; ibinY <= hist->GetYaxis()->GetNbins(); ibinY++)
      {
	const Float_t content = hist->GetBinContent(ibinX,ibinY);
	if (content == 0.f) hist->SetBinContent(ibinX,ibinY,-1000.f);
      }
    }
  };

  //  FixHist(h_T);
  //  FixHist(h_E);
  
  drawHists(h_T,h_E,outfile,outtext,"box");
  drawHists(h_T,h_E,outfile,outtext,"colz");

  // delete it all 
  delete h_E;
  delete h_T;
  delete outfile;
  delete tree;
  delete file;
  delete tdrStyle;
}
