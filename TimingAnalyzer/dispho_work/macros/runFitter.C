#include "TString.h"
#include "Common.cpp+"
#include "Fitter.cpp+"

void runFitter(const TString & fitconfig, const TString & outfiletext)
{
  Fitter fitter(fitconfig,outfiletext);
  fitter.DoMain();
}
