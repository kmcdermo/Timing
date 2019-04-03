#ifndef __CombinePreparer__
#define __CombinePreparer__

#include "ABCD.hh"
#include "Common.hh"

#include "TFile.h"
#include "TH2F.h"
#include "TH1D.h"
#include "TString.h"
#include "TPaveText.h"

#include "RooRealVar.h"
#include "RooWorkspace.h"

#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>

// replace NaNs
inline Double_t checkNaN(const Double_t n)
{
  if (std::isnan(n)) return 0.0;
  else return n;
}

// Combine Preparation Class
class CombinePreparer
{
public:

  // internal calls
  CombinePreparer(const TString & infilename, const TString & bininfoname,
		  const TString & ratioinfoname, const TString & binratioinfoname,
		  const TString & systfilename, const TString & datacardname, 
		  const Bool_t blinddata, const Bool_t includesystematics);
  ~CombinePreparer();
  
  // config calls
  void SetupCommon();
  void SetupABCD();
  void ReadSystematics();
  void SetupInputHists();
  void SetupTemplateHists();
    
  // main calls
  void PrepareCombine();
  void MakeParameters();
  void MakePredictionHistogram();
  void MakeDatacard(const TString & sample, const TH2F * SignHist);
  
  // subroutines for datacard making
  void FillTopSection(std::ofstream & datacard);
  void FillObservationSection(std::ofstream & datacard);
  void FillProcessSection(std::ofstream & datacard, const TString & sample, const TH2F * SignHist);
  void FillSystematicsSection(std::ofstream & datacard);
  void FillRateParamSection(std::ofstream & datacard);

  // meta data
  void MakeConfigPave();

private:
  const TString fInFileName;
  const TString fBinInfoName;
  const TString fRatioInfoName;
  const TString fBinRatioInfoName;
  const TString fSystFileName;
  const TString fDatacardName;
  const Bool_t  fBlindData;
  const Bool_t  fIncludeSystematics;

  // Input
  TFile * fInFile;
  TH2F * DataHist;
  TH2F * BkgdHist;
  std::map<TString,TH2F*> SignHistMap;

  // Template Input
  TFile * fTemplateFile;
  TH1D * TemplateHistX;
  TH1D * TemplateHistY;

  // Tmp I/O
  TH2F * PredHist;
  std::map<TString,Double_t> fParameterMap;

  // read in systematics
  std::vector<TString> fSystematics;
};

#endif
