#ifndef __Fitter2D__
#define __Fitter2D__

// ROOT inludes
#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH2F.h"
#include "TH1F.h"
#include "TString.h"
#include "TCanvas.h"

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

class Fitter2D
{
public:
  Fitter2D(const TString & infilename, const TString & outfilename);
  ~Fitter2D();

  // Initialize
  void InitConfig();

  // Main call
  void MakeFit();

  // Subroutines for fitting
  void GetInputHists();
  void GetMinMax();
  void DeclareVars();
  void DeclareDatasets();
  void MakeSamplePdfs();
  void DeclareFractions();
  void FitModel();
  void DrawProjectedFits();
  void ImportToWS();

private:
  // settings
  const TString fInFileName;
  const TString fOutFileName;

  // Input
  TFile * fInFile;
  std::map<SampleType,TH2F*> HistMap;

  // Style
  TStyle * fTDRStyle;

  // Hist info
  Float_t fXmin;
  Float_t fXmax;
  Float_t fYmin;
  Float_t fYmax;

  // Roo vars
  RooRealVar * fX;
  RooRealVar * fY;
  std::map<SampleType,RooDataHist*> RooDHMap;
  std::map<SampleType,RooHistPdf*> RooHPdfMap;
  std::map<SampleType,RooRealVar*> FracMap;
  RooAddPdf * ModelPdf;

  // Output
  TFile * fOutFile;
  RooWorkspace * fWorkspace;
};

#endif
