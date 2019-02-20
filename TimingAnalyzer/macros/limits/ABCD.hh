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
  // Maps for generating ABCD layout
  std::map<Int_t,BinXY> BinMap; // Global bin number to histogram x,y bin numbers
  std::map<Int_t,Int_t> RatioMap; // Ratio of bins name to numerator bin number
  std::map<Int_t,std::vector<Int_t> > BinRatioVecMap; // Global bin number to all ratios modifying it

  // Setup Functions
  void SetupBinMap(const TString & binInfoName);
  void SetupRatioMap(const TString & ratioInfoName);
  void SetupBinRatioVecMap(const TString & binRatioInfoName);

  // Dump ABCD Maps
  void DumpInfo();
};

#endif
