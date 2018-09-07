#ifndef __TimeAdjuster__
#define __TimeAdjuster__

// ROOT inludes
#include "TFile.h"
#include "TH1F.h"
#include "TF1.h"
#include "TRandomGen.h"
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

struct Event
{
  // vars
  UInt_t run;
  Int_t  nphotons;

  // strings
  std::string s_run = "run";
  std::string s_nphotons = "nphotons";

  // branches
  TBranch * b_run;
  TBranch * b_nphotons;
};

struct Photon
{
  // old vars
  Float_t pt;
  Bool_t  isEB;
  // new vars
  Float_t seedtimeSHIFT;
  Float_t seedtimeSMEAR;
  
  // strings
  std::string s_pt = "phopt";
  std::string s_isEB = "phoisEB";
  std::string s_seedtimeSHIFT = "phoseedtimeSHIFT";
  std::string s_seedtimeSMEAR = "phoseedtimeSMEAR";

  // branches
  TBranch * b_pt;
  TBranch * b_isEB;
  TBranch * b_seedtimeSHIFT;
  TBranch * b_seedtimeSMEAR;
};

struct FitStruct
{
  FitStruct() {}
  FitStruct(const TString & label) : label(label) {}

  const TString label;
  std::map<TString,TH1F*> MuHistMap;
  std::map<TString,TF1*>  SigmaFitMap;
};

class TimeAdjuster
{
public:
  TimeAdjuster(const TString & skimfilename, const TString & signalskimfilename, const TString & infilesconfig,
	       const Bool_t doshift, const Bool_t dosmear);
  ~TimeAdjuster();

  // Config
  void SetupCommon();
  void SetupInFilesConfig();

  // Main calls
  void AdjustTime();
  void PrepAdjustments(FitStruct & FitInfo);
  void CorrectData(FitStruct & DataInfo);
  void CorrectMC(FitStruct & DataInfo, FitStruct & MCInfo);
  void DeleteInfo(FitStruct & FitInfo);
  
  // Getting adjustments ready
  void GetInputMuHists(FitStruct & FitInfo);
  void GetInputSigmaFits(FitStruct & FitInfo);

  // Meta data
  void MakeConfigPave(TFile *& SkimFile);

  // Helper functions
  template <typename T>
  void DeleteMap(T & Map);

private:
  // settings
  const TString fSkimFileName;
  const TString fSignalSkimFileName;
  const TString fInFilesConfig;
  const Bool_t  fDoShift;
  const Bool_t  fDoSmear;

  // inputs
  std::map<TString,TString> fInFileNameMap;
  std::map<TString,TFile*> fInFileMap;

  // I/O
  TFile * fSkimFile;
  TFile * fSignalSkimFile;
};

#endif
