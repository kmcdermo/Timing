#include "TString.h"
#include "Common.cpp+"
#include "Fitter.cpp+"

void runFitter(const TString & fitconfig, const TString & miscconfig, const TString & outfiletext)
{
  Fitter fitter(fitconfig,miscconfig,outfiletext);
  fitter.DoMain();
}
