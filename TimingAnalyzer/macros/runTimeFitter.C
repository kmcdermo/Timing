#include "TString.h"
#include "Common.cpp+"
#include "CommonTimeFit.cpp+"
#include "TimeFitter.cpp+"

void runTimeFitter(const TString & infilename, const TString & plotconfig, const TString & miscconfig, const TString & timefitconfig,
		   const TString & era, const Bool_t savemetadata, const TString & outfiletext)
{
  TimeFitter fitter(infilename,plotconfig,miscconfig,timefitconfig,era,savemetadata,outfiletext);
  fitter.MakeTimeFits();
}
