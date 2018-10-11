#include "Common.cpp+"

#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TStyle.h"

#include <iostream>
#include <vector>
#include <map>

void ietaiphimap()
{
  // always set style
  auto tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // setup eras
  Common::SetupEras();

  // initialize ecal map
  Common::SetupDetIDs();

  // initialize tree and file + options
  auto file = TFile::Open("tmp_in/dispho_2.root");
  auto tree = (TTree*)file->Get("tree/disphotree");

  // initialize branches
  std::vector<Float_t> * rhtime_t = 0; tree->SetBranchAddress("rhtime", &rhtime_t);
  std::vector<Float_t> * rhE_t    = 0; tree->SetBranchAddress("rhE", &rhE_t);
  std::vector<UInt_t>  * rhID_t   = 0; tree->SetBranchAddress("rhID", &rhID_t);

  // photon stuff
  std::vector<Int_t> * phorecHits_0_t = 0; tree->SetBranchAddress("phorecHits_0", &phorecHits_0_t);
  Int_t phoseed_0 = 0; tree->SetBranchAddress("phoseed_0", &phoseed_0);

  // tree info:  disphotree->Scan("@phorecHits_0.size():phoseed_0:phoE_0","phohasPixSeed_0&&phohasPixSeed_1&&phoisEB_0&&phoisEB_1&&phogedID_0>=3&&phogedID_1>=3&&(sqrt(pow(phoE_0+phoE_1,2)-pow(phopt_0*cos(phophi_0)+phopt_1*cos(phophi_1),2)-pow(phopt_0*sin(phophi_0)+phopt_1*sin(phophi_1),2)-pow(phopt_0*sinh(phoeta_0)+phopt_1*sinh(phoeta_1),2))>60)&&(sqrt(pow(phoE_0+phoE_1,2)-pow(phopt_0*cos(phophi_0)+phopt_1*cos(phophi_1),2)-pow(phopt_0*sin(phophi_0)+phopt_1*sin(phophi_1),2)-pow(phopt_0*sinh(phoeta_0)+phopt_1*sinh(phoeta_1),2))<150)")

  // get entry
  UInt_t entry = 6892;
  tree->GetEntry(entry);

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

  auto h_T = MakeHist("h_T","RecHit Time in SC","[ns]",minT,maxT);
  auto h_E = MakeHist("h_E","RecHit Energy in SC","[GeV]",minE,maxE);

  // compute weighted time + fill hists
  Float_t wgtT = 0.0f;
  Float_t sumS = 0.0f;

  for (const auto irh : phorecHits_0)
  {
    const auto tmpID   = rhID[irh];
    const auto tmpiphi = Common::DetIDMap[tmpID].i1;
    const auto tmpieta = Common::DetIDMap[tmpID].i2;

    const auto tmpT = rhtime[irh];
    const auto tmpE = rhE[irh];

    h_T->Fill(tmpieta,tmpiphi,tmpT);
    h_E->Fill(tmpieta,tmpiphi,tmpE);

    if (tmpE > 120.f) continue;

    const auto inv_weight_2 = 1.f/(std::pow(Common::timefitN/tmpE,2)+2.f*std::pow(Common::timefitC,2));
    
    wgtT += tmpT*inv_weight_2;
    sumS += inv_weight_2;
  }
  // divide time weighted by weights to normalize
  wgtT /= sumS;
  std::cout << "seedtime: " << rhtime[phoseed_0] << " weighted time: " << wgtT << std::endl;
  
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

  FixHist(h_T);
  FixHist(h_E);
  
  auto canv = new TCanvas();
  canv->cd();

  // draw time
  h_T->Draw("colz"); // "box"
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,h_T->GetName());

  // draw energy
  h_E->Draw("colz"); // "box"
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,h_E->GetName());
}


// TOF Info
//   std::vector<Float_t> * rhphi_t  = 0; tree->SetBranchAddress("rhphi", &rhphi_t);
//   std::vector<Float_t> * rheta_t  = 0; tree->SetBranchAddress("rheta", &rheta_t);
//   Float_t vtxX = 0.f; tree->SetBranchAddress("vtxX", &vtxX);
//   Float_t vtxY = 0.f; tree->SetBranchAddress("vtxY", &vtxY);
//   Float_t vtxZ = 0.f; tree->SetBranchAddress("vtxZ", &vtxZ);
//   const auto rhphi = *rhphi_t;
//   const auto rheta = *rheta_t;
