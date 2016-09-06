#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1F.h"
#include "TObject.h"
#include "TStyle.h"

#include <iostream>
#include <vector>
#include <map>
#include <fstream>

static const Float_t zlow   = 76.0;
static const Float_t zhigh  = 106.0;
static const Int_t   nrhcut = 0; // nRecHits cut
static const Float_t scEcut = 0.0; // super cluster E cut
static const Float_t etaEB  = 1.4442; // eta boundary of EB
static const Float_t rhEcut = 1.0; // rh Energy cut
static const Float_t dRcut  = 0.3; // isolation value -- deltaR cut on rh

inline Float_t rad2  (Float_t x, Float_t y){return x*x + y*y;}
inline Float_t theta (Float_t r, Float_t z){return std::atan2(r,z);}
inline Float_t eta   (Float_t x, Float_t y, Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.0f));
}
inline Float_t phi   (Float_t x, Float_t y){return std::atan2(y,x);}
inline Float_t deltaR(Float_t eta1, Float_t eta2, Float_t phi1, Float_t phi2)
{
  return std::sqrt(rad2(eta2-eta1,phi1-phi2));
}

void nrechits()
{
  // initialize tree and file + options
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TString el = "el1";

  // initialize branches
  std::vector<Float_t> * rhEs = 0; tree->SetBranchAddress(Form("%srhEs",el.Data()), &rhEs);
  std::vector<Float_t> * rhXs = 0; tree->SetBranchAddress(Form("%srhXs",el.Data()), &rhXs);
  std::vector<Float_t> * rhYs = 0; tree->SetBranchAddress(Form("%srhYs",el.Data()), &rhYs);
  std::vector<Float_t> * rhZs = 0; tree->SetBranchAddress(Form("%srhZs",el.Data()), &rhZs);
  std::vector<Int_t>   * rhids= 0; tree->SetBranchAddress(Form("%srhids",el.Data()), &rhids);

  Int_t nrh; tree->SetBranchAddress(Form("%snrh",el.Data()),&nrh);
  Float_t scE; tree->SetBranchAddress(Form("%sscE",el.Data()), &scE);

  Float_t seedX; tree->SetBranchAddress(Form("%sseedX",el.Data()), &seedX);
  Float_t seedY; tree->SetBranchAddress(Form("%sseedY",el.Data()), &seedY);
  Float_t seedZ; tree->SetBranchAddress(Form("%sseedZ",el.Data()), &seedZ);

  Float_t vtxX; tree->SetBranchAddress("vtxX", &vtxX);
  Float_t vtxY; tree->SetBranchAddress("vtxY", &vtxY);
  Float_t vtxZ; tree->SetBranchAddress("vtxZ", &vtxZ);

  Float_t eleta; tree->SetBranchAddress(Form("%seta",el.Data()), &eleta);
  Float_t elphi; tree->SetBranchAddress(Form("%sphi",el.Data()), &elphi);
  
  Float_t zmass; tree->SetBranchAddress("zmass", &zmass);
  Bool_t hltdoubleel; tree->SetBranchAddress("hltdoubleel", &hltdoubleel);
  Int_t el1pid; tree->SetBranchAddress("el1pid", &el1pid);
  Int_t el2pid; tree->SetBranchAddress("el2pid", &el2pid);

  // output histograms
  TH1F * h_nrhs       = new TH1F(Form("h_%snrhs",el.Data()),Form("%s nRecHits",el.Data()),500,0.,500.); h_nrhs->Sumw2();
  TH1F * h_nprunedrhs = new TH1F(Form("h_%snprunedrhs",el.Data()),Form("%s nRecHits (pruned)",el.Data()),100,0.,100.); h_nprunedrhs->Sumw2();
  TH1F * h_ratio      = new TH1F(Form("h_%sratio",el.Data()),Form("%s ratio of nRH (pruned) / nRH (total)",el.Data()),100,0.,1.); h_ratio->Sumw2();

  ofstream outputids;
  outputids.open("config/el1_rhids_20.txt",std::ios_base::app);
 
  //  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  for (UInt_t entry = 2549; entry < 2550; entry++)
  {
    tree->GetEntry(entry);
    
    if (zmass < zlow || zmass > zhigh || !hltdoubleel || (el1pid != -el2pid)) continue;

    const Float_t seedeta = eta(seedX,seedY,seedZ);
    if (std::abs(seedeta) > etaEB || nrh < nrhcut || scE < scEcut) continue;

    Int_t nprunedrh = 0;
    std::map<Int_t,Int_t> rhIds;
    for (Int_t rh = 0; rh < nrh; rh++)
    {
      if (rhIds.count((*rhids)[rh]) > 0) continue;
      rhIds[(*rhids)[rh]]++;

      if ((*rhEs)[rh] < rhEcut) continue; // 1 GeV cut on recHit times
      
      const Float_t rhX = (*rhXs)[rh]; const Float_t rhY = (*rhYs)[rh]; const Float_t rhZ = (*rhZs)[rh];  
      const Float_t rhphi = phi(rhX,rhY); const Float_t rheta = eta(rhX, rhY, rhZ);
      if (deltaR(eleta,rheta,elphi,rhphi) > dRcut) continue; 

      nprunedrh++;
      outputids << (*rhids)[rh] << std::endl;
    }

    h_nrhs->Fill(nrh);
    h_nprunedrhs->Fill(nprunedrh);
    h_ratio->Fill(Float_t(nprunedrh)/Float_t(nrh));
  }
  outputids.close();

  TFile * outfile = new TFile(Form("%soutrhs.root",el.Data()),"UPDATE");
  outfile->cd();

  h_nrhs->Write(h_nrhs->GetName(),TObject::kWriteDelete);
  h_nprunedrhs->Write(h_nprunedrhs->GetName(),TObject::kWriteDelete);
  h_ratio->Write(h_ratio->GetName(),TObject::kWriteDelete);
}
