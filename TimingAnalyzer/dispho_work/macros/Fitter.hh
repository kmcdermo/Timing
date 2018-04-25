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
#include "TRandom.h"
#include "TCanvas.h"
#include "TLegend.h"
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
  RooAddPdf * BkgdPdf;

  // model for fit
  RooExtendPdf * BkgdExtPdf;
  RooExtendPdf * SignExtPdf;
  RooAddPdf    * ModelPdf;
};

class Fitter
{
public:
  Fitter(const TString & fitconfig, const TString & outfiletext);
  ~Fitter();

  // Initialize
  void SetupDefaultValues();
  void SetupConfig();
  void ReadFitConfig();
  void ReadPlotConfig();
  void SetupOutTree();

  // Deleting
  void DeleteFitInfo(FitInfo & fitInfo);
  template <typename T>
  void DeleteMap(T & Map);

  // Main calls
  void DoMain();
  void PrepareCommon();
  template <typename T>
  void PreparePdfs(const T & HistMap, FitInfo & fitInfo);
  void RunExperiments(FitInfo & fitInfo);
  void ImportToWS(FitInfo & fitInfo);
  void SaveOutTree();
  void MakeConfigPave();
  
  // Prep for common variables/datasets/etc
  void GetInputHists();
  void Project2DHistTo1D();
  void GetCoefficients();
  void DeclareXYVars();

  // Prep for pdfs
  template <typename T>
  void DeclareDatasets(const T & HistMap, FitInfo & fitInfo);
  void MakeSamplePdfs(FitInfo & fitInfo);

  // Subroutine for fitting
  void MakeFit(FitInfo & fitInfo);
  void ThrowPoisson(const FitInfo & fitInfo);
  void BuildModel(FitInfo & fitInfo);
  void GenerateData(FitInfo & fitInfo);
  void FitModel(FitInfo & fitInfo);
  void GetPredicted(FitInfo & fitInfo);
  void DrawFit(RooRealVar *& var, const TString & title, const FitInfo & fitInfo);
  void FillOutTree(const FitInfo & fitInfo);
  void DeleteModel(FitInfo & fitInfo, const Int_t ifit);

  // Helper Routines
  void ScaleUp(TH2F *& hist);
  void ScaleDown(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins);
  void ScaleDown(TH1F *& hist);
  Float_t GetMinimum(TGraphAsymmErrors *& graph, TH1F *& hist1, TH1F *& hist2);
  Float_t GetMaximum(TGraphAsymmErrors *& graph, TH1F *& hist);

private:
  // settings
  const TString fFitConfig;
  const TString fOutFileText;
  TString fPlotConfig;

  // Bools+Values
  Bool_t fBkgdOnly;
  Bool_t fGenData;
  Bool_t fDoFits;
  Bool_t fMakeWS;
  Int_t  fNFits;
  Int_t  fNDraw;

  // fractional range of initial integral guess
  Float_t fFracLow;
  Float_t fFracHigh;

  // Hist info
  std::vector<Double_t> fXBins;
  Bool_t fXVarBins;
  TString fXTitle;
  std::vector<Double_t> fYBins;
  Bool_t fYVarBins;
  TString fYTitle;

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

  // Counts 
  Float_t fNTotalBkgd;
  Float_t fNTotalSign;
  std::map<SampleType,RooRealVar*> fFracMap; 
  RooRealVar * fNPredBkgd;
  RooRealVar * fNPredSign;

  // Number of events to generate
  Float_t fFracGen;
  Float_t fNGenBkgd;
  Float_t fNGenSign;

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

  // Experiment output
  TTree * fOutTree;
  Float_t fNFitBkgd;
  Float_t fNFitBkgdErr;
  Float_t fNFitSign;
  Float_t fNFitSignErr;
  std::string fFitID;
};

#endif
