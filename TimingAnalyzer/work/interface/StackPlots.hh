#ifndef _stackplots_
#define _stackplots_

#include "Common.hh"

#include "TROOT.h"
#include "TSystem.h"
#include "TPad.h"
#include "TCanvas.h"
#include "THStack.h"
#include "TLegend.h"
#include "TColor.h"
#include "TGaxis.h"
#include "TLine.h"
#include "TLatex.h"

#include <utility>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

typedef std::vector<TFile*>   TFileVec;
typedef std::vector<TH1F*>    TH1FVec;
typedef std::vector<TH1FVec>  TH1FVecVec;
typedef std::vector<THStack*> THStackVec;
typedef std::vector<TLine*>   TLineVec;
typedef std::vector<TLegend*> TLegVec;
typedef std::vector<TCanvas*> TCanvVec;
typedef std::vector<TPad*>    TPadVec;
typedef std::vector<TGaxis*>  TGaxisVec;

class StackPlots
{
public:
  StackPlots(const SamplePairVec Samples, const TString selection, const Int_t njetsselection, const Double_t lumi, const TString outname, const ColorMap & colorMap, const TString outtype);

  void InitTH1FNamesAndSubDNames();
  void OpenInputFiles();
  void InitInputPlots();
  void InitOutputPlots();
  void InitOutputLegends();
  void InitRatioPlots();
  void InitRatioLines();
  void InitOutputCanvPads();
  void InitUpperAxes();

  void DoStacks(std::ofstream & yields);

  void MakeStackPlots(std::ofstream & yields);
  void MakeRatioPlots();
  void MakeOutputCanvas();

  void DrawUpperPad(const UInt_t th1d, const Bool_t isLogY);
  Double_t GetMaximum(const UInt_t th1d);
  Double_t GetMinimum(const UInt_t th1d);
  void SetUpperAxes(const UInt_t th1d);
  void DrawLowerPad(const UInt_t th1d);
  void SetLines(const UInt_t th1d);
  void SaveCanvas(const UInt_t th1d, const Bool_t isLogY);
  void CMSLumi(TCanvas *& pad, const Int_t iPosX);

  ~StackPlots();

private:
  TStrVec fDataNames;
  TStrVec fMCNames;
  TStrVec fTH1FNames;
  TStrMap fTH1FSubDMap;
  TString fSelection;
  Int_t   fNJetsSeln;

  TString fNJetsStr;

  Double_t fLumi;
  
  UInt_t fNData;
  UInt_t fNMC;
  UInt_t fNTH1F;

  TFileVec fDataFiles;
  TFileVec fMCFiles;

  TH1FVecVec fInDataTH1FHists;
  TH1FVecVec fInMCTH1FHists;

  TH1FVec    fOutDataTH1FHists;
  TH1FVec    fOutMCTH1FHists;
  THStackVec fOutMCTH1FStacks;
  TH1FVec    fOutRatioTH1FHists;  
  TH1FVec    fOutRatioMCErrs;  
  TLegVec    fTH1FLegends;
  TLineVec   fOutTH1FRatioLines;

  TGaxisVec  fOutTH1FTGaxes;

  TCanvVec   fOutTH1FCanvases;
  TPadVec    fOutTH1FStackPads;
  TPadVec    fOutTH1FRatioPads;

  TString fOutDir;
  TString fOutName;
  TFile * fOutFile;
  TString fOutType;

  TStrMap  fSampleTitleMap;
  ColorMap fColorMap;
};

#endif
