#ifndef _config_
#define _config_

#include "TString.h"

namespace Config{
  constexpr    Float_t PI = 3.14159265358979323846;

  constexpr    Float_t lumi      = 2.301;
  static const TString outtype   = "png";
  static const TString extraText = "Preliminary";
  constexpr    Int_t   nvtxbins  = 50;
  constexpr    Float_t zlow      = 76.;
  constexpr    Float_t zup       = 106.;

  constexpr    UInt_t  demoNum   = 1000;

  extern TString outdir;
  extern Bool_t  doPURW;
  extern Bool_t  doAnalysis;
  extern Bool_t  doStacks;
  extern Bool_t  doDemo;
};

#endif
