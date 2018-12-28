#ifndef __Limits2D__
#define __Limits2D__

#include "TROOT.h"
#include "TStyle.h"
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"
#include "TLegend.h"
#include "TCanvas.h"
#include "TPaveText.h"

#include "../Common.hh"
#include "Combine.hh"

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <utility>
#include <map>
#include <algorithm>
#include <string>

struct Bin2D
{
  Bin2D() {}
  Bin2D(const Float_t xlow, const Float_t xcenter, const Float_t xup, 
	const Float_t ylow, const Float_t ycenter, const Float_t yup,
	const std::map<TString,Float_t> & rvalmap)
  : xlow(xlow), xcenter(xcenter), xup(xup), 
    ylow(ylow), ycenter(ycenter), yup(yup),
    rvalmap(std::move(rvalmap)) {}

  // bin info
  Float_t xlow;
  Float_t xcenter;
  Float_t xup;
  Float_t ylow;
  Float_t ycenter;
  Float_t yup;

  // rvalues for filling (z): copied from GMSBMap
  std::map<TString,Float_t> rvalmap;
};

const auto sortPairs = [](const auto & obj1, const auto & obj2){return obj1.second < obj2.second;};

class Limits2D
{
public:
  Limits2D(const TString & indir, const TString & infilename, const TString & limitconfig, const TString & era, const TString & outtext);
  ~Limits2D();

  // setup functions
  void SetupDefaults();
  void SetupLimitConfig();
  void SetupCommon();
  void SetupCombine();

  // main plotting routine
  void MakeLimits2D();

  // main subroutines
  void FillKnownBins();
  void InterpolateKnownBins();
  void DumpAllBins();
  void GetHistBinBoundaries();
  void FillRValHists();
  void DrawLimits();
  void MakeConfigPave();

  // Helper functions
  Float_t ZValue(const Float_t x, const Float_t x1, const Float_t x2, 
		 const Float_t y, const Float_t y1, const Float_t y2, 
		 const Float_t fQ11, const Float_t fQ12, const Float_t fQ21, const Float_t fQ22);

private:
  const TString fInDir;
  const TString fInFileName;
  const TString fLimitConfig;
  const TString fEra;
  const TString fOutText;
  
  // config parameters
  Bool_t fDoObserved;
  Bool_t fDumpBins;
  Float_t fXMinWidthDiv;
  Float_t fYMinWidthDiv;
  Int_t fNX_Interp;
  Int_t fNY_Interp;

  // stored info
  std::vector<std::vector<Bin2D> > fKnownBins;
  std::vector<Bin2D> fAllBins;

  // bin boundaries
  std::vector<Double_t> fXBins;
  std::vector<Double_t> fYBins;

  // Histogram map
  std::map<TString,TH2F*> fHistMap;

  // style
  TStyle * fTDRStyle;

  // output
  TFile * fOutFile;

  // config pave
  TPaveText * fConfigPave;
};

#endif
