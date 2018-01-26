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
  void SetupDump();
  void InitConfig();
  void ReadCutConfig();
  void ReadPlotConfig();
  void SetupHists();

  // Main call
  void MakePlot();

  // Subroutines for plotting
  void MakeHistFromTrees();
  void MakeBkgdOutput();
  void MakeRatioOutput();

  // Helper functions
  TH2F * SetupHist(const TString & name);
  void WriteDump();

private:
  // Settings
  const TString fCutConfig;
  const TString fPlotConfig;
  const TString fOutFileName;

  // cut vars
  TString fCommonCut;
  TString fBkgdCut;
  TString fSignCut;
  TString fDataCut;

  // plot vars
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
  TPaveText * fConfigPave;
  std::map<SampleType,TH2F*> HistMap;
  TH2F * BkgdHist;
  TH2F * RatioHist;
  TH2F * RatioMCErrs;
};

#endif
