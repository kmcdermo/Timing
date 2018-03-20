#ifndef __TreePlotter2D__
#define __TreePlotter2D__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TString.h"
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
#include "common/Common.hh"

class TreePlotter2D
{
public:
  TreePlotter2D(const TString & cutconfig, const TString & plotconfig, const TString & outfilename);
  ~TreePlotter2D();

  // Initialize
  void SetupConfig();
  void SetupHists();

  // Main call
  void MakePlot();

  // Subroutines for plotting
  void MakeHistFromTrees();
  void MakeBkgdOutput();
  void MakeRatioOutput();

  // Helper functions
  void ReadPlotConfig();
  TH2F * SetupHist(const TString & name);

  // Meta data
  void MakeConfigPave();

private:
  // Settings
  const TString fCutConfig;
  const TString fPlotConfig;
  const TString fOutFileName;

  // plot vars
  TString fTitle;
  TString fXTitle;
  TString fXVar;
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  TString fYTitle;
  TString fYVar;
  std::vector<Double_t> fYBins;
  Bool_t fYVarBins;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<SampleType,TH2F*> HistMap;
  TH2F * BkgdHist;
  TH2F * RatioHist;
  TH2F * RatioMCErrs;
  TPaveText * fConfigPave;
};

#endif
