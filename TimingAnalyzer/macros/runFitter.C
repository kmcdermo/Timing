#include "TString.h"
#include "Common.cpp+"
#include "Fitter.cpp+"

void runFitter(const TString & fitconfig, const TString & miscconfig,
	       const Bool_t savemetadata, const TString & outfiletext)
{
  Fitter fitter(fitconfig,miscconfig,savemetadata,outfiletext);
  fitter.DoMain();
}
