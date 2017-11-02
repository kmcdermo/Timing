#include "../interface/Config.hh"

namespace Config 
{
  // default parameters for Config
  TString outdir     = "output";
  Bool_t  doPURW     = false;
  Bool_t  doAnalysis = false;
  Bool_t  doHadd     = false;
  Bool_t  doEACalc   = false;
  Bool_t  doPtCalc   = false;
  Bool_t  doStacks   = false;
  Bool_t  doPhoStacks = false;
  Bool_t  doDemo     = false;
  Bool_t  useDEG     = false; 
  Bool_t  useSPH     = false; 
  Bool_t  useDYll    = false; 
  Bool_t  useGMSB    = false;
  Bool_t  useHVDS    = false;
  Bool_t  useQCDPt   = false;
  Bool_t  useGJetsHT = false;
  Bool_t  useGJetsEM = false;
  Bool_t  useGJetsFlatPt = false;
  Bool_t  splitPho   = false;
  Int_t   nTotalPhotons = 4;
  Int_t   nPhotons   = nTotalPhotons;
  Bool_t  doEvStd    = false;
  Bool_t  doPhoStd   = false;
  Bool_t  pfIsoEA    = false;    
  Bool_t  detIsoEA   = false;    
  Bool_t  pfIsoPt    = false;    
  Bool_t  detIsoPt   = false;    
  Bool_t  doIso      = false;
  Bool_t  doIsoNvtx  = false;
  Bool_t  doIsoPt    = false;
  Bool_t  useMeanIso = false;
  Bool_t  useMeanRho = false;
  Bool_t  useMeanPt  = false;
  Float_t quantProbIso = 0.9;
  Float_t quantProbRho = 0.5; // 50% = median
  Float_t quantProbPt  = 0.9;
  Int_t   year       = 2017;
  Bool_t  saveHists  = false;
  Bool_t  saveTempHists = false;
  TString outtype    = "png";

  // set in initialize of main.cc
  TStrBoolMap mcSampleMap;
  TColorMap   mcColorMap;
  TStrMap     mcTitleMap;
  TStrVecMap  mcSampleVecMap;
  TStrBoolMap SampleMap;
  TColorMap   ColorMap;
  TStrMap     TitleMap;
}
