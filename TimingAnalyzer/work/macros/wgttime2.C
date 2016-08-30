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
#include <map>

typedef std::map<int,int> rhIdMap;
typedef std::pair<Float_t,Float_t> ETPair; // time, energy
typedef std::vector<ETPair> ETPairVec;

static const Float_t N     = 38.1; // ns
static const Float_t C     = 0.2439; // ns
static const Float_t sn    = 0.0513; // GeV
static const Float_t sol   = 29.9792458; // cm / ns
static const Float_t etaEB = 1.4442; // eta boundary of EB
static const Float_t rhEcut = 1.0; // rh Energy cut
static const Float_t dRcut  = 0.3; // isolation value -- deltaR cut on rh
static const UInt_t  nrhcut = 10; // n good rec hits cut

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
inline Float_t TOF   (Float_t x, Float_t y, Float_t z, Float_t vx, Float_t vy, Float_t vz, Float_t time)
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

void wgttime2()
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
  std::vector<Int_t>   * rhids;tree->SetBranchAddress(Form("%srhids",el.Data()), &rhids);

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

  Float_t eleta; tree->SetBranchAddress(Form("%seta",el.Data()), &eleta);
  Float_t elphi; tree->SetBranchAddress(Form("%sphi",el.Data()), &elphi);
  
  // output histograms
  TH1F * h_seedtimeTOF = new TH1F("h_seedtimeTOF",Form("%s seed time [TOF]",el.Data()),100,-5.0,5.0); h_seedtimeTOF->Sumw2();
  TH1F * h_weighttime  = new TH1F("h_weighttime",Form("%s weighted time",el.Data()),100,-5.0,5.0); h_weighttime->Sumw2();
  TH1F * h_difftime    = new TH1F("h_difftime",Form("%s weighted time - seed time [TOF]",el.Data()),200,-25.0,25.0); h_difftime->Sumw2();

  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    tree->GetEntry(entry);
    
    const Float_t seedeta = eta(seedX,seedY,seedZ);
    if (std::abs(seedeta) > etaEB || nrh < 0 || scE < 30.0f) continue;

    ETPairVec etrhpairs;
    rhIdMap rhIds;
    for (Int_t rh = 0; rh < nrh; rh++)
    {
      if (rhIds.count((*rhids)[rh]) > 0) continue;
      rhIds[(*rhids)[rh]]++;

      if ((*rhEs)[rh] < rhEcut) continue; // 1 GeV cut on recHit times
      
      const Float_t rhX = (*rhXs)[rh]; const Float_t rhY = (*rhYs)[rh]; const Float_t rhZ = (*rhZs)[rh];  
      const Float_t rhphi = phi(rhX,rhY); const Float_t rheta = eta(rhX, rhY, rhZ);
      if (deltaR(eleta,rheta,elphi,rhphi) > dRcut) continue; 
      
      etrhpairs.push_back( std::make_pair( TOF( rhX, rhY, rhZ, vtxX, vtxY, vtxZ, (*rhtimes)[rh] ) , (*rhEs)[rh] ) );
    }

    //    if (etrhpairs.size() < nrhcut) continue;

    const Float_t seedtimeTOF = TOF(seedX,seedY,seedZ,vtxX,vtxY,vtxZ,seedtime);
    h_seedtimeTOF->Fill(seedtimeTOF);

    const Float_t weighttime = WeightTime(etrhpairs);
    h_weighttime->Fill(weighttime);
    h_difftime->Fill(weighttime-seedtimeTOF);
  }

  TFile * outfile = new TFile("outtimes.root","UPDATE");
  outfile->cd();

  h_seedtimeTOF->Write(h_seedtimeTOF->GetName(),TObject::kWriteDelete);
  h_weighttime->Write(h_weighttime->GetName(),TObject::kWriteDelete);
  h_difftime->Write(h_difftime->GetName(),TObject::kWriteDelete);
}
