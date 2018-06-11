#include "TString.h"
#include "Common.cpp+"
#include "FastSkimmer.cpp+"

void runFastSkimmer(const TString & cutflowconfig, const TString & pdname, const TString outfiletext)
{
  FastSkimmer skimmer(cutflowconfig,pdname,outfiletext);
  skimmer.MakeSkim();
}
