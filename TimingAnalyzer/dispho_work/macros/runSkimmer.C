#include "TString.h"
#include "common/Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename, const Float_t sumwgts)
{
  Skimmer skimmer(indir, outdir, filename, sumwgts);
  skimmer.EventLoop();
}
