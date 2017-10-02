#include "../interface/Config.hh"

namespace Config 
{
  // default parameters for Config
  TString outdir     = "output";
  Bool_t  doPURW     = false;
  Bool_t  doAnalysis = false;
  Bool_t  doStacks   = false;
  Bool_t  doDemo     = false;
  Bool_t  useDEG     = false; 
  Bool_t  useSPH     = false; 
  Bool_t  useDYll    = false; 
  Bool_t  useGMSB    = false;
  Bool_t  useHVDS    = false;
  Bool_t  useQCD     = false;
  Bool_t  useGJets   = false;
  Bool_t  doStandard = false;
  Bool_t  dumpStatus = false;
  TString year       = "2017";
  Bool_t  saveHists  = false;
  TString outtype    = "png";

  TStrBoolMap SampleMap;       // set in main.cc
  ColorMap    colorMap;        // set in main.cc
  TStrMap     SampleTitleMap;  // set in main.cc
  TStrFltMap  SampleXsecMap;   // set in main.cc
  TStrFltMap  SampleWgtsumMap; // set in main.cc
}
