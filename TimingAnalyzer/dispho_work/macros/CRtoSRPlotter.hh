#ifndef __CRtoSRPlotter__
#define __CRtoSRPlotter__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
#include "TColor.h"
#include "TCanvas.h"
#include "TLegend.h"
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

class CRtoSRPlotter
{
public:
  CRtoSRPlotter(const TString & crtosrconfig, const TString & plotconfig, const TString & outfiletext);
  ~CRtoSRPlotter() {}

  // Initialize
  void SetupCRtoSRConfig();
  void SetupPlotConfig();

  // Main call
  void MakeCRtoSRPlot();

  // Subroutines for plotting
  void GetInputHists();
  void SetHistsStlye();
  void MakeLegend();
  void DrawOutCanv(const Bool_t isScaled);
  void SaveOutput(const Bool_t isScaled);

  // Helper functions
  void ScaleToUnity();
  void GetHistMinimum();
  void GetHistMaximum();
  void PrintCanvas(const Bool_t isScaled, const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();
  
private:
  // Settings
  const TString fCRtoSRConfig;
  const TString fPlotConfig;
  const TString fOutFileText;

  // input
  TFile * fCRFile;
  TFile * fSRFile;

  // plot info
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<TString,TH1F*> HistMap;
  TLegend * Legend;
  TCanvas * OutCanv;
  TPaveText * fConfigPave;
  Float_t fMinY;
  Float_t fMaxY;
};

#endif
