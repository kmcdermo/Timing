#ifndef _config_
#define _config_

#include "TString.h"
#include "TColor.h"

#include <map>

typedef std::map<TString,Color_t> ColorMap;
typedef std::map<TString,TString> TStrMap;
typedef std::map<TString,Bool_t>  TStrBoolMap;

namespace Config{
  constexpr    Float_t PI = 3.14159265358979323846;

  constexpr    Float_t lumi      = 2.301;
  static const TString outtype   = "png";
  static const TString extraText = "Preliminary";
  constexpr    Int_t   nvtxbins  = 50;
  constexpr    Float_t zlow      = 76.;
  constexpr    Float_t zup       = 106.;
  static const TString runs      = "config/runs2015D.txt";

  constexpr    UInt_t  demoNum   = 1000;

  // set at command line and in main
  extern TString     outdir;
  extern Bool_t      doPURW;
  extern Bool_t      doAnalysis;
  extern Bool_t      doStacks;
  extern Bool_t      doDemo;
  extern Bool_t      useDEG;
  extern Bool_t      useDYll;
  extern Bool_t      useQCD;
  extern Bool_t      useGJets;

  extern TStrBoolMap SampleMap;
  extern ColorMap    colorMap;
  extern TStrMap     SampleTitleMap;
};

#endif
