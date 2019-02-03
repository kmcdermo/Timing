#ifndef __TimeFitter__
#define __TimeFitter__

// ROOT inludes
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TString.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TPaveText.h"

// STL includes
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

// Common include(s)
#include "Common.hh"
#include "CommonTimeFit.hh"

// fit struct
struct FitStruct
{
  FitStruct() {}
  FitStruct(const TString & label, const TString & inHistName) : label(label), inHistName(inHistName) {}

  const TString label;
  const TString inHistName;

  TH2F * Hist2D;
  std::map<Int_t,TimeFitStruct*> TimeFitStructMap;
  std::map<TString,TH1F*> ResultsMap;

  TFormula * SigmaForm;
  TF1 * SigmaFit;
};

// sigma fit params
struct SigmaFitParams
{
  SigmaFitParams() {}
  
  Float_t low;
  Float_t val;
  Float_t up;
};

class TimeFitter
{
public:
  TimeFitter(const TString & infilename, const TString & plotconfig, const TString & miscconfig, const TString & timefitconfig,
	     const TString & era, const Bool_t savemetadata, const TString & outfiletext);
  ~TimeFitter();

  // config
  void SetupDefaults();
  void SetupCommon();
  void SetupPlotConfig();
  void SetupMiscConfig();
  void SetupTimeFitConfig();
  void ReadInitParams(const std::string & str, SigmaFitParams & params);

  // main calls
  void MakeTimeFits();
  void MakePlots(FitStruct & DataInfo, FitStruct & BkgdInfo);
  void DeleteInfo(FitStruct & FitInfo);

  // subroutines for making fits to variables
  void MakeTimeFit(FitStruct & FitInfo);
  void GetInputHist(FitStruct & FitInfo);
  void InitTimeFits(FitStruct & FitInfo);
  void Project2Dto1DHists(FitStruct & FitInfo);
  void Fit1DHists(FitStruct & FitInfo);
  void ExtractFitResults(FitStruct & FitInfo);

  // subroutines for making fits to sigma
  void MakeSigmaFit(FitStruct & FitInfo);
  void PrepSigmaFit(FitStruct & FitInfo);
  void FitSigmaHist(FitStruct & FitInfo);

  // subroutines for plotting
  void PrintCanvas(FitStruct & DataInfo, FitStruct & MCInfo, Float_t min, Float_t max, 
		   const TString & key, const Bool_t isLogy);

  // save meta data and extra info
  void MakeConfigPave();
  void DumpFitInfo(FitStruct & DataInfo, FitStruct & MCInfo);

  // additional helper functions
  void GetMinMax(const TH1F * hist, Float_t & min, Float_t & max, const TString & key);
  TH1F * SetupHist(const TString & ytitle, const TString & yextra, const TString & label);

private:
  // settings
  const TString fInFileName;
  const TString fPlotConfig;
  const TString fMiscConfig;
  const TString fTimeFitConfig;
  const TString fEra;
  const Bool_t  fSaveMetaData;
  const TString fOutFileText;

  // style
  TStyle * fTDRStyle;
  TString fTitle;
  TString fXTitle;
  std::vector<Double_t> fXBins;
  Int_t fNBinsX;
  Bool_t fXVarBins;
  Bool_t fDoLogX; // technically, read in from miscconfig

  // var fit config
  TimeFitType fTimeFitType;
  Float_t fRangeLow;
  Float_t fRangeUp;
  TString fTimeText;

  // sigma fit config
  Bool_t fDoSigmaFit;
  Bool_t fUseSqrt2;
  TString fSigmaVarText;
  TString fSigmaVarUnit;
  SigmaFitParams fSigmaInitN;
  SigmaFitParams fSigmaInitC;

  // input
  TFile * fInFile;

  // output
  TFile * fOutFile;
  TPaveText * fConfigPave;
};

#endif
