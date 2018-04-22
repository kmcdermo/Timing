#ifndef __Fitter__
#define __Fitter__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"
#include "TGraphAsymmErrors.h"
#include "TPaveText.h"

// RooFit includes
#include "RooFit.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
#include "RooAbsPdf.h"
#include "RooWorkspace.h"

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

// Special enum for type of fit
enum FitType {TwoD, X, Y};

// Special struct for each fit
struct FitInfo
{
  FitInfo(const RooArgList & arglist, const TString & text, const FitType & fit)
    : ArgList(arglist), Text(text), Fit(fit) {}

  // input params
  const RooArgList ArgList;
  const TString Text;
  const FitType Fit;

  // PDF inputs/outputs
  std::map<SampleType,RooDataHist*> DataHistMap;
  std::map<SampleType,RooHistPdf*>  HistPdfMap;
  RooAddPdf    * BkgdPdf;
  RooExtendPdf * EBkgdPdf;
  RooExtendPdf * ESignPdf;
  RooAddPdf    * ModelPdf;
  RooWorkspace * Workspace;
};

class Fitter
{
public:
  Fitter(const TString & fitconfig, const TString & outfiletext);
  ~Fitter();

  // Initialize
  void SetupDefaultBools();
  void SetupConfig();
  void ReadFitConfig();
  void ReadPlotConfig();
  void SetupOutTree();

  // Deleting
  void DeleteFitInfo(FitInfo & fitInfo);
  template <typename T>
  void DeleteMap(T & Map);

  // Main calls
  void MakeFits();
  void PrepareFits();
  template <typename T>
  void MakeFit(const T & HistMap, FitInfo & fitInfo);
  void Project2DHistTo1D();

  // meta data
  void MakeConfigPave();

  // Prep for fits
  void GetInputHists();
  void GetConstants();
  void DeclareVars();

  // Subroutine for fitting
  template <typename T>
  void DeclareDatasets(const T & HistMap, FitInfo & fitInfo);
  void MakeSamplePdfs(FitInfo & fitInfo);
  void BuildModel(FitInfo & fitInfo);
  void GenerateData(FitInfo & fitInfo);
  void FitModel(FitInfo & fitInfo);
  void DrawFit(RooRealVar *& var, const TString & title, const FitInfo & fitInfo);
  void ImportToWS(FitInfo & fitInfo);

  // Helper Routines
  void ScaleUp(TH2F *& hist);
  void ScaleDown(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins);
  void ScaleDown(TH1F *& hist);
  Float_t GetMinimum(TGraphAsymmErrors *& graph, TH1F *& hist);
  Float_t GetMaximum(TGraphAsymmErrors *& graph, TH1F *& hist);

private:
  // settings
  const TString fFitConfig;
  const TString fOutFileText;
  TString fPlotConfig;

  // Bools
  Bool_t fBkgdOnly;
  Bool_t fGenData;
  Bool_t fRunExp;

  // Input names
  TString fGJetsFileName;
  TString fQCDFileName;
  TString fSRFileName;

  // Input files
  TFile * fGJetsFile;
  TFile * fQCDFile;
  TFile * fSRFile;

  // Input Hists
  TH2F * fGJetsHistMC_CR;
  TH2F * fGJetsHistMC_SR;
  TH2F * fQCDHistMC_CR;
  TH2F * fQCDHistMC_SR;
  std::map<SampleType,TH2F*> fHistMap2D;
  std::map<SampleType,TH1F*> fHistMapX;
  std::map<SampleType,TH1F*> fHistMapY;

  // Hist info
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  TString fXTitle;
  std::vector<Double_t> fYBins;
  Bool_t fYVarBins;
  TString fYTitle;

  // Counts 
  Float_t fNBkgdTotal;
  Float_t fNSignTotal;
  Float_t fNInTotal;
  std::map<SampleType,RooRealVar*> fFracMap; 
  RooRealVar * fNPredBkgd;
  RooRealVar * fNPredSign;

  // fractional range of initial integral guess
  Float_t fFracLow;
  Float_t fFracHigh;

  // Number of events to generate
  Float_t fFracGen;
  Float_t fNGen;

  // Blinding
  TString fXCut;
  TString fYCut;

  // Roo vars
  RooRealVar * fX;
  RooRealVar * fY;

  // Style
  TStyle * fTDRStyle;

  // Output
  TFile     * fOutFile;
  TPaveText * fConfigPave;

  // Experiment input
  Int_t fNExperiments;

  // Experiment output
  TTree * fOutTree;
  Float_t fNFitBkgd;
  Float_t fNFitSign;
  Float_t fNExpected;
  std::string fFitID;
};

#endif
