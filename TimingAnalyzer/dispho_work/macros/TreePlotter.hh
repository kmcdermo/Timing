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
	      const TString & plotconfig, const TString & miscconfig, const TString & outfiletext);
  ~TreePlotter();

  // Initialize
  void SetupDefaults();
  void SetupConfig();
  void SetupPlotConfig();
  void SetupMiscConfig(const TString & miscconfig);
  void SetupHists();

  // Main call
  void MakePlot();

  // Subroutines for plotting
  void MakeHistFromTrees();
  void MakeDataOutput();
  void MakeBkgdOutput();
  void MakeSignalOutput();
  void MakeRatioOutput();
  void MakeLegend();
  void InitOutputCanvPads();
  void DrawUpperPad();
  void DrawLowerPad();
  void SaveOutput(const TString & outfiletext);

  // Helper functions
  TH1F * SetupHist(const TString & name);
  void ScaleMCToUnity();
  void ScaleMCToData();
  void GetHistMinimum();
  void GetHistMaximum();
  void PrintCanvas(const TString & outfiletext, const Bool_t isLogy);
  
  // Meta data and extra info
  void MakeConfigPave();
  void DumpIntegrals(const TString & outfiletext);

private:
  // Settings
  const TString fInFileName;
  const TString fInSignalFileName;
  const TString fCutConfig;
  const TString fPlotConfig;
  const TString fMiscConfig;
  const TString fOutFileText;

  // input
  TFile * fInFile;
  TFile * fInSignalFile;

  // plot vars
  TString fTitle;
  TString fXTitle;
  Bool_t  fIsLogX;
  TString fXVar;
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  std::vector<TString> fXLabels;
  TString fYTitle;
  Bool_t  fIsLogY;
  std::vector<BlindStruct> fBlinds;

protected:
  // other plotting config
  std::vector<TString> fPlotSignalVec;
  Bool_t fScaleToUnity;
  Bool_t fScaleMCToData;
  Bool_t fBlindData;
  Bool_t fSignalsOnly;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<TString,TH1F*> HistMap;
  TH1F * DataHist;
  TH1F * BkgdHist;
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
