#ifndef _stackeffs_
#define _stackeffs_

#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TGraphAsymmErrors.h"

typedef std::vector<TEfficiency*> TEffVec;
typedef std::vector<TEffVec>      TEffVecVec;

typedef std::vector<TGraphAsymmErrors*> TGAEVec;
typedef std::vector<TGAEVec>            TGAEVecVec;  

class StackEffs
{
public:
  StackEffs();
  ~StackEffs();

  void InitSampleNames();
  void InitTEffNamesAndSubDNames();
  void OpenInputFiles();
  void InitInputPlots();
  void InitOutputPlots();
  void InitOutputLegends();
  void InitOutputCanvases();

  void DoStacks();

  void MakeStackEffs();
  void MakeOutputCanvas();

  void PrintEffHeader(const Int_t teff);
  void PrintTotalEff(const Int_t teff, const Int_t sample);
  void PrintEffFooter();

  void DrawCanvas(const Int_t th1f);
  void SaveCanvas(const Int_t th1f);

private:
  TStrVec fSampleNames;
  TStrVec fTEffNames;
  TStrMap fTEffSubDMap;

  Int_t fNSample;
  Int_t fNTEff;

  TFileVec fSampleFiles;

  TEffVecVec fInTEffs;
  TGAEVecVec fOutTGAEs;

  TCanvVec   fOutCanvases;
  TLegVec    fOutLegends;

  TString fOutDir;
  TFile * fOutFile;
  std::ofstream fEffDump;
};

#endif
