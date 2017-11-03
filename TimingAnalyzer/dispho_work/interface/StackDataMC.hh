#ifndef _stackdatamc_
#define _stackdatamc_

#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

class StackDataMC
{
public:
  StackDataMC();

  void InitTH1FNamesAndSubDNames();
  void OpenInputFiles();
  void InitInputPlots();
  void InitOutputPlots();
  void InitOutputLegends();
  void InitRatioPlots();
  void InitRatioLines();
  void InitOutputCanvPads();

  void DoStacks(std::ofstream & yields);

  void MakeStackDataMC(std::ofstream & yields);
  void MakeRatioPlots();
  void MakeOutputCanvas();

  void DrawUpperPad(const Int_t th1f, const Bool_t isLogY);
  Float_t GetMaximum(const Int_t th1f);
  Float_t GetMinimum(const Int_t th1f);
  void DrawLowerPad(const Int_t th1f);
  void SetLines(const Int_t th1f);
  void SaveCanvas(const Int_t th1f, const Bool_t isLogY);

  ~StackDataMC();

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
  TFile * fOutFile;
};

#endif
