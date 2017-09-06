#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

inline Float_t phi  (Float_t x, Float_t y){return std::atan2(y,x);}
inline Float_t rad2 (Float_t x, Float_t y){return x*x + y*y;}
inline Float_t theta(Float_t r, Float_t z){return std::atan2(r,z);}
inline Float_t eta  (Float_t x, Float_t y, Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}

void etaphimap()
{
  // initialize tree and file + options
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TString el = "el1";

  // initialize branches !!!!!!!!!!! ALWAYS SET VECTOR OF BRANCHES TO ZERO OR SEG FAULTS BECAUSE ROOT IS PIECE OF %^&*!
  std::vector<Float_t> * rhEs = 0; tree->SetBranchAddress(Form("%srhEs",el.Data()), &rhEs);
  std::vector<Float_t> * rhXs = 0; tree->SetBranchAddress(Form("%srhXs",el.Data()), &rhXs);
  std::vector<Float_t> * rhYs = 0; tree->SetBranchAddress(Form("%srhYs",el.Data()), &rhYs);
  std::vector<Float_t> * rhZs = 0; tree->SetBranchAddress(Form("%srhZs",el.Data()), &rhZs);  
  Int_t   nrh   = 0;               tree->SetBranchAddress(Form("%snrh",el.Data()),&nrh);
  Float_t seedE = 0.0;             tree->SetBranchAddress(Form("%sseedE",el.Data()),&seedE);

  // output histograms
  UInt_t entry = 1213;
  tree->GetEntry(entry);

  Float_t minphi = 1000.f, maxphi = -1000.f;
  Float_t mineta = 1000.f, maxeta = -1000.f;
  Float_t maxE = -1.f;
  for (Int_t rh = 0; rh < nrh; rh++)
  {
    const Float_t tmpE = (*rhEs)[rh];
    const Float_t tmpX = (*rhXs)[rh]; 
    const Float_t tmpY = (*rhYs)[rh]; 
    const Float_t tmpZ = (*rhZs)[rh]; 

    const Float_t tmpphi = phi(tmpX,tmpY); 
    const Float_t tmpeta = eta(tmpX,tmpY,tmpZ);

    if (tmpphi > maxphi) maxphi = tmpphi;
    if (tmpphi < minphi) minphi = tmpphi;
    if (tmpeta > maxeta) maxeta = tmpeta;
    if (tmpeta < mineta) mineta = tmpeta;

    if (tmpE > maxE) maxE = tmpE;
  }

  std::cout << "maxE: " << maxE << " seedE: " << seedE << std::endl;
  std::cout << "minphi: " << minphi << " maxphi: " << maxphi << std::endl;
  std::cout << "mineta: " << mineta << " maxeta: " << maxeta << std::endl;

  minphi = (minphi > 0 ? minphi-0.1 : minphi+0.1);
  maxphi = (maxphi > 0 ? minphi+0.1 : minphi-0.1);
  mineta = (mineta > 0 ? mineta-0.1 : mineta+0.1);
  maxeta = (maxeta > 0 ? mineta+0.1 : mineta-0.1);

  TH2F * h_map = new TH2F("h_map","Energy Deposit in ECAL (GeV)",20,mineta,maxeta,20,minphi,maxphi);
  h_map->SetXTitle("#eta"); h_map->SetYTitle("#phi"); h_map->SetMinimum(0.0);
  for (Int_t rh = 0; rh < nrh; rh++)
  {
    const Float_t tmpE = (*rhEs)[rh];
    const Float_t tmpX = (*rhXs)[rh]; 
    const Float_t tmpY = (*rhYs)[rh]; 
    const Float_t tmpZ = (*rhZs)[rh]; 

    const Float_t tmpphi = phi(tmpX,tmpY); 
    const Float_t tmpeta = eta(tmpX,tmpY,tmpZ);

    h_map->Fill(tmpeta,tmpphi,tmpE);
  }

  h_map->Draw("colz");
  h_map->Draw("box");
}
