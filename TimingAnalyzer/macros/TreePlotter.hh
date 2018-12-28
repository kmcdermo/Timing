#ifndef __TreePlotter__
#define __TreePlotter__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TString.h"
#include "TColor.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"
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

class TreePlotter
{
public:
  TreePlotter() {}
  TreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
	      const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
	      const TString & era, const TString & outfiletext);
  ~TreePlotter() {}

  // Initialize
  void SetupDefaults();
  void SetupCommon();
  void SetupPlotConfig(const TString & plotconfig);
  void SetupMiscConfig(const TString & miscconfig);
  void SetupHists();
  void SetupHistsStyle();

  // Main call
  void MakeTreePlot();

  // Subroutines for plotting
  void MakeHistFromTrees(TFile *& inFile, TFile *& inSignalFile);
  void MakeDataOutput();
  void MakeBkgdOutput();
  void MakeSignalOutput();
  void MakeRatioOutput();
  void MakeLegend();
  void InitOutputCanvPads();
  void DrawUpperPad();
  void DrawLowerPad();
  void SaveOutput(const TString & outfiletext, const TString & era);

  // Helper functions
  TH1F * SetupHist(const TString & name);
  void ScaleMCToUnity();
  void ScaleMCToData();
  void GetHistMinimum();
  void GetHistMaximum();
  void PrintCanvas(const TString & outfiletext, const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();
  TString DumpIntegrals(const TString & outfiletext);

  // Delete Function
  void DeleteMemory(const Bool_t deleteInternal);

private:
  // Settings
  const TString fInFileName;
  const TString fInSignalFileName;
  const TString fCutConfig;
  const TString fVarWgtMapConfig;
  const TString fPlotConfig;
  const TString fMiscConfig;
  const TString fEra;
  const TString fOutFileText;

  // input
  TFile * fInFile;
  TFile * fInSignalFile;

protected:
  // plot vars
  TString fTitle;
  TString fXTitle;
  Bool_t  fIsLogX;
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  std::vector<TString> fXLabels;
  TString fYTitle;
  Bool_t  fIsLogY;
  std::vector<BlockStruct> fBlinds;

  // other plotting config
  std::vector<TString> fPlotSignalVec;
  Bool_t fScaleToUnity;
  Bool_t fScaleMCToData;
  Bool_t fBlindData;
  Bool_t fSkipData;
  Bool_t fSignalsOnly;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<TString,TH1F*> HistMap;
  TH1F * DataHist;
  TH1F * BkgdHist;
  TH1F * EWKHist;
  THStack * BkgdStack;
  TH1F * RatioHist;
  TH1F * RatioMCErrs;
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
