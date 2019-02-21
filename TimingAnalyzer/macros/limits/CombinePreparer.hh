#ifndef __CombinePreparer__
#define __CombinePreparer__

#include "ABCD.hh"
#include "../Common.hh"

#include "TFile.h"
#include "TH2F.h"
#include "TString.h"
#include "TPaveText.h"

#include "RooRealVar.h"
#include "RooWorkspace.h"

#include <cmath>
#include <vector>
#include <fstream>
#include <iostream>

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

inline Double_t checkNaN(const Double_t n)
{
  if (std::isnan(n)) return 0.0;
  else return n;
}

class CombinePreparer
{
public:
  // internal calls
  CombinePreparer(const TString & infilename, const TString & bininfoname,
		  const TString & ratioinfoname, const TString & binratioinfoname,
		  const TString & systfilename, const TString & wsname,
		  const TString & datacardname, const Bool_t blinddata,
		  const TString & savemetadata, const TString & wsfilename);
  ~CombinePreparer();
  
  // config calls
  void SetupCommon();
  void SetupABCD();
  void ReadSystematics();
  
  // main calls
  void PrepareCombine();
  void MakeWS(const TString & sample, const TH2F * SignHist, const TString & sample_wsname);
  void MakeDatacard(const TString & sample, const TH2F * SignHist, const TString & sample_wsname);
  
  // subroutines for datacard making
  void FillTopSection(std::ofstream & datacard);
  void FillObservationSection(std::ofstream & datacard);
  void FillProcessSection(std::ofstream & datacard, const TH2F * SignHist);
  void FillSystematicsSection(std::ofstream & datacard);
  void FillRateParamSection(std::ofstream & datacard, const TString & sample_wsname);

  // meta data
  void MakeConfigPave();

private:
  const TString fInFileName;
  const TString fBinInfoName;
  const TString fRatioInfoName;
  const TString fBinRatioInfoName;
  const TString fSystFileName;
  const TString fWSName;
  const TString fDataCardName;
  const Bool_t  fBlindData;
  const Bool_t  fSaveMetaData;
  const TString fWSFileName;

  // Input
  TFile * fInFile;
  TH2F * DataHist;

  // Output
  TFile * fOutFile;
  TPaveText * fConfigPave;

  // read in systematics
  std::vector<Systematic> fSystematics;
};

#endif
