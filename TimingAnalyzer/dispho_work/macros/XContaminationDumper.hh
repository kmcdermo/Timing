#ifndef __XContaminationDumper__
#define __XContaminationDumper__

// ROOT inludes
#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TString.h"

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

// Dumper info
struct ContInfo
{
  ContInfo(){}
  ContInfo(TH2F * hist) : hist(hist) {}

  TH2F * hist;

  Double_t int_full;
  Double_t int_err_full;
  Double_t frac_full;
  Double_t frac_err_full;

  Double_t int_1D;
  Double_t int_err_1D;
  Double_t frac_1D;
  Double_t frac_err_1D;

  Double_t int_2D;
  Double_t int_err_2D;
  Double_t frac_2D;
  Double_t frac_err_2D;
};

class XContaminationDumper
{
public:
  XContaminationDumper(const TString & infilename, const TString & xcontdumpconfig, 
		       const TString & plotconfig, const TString & era, const TString & outfiletext);
  ~XContaminationDumper();

  // Initialize
  void SetupCommon();
  void SetupXContDumpConfig();
  void SetupSampleVec();
  void SetupOutSignalHists();

  // Additional setup functions
  void SetupLambdas(const std::string & str);
  void SetupCTaus(const std::string & str);
  TH2F * MakeSignalHist(const TString & name, const TString & title);
  
  // Main calls
  void MakeContaminationDump();
  void PrepContMap();
  void ComputeDumpInfos();
  void DumpTextFile();
  void FillSignalHists();
  void PrintSignalHists();

  // Dump meta info
  void MakeConfigPave();

  // Worker functions
  void ComputeIntegrals(const TString & sample);
  void ComputeFractions(const TString & sample);

private:
  // input config
  const TString fInFileName;
  const TString fXContDumpConfig;
  const TString fPlotConfig;
  const TString fEra;
  const TString fOutFileText;

  // style
  TStyle * fTDRStyle;

  // input
  TFile * fInFile;

  // internal members / settings read from config
  Variable fVar1D;
  std::vector<BlockStruct> fBlocks1D;
  std::vector<BlockStruct> fBlocks2D;
  std::vector<TString> fLambdas;
  std::vector<TString> fCTaus;

  Bool_t fXVarBins;
  Bool_t fYVarBins;

  // I/O
  std::vector<TString> fSampleVec;
  std::map<TString,ContInfo> fContMap;

  // output
  TFile * fOutFile;
  std::map<TString,TH2F*> fHistMap;
  TPaveText * fConfigPave;
};

#endif

