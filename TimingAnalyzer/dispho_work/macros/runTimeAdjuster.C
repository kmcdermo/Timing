#include "TString.h"
#include "Common.cpp+"
#include "TimeAdjuster.cpp+"

void runTimeAdjuster(const TString & skimfilename, const TString & signalskimfilename, const TString & infilesconfig)
{
  TimeAdjuster adjuster(skimfilename,signalskimfilename,infilesconfig);
  adjuster.AdjustTime();
}
