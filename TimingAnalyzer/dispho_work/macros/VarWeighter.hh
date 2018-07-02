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
  void SetupConfig();
  void SetupVarWgtConfig();

  // Main call
  void MakeVarWeights();

  // Subroutines for making weight branches
  void GetInputHists();
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
  TString fCRFileName;
  TString fSRFileName;
  TString fSkimFileName;

  // input
  TFile * fCRFile;
  TFile * fSRFile;
  std::map<TString,TH1F*> HistMap;

  // I/O
  TFile * fSkimFile;
};

#endif
