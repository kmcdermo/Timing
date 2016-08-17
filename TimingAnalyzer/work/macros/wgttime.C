// run by root -l -b -q macros/wgttime.C+

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1F.h"
#include "TObject.h"
#include "TStyle.h"

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>

typedef std::pair<Float_t,Float_t> ETPair; // time, energy
typedef std::vector<ETPair> ETPairVec;

static const Float_t N     = 33.2; // ns
static const Float_t C     = 0.154; // ns
static const Float_t sn    = 0.0513; // GeV
static const Float_t sol   = 29.9792458; // cm / ns
static const Float_t etaEB = 1.4442; // eta boundary of EB

inline Float_t rad2 (Float_t x, Float_t y){return x*x + y*y;}
inline Float_t theta(Float_t r, Float_t z){return std::atan2(r,z);}
inline Float_t eta  (Float_t x, Float_t y, Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline Float_t TOF  (Float_t x, Float_t y, Float_t z, Float_t vx, Float_t vy, Float_t vz, Float_t time)
{
  return time + (std::sqrt(z*z+rad2(x,y))-std::sqrt((z-vz)*(z-vz)+rad2((x-vx),(y-vy))))/sol;
}

Float_t WeightTime(const ETPairVec & etrhpairs)
{
  Float_t wgtT = 0.0f;
  Float_t sumS = 0.0f;
  for (UInt_t rh = 0; rh < etrhpairs.size(); rh++)
  {
    const ETPair & etrhpair = etrhpairs[rh];
    const Float_t tmpS = (N / ( etrhpair.second / sn)) + (std::sqrt(2.0f)*C);
    sumS += 1.0f / (tmpS*tmpS);
    wgtT += etrhpair.first / (tmpS*tmpS);
  }
  return wgtT / sumS;
}

void wgttime()
{
  gStyle->SetOptStat("emou");

  // initialize tree and file + options
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TString el = "el1";

  // initialize branches
  std::vector<Float_t> * rhtimes; tree->SetBranchAddress(Form("%srhtimes",el.Data()), &rhtimes);
  std::vector<Float_t> * rhEs; tree->SetBranchAddress(Form("%srhEs",el.Data()), &rhEs);
  std::vector<Float_t> * rhXs; tree->SetBranchAddress(Form("%srhXs",el.Data()), &rhXs);
  std::vector<Float_t> * rhYs; tree->SetBranchAddress(Form("%srhYs",el.Data()), &rhYs);
  std::vector<Float_t> * rhZs; tree->SetBranchAddress(Form("%srhZs",el.Data()), &rhZs);

  Int_t nrh; tree->SetBranchAddress(Form("%snrh",el.Data()),&nrh);

  Float_t scE; tree->SetBranchAddress(Form("%sscE",el.Data()), &scE);

  Float_t seedtime; tree->SetBranchAddress(Form("%sseedtime",el.Data()), &seedtime);
  Float_t seedE; tree->SetBranchAddress(Form("%sseedE",el.Data()), &seedE);
  Float_t seedX; tree->SetBranchAddress(Form("%sseedX",el.Data()), &seedX);
  Float_t seedY; tree->SetBranchAddress(Form("%sseedY",el.Data()), &seedY);
  Float_t seedZ; tree->SetBranchAddress(Form("%sseedZ",el.Data()), &seedZ);

  Float_t vtxX; tree->SetBranchAddress("vtxX", &vtxX);
  Float_t vtxY; tree->SetBranchAddress("vtxY", &vtxY);
  Float_t vtxZ; tree->SetBranchAddress("vtxZ", &vtxZ);
  
  // output histograms
  TH1F * h_seedtime    = new TH1F("h_seedtime",Form("%s seed time",el.Data()),200,-25.0,25.0);
  TH1F * h_seedtimeTOF = new TH1F("h_seedtimeTOF",Form("%s seed time [TOF]",el.Data()),200,-25.0,25.0);
  TH1F * h_rhtimes     = new TH1F("h_rhtimes",Form("%s recHits (E > 1 GeV) time",el.Data()),200,-25.0,25.0);
  TH1F * h_rhtimesTOF  = new TH1F("h_rhtimesTOF",Form("%s recHits (E > 1 GeV) time [TOF]",el.Data()),200,-25.0,25.0);
  TH1F * h_diffE       = new TH1F("h_diffE",Form("%s maxE - seedE",el.Data()),100,0.0,20.0);
  TH1F * h_weighttime  = new TH1F("h_weighttime",Form("%s weighted time",el.Data()),200,-25.0,25.0);
  TH1F * h_difftime    = new TH1F("h_difftime",Form("%s weighted time - seed time [TOF]",el.Data()),200,-25.0,25.0);

  TH1F * h_nrh_sc30     = new TH1F("h_nrh_sc30",Form("%s nRecHits (SCE > 30 GeV)",el.Data()),200,0,200);
  TH1F * h_nrh_sc30_rh1 = new TH1F("h_nrh_sc30_rh1",Form("%s nRecHits (SCE > 30 GeV, rhE > 1 GeV)",el.Data()),200,0,200);

  h_seedtime->Sumw2();
  h_seedtimeTOF->Sumw2();
  h_rhtimes->Sumw2();
  h_rhtimesTOF->Sumw2();
  h_diffE->Sumw2();
  h_weighttime->Sumw2();
  h_difftime->Sumw2();
  
  h_nrh_sc30->Sumw2();
  h_nrh_sc30_rh1->Sumw2();

  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    tree->GetEntry(entry);
    
    const Float_t seedeta = eta(seedX,seedY,seedZ);
    if (std::abs(seedeta) > etaEB || nrh < 0) continue;

    h_seedtime->Fill(seedtime);
    const Float_t seedtimeTOF = TOF(seedX,seedY,seedZ,vtxX,vtxY,vtxZ,seedtime);
    h_seedtimeTOF->Fill(seedtimeTOF);

    ETPairVec etrhpairs;
    Float_t maxE = 0, maxtime = 0;
    Int_t nrh_gt1 = 0;
    for (Int_t rh = 0; rh < nrh; rh++)
    {
      if ((*rhEs)[rh] < 1.f) continue; // 1 GeV cut on recHit times
      nrh_gt1++;

      h_rhtimes->Fill((*rhtimes)[rh]);
      etrhpairs.push_back( std::make_pair( TOF( (*rhXs)[rh], (*rhYs)[rh], (*rhZs)[rh], vtxX, vtxY, vtxZ, (*rhtimes)[rh] ) , (*rhEs)[rh] ) );
      h_rhtimesTOF->Fill(etrhpairs.back().first); 

      if (etrhpairs.back().second > maxE) { maxE = etrhpairs.back().second; maxtime = etrhpairs.back().first; } 
    }
    if (maxE != seedE) { h_diffE->Fill(maxE - seedE); std::cout << entry << std::endl; }

    const Float_t weighttime = WeightTime(etrhpairs);
    h_weighttime->Fill(weighttime);
    h_difftime->Fill(weighttime-seedtimeTOF);

    if (scE > 30.0) 
    {
      h_nrh_sc30->Fill(nrh);
      h_nrh_sc30_rh1->Fill(nrh_gt1);
    }
  }

  TFile * outfile = new TFile("outtimes.root","UPDATE");
  outfile->cd();

  h_seedtime->Write(h_seedtime->GetName(),TObject::kWriteDelete);
  h_seedtimeTOF->Write(h_seedtimeTOF->GetName(),TObject::kWriteDelete);
  h_rhtimes->Write(h_rhtimes->GetName(),TObject::kWriteDelete);
  h_rhtimesTOF->Write(h_rhtimesTOF->GetName(),TObject::kWriteDelete);
  h_diffE->Write(h_diffE->GetName(),TObject::kWriteDelete);
  h_weighttime->Write(h_weighttime->GetName(),TObject::kWriteDelete);
  h_difftime->Write(h_difftime->GetName(),TObject::kWriteDelete);

  h_nrh_sc30->Write(h_nrh_sc30->GetName(),TObject::kWriteDelete);
  h_nrh_sc30_rh1->Write(h_nrh_sc30_rh1->GetName(),TObject::kWriteDelete);
}
