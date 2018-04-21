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
#include "TPaveText.h"

// RooFit includes
#include "RooFit.h"
#include "RooRealVar.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooAddPdf.h"
#include "RooExtendPdf.h"
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
  void ReadInFitConfig();

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
  void GetMinMax();
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

private:
  // settings
  const TString fFitConfig;
  const TString fOutFileText;

  // Bools
  Bool_t fBkgdOnly;
  Bool_t fGenData;

  // Input names
  TString fPlotName;
  TString fGJetsFileBase;
  TString fQCDFileBase;
  TString fSRFileBase;

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
  Float_t fNBkgdTotal;
  Float_t fNSignTotal;
  std::map<SampleType,RooRealVar*> fFracMap; 
  RooRealVar * fNPredBkgd;
  RooRealVar * fNPredSign;

  // Style
  TStyle * fTDRStyle;

  // Hist info
  Float_t fXmin;
  Float_t fXmax;
  Float_t fYmin;
  Float_t fYmax;

  // Blinding
  TString fXCut;
  TString fYCut;

  // Roo vars
  RooRealVar * fX;
  RooRealVar * fY;

  // Percent range of variables
  Float_t fInitRange;

  // Output
  TFile        * fOutFile;
  TPaveText    * fConfigPave;
};

#endif
