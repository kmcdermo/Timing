#include "TString.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TPaveText.h"
#include "common/common.C"

#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>

namespace Config
{
  static const Float_t etaEB      = 1.4442;
  static const Float_t etaEElow   = 1.566;
  static const Float_t etaEEhigh  = 2.5;
  static const Bool_t  useSigma_n = true;
};

struct EcalID
{
  EcalID(){}
  EcalID(Int_t i1, Int_t i2, TString name) : i1_(i1), i2_(i2), name_(name) {}
  Int_t i1_; // iphi (EB) or ix (EE)
  Int_t i2_; // ieta (EB) or iy (EE)
  TString name_;
};
typedef std::unordered_map<Int_t,EcalID> EcalIDMap;

struct IOVPair // Interval of Validty object --> run beginning through run end
{
  IOVPair(Int_t beg, Int_t end) : beg_(beg), end_(end) {}
  Int_t beg_;
  Int_t end_;
};
typedef std::vector<IOVPair> IOVPairVec;

struct ADC2GeVPair // Interval of Validty object --> run beginning through run end
{
  ADC2GeVPair(float EB, float EE) : EB_(EB), EE_(EE) {}
  float EB_;
  float EE_;
};
typedef std::vector<ADC2GeVPair> ADC2GeVPairVec;

typedef std::unordered_map<Int_t,Float_t> IDNoiseMap;
typedef std::vector<IDNoiseMap>           IDNoiseMapVec;

inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t theta(const Float_t r, const Float_t z){return std::atan2(r,z);}
inline Float_t eta  (const Float_t x, const Float_t y, const Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline Float_t effA (const Float_t e1, const Float_t e2){return e1*e2/std::sqrt(rad2(e1,e2));}

void quickcms();
void GetDetIDs(EcalIDMap &);
void GetPedestalNoise(IOVPairVec &, IDNoiseMapVec &);
void GetADC2GeVConvs(IOVPairVec &, ADC2GeVPairVec &);
