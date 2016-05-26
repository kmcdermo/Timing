#ifndef _stackplots_
#define _stackplots_

#include "Config.hh"
#include "Common.hh"

#include "TPad.h"
#include "THStack.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"

typedef std::vector<TFile*>   TFileVec;
typedef std::vector<TH1F*>    TH1FVec;
typedef std::vector<TH1FVec>  TH1FVecVec;
typedef std::vector<THStack*> THStackVec;
typedef std::vector<TLine*>   TLineVec;
typedef std::vector<TLegend*> TLegVec;
typedef std::vector<TCanvas*> TCanvVec;
typedef std::vector<TPad*>    TPadVec;

class StackPlots
{
public:
  StackPlots();

  void InitTH1FNamesAndSubDNames();
  void OpenInputFiles();
  void InitInputPlots();
  void InitOutputPlots();
  void InitOutputLegends();
  void InitRatioPlots();
  void InitRatioLines();
  void InitOutputCanvPads();

  void DoStacks(std::ofstream & yields);

  void MakeStackPlots(std::ofstream & yields);
  void MakeRatioPlots();
  void MakeOutputCanvas();

  void DrawUpperPad(const Int_t th1f, const Bool_t isLogY);
  Float_t GetMaximum(const Int_t th1f);
  Float_t GetMinimum(const Int_t th1f);
  void DrawLowerPad(const Int_t th1f);
  void SetLines(const Int_t th1f);
  void SaveCanvas(const Int_t th1f, const Bool_t isLogY);

  ~StackPlots();

private:
  TStrVec fDataNames;
  TStrVec fMCNames;
  TStrVec fTH1FNames;
  TStrMap fTH1FSubDMap;
  
  Int_t fNData;
  Int_t fNMC;
  Int_t fNTH1F;

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

  TCanvVec   fOutTH1FCanvases;
  TPadVec    fOutTH1FStackPads;
  TPadVec    fOutTH1FRatioPads;

  TString fOutDir;
  TString fOutName;
  TFile * fOutFile;
};

#endif
