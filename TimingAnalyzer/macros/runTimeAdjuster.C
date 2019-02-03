#include "TString.h"
#include "Common.cpp+"
#include "TimeAdjuster.cpp+"

void runTimeAdjuster(const TString & skimfilename, const TString & signalskimfilename, const TString & infilesconfig, 
		     const TString & sadjustvar, const TString & stime, const Bool_t doshift, const Bool_t dosmear, const Bool_t savemetadata)
{
  TimeAdjuster adjuster(skimfilename,signalskimfilename,infilesconfig,sadjustvar,stime,doshift,dosmear,savemetadata);
  adjuster.AdjustTime();
}
