#include "../interface/Config.hh"

namespace Config {

  // default parameters for Config
  TString outdir     = "output";
  Bool_t  doPURW     = false;
  Bool_t  doAnalysis = false;
  Bool_t  doStacks   = false;
  Bool_t  doDemo     = false;
  Bool_t  useDEG     = true; 
  Bool_t  useDYll    = true; 
  Bool_t  useQCD     = false;
  Bool_t  useGJets   = false;
  Bool_t  skipRuns   = false;

  TStrBoolMap SampleMap;       // set in main.cc
  ColorMap    colorMap;        // set in main.cc
  TStrMap     SampleTitleMap;  // set in main.cc
  TStrFltMap  SampleXsecMap;   // set in main.cc
  TStrFltMap  SampleWgtsumMap; // set in main.cc
}
