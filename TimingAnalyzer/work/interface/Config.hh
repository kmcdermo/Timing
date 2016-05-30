#ifndef _config_
#define _config_

#include "TString.h"
#include "TColor.h"

#include <map>

typedef std::map<TString,Color_t> ColorMap;
typedef std::map<TString,TString> TStrMap;
typedef std::map<TString,Bool_t>  TStrBoolMap;
typedef std::map<TString,Float_t> TStrFltMap;

namespace Config{
  // general config
  constexpr    Float_t PI = 3.14159265358979323846;

  // output config
  constexpr    Float_t lumi      = 2.301; // brilcalc lumi --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/moriond16_normtag.json -i rereco2015D.txt -u /fb
  static const TString outtype   = "png";
  static const TString extraText = "Preliminary";

  // pu config
  constexpr    Int_t   nvtxbins   = 50;
  static const TString pusubdir   = "purw";
  static const TString pufilename = "PURW.root";
  static const TString puplotname = "nvtx_dataOverMC";

  // selection config
  constexpr    Float_t zlow      = 76.;
  constexpr    Float_t zhigh     = 106.;
  static const TString selection = Form("(zmass>%f && zmass<%f) && hltdoubleel && (el1pid == -el2pid)",zlow,zhigh);

  // data config
  static const TString plotdumpname = "plotnames.txt";
  static const TString runs         = "config/runs2015D.txt";

  // plot config
  constexpr    Int_t    ntimebins  = 100;
  constexpr    Double_t timerange  = 5.0;
  constexpr    Double_t fitrange   = 3.0;
  constexpr    Int_t    nEventsCut = 100; // for run number plots

  // ECAL config
  constexpr    Float_t etaEB     = 1.4442;
  constexpr    Float_t etaEElow  = 1.566;
  constexpr    Float_t etaEEhigh = 2.5;

  // if in demo mode
  constexpr    UInt_t demoNum = 1000;

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
  extern Bool_t      skipRuns;
  extern Bool_t      useFull;
  extern Bool_t      doStandard;
  extern Bool_t      doTimeRes;
  extern Bool_t      doTrigEff;

  extern TStrBoolMap SampleMap;
  extern ColorMap    colorMap;
  extern TStrMap     SampleTitleMap;
  extern TStrFltMap  SampleXsecMap;
  extern TStrFltMap  SampleWgtsumMap;
};

#endif
