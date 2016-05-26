#ifndef _config_
#define _config_

#include "TString.h"
#include "TColor.h"

#include <map>

typedef std::map<TString,Color_t> ColorMap;
typedef std::map<TString,TString> TStrMap;
typedef std::map<TString,Bool_t>  TStrBoolMap;

namespace Config{
  // general config
  constexpr    Float_t PI = 3.14159265358979323846;

  // output config
  constexpr    Float_t lumi      = 2.301;
  static const TString outtype   = "png";
  static const TString extraText = "Preliminary";

  // pu config
  constexpr    Int_t   nvtxbins   = 50;
  static const TString pusubdir   = "purw";
  static const TString pufilename = "PURW.root";
  static const TString puplotname = "nvtx_dataOverMC";

  // selection config
  constexpr    Float_t zlow      = 76.;
  constexpr    Float_t zup       = 106.;

  // data config
  static const TString plotdumpname = "plotnames.txt";
  static const TString runs         = "config/runs2015D.txt";

  // if in demo mode
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
