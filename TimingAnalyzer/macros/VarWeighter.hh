#ifndef __VarWgtWeighter__
#define __VarWgtWeighter__

// ROOT inludes
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "TString.h"
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

class VarWeighter
{
public:
  VarWeighter(const TString & crtosrconfig);
  ~VarWeighter();

  // Initialize
  void SetupVarWgtConfig();
  void SetupCommon();

  // Main call
  void MakeVarWeights();

  // Subroutines for making weight branches
  void GetInputHists();
  void ScaleInputToUnity();
  void MakeRatioHist();
  void MakeWeightBranches();

  // Helper functions
  void MakeWeightBranch(const TString & treename);

  // Meta data and extra info
  void MakeConfigPave();
  
private:
  // Settings
  const TString fVarWgtConfig;

  // VarWgt info
  TString fSample;
  TString fVar;
  TString fPlotConfig;
  TString fCRFileName;
  TString fSRFileName;
  TString fSkimFileName;

  // input
  TFile * fCRFile;
  TFile * fSRFile;
  std::map<TString,TH1F*> HistMap;

  // plot info
  Bool_t fXVarBins;

  // I/O
  TFile * fSkimFile;
  TPaveText * fConfigPave;
};

#endif
