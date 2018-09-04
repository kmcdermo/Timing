#include "TString.h"
#include "Common.cpp+"
#include "TimeFitter.cpp+"

void runTimeFitter(const TString & infilename, const TString & plotconfig, const TString & timefitconfig,
		   const TString & era, const TString & outfiletext)
{
  TimeFitter fitter(infilename,plotconfig,timefitconfig,era,outfiletext);
  fitter.MakeTimeFits();
}
