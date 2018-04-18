#include "TString.h"
#include "common/Common.cpp+"
#include "FastSkimmer.cpp+"

void runFastSkimmer(const TString & cutconfig, const TString & pdname)
{
  FastSkimmer skimmer(cutconfig,pdname);
  skimmer.MakeSkim();
}
