#include "TString.h"
#include "common/Common.cpp+"
#include "FastSkimmer.cpp+"

void runFastSkimmer(const TString & cutconfig, const TString & pdname, const TString outfiletext)
{
  FastSkimmer skimmer(cutconfig,pdname,outfiletext);
  skimmer.MakeSkim();
}
