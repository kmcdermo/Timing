#include "../interface/Config.hh"

namespace Config 
{
  // default parameters for Config
  TString outdir     = "output";
  Bool_t  doPURW     = false;
  Bool_t  doAnalysis = false;
  Bool_t  doStacks   = false;
  Bool_t  doPhoStacks = false;
  Bool_t  doDemo     = false;
  Bool_t  useDEG     = false; 
  Bool_t  useSPH     = false; 
  Bool_t  useDYll    = false; 
  Bool_t  useGMSB    = false;
  Bool_t  useHVDS    = false;
  Bool_t  useQCD     = false;
  Bool_t  useGJets   = false;
  Bool_t  splitOOT   = false;
  Int_t   nTotalPhotons = 4;
  Int_t   nPhotons   = nTotalPhotons;
  Bool_t  doEvStd    = false;
  Bool_t  doPhoStd   = false;
  Bool_t  pfIsoEA    = false;    
  Bool_t  doIso      = false;
  Bool_t  doIsoNvtx  = false;
  Bool_t  useMean    = false;
  Float_t quantProb  = 0.9;
  Bool_t  dumpStatus = false;
  TString year       = "2017";
  Bool_t  saveHists  = false;
  Bool_t  saveTempHists = false;
  TString outtype    = "png";

  TStrBoolMap SampleMap;       // set in main.cc
  ColorMap    colorMap;        // set in main.cc
  TStrMap     SampleTitleMap;  // set in main.cc
  TStrFltMap  SampleXsecMap;   // set in main.cc
  TStrFltMap  SampleWgtsumMap; // set in main.cc
}
