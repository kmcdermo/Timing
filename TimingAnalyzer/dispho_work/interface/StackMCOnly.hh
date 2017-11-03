#ifndef _stackmconly_
#define _stackmconly_

#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

class StackMCOnly
{
public:
  StackMCOnly();
  ~StackMCOnly();

  void InitTH1FNamesAndSubDNames();
  void OpenInputFiles();
  void InitInputPlots();
  void InitOutputPlots();
  void InitOutputLegends();
  void InitOutputCanvases();

  void DoStacks();

  void MakeStackMCOnly();
  void MakeOutputCanvas();

  void DrawCanvas(const Int_t th1f, const Bool_t isLogY);
  void SaveCanvas(const Int_t th1f, const Bool_t isLogY);
  Double_t GetMaximum(const Int_t th1f);
  Double_t GetMinimum(const Int_t th1f);

private:
  TStrVec fSignalNames;
  TStrVec fBkgdNames;
  TStrVec fTH1FNames;
  TStrMap fTH1FSubDMap;
  
  Int_t fNSignal;
  Int_t fNBkgd;
  Int_t fNTH1F;

  TFileVec fSignalFiles;
  TFileVec fBkgdFiles;

  TH1FVecVec fInSignalTH1FHists;
  TH1FVecVec fInBkgdTH1FHists;

  TH1FVec    fOutBkgdTH1FHists;
  THStackVec fOutBkgdTH1FStacks;
  TLegVec    fTH1FLegends;

  TCanvVec   fOutTH1FCanvases;

  TString fOutDir;
  TFile * fOutFile;
};

#endif
