#ifndef __TreePlotter2D__
#define __TreePlotter2D__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TString.h"
#include "TColor.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"

// STL includes
#include <iostream>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

// Common include
#include "common/Common.hh"

class TreePlotter2D
{
public:
  TreePlotter2D(const TString & text, const TString & commoncut, const TString & inconfig);
  ~TreePlotter2D();

  // Initialize
  void InitConfig();
  void SetupHists();

  // Main call
  void MakePlot();

  // Subroutines for plotting
  void MakeHistFromTrees();
  void MakeBkgdOutput();
  void MakeRatioOutput();
  void SaveOutput();

  // Helper functions
  TH2F * SetupHist(const TString & name);
  
private:
  // Settings
  const TString fText;
  const TString fCommonCut;
  const TString fInConfig;

  TString fXVar;
  TString fYVar;
  TString fTitle;
  TString fXTitle;
  TString fYTitle;
  std::vector<Double_t> fXBins;
  std::vector<Double_t> fYBins;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<SampleType,TH2F*> HistMap;
  TH2F * BkgdHist;
  TH2F * RatioHist;
  TH2F * RatioMCErrs;
};

#endif
