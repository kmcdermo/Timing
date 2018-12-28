#ifndef __CommonTimeFit__
#define __CommonTimeFit__

// ROOT inludes
#include "TFormula.h"
#include "TF1.h"
#include "TH1F.h"

// STL includes
#include <iostream>
#include <cmath>

// Common include
#include "Common.hh"

// Time fit enum
enum TimeFitType {Gaus1, Gaus1core, Gaus2fm, Gaus2fmcore, Gaus3fm, Gaus3fmcore};

namespace Common
{
  void SetupTimeFitType(const std::string & str, TimeFitType & type);
};

// fit result struct
struct TimeFitResult
{
  TimeFitResult() {}
  
  Float_t chi2ndf;
  Float_t chi2prob;
  Float_t mu;
  Float_t emu;
  Float_t sigma;
  Float_t esigma;
};

struct TimeFitStruct 
{
  TimeFitStruct() {}
  TimeFitStruct(const TimeFitType type, const Float_t rangeLow, const Float_t rangeUp)
    : type(type), rangeLow(rangeLow), rangeUp(rangeUp) {}
  
  // helper functions for making fits to variables
  Bool_t isEmpty() const {return (hist->GetEntries() == 0);}
  void PrepFit();
  void DoFit();
  void GetFitResult();

  // cleanup
  void DeleteInternal();

  // internal data members
  TimeFitType type;
  Float_t rangeLow;
  Float_t rangeUp;
  TH1F * hist;
  Bool_t varBinsX;
  TFormula * form;
  TF1 * fit;
  TimeFitResult result;
};

#endif
