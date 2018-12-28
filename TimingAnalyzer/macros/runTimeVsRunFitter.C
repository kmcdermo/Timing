#include "TString.h"
#include "Common.cpp+"
#include "CommonTimeFit.cpp+"
#include "TimeVsRunFitter.cpp+"

void runTimeVsRunFitter(const TString & infilename, const TString & plotconfig,
			const TString & timefitconfig, const TString & outfiletext)
{
  TimeVsRunFitter fitter(infilename,plotconfig,timefitconfig,outfiletext);
  fitter.MakeTimeVsRunFits();
}
