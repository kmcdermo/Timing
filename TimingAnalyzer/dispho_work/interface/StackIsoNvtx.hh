#ifndef _stackisonvtx_
#define _stackisonvtx_

#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TPad.h"
#include "THStack.h"
#include "TLegend.h"
#include "TLine.h"
#include "TLatex.h"

typedef std::vector<TFile*>   TFileVec;
typedef std::vector<TH1F*>    TH1FVec;
typedef std::vector<TLine*>   TLineVec;
typedef std::vector<TLegend*> TLegVec;
typedef std::vector<TCanvas*> TCanvVec;
typedef std::vector<TPad*>    TPadVec;

class StackIsoNvtx
{
public:
  StackIsoNvtx(const TString & sample, const Bool_t isMC);

  void InitTH1FNamesAndSubDNames();
  void InitInputPlots();
  void InitOutputLegends();
  void InitRatioPlots();
  void InitRatioLines();
  void InitOutputCanvPads();

  void DoStacks();

  void MakeStackIsoNvtx();
  void MakeRatioPlots();
  void MakeOutputCanvas();

  void DrawUpperPad(const Int_t th1f);
  Float_t GetMaximum(const Int_t th1f);
  Float_t GetMinimum(const Int_t th1f);
  void DrawLowerPad(const Int_t th1f);
  void SetLines(const Int_t th1f);
  void SaveCanvas(const Int_t th1f);

  ~StackIsoNvtx();

private:
  const TString fSample;
  const Bool_t fIsMC;

  TFile * fInFile;

  Int_t   fNTH1F;
  TStrVec fTH1FNames;
  TStrMap fTH1FSubDMap;
  
  TH1FVec fInGEDTH1FHists;
  TH1FVec fInOOTTH1FHists;

  TH1FVec  fOutRatioTH1FHists;  
  TLegVec  fTH1FLegends;
  TLineVec fOutTH1FRatioLines;

  TCanvVec fOutTH1FCanvases;
  TPadVec  fOutTH1FStackPads;
  TPadVec  fOutTH1FRatioPads;

  TString fOutDir;
  TFile * fOutFile;
};

#endif
