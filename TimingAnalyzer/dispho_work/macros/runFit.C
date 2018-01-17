#include "TString.h"
#include "common/Common.cpp+"
#include "Fitter2D.cpp+"

void runFit(const TString & infilename, const TString & outfilename)
{
  Fitter2D fitter(infilename,outfilename);
  fitter.MakeFit();
}
