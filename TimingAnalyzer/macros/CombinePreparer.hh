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

// Structure to contain each systematic info
struct Systematic
{
  Systematic () {}
  Systematic (const TString & name, const TString & type, const Double_t val, const Bool_t isSig, const Bool_t isBkg)
    : name(name), type(type), val(val), isSig(isSig), isBkg(isBkg) {}

  TString name;
  TString type;
  Double_t val;
  Bool_t isSig;
  Bool_t isBkg;
};

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
		  const Bool_t blinddata);
  ~CombinePreparer();
  
  // config calls
  void SetupCommon();
  void SetupABCD();
  void ReadSystematics();
  
  // main calls
  void PrepareCombine();
  void MakeParameters();
  void MakeDatacard(const TString & sample, const TH2F * SignHist);
  
  // subroutines for datacard making
  void FillTopSection(std::ofstream & datacard);
  void FillObservationSection(std::ofstream & datacard);
  void FillProcessSection(std::ofstream & datacard, const TH2F * SignHist);
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

  // Input
  TFile * fInFile;
  TH2F * DataHist;
  TH2F * BkgdHist;

  // Tmp Input
  TFile * fTemplateFile;
  TH1D * TemplateHistX;
  TH1D * TemplateHistY;

  // Tmp I/O
  std::map<TString,Double_t> fParameterMap;

  // read in systematics
  std::vector<Systematic> fSystematics;
};

#endif
