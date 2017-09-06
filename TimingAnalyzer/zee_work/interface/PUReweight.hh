#ifndef _pureweight_
#define _pureweight_

#include "Config.hh"
#include "Common.hh"

class PUReweight
{
public:

  PUReweight();
  void GetPUWeights();
  ~PUReweight();

private:
  
  TStrVec fDataNames;
  TStrVec fMCNames;
  
  Int_t fNData;
  Int_t fNMC;

  TString fOutDir;

  TH1F * fOutDataNvtx;
  TH1F * fOutMCNvtx;
  TH1F * fOutDataOverMCNvtx;

  TFile * fOutFile;
};

#endif
