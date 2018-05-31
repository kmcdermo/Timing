#ifndef __gencalc__
#define __gencalc__

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TLorentzVector.h"
#include "TH1F.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TF1.h"
#include "TFormula.h"

#include <iostream>
#include <fstream>
#include <vector>

struct GenStruct
{
  // branches and values
  Float_t     genNmass = 0; 
  TBranch * b_genNmass = 0;
  TString   s_genNmass = "genNmass";

  Float_t     genNE = 0; 
  TBranch * b_genNE = 0;
  TString   s_genNE = "genNE";

  Float_t     genNpt = 0; 
  TBranch * b_genNpt = 0; 
  TString   s_genNpt = "genNpt";

  Float_t     genNphi = 0; 
  TBranch * b_genNphi = 0; 
  TString   s_genNphi = "genNphi";

  Float_t     genNeta = 0; 
  TBranch * b_genNeta = 0;
  TString   s_genNeta = "genNeta";

  Float_t     genNprodvx = 0; 
  TBranch * b_genNprodvx = 0;
  TString   s_genNprodvx = "genNprodvx";

  Float_t     genNprodvy = 0; 
  TBranch * b_genNprodvy = 0;
  TString   s_genNprodvy = "genNprodvy";

  Float_t     genNprodvz = 0; 
  TBranch * b_genNprodvz = 0;
  TString   s_genNprodvz = "genNprodvz";
  
  Float_t     genNdecayvx = 0; 
  TBranch * b_genNdecayvx = 0; 
  TString   s_genNdecayvx = "genNdecayvx";

  Float_t     genNdecayvy = 0; 
  TBranch * b_genNdecayvy = 0; 
  TString   s_genNdecayvy = "genNdecayvy";

  Float_t     genNdecayvz = 0; 
  TBranch * b_genNdecayvz = 0; 
  TString   s_genNdecayvz = "genNdecayvz";

  Float_t     genphE = 0; 
  TBranch * b_genphE = 0; 
  TString   s_genphE = "genphE";
  
  Float_t     genphpt = 0; 
  TBranch * b_genphpt = 0;
  TString   s_genphpt = "genphpt";
  
  Float_t     genphphi = 0;
  TBranch * b_genphphi = 0;
  TString   s_genphphi = "genphphi";

  Float_t     genpheta = 0; 
  TBranch * b_genpheta = 0; 
  TString   s_genpheta = "genpheta";
};

namespace Config
{
  static const Float_t sol  = 2.99792458e10; // cm/s
  static const Float_t ns_per_s = 1e9; 
  static const Float_t cm_per_m = 1e2; 
};

inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z = 0.f){return x*x + y*y + z*z;}
inline Float_t hypo (const Float_t x, const Float_t y, const Float_t z = 0.f){return std::sqrt(rad2(x,y,z));}
inline Float_t theta(const Float_t eta){return 2.f*std::atan(std::exp(-eta));}
inline Float_t slope_of_eta(const Float_t eta){return std::tan(theta(eta));}
inline Float_t r_of_eta(const Float_t eta, const Float_t z){return z * slope_of_eta(eta);}
inline Float_t z_of_eta(const Float_t eta, const Float_t r){return r / slope_of_eta(eta);}
inline Float_t gamma(const Float_t p, const Float_t m){return std::sqrt(1.f+std::pow(p/m,2));}
inline Float_t betag(const Float_t p, const Float_t m){return std::abs(p/m);}
inline Float_t tau  (const Float_t ctau, const Float_t gamma){return ctau * gamma / Config::sol;}

namespace ECAL
{
  static const Float_t etaEB    = 1.4442;
  static const Float_t etaEEmin = 1.566;
  static const Float_t etaEEmax = 2.5;

  static const Float_t rEB = 129.f; // 1.29 m
  static const Float_t zEB = z_of_eta(ECAL::etaEB,ECAL::rEB);

  static const Float_t zEE    = 314.f; // 3.14 m 
  static const Float_t rEEmin = r_of_eta(ECAL::etaEEmax,ECAL::zEE);
  static const Float_t rEEmax = r_of_eta(ECAL::etaEEmin,ECAL::zEE);
};

void gencalc();
Float_t GetGenPhotonArrivalTime(const Float_t r0, const Float_t z0, const Float_t slope, const Float_t tau0, Float_t& arrival);

#endif
