#ifndef __ABCDGenerator__
#define __ABCDGenerator__

///////////////////////////////////////////////////////////////////////
// Input: NxM binning.                                               //
//                                                                   //
// First we generate ABCD grid for NxN binning, where N >= M, and    //
// then prune to size requested of NxM. This keeps the bin numbering //
// consistent across different choices of NxM.                       //
//                                                                   //
// Output: Text files to be read by WS+Datacard Writer mapping bin   //
// scheme to ABCD equations and info needed for WS+datacards         //
//                                                                   //
// Example numbering for x=5,y=3 bins                                //
//  ___ ___ ___ ____ ____                                            //
// |_5_|_6_|_7_|_14_|_23_|                                           //
// |_2_|_3_|_8_|_15_|_24_|                                           //
// |_1_|_4_|_9_|_16_|_25_|                                           //
//                                                                   //
///////////////////////////////////////////////////////////////////////

#include "ABCD.cpp+"

#include <vector>
#include <fstream>

class ABCDGenerator
{
public:
  ABCDGenerator(const Int_t nbinsX, const Int_t nbinsY);
  ~ABCDGenerator() {}

  // main call
  void ProduceABCDConfig();

  // calls for making config
  void MakeMaxGrid();
  void PruneGrid();
  void WriteConfigs();

private:
  const Int_t fNbinsX;
  const Int_t fNbinsY;
  
  Int_t fNMaxBins;
};

#endif
