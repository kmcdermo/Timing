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
  TreePlotter2D() {}
  TreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		const TString & plotconfig, const TString & miscconfig, const TString & era,
		const Bool_t savemetadata, const TString & outfiletext);
  ~TreePlotter2D() {}  

  // Initialize
  void SetupDefaults();
  void SetupSaveMetaData(const Bool_t savemetadata);
  void SetupCommon();
  void SetupPlotConfig(const TString & plotconfig);
  void SetupMiscConfig(const TString & miscconfig);
  void SetupHists();

  // Main call
  void MakeTreePlot2D();

  // Subroutines for plotting
  void MakeHistFromTrees(TFile *& inFile, TFile *& inSignalFile);
  void MakeDataOutput();
  void MakeBkgdOutput();
  void MakeRatioOutput();
  void DeleteMemory(const Bool_t deleteMemory);

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
  const TString fMiscConfig;
  const TString fEra;
  const TString fOutFileText;

  // input
  TFile * fInFile;
  TFile * fInSignalFile;

protected:
  // Config
  Bool_t fSaveMetaData;

  // plot vars
  TString fTitle;
  TString fXTitle;
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  std::vector<TString> fXLabels;
  TString fYTitle;
  std::vector<Double_t> fYBins;
  Bool_t fYVarBins;
  std::vector<TString> fYLabels;
  TString fZTitle;
  std::vector<BlockStruct> fBlinds;

  // other plotting config
  Bool_t fScaleToBinWidths;
  Bool_t fBlindData;
  Bool_t fSkipData;
  Bool_t fSkipBkgdMC;
  Bool_t fSkipSignal;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<TString,TH2F*> HistMap;
  TH2F * DataHist;
  TH2F * BkgdHist;
  TH2F * EWKHist;
  TH2F * RatioHist;
  TH2F * RatioMCErrs;
  TPaveText * fConfigPave;
};

#endif
