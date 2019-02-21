#ifndef __ABCD__
#define __ABCD__

#include "../Common.hh"

#include "TFile.h"
#include "TString.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

struct BinXY
{
  BinXY () {}
  BinXY (const Int_t ibinX, const Int_t ibinY)
    : ibinX(ibinX), ibinY(ibinY) {}

  Int_t ibinX;
  Int_t ibinY;
};

namespace ABCD
{
  // Bin ranges
  extern Int_t nbinsX;
  extern Int_t nbinsY;
  extern Int_t nMaxBins;
  extern Int_t nbinsXY;

  // Maps for generating ABCD layout
  extern std::map<Int_t,BinXY> BinMap; // Global bin number to histogram x,y bin numbers
  extern std::map<Int_t,Int_t> RatioMap; // Ratio of bins name to numerator bin number
  extern std::map<Int_t,std::vector<Int_t> > BinRatioVecMap; // Global bin number to all ratios modifying it

  // text file ext
  static const TString inTextExt = "txt";

  // Datacard/WS names
  static const TString bkg1name = "bkg1";
  static const TString ratiobase = "c";
  static const TString binbase = "Bin";

  // Setup Functions
  void SetupNBins(const TH2F * hist);
  void SetupBinMap(const TString & binInfoName);
  void SetupRatioMap(const TString & ratioInfoName);
  void SetupBinRatioVecMap(const TString & binRatioInfoName);

  // Dump ABCD Maps
  void DumpInfo();
};

#endif
