#include "TString.h"
#include "Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename, const TString & skimconfig)
{
  Skimmer skimmer(indir, outdir, filename, skimconfig);
  skimmer.EventLoop();
}
