#ifndef _config_
#define _config_

#include "CommonTypes.hh"

namespace Config
{
  // general config
  constexpr    Float_t Sqrt2 = 1.4121356237; 
  constexpr    Float_t PI    = 3.14159265358979323846;
  constexpr    Float_t TWOPI = 2.f*PI;
  constexpr    Float_t sol   = 29.9792458; // speed of light in cm / ns

  // output config
  constexpr    Float_t lumi      = 20.0; // inv fb for current 2017
  static const TString extraText = "Preliminary";

  // nEvents
  constexpr    Int_t   nEvCheck = 100000;

  // pu config
  constexpr    Int_t   nbinsvtx   = 80;
  static const TString pusubdir   = "purw";
  static const TString pufilename = "PURW.root";
  static const TString puplotname = "nvtx_dataOverMC";

  // Iso config
  constexpr    Int_t    nBinsX_iso = nbinsvtx/5;
  constexpr    Double_t xhigh_iso  = Double_t(nbinsvtx);

  // selection config
  static const TString selection = Form("");

  // standard analysis config
  constexpr    Int_t   nGMSBs   = 2;
  constexpr    Int_t   nHVDSs   = 4;
  constexpr    Int_t   nJets    = 4;

  static const TStrVec regions = {"EB","EE"};
  static const TStrVec splits = {"GED","OOT"};

  // data analysis config
  static const TString plotdumpname = "plotnames.txt";
  static const TString phoplotdumpname = "phoplotnames.txt";

  // pho stacking config
  static const TString phosubdir = "gedootstacks";

  // EA config
  static const TString easubdir = "effarea";
  static const TString eadumpname = "effareas.txt";

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
  constexpr    Float_t etaEBcutoff = 1.479;
  constexpr    Float_t etaEBmax = 1.4442;
  constexpr    Float_t etaEEmin = 1.566;
  constexpr    Float_t etaEEmax = 2.5;

  // if in demo mode
  constexpr    UInt_t demoNum = 1000;

  // set at command line and in main
  extern TString     outdir;
  extern Bool_t      doPURW;
  extern Bool_t      doAnalysis;
  extern Bool_t      doEACalc;
  extern Bool_t      doStacks;
  extern Bool_t      doPhoStacks;
  extern Bool_t      doDemo;
  extern Bool_t      useDEG;
  extern Bool_t      useSPH;
  extern Bool_t      useDYll;
  extern Bool_t      useGMSB;
  extern Bool_t      useHVDS;
  extern Bool_t      useQCD;
  extern Bool_t      useGJets;
  extern Bool_t      splitPho;
  extern Int_t       nTotalPhotons; // total stored in the event
  extern Int_t       nPhotons; // allowed photons per split
  extern Bool_t      doEvStd;
  extern Bool_t      doPhoStd;
  extern Bool_t      pfIsoEA; 
  extern Bool_t      doIso;
  extern Bool_t      doIsoNvtx;
  extern Bool_t      useMean;
  extern Bool_t      useMeanRho;
  extern Float_t     quantProb;
  extern Float_t     quantProbRho;
  extern Bool_t      dumpStatus;
  extern TString     year;
  extern Bool_t      saveHists;
  extern Bool_t      saveTempHists;
  extern TString     outtype;

  extern TStrBoolMap SampleMap;
  extern ColorMap    colorMap;
  extern TStrMap     SampleTitleMap;
  extern TStrFltMap  SampleXsecMap;
  extern TStrFltMap  SampleWgtsumMap;
};

#endif
