#ifndef _eacalculator_
#define _eacalculator_

#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TF1.h"
#include "TFormula.h"

#include <fstream>

typedef std::vector<TF1*> TF1Vec;

class EACalculator
{
public:
  EACalculator(const TString & sample, const Bool_t isMC);
  ~EACalculator();

  void ExtractEA();
  void FitHist(TH1F *& hist, TCanvas *& canv, const TString & name);
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
  TH1F * fInRhoHist;
  
  Int_t fNTH1F;
  TStrVec fTH1FNames;
  TStrMap fTH1FSubDMap;
  TString fRhoName;
  TString fRhoSubD;

  TString fOutDir;

  TFile * fOutFile;
  TCanvVec fOutTH1FCanvases;
  TF1Vec fOutTH1FTF1s;
  TCanvas * fOutRhoCanvas;
  TF1 * fOutRhoTF1;
  std::ofstream fEAFile;
};

#endif
