#ifndef __TimeVsRunFitter__
#define __TimeVsRunFitter__

// ROOT inludes
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"
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

class TimeVsRunFitter
{
public:
  TimeVsRunFitter(const TString & infilename, const TString & plotconfig,
		  const TString & timefitconfig, const TString & outfiletext);
  ~TimeVsRunFitter();

  // config
  void SetupCommon();
  void SetupPlotConfig();
  void SetupTimeFitConfig();

  // main calls
  void MakeTimeVsRunFits();
  void GetInputHist();
  void InitTimeFits();
  void Project2Dto1DHists();
  void Fit1DHists();
  void ExtractFitResults();
  void MakePlots();

  // subroutines for plotting
  void PrintCanvas(Float_t min, Float_t max, const TString & key, const Bool_t isLogy);

  // save meta data and extra info
  void MakeConfigPave();
  void DumpFitInfo();

  // additional helper functions
  void GetMinMax(const TH1F * hist, Float_t & min, Float_t & max, const TString & key);
  TH1F * SetupHist(const TString & ytitle, const TString & yextra, const TString & label);

private:
  // settings
  const TString fInFileName;
  const TString fPlotConfig;
  const TString fTimeFitConfig;
  const TString fOutFileText;

  // style
  TStyle * fTDRStyle;
  TString fTitle;
  TString fXTitle;
  std::vector<Double_t> fXBins;
  Int_t fNBinsX;
  Bool_t fXVarBins;

  // var fit config
  TimeFitType fTimeFitType;
  Float_t fRangeLow;
  Float_t fRangeUp;
  TString fTimeText;

  // input
  TFile * fInFile;
  
  // tmp I/O
  TH2F * Hist2D;
  std::map<Int_t,TimeFitStruct*> TimeFitStructMap;
  std::map<TString,TH1F*> ResultsMap;

  // output
  TFile * fOutFile;
  TPaveText * fConfigPave;
};

#endif
