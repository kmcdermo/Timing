#include "TString.h"
#include "Common.cpp+"
#include "CommonTimeFit.cpp+"
#include "TimeVsRunFitter.cpp+"

void runTimeVsRunFitter(const TString & infilename, const TString & plotconfig, const TString & timefitconfig,
			const Bool_t savemetadata, const TString & outfiletext)
{
  TimeVsRunFitter fitter(infilename,plotconfig,timefitconfig,savemetadata,outfiletext);
  fitter.MakeTimeVsRunFits();
}
