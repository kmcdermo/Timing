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
  CRtoSRPlotter(const TString & crtosrconfig, const TString & era, const TString & outfiletext);
  ~CRtoSRPlotter();

  // Initialize
  void SetupDefaults();
  void SetupCRtoSRConfig();
  void SetupCommon();

  // Main call
  void MakeCRtoSRPlot();

  // Subroutines for plotting
  void GetInputHists();
  void SetupHistsStlye();
  void SaveOutHists();
  void MakeMCErrs();
  void MakeLegend();
  void MakeOutCanv();
  void DrawOutCanv(const Bool_t isScaled);
  void SaveOutput(const Bool_t isScaled);

  // Helper functions
  void MakeMCErr(const TString & key);
  void ScaleToUnity();
  void GetHistMinimum();
  void GetHistMaximum();
  void PrintCanvas(const Bool_t isScaled, const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();
  
private:
  // Settings
  const TString fCRtoSRConfig;
  const TString fEra;
  const TString fOutFileText;

  // CRtoSR info
  TString fSample;
  TString fCRFileName;
  TString fSRFileName;
  TString fPlotConfig;
  Bool_t  fDrawNorm;
  Bool_t  fDrawScaled;

  // input
  TFile * fCRFile;
  TFile * fSRFile;
  std::map<TString,TH1F*> HistMap;

  // plot info
  Bool_t fXVarBins;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  TLegend * Legend;
  TCanvas * OutCanv;
  TPaveText * fConfigPave;
  Float_t fMinY;
  Float_t fMaxY;
};

#endif
