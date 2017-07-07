#include "TSystem.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"

#include <cmath>
#include <vector>
#include <map>
#include <utility>

// global functions

void CMSLumi(TCanvas *& canv, TString extraText = "", Int_t iPosX = 10);
void SetTDRStyle(TStyle *& tdrStyle);

inline Float_t ineta (const Float_t eta){return std::tan(2.f*std::atan(std::exp(-eta)));}
inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z){return x*x + y*y + z*z;}
inline Float_t gamma(const Float_t p, const Float_t m){return std::sqrt(1.f+std::pow(p/m,2));}
inline Float_t bg   (const Float_t p, const Float_t m){return std::abs(p/m);}
inline void    semiR(const Float_t insmaj, const Float_t insmin, Float_t & smaj, Float_t & smin)
{
  smaj = 1.f/std::sqrt(insmin);
  smin = 1.f/std::sqrt(insmaj);
}

inline Float_t mphi(Float_t phi)
{
  while (phi >= TMath::Pi()) phi -= TMath::TwoPi();
  while (phi < -TMath::Pi()) phi += TMath::TwoPi();
  return phi;
}

inline Float_t deltaR(const Float_t phi1, const Float_t eta1, const Float_t phi2, const Float_t eta2)
{
  const Float_t deta = eta1-eta2;
  const Float_t dphi = mphi(phi1-phi2);
  return std::sqrt(deta*deta+dphi*dphi);
}

inline void makeOutDir(TString outdir)
{
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(outdir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",outdir.Data());
    gSystem->Exec(mkDir.Data());
  }
}

// global namespaces

namespace ECAL
{
  static const Float_t etaEB = 1.4442;
  static const Float_t etaEEmin = 1.566;
  static const Float_t etaEEmax = 2.5;

  static const Float_t rEB = 129.f; // 1.29 m
  static const Float_t zEB = rEB / ineta(etaEB);

  static const Float_t zEE = 314.f; // 3.14 m 
  static const Float_t rEEmin = zEE * ineta(etaEEmin);
  static const Float_t rEEmax = zEE * ineta(etaEEmax);
};

// global variables

static const Float_t sol = 2.99792458e8; // cm/s

// global typedefs

typedef std::pair<TH1F*,TH1F*>     TH1Pair;
typedef std::map<TString, TH1Pair> TH1PairMap;
typedef TH1PairMap::iterator       TH1PairMapIter;

typedef std::pair<TH2F*,TH2F*>     TH2Pair;
typedef std::map<TString, TH2Pair> TH2PairMap;
typedef TH2PairMap::iterator       TH2PairMapIter;

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

typedef std::map<TString,Int_t> TStrIntMap;
typedef TStrIntMap::iterator    TStrIntMapIter;

typedef std::map<Int_t,Int_t> IntMap;
typedef IntMap::iterator      IntMapIter;

typedef std::vector<TH1F*> TH1FVec;
typedef std::vector<TH2F*> TH2FVec;
typedef std::vector<Bool_t> BoolVec;
