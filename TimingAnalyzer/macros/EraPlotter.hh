#ifndef __EraPlotter__
#define __EraPlotter__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TColor.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "TPaveText.h"

// STL includes
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

// Common include
#include "Common.hh"

class EraPlotter
{
public:
  EraPlotter(const TString & eraplotconfig, const TString & plotconfig, 
	     const Bool_t savemetadata, const TString & outfiletext);
  ~EraPlotter();

  // Initialize
  void SetupDefaults();
  void SetupCommon();
  void SetupEraPlotConfig();
  void SetupPlotConfig();
  void SetupInFiles();

  // Main call
  void MakeEraPlot();

  // Subroutines for plotting
  void SetupBkgdHists();
  void SetupDataHists();
  void SetupRatioOutput();
  void SetupLegend();
  void InitOutputCanvPads();
  void DrawUpperPad();
  void DrawLowerPad();
  void SaveOutput();

  // Helper functions
  void GetHistMinimum();
  void GetHistMaximum();
  void PrintCanvas(const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();

private:
  // Settings
  const TString fEraPlotConfig;
  const TString fPlotConfig;
  const Bool_t  fSaveMetaData;
  const TString fOutFileText;

  // input
  std::vector<TFile*> fInFiles;

  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;

  // other plotting config
  TString fInFileText;
  std::vector<TString> fEras;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::vector<TH1F*> DataHistVec;
  std::vector<TH1F*> RatioHistVec;
  TH1F * BkgdHist;
  TH1F * BkgdHistErr;
  TH1F * BkgdHistErrRatio;
  TLine * RatioLine;
  TLegend * Legend;
  TCanvas * OutCanv;
  TPad * UpperPad;
  TPad * LowerPad;
  TPaveText * fConfigPave;
  Float_t fMinY;
  Float_t fMaxY;
};

#endif
