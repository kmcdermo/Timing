#ifndef _config_
#define _config_

#include "TString.h"
#include "TColor.h"

#include <map>
#include <vector>

typedef std::vector<Double_t>      DblVec;
typedef std::map<TString,Color_t>  ColorMap;
typedef std::map<TString,TString>  TStrMap;
typedef std::map<TString,Bool_t>   TStrBoolMap;
typedef std::map<TString,Float_t>  TStrFltMap;
typedef std::map<TString,Double_t> TStrDblMap;
typedef std::map<TString,DblVec>   TStrDblVMap;

namespace Config{
  // general config
  constexpr    Float_t PI  = 3.14159265358979323846;
  constexpr    Float_t sol = 29.9792458; // speed of light in cm / ns
  constexpr    Float_t sigma_n = 0.2; // 200 MeV

  // output config
  constexpr    Float_t lumi      = 2.301; // brilcalc lumi --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/moriond16_normtag.json -i rereco2015D.txt -u /fb
  static const TString outtype   = "png";
  static const TString extraText = "Preliminary";

  // pu config
  constexpr    Int_t   nbinsvtx   = 50;
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
  constexpr    Int_t    nEventsCut = 1000; // for run number plots

  // E/pt el1/e2/eff stuff
  constexpr    Double_t el1E_high = 2100.0;
  constexpr    Double_t el2E_high = 910.0;
  constexpr    Double_t effE_high = 725.0;

  constexpr    Double_t EtoPt    = 3.0;
  constexpr    Double_t EtoSeedE = 4.0 / 3.0;

  // TDRStyle Config --> needed by stacker
  constexpr    Float_t  TitleSize    = 0.035;
  constexpr    Float_t  TitleXOffset = 1.1;
  constexpr    Float_t  TitleYOffset = 1.1;

  constexpr    Float_t  LabelOffset = 0.007;
  constexpr    Float_t  LabelSize   = 0.03;

  constexpr    Float_t  TickLength  = 0.03;

  constexpr    Float_t  TitleFF     = 1.4; // offset fudge factor
  
  // stacker config: lp = lower pad, up = upper pad
  constexpr    Float_t  left_up   = 0.0;
  constexpr    Float_t  bottom_up = 0.3;
  constexpr    Float_t  right_up  = 1.0;
  constexpr    Float_t  top_up    = 0.99;
  constexpr    Float_t  height_up = top_up - bottom_up;

  constexpr    Float_t  left_lp   = left_up;
  constexpr    Float_t  bottom_lp = 0.05;
  constexpr    Float_t  right_lp  = right_up;
  constexpr    Float_t  top_lp    = bottom_up;
  constexpr    Float_t  height_lp = top_lp - bottom_lp;

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
  extern Bool_t      useSEL;
  extern Bool_t      useDYll;
  extern Bool_t      useQCD;
  extern Bool_t      useGJets;
  extern Bool_t      doRuns;
  extern Bool_t      useFull;
  extern Bool_t      doStandard;
  extern Bool_t      doTimeRes;
  extern Bool_t      doTrigEff;
  extern Bool_t      applyTOF;
  extern Bool_t      wgtedtime;
  extern Bool_t      useSigma_n;
  extern Bool_t      saveFits;
  extern TString     formname; // fitting function to be used

  extern TStrBoolMap SampleMap;
  extern ColorMap    colorMap;
  extern TStrMap     SampleTitleMap;
  extern TStrFltMap  SampleXsecMap;
  extern TStrFltMap  SampleWgtsumMap;
  extern TStrMap     XTitleMap;
  extern TStrDblMap  XHighMap;
  extern TStrDblVMap XBinsMap;
};

#endif
