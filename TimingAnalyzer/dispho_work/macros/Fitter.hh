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
#include "RooBinning.h"
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
#include <cstdlib>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

// Common include
#include "Common.hh"

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
  std::map<TString,RooDataHist*> DataHistMap;
  std::map<TString,RooHistPdf*>  HistPdfMap;
  RooAddPdf * BkgdPdf;

  // model for fit
  RooExtendPdf * BkgdExtPdf;
  RooExtendPdf * SignExtPdf;
  RooAddPdf    * ModelPdf;
};

class Fitter
{
public:
  Fitter(const TString & fitconfig, const TString & miscconfig, const TString & outfiletext);
  ~Fitter();

  // Initialize
  void SetupDefaults();
  void SetupCommon();
  void SetupFitConfig();
  void SetupPlotConfig();
  void SetupMiscConfig();
  void SetupOutTree();

  // Deleting
  void DeleteFitInfo(FitInfo & fitInfo);

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
  void DeclareCoefficients();
  void DeclareXYVars();

  // Subroutines for dumping input info
  void DumpInputInfo();
  void DumpIntegralsAndDraw(TH2F *& hist2D, const TString & text, const Bool_t isBlind, const Bool_t isDraw);
  void DrawProjection(TH2F *& hist2D, const TString & text, const Bool_t isBlind, const TString & proj);
  void SaveHist(TH1F *& hist, const TString & text);
  void SaveHist(TH1F *& hist, const TString & text, const Bool_t isLogY);

  // Subroutines for dumping input significance
  void DumpSignificance(TH2F *& bkgdHist2D);
  void DumpSignificance1D(std::vector<TH2F*> hists2D, const TString & proj);
  void DrawSignificance1D(std::vector<TH1F*> & hists1D, const TString & proj);
  void DrawSignificance1D(std::vector<TH1F*> & hists1D, const TString & proj, const Float_t min, const Float_t max, const Bool_t isLogY);
  void ReadInSignalHists(std::vector<TH2F*> & hists2D, const TString & text);
  Float_t GetMinimum(const std::vector<TH1F*> & hists1D);
  Float_t GetMaximum(const std::vector<TH1F*> & hists1D);

  // Prep for pdfs
  template <typename T>
  void DeclareDatasets(const T & HistMap, FitInfo & fitInfo);
  void DeclareSamplePdfs(FitInfo & fitInfo);

  // Subroutines for fitting
  void MakeFit(FitInfo & fitInfo);
  void ThrowPoisson(const FitInfo & fitInfo);
  void BuildModel(FitInfo & fitInfo);
  void GenerateData(FitInfo & fitInfo);
  void FitModel(FitInfo & fitInfo);
  void GetPredicted(FitInfo & fitInfo);
  void DrawFit(RooRealVar *& var, const TString & title, const FitInfo & fitInfo);
  void FillOutTree(const FitInfo & fitInfo);
  void DeleteModel(FitInfo & fitInfo, const Int_t ifit);

  // Subroutines for dumping ws 
  void DumpWS(const FitInfo & fitInfo, const TString & label);

  // Helper Routines
  Bool_t  IsData(const TString & sample);
  TString GetHistName(const TString & sample);
  Float_t GetMinimum(TGraphAsymmErrors *& graph, TH1F *& hist1, TH1F *& hist2);
  Float_t GetMaximum(TGraphAsymmErrors *& graph, TH1F *& hist);

private:
  // settings
  const TString fFitConfig;
  const TString fOutFileText;
  const TString fMiscConfig;
  TString fPlotConfig;
  TString fEra;

  // Bools+Values
  Bool_t fBkgdOnly;
  Bool_t fGenData;
  Bool_t fDoFits;
  Bool_t fMakeWS;
  Bool_t fDumpWS;
  Int_t  fNFits;
  Int_t  fNDraw;

  // scale factors of initial guess for fit range * fNTotal{Bkgd/Sign}
  Float_t fScaleRangeLow;
  Float_t fScaleRangeHigh;

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
  std::map<TString,TH2F*> fHistMap2D;
  std::map<TString,TH2F*> fHistMap2DTmp;
  std::map<TString,TH1F*> fHistMapX;
  std::map<TString,TH1F*> fHistMapY;

  // misc plot info + model info
  std::vector<TString> fPlotSignalVec;
  TString fSignalSample;

  // scale factors
  std::map<TString,Float_t> fCRKFMap;
  std::map<TString,Float_t> fCRXFMap;

  // Counts + scaling norm from the start
  Float_t fScaleTotalBkgd;
  Float_t fScaleTotalSign;
  Float_t fNTotalBkgd;
  std::map<TString,Float_t> fNTotalSignMap;

  // Constants used to build model + post-fit numbers
  std::map<TString,RooRealVar*> fFracMap; 
  RooRealVar * fNPredBkgd;
  std::map<TString,RooRealVar*> fNPredSignMap;

  // Number of events to generate, and scale norm up or down when throwing poisson for building generation template
  Float_t fScaleGenBkgd;
  Float_t fScaleGenSign;
  Float_t fNGenBkgd;
  Float_t fNGenSign;

  // Blinding (for RooPlots)
  TString fXCut;
  TString fYCut;
  
  // Blinding (for TH's)
  Bool_t fBlindData;
  std::vector<BlindStruct> fBlinds;

  // Roo vars and bins
  RooRealVar * fX;
  RooRealVar * fY;
  
  RooBinning * fXRooBins;
  RooBinning * fYRooBins;

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
