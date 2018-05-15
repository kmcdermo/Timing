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
#include "Common.hh"

class TreePlotter2D
{
public:
  TreePlotter2D(const TString & infilename, const TString & insignalfilename, 
		const TString & cutconfig, const TString & plotconfig, const TString & outfiletext);
  ~TreePlotter2D();

  // Initialize
  void SetupDefaults();
  void SetupConfig();
  void SetupPlotConfig();
  void SetupHists();

  // Main call
  void MakePlot();

  // Subroutines for plotting
  void MakeHistFromTrees();
  void MakeHistFromSignalTrees();
  void MakeBkgdOutput();
  void MakeRatioOutput();

  // Helper functions
  TH2F * SetupHist(const TString & name);

  // Meta data
  void MakeConfigPave();

private:
  // Settings
  const TString fInFileName;
  const TString fInSignalFileName;
  const TString fCutConfig;
  const TString fPlotConfig;
  const TString fOutFileText;

  // input
  TFile * fInFile;
  TFile * fInSignalFile;

  // plot vars
  TString fTitle;
  TString fXTitle;
  TString fXVar;
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  std::vector<TString> fXLabels;
  TString fYTitle;
  TString fYVar;
  std::vector<Double_t> fYBins;
  Bool_t fYVarBins;
  std::vector<TString> fYLabels;
  TString fZTitle;
  std::vector<BlindStruct> fBlinds;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<TString,TH2F*> HistMap;
  TH2F * DataHist;
  TH2F * BkgdHist;
  TH2F * RatioHist;
  TH2F * RatioMCErrs;
  TPaveText * fConfigPave;
};

#endif
