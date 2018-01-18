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
typedef std::map<SampleType,RooDataHist*> RDHMap;
typedef std::map<SampleType,RooHistPdf*>  RHPMap;
typedef std::map<SampleType,RooRealVar*>  RRVMap;

class Fitter
{
public:
  Fitter(const TString & infilename, const TString & outfilename);
  ~Fitter();

  // Initialize
  void InitConfig();

  // Deleting
  template <typename T>
  void DeleteMap(T & Map);

  // Main calls
  void MakeFit();
  void PrepareFits();
  void Fit2D();

  // Prep for fits
  void GetInputHists();
  void GetMinMax();
  void DeclareVars();

  // Subroutine for fitting
  template <typename T>
  void DeclareDatasets(const T & HistMap, RDHMap & RooDHMap, const RooArgList & ArgList, const TString & text);
  void MakeSamplePdfs(const RDHMap & RooDHMap, RHPMap & RooHPdfMap, const RooArgList & ArgList, const TString & text);
  void DeclareFractions(const RHPMap & RooHPdfMap, RRVMap & FracMap, const TString & text);
  void FitModel(RooAddPdf *& ModelPdf, const RHPMap & RooHPdfMap, const RRVMap & FracMap, const RDHMap & RooDHmap, const TString & text);
  void DrawFit(RooRealVar *& var, const RDHMap & RooDHMap, RooAddPdf *& ModelPdf, const TString & text);
  void ImportToWS(RooWorkspace *& Workspace, RooAddPdf *& ModelPdf, const RDHMap & RooDHMap, const TString & text);

private:
  // settings
  const TString fInFileName;
  const TString fOutFileName;

  // Input
  TFile * fInFile;
  TH2Map  HistMap2D;

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
  RDHMap       RooDHMap2D;
  RHPMap       RooHPdfMap2D;
  RRVMap       FracMap2D;
  RooAddPdf  * ModelPdf2D;

  // Output
  TFile        * fOutFile;
  RooWorkspace * Workspace2D;
};

#endif
