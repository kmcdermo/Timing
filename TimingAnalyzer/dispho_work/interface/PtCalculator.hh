#ifndef _ptcalculator_
#define _ptcalculator_

#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TF1.h"
#include "TFormula.h"

#include <fstream>

typedef std::vector<TF1*> TF1Vec;

class PtCalculator
{
public:
  PtCalculator(const TString & sample, const Bool_t isMC);
  ~PtCalculator();

  void ExtractPtScaling();
  void OutputFitCanvases();
  void InitTH1FNamesAndSubDNames();
  void InitSubDirs();
  void InitInputPlots();
  void InitFits();
  void InitOutputCanvs();

private:
  const TString fSample;
  const Bool_t fIsMC;

  TFile * fInFile;
  TH1FVec fInTH1FHists;
  
  Int_t fNTH1F;
  TStrVec fTH1FNames;
  TStrMap fTH1FSubDMap;

  TString fOutDir;

  TFile * fOutFile;
  TCanvVec fOutTH1FCanvases;
  TF1Vec fOutTH1FTF1s;
  std::ofstream fPtFile;
};

#endif
