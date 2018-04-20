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

// Typedefs needed
typedef std::map<SampleType,TH2F*>        TH2Map;
typedef std::map<SampleType,TH1F*>        TH1Map;
typedef std::map<SampleType,Float_t>      FltMap;

typedef std::map<SampleType,RooDataHist*> RDHMap;
typedef std::map<SampleType,RooHistPdf*>  RHPMap;
typedef std::map<SampleType,RooRealVar*>  RRVMap;

// Special enum for type of fit
enum FitType {TwoD, X, Y};

// Special struct for each fit
struct FitInfo
{
  FitInfo(const RooArgList & ArgList, const TString & Text, const FitType & Fit)
    : ArgList_(ArgList), Text_(Text), Fit_(Fit) {}
  const RooArgList ArgList_;
  const TString Text_;
  const FitType Fit_;
};

class Fitter
{
public:
  Fitter(const TString & fitconfig, const TString & outfiletext);
  ~Fitter();

  // Initialize
  void SetupConfig();
  void ReadInFitConfig();

  // Deleting
  template <typename T>
  void Delete(T & HistMap, RDHMap & RooDHMap, RHPMap & RooHPdfMap, RRVMap & FracMap,
	      RooAddPdf *& ModelPdf, RooWorkspace *& Workspace);
  template <typename T>
  void DeleteMap(T & Map);

  // Main calls
  void MakeFits();
  void PrepareFits();
  void Project2DHistTo1D();
  template <typename T>
  void MakeFit(const T & HistMap, RDHMap & RooDHMap, RHPMap & RooHPdfMap, RRVMap & FracMap,
	       RooAddPdf *& ModelPdf, RooWorkspace *& Workspace, const FitInfo & fitInfo);

  // meta data
  void MakeConfigPave();

  // Prep for fits
  void GetInputHists();
  void GetConstants();
  void GetMinMax();
  void DeclareVars();

  // Subroutine for fitting
  template <typename T>
  void DeclareDatasets(const T & HistMap, RDHMap & RooDHMap, const FitInfo & fitInfo);
  void MakeSamplePdfs(const RDHMap & RooDHMap, RHPMap & RooHPdfMap, const FitInfo & fitInfo);
  void DeclareFractions(const RHPMap & RooHPdfMap, RRVMap & FracMap, const FitInfo & fitInfo);
  void FitModel(RooAddPdf *& ModelPdf, const RHPMap & RooHPdfMap, const RRVMap & FracMap, const RDHMap & RooDHmap, const FitInfo & fitInfo);
  void DrawFit(RooRealVar *& var, const RDHMap & RooDHMap, RooAddPdf *& ModelPdf, const TString & title, const FitInfo & fitInfo);
  void ImportToWS(RooWorkspace *& Workspace, RooAddPdf *& ModelPdf, const RDHMap & RooDHMap, const FitInfo & fitInfo);

private:
  // settings
  const TString fFitConfig;
  const TString fOutFileText;

  // Input files
  TFile * GJetsFile;
  TFile * QCDFile;
  TFile * SRFile;

  // Input Hists
  TH2Map HistMap2D;
  TH2F * GJetsHistMC_CR;
  TH2F * GJetsHistMC_SR;
  TH2F * QCDHistMC_CR;
  TH2F * QCDHistMC_SR;

  // Input names
  TString fPlotName;
  TString fGJetsFileBase;
  TString fQCDFileBase;
  TString fSRFileBase;

  // Counts 
  FltMap NPredMap; 
  Float_t NPredTotal;

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

  // 2D Fit
  RDHMap         RooDHMap2D;
  RHPMap         RooHPdfMap2D;
  RRVMap         FracMap2D;
  RooAddPdf    * ModelPdf2D;
  RooWorkspace * Workspace2D;

  // 1D Fit X
  TH1Map         HistMapX;
  RDHMap         RooDHMapX;
  RHPMap         RooHPdfMapX;
  RRVMap         FracMapX;
  RooAddPdf    * ModelPdfX;
  RooWorkspace * WorkspaceX;

  // 1D Fit Y
  TH1Map         HistMapY;
  RDHMap         RooDHMapY;
  RHPMap         RooHPdfMapY;
  RRVMap         FracMapY;
  RooAddPdf    * ModelPdfY;
  RooWorkspace * WorkspaceY;

  // Output
  TFile        * fOutFile;
  TPaveText    * fConfigPave;
};

#endif
