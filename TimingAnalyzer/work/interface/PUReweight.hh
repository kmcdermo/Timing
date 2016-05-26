#ifndef _pureweight_
#define _pureweight_

#include "Common.hh"

class PUReweight
{
public:

  PUReweight(TStrBoolMap Samples, TString outdir, TString outtype, Float_t lumi, Int_t nbins);
  void GetPUWeights();
  ~PUReweight();

private:
  
  TStrVec fDataNames;
  TStrVec fMCNames;
  
  Int_t fNData;
  Int_t fNMC;

  TString  fSelection;
  
  TString fOutDir;
  TString fSubDir;
  TString fOutType;

  Float_t fLumi;
  Int_t   fNBins;

  TH1F * fOutDataNvtx;
  TH1F * fOutMCNvtx;
  TH1F * fOutDataOverMCNvtx;

  TFile * fOutFile;
};

#endif
