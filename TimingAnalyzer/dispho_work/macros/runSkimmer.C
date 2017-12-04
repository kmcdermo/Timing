#include "TString.h"
#include "common/Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename)
{
  Skimmer skimmer(indir, outdir, filename);
  skimmer.EventLoop();
}
