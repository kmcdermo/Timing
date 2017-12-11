#ifndef __Common__
#define __Common__

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"

// global functions
void CheckValidFile(const TFile * file, const TString & fname);
void CheckValidTree(const TTree * tree, const TString & tname, const TString & fname);
void CheckValidTH1F(const TH1F * plot, const TString & pname, const TString & fname);
void CMSLumi(TCanvas * canv, const Int_t iPosX = 10);
void SetTDRStyle(TStyle * tdrStyle);

namespace Config
{
  // basics
  constexpr Float_t lumi = 41.96; // CMS lumi (as of Nov 30 PR JSON)
  constexpr Float_t invfbToinvpb = 1000.f; // conversion for the world's dumbest unit
  static const TString extraText = "Preliminary";

  // input
  static const TString baseDir = "/store/user/kmcdermo/nTuples/skims/2017";
  static const TString puwgtFileName = "puweights";
  static const TString puwgtHistName = "PUWeightsHist";
    
  // plotting style
  constexpr Float_t TitleSize    = 0.035;
  constexpr Float_t TitleXOffset = 1.1;
  constexpr Float_t TitleYOffset = 1.1;
  constexpr Float_t LabelOffset  = 0.007;
  constexpr Float_t LabelSize    = 0.03;
  constexpr Float_t TickLength   = 0.03;
  constexpr Float_t TitleFF      = 1.4; 
  constexpr Int_t   Ndivisions   = 505;
  
  // up = upper pad, lp = lower pad
  constexpr Float_t left_up   = 0.0;
  constexpr Float_t bottom_up = 0.3;
  constexpr Float_t right_up  = 1.0;
  constexpr Float_t top_up    = 0.99;
  constexpr Float_t height_up = top_up - bottom_up;

  constexpr Float_t left_lp   = left_up;
  constexpr Float_t bottom_lp = 0.05;
  constexpr Float_t right_lp  = right_up;
  constexpr Float_t top_lp    = bottom_up;
  constexpr Float_t height_lp = top_lp - bottom_lp;
};

#endif
