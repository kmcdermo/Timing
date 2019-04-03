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

struct ReducedEvent
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

struct ReducedPhoton
{
  // old vars
  Float_t adjustvar;
  Bool_t  isEB;
  // new vars
  Float_t timeSHIFT;
  Float_t timeSMEAR;
  
  // strings 
  std::string s_isEB = "phoisEB";

  // branches
  TBranch * b_adjustvar;
  TBranch * b_isEB;
  TBranch * b_timeSHIFT;
  TBranch * b_timeSMEAR;
};

struct FitStruct
{
  FitStruct() {}
  FitStruct(const TString & label) : label(label) {}

  const TString label;
  std::map<TString,TH1F*> MuHistMap;
  std::map<TString,TH1F*> SigmaHistMap;
  std::map<TString,TF1*>  SigmaFitMap;
};

class TimeAdjuster
{
public:
  TimeAdjuster(const TString & skimfilename, const TString & signalskimfilename, const TString & infilesconfig,
	       const TString & sadjustvar, const TString & stime, const Bool_t doshift, const Bool_t dosmear,
	       const Bool_t skipdata, const Bool_t skipbkgdmc, const Bool_t savemetadata);
  ~TimeAdjuster();

  // Config
  void SetupCommon();
  void SetupInFilesConfig();
  void SetupStrings();

  // Main calls
  void AdjustTime();
  void PrepAdjustments(FitStruct & FitInfo);
  void CorrectData(FitStruct & DataInfo);
  void CorrectMC(FitStruct & DataInfo, FitStruct & MCInfo);
  void DeleteInfo(FitStruct & FitInfo);
  
  // Getting adjustments ready
  void GetInputMuHists(FitStruct & FitInfo);
  void GetInputSigmaHists(FitStruct & FitInfo);
  void GetInputSigmaFits(FitStruct & FitInfo);

  // Meta data
  void MakeConfigPave(TFile *& SkimFile);

private:
  // settings
  const TString fSkimFileName;
  const TString fSignalSkimFileName;
  const TString fInFilesConfig;
  const TString fSAdjustVar;
  const TString fSTime;
  const Bool_t  fDoShift;
  const Bool_t  fDoSmear;
  const Bool_t  fSkipData;
  const Bool_t  fSkipBkgdMC;
  const Bool_t  fSaveMetaData;

  // config strings
  TString fSTimeSHIFT;
  TString fSTimeSMEAR;

  // inputs
  std::map<TString,TString> fInFileNameMap;
  std::map<TString,TFile*> fInFileMap;

  // I/O
  TFile * fSkimFile;
  TFile * fSignalSkimFile;
};

#endif
