// run by root -l -b -q macros/tdwgttime.C+

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
#include <unordered_map>

typedef std::unordered_map<int,int> rhIdMap;
typedef std::pair<Float_t,Float_t> ETPair; // time, energy
typedef std::vector<ETPair> ETPairVec;

static const Float_t zlow   = 76.0;
static const Float_t zhigh  = 106.0;
static const Float_t el1shift = 0.00169548; //ns (EB data)
static const Float_t el2shift = 0.00070326; //ns (EB data)
static const Float_t N     = 38.1; // ns
static const Float_t C     = 0.2439; // ns
static const Float_t sn    = 0.0513; // GeV
static const Float_t sol   = 29.9792458; // cm / ns
static const Float_t etaEB = 1.4442; // eta boundary of EB
static const Float_t rhEcut = 1.0; // rh Energy cut
static const Float_t scEcut = 0.0; // rh Energy cut
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

void tdwgttime()
{
  gStyle->SetOptStat("emou");

  // initialize tree and file + options
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  // initialize branches

  // el1
  std::vector<Float_t> * el1rhtimes = 0; tree->SetBranchAddress("el1rhtimes", &el1rhtimes);
  std::vector<Float_t> * el1rhEs    = 0; tree->SetBranchAddress("el1rhEs", &el1rhEs);
  std::vector<Float_t> * el1rhXs    = 0; tree->SetBranchAddress("el1rhXs", &el1rhXs);
  std::vector<Float_t> * el1rhYs    = 0; tree->SetBranchAddress("el1rhYs", &el1rhYs);
  std::vector<Float_t> * el1rhZs    = 0; tree->SetBranchAddress("el1rhZs", &el1rhZs);
  std::vector<Int_t>   * el1rhids   = 0; tree->SetBranchAddress("el1rhids", &el1rhids);
  Int_t el1nrh; tree->SetBranchAddress("el1nrh", &el1nrh);
  Float_t el1scE; tree->SetBranchAddress("el1scE", &el1scE);
  Float_t el1seedtime; tree->SetBranchAddress("el1seedtime", &el1seedtime);
  Float_t el1seedE; tree->SetBranchAddress("el1seedE", &el1seedE);
  Float_t el1seedX; tree->SetBranchAddress("el1seedX", &el1seedX);
  Float_t el1seedY; tree->SetBranchAddress("el1seedY", &el1seedY);
  Float_t el1seedZ; tree->SetBranchAddress("el1seedZ", &el1seedZ);
  Float_t el1eta; tree->SetBranchAddress("el1eta", &el1eta);
  Float_t el1phi; tree->SetBranchAddress("el1phi", &el1phi);

  // el2
  std::vector<Float_t> * el2rhtimes = 0; tree->SetBranchAddress("el2rhtimes", &el2rhtimes);
  std::vector<Float_t> * el2rhEs    = 0; tree->SetBranchAddress("el2rhEs", &el2rhEs);
  std::vector<Float_t> * el2rhXs    = 0; tree->SetBranchAddress("el2rhXs", &el2rhXs);
  std::vector<Float_t> * el2rhYs    = 0; tree->SetBranchAddress("el2rhYs", &el2rhYs);
  std::vector<Float_t> * el2rhZs    = 0; tree->SetBranchAddress("el2rhZs", &el2rhZs);
  std::vector<Int_t>   * el2rhids   = 0; tree->SetBranchAddress("el2rhids", &el2rhids);
  Int_t el2nrh; tree->SetBranchAddress("el2nrh", &el2nrh);
  Float_t el2scE; tree->SetBranchAddress("el2scE", &el2scE);
  Float_t el2seedtime; tree->SetBranchAddress("el2seedtime", &el2seedtime);
  Float_t el2seedE; tree->SetBranchAddress("el2seedE", &el2seedE);
  Float_t el2seedX; tree->SetBranchAddress("el2seedX", &el2seedX);
  Float_t el2seedY; tree->SetBranchAddress("el2seedY", &el2seedY);
  Float_t el2seedZ; tree->SetBranchAddress("el2seedZ", &el2seedZ);
  Float_t el2eta; tree->SetBranchAddress("el2eta", &el2eta);
  Float_t el2phi; tree->SetBranchAddress("el2phi", &el2phi);

  // vtx
  Float_t vtxX; tree->SetBranchAddress("vtxX", &vtxX);
  Float_t vtxY; tree->SetBranchAddress("vtxY", &vtxY);
  Float_t vtxZ; tree->SetBranchAddress("vtxZ", &vtxZ);
  
  // for selection
  Float_t zmass; tree->SetBranchAddress("zmass", &zmass);
  Bool_t hltdoubleel; tree->SetBranchAddress("hltdoubleel", &hltdoubleel);
  Int_t el1pid; tree->SetBranchAddress("el1pid", &el1pid);
  Int_t el2pid; tree->SetBranchAddress("el2pid", &el2pid);

  // output histograms
  TH1F * h_tdseedtimeTOF = new TH1F("h_tdseedtimeTOF","td seed time [TOF]",100,-5.0,5.0); h_tdseedtimeTOF->Sumw2();
  TH1F * h_tdweighttime  = new TH1F("h_tdweighttime","td weighted time",100,-5.0,5.0); h_tdweighttime->Sumw2();
  TH1F * h_tddifftime    = new TH1F("h_tddifftime","td weighted time - td seed time [TOF]",100,-5.0,5.0); h_tddifftime->Sumw2();

  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    tree->GetEntry(entry);

    if (zmass < zlow || zmass > zhigh || !hltdoubleel || (el1pid != -el2pid)) continue; // normal selection
        
    const Float_t el1seedeta = eta(el1seedX,el1seedY,el1seedZ); const Float_t el2seedeta = eta(el2seedX,el2seedY,el2seedZ);  
    if (std::abs(el1seedeta) > etaEB || std::abs(el2seedeta) > etaEB) continue; // EBEB events only
    
    if (el1nrh <= 0 || el1scE < scEcut || el2nrh <= 0 || el2scE < scEcut) continue; // SC selection

    ETPairVec el1etrhpairs;
    rhIdMap el1rhIds;
    for (Int_t rh = 0; rh < el1nrh; rh++)
    {
      if (el1rhIds.count((*el1rhids)[rh]) > 0) continue;
      el1rhIds[(*el1rhids)[rh]]++;

      if ((*el1rhEs)[rh] < rhEcut) continue; // 1 GeV cut on recHit times
      
      const Float_t rhX = (*el1rhXs)[rh]; const Float_t rhY = (*el1rhYs)[rh]; const Float_t rhZ = (*el1rhZs)[rh];  
      const Float_t rhphi = phi(rhX,rhY); const Float_t rheta = eta(rhX, rhY, rhZ);
      if (deltaR(el1eta,rheta,el1phi,rhphi) > dRcut) continue; 
      
      el1etrhpairs.push_back( std::make_pair( TOF( rhX, rhY, rhZ, vtxX, vtxY, vtxZ, (*el1rhtimes)[rh] ) - el1shift , (*el1rhEs)[rh] ) );
    }

    ETPairVec el2etrhpairs;
    rhIdMap el2rhIds;
    for (Int_t rh = 0; rh < el2nrh; rh++)
    {
      if (el2rhIds.count((*el2rhids)[rh]) > 0) continue;
      el2rhIds[(*el2rhids)[rh]]++;

      if ((*el2rhEs)[rh] < rhEcut) continue; // 1 GeV cut on recHit times
      
      const Float_t rhX = (*el2rhXs)[rh]; const Float_t rhY = (*el2rhYs)[rh]; const Float_t rhZ = (*el2rhZs)[rh];  
      const Float_t rhphi = phi(rhX,rhY); const Float_t rheta = eta(rhX, rhY, rhZ);
      if (deltaR(el2eta,rheta,el2phi,rhphi) > dRcut) continue; 
      
      el2etrhpairs.push_back( std::make_pair( TOF( rhX, rhY, rhZ, vtxX, vtxY, vtxZ, (*el2rhtimes)[rh] ) - el2shift , (*el2rhEs)[rh] ) );
    }

    //    if (el1etrhpairs.size() < nrhcut || el2etrhpairs.size() < nrhcut) continue;

    const Float_t el1seedtimeTOF = TOF(el1seedX,el1seedY,el1seedZ,vtxX,vtxY,vtxZ,el1seedtime) - el1shift;
    const Float_t el2seedtimeTOF = TOF(el2seedX,el2seedY,el2seedZ,vtxX,vtxY,vtxZ,el2seedtime) - el2shift;
    const Float_t tdseedtimeTOF  = el1seedtimeTOF-el2seedtimeTOF;
    h_tdseedtimeTOF->Fill(tdseedtimeTOF);

    const Float_t el1weighttime = el1etrhpairs.size() > 0 ? WeightTime(el1etrhpairs) : -1000.0f;
    const Float_t el2weighttime = el2etrhpairs.size() > 0 ? WeightTime(el2etrhpairs) : -2000.0f;
    const Float_t tdweighttime  = el1weighttime-el2weighttime;
    h_tdweighttime->Fill(tdweighttime);
    h_tddifftime->Fill(tdweighttime-tdseedtimeTOF);
  }

  TFile * outfile = new TFile("tdouttimes.root","UPDATE");
  outfile->cd();

  h_tdseedtimeTOF->Write(h_tdseedtimeTOF->GetName(),TObject::kWriteDelete);
  h_tdweighttime->Write(h_tdweighttime->GetName(),TObject::kWriteDelete);
  h_tddifftime->Write(h_tddifftime->GetName(),TObject::kWriteDelete);
}
