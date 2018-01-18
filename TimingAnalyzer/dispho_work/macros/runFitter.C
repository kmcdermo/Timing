#include "TString.h"
#include "common/Common.cpp+"
#include "Fitter.cpp+"

void runFitter(const TString & infilename, const TString & outfilename)
{
  Fitter fitter(infilename,outfilename);
  fitter.MakeFits();
}
