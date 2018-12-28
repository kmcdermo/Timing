#ifndef __TnPPlotter__
#define __TnPPlotter__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
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

struct TnPStruct
{
  TnPStruct() {}

  Bool_t phohasPixSeed_tag;
  Bool_t phoisEB_probe; 
  Bool_t phoisTrk_probe;
  Float_t phopt_probe;

  TBranch * b_phohasPixSeed_tag;
  TBranch * b_phoisEB_probe;
  TBranch * b_phoisTrk_probe;
  TBranch * b_phopt_probe;
};

struct MinMaxStruct
{
  MinMaxStruct() {}

  Float_t xmin;
  Float_t xmax;
  Float_t ymin;
  Float_t ymax;
};

class TnPPlotter
{
public:
  TnPPlotter(const TString & infilename, const TString & outfiletext);
  ~TnPPlotter();

  // Initialize
  void SetupCommon();
  void SetupStrVecs();
  void SetupOutputEffs();

  // Main call
  void MakeTnPPlots();

  // Subroutines for plotting
  void EventLoop();
  void MakeGraph();
  void GetGraphMinMax();
  void MakeRatioOutput();
  void MakeLegend();
  void InitOutputCanvPads();
  void DrawUpperPad();
  void DrawLowerPad();
  void SaveOutput();

  // Helper functions
  void FillTnP(TnPStruct & info, const UInt_t entry, const TString & sample_label, const Float_t wgt);
  void PrintCanvas(const TString & eta, const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();

private:
  // Settings
  const TString fInFileName;
  const TString fOutFileText;

  // Additional settings
  std::vector<TString> fSamples;
  std::vector<TString> fEtas;

  // Input
  TFile * fInFile;

  // Style
  TStyle * fTDRStyle;

  // Min and maxes
  std::map<TString,MinMaxStruct> fMinMaxMap;

  // Output
  TFile * fOutFile;
  std::map<TString,TEfficiency*> EffMap;
  std::map<TString,TGraphAsymmErrors*> GraphMap;
  std::map<TString,TH1F*> HistMap;
  std::map<TString,TLine*> RatioLineMap;
  std::map<TString,TLegend*> LegendMap;
  std::map<TString,TCanvas*> OutCanvMap;
  std::map<TString,TPad*> UpperPadMap;
  std::map<TString,TPad*> LowerPadMap;
  TPaveText * fConfigPave;
};

#endif
