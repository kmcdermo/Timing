#ifndef __PlotComparator__
#define __PlotComparator__

// ROOT inludes
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

// Common include
#include "Common.hh"

enum RatioEnum {DIVONLY,RES};

class PlotComparator
{
public:
  PlotComparator(const TString & compareconfig, const TString & outfiletext);
  ~PlotComparator();

  // Initialize
  void SetupDefaults();
  void SetupCompareConfig();

  // Main call
  void MakeComparisonPlot();

  // Subroutines for plotting
  void SetupInputs();
  void MakeRatioOutputs();
  void MakeLegend();
  void SetupCanvasAndPads();
  void DrawUpperPad();
  void DrawLowerPad();
  void SaveOutput(const Float_t lumi = Common::lumi);

  // Helper Functions
  void PrintCanvas(const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();

private:
  // Settings
  const TString fCompareConfig;
  const TString fOutFileText;

  // Inputs
  TFile * fFile1;
  TFile * fFile2;
  TH1F * fHist1;
  TH1F * fHist2;

  // Colors
  Color_t fColor1 = kRed;
  Color_t fColor2 = kBlue;
  Color_t fColorRatio = kBlack;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  TH1F * fRatioHist;
  TLine * fRatioLine;
  TLegend * fLegend;
  TCanvas * fOutCanv;
  TPad * fUpperPad;
  TPad * fLowerPad;
  TPaveText * fConfigPave;
  Float_t fMinY;
  Float_t fMaxY;

  // Configurables
  RatioEnum fRatioType;
  TString fRatioYTitle;  
  Float_t fRatioMinY;
  Float_t fRatioMaxY;
};

#endif
