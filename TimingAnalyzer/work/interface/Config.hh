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
  constexpr    Float_t Sqrt2 = 1.4121356237; 
  constexpr    Float_t PI    = 3.14159265358979323846;
  constexpr    Float_t sol   = 29.9792458; // speed of light in cm / ns
  constexpr    Float_t sigma_nEB = 0.0513466; // 51 MeV
  constexpr    Float_t sigma_nEE = 0.126938; // 127 MeV
  constexpr    Float_t N_EB   = 38.1; // ns
  constexpr    Float_t C_EB   = 0.2439; // ns
  constexpr    Float_t rhEcut = 1.0; // rh Energy cut
  constexpr    Float_t dRcut  = 0.3; // isolation value -- deltaR cut on rh
  constexpr    UInt_t  nrhcut = 10; // n good rec hits cut

  // fitting config
  constexpr    Float_t ncore = 1.5;

  // output config
  constexpr    Float_t lumi      = 27.22; // inv fb for current 2016
  static const TString extraText = "Preliminary";

  // pu config
  constexpr    Int_t   nbinsvtx   = 50;
  static const TString pusubdir   = "purw";
  static const TString pufilename = "PURW.root";
  static const TString puplotname = "nvtx_dataOverMC";

  // selection config
  constexpr    Float_t zlow      = 76.;
  constexpr    Float_t zhigh     = 106.;
  static const TString selection = Form("(zmass>%f && zmass<%f) && (el1pid == -el2pid)",zlow,zhigh);

  // data config
  static const TString plotdumpname = "plotnames.txt";
  static const TString runs         = "config/runs2015D.txt";

  // plot config
  constexpr    Int_t    ntimebins  = 40;
  constexpr    Double_t timerange  = 5.0;
  constexpr    Double_t fitrange   = 3.0;
  constexpr    Int_t    nEventsCut = 1000; // for run number plots

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

  // time calib
  constexpr    Float_t el1data = 0.00169548; // 0.00314862;  
  constexpr    Float_t el2data = 0.00070326; // 0.00255739; 
  constexpr    Float_t el1mc   = -0.227449; // -0.227598;
  constexpr    Float_t el2mc   = -0.234309; // -0.234269; 

  // if in demo mode
  constexpr    UInt_t demoNum = 1000;

  // set at command line and in main
  extern TString     outdir;
  extern Bool_t      dumpRanges;
  extern Bool_t      doPURW;
  extern Bool_t      doAnalysis;
  extern Bool_t      doStacks;
  extern Bool_t      doDemo;
  extern Bool_t      useDEG;
  extern Bool_t      useSEL;
  extern Bool_t      useDYll;
  extern Bool_t      useQCD;
  extern Bool_t      useGJets;
  extern Bool_t      useFull;
  extern Bool_t      doStandard;
  extern Bool_t      doTimeRes;
  extern Bool_t      doZvars;
  extern Bool_t      doEffE;
  extern Bool_t      doNvtx;
  extern Bool_t      doEta;
  extern Bool_t      doVtxZ;
  extern Bool_t      doSingleE;
  extern Bool_t      doRuns;
  extern Bool_t      doTrigEff;
  extern Bool_t      applyTOF;
  extern Bool_t      wgtedTime;
  extern Bool_t      useSigma_n;
  extern Bool_t      saveFits;
  extern TString     year;
  extern TString     formname; // fitting function to be used
  extern TString     outtype;

  extern TStrBoolMap SampleMap;
  extern ColorMap    colorMap;
  extern TStrMap     SampleTitleMap;
  extern TStrFltMap  SampleXsecMap;
  extern TStrFltMap  SampleWgtsumMap;
  extern TStrMap     XTitleMap;
  extern TStrDblVMap XBinsMap;
};

#endif
