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

// STL includes
#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

// Common include
#include "common/Common.hh"

class TreePlotter
{
public:
  TreePlotter(const TString & var, const TString & commoncut, const TString & text, const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
	      const Bool_t islogx, const Bool_t islogy, const TString & title, const TString & xtitle, const TString & ytitle, const TString & delim);
  ~TreePlotter();

  // Initialize
  void InitConfig();
  void SetupHists();

  // Main call
  void MakePlot();

  // Subroutines for plotting
  void MakeHistFromTrees();
  void MakeBkgdOutput();
  void MakeRatioOutput();
  void MakeLegend();
  void InitOutputCanvPads();
  void DrawUpperPad();
  void DrawLowerPad();
  void SaveOutput();

  // Helper functions
  TH1F * SetupHist(const TString & name);
  Float_t GetHistMinimum();
  Float_t GetHistMaximum();
  
private:
  // Settings
  const TString fVar;
  const TString fCommonCut;
  const TString fText;
  const Int_t   fNbinsX;
  const Float_t fXLow;
  const Float_t fXHigh;
  const Bool_t  fIsLogX;
  const Bool_t  fIsLogY;
  const TString fDelim;
  TString fTitle;
  TString fXTitle;
  TString fYTitle;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile * fOutFile;
  std::map<SampleType,TH1F*> HistMap;
  TH1F * BkgdHist;
  THStack * BkgdStack;
  TH1F * RatioHist;
  TH1F * RatioMCErrs;
  TLine * RatioLine;
  TLegend * Legend;
  TCanvas * OutCanv;
  TPad * UpperPad;
  TPad * LowerPad;
};

#endif
