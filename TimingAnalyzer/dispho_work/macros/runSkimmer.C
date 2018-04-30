#include "TString.h"
#include "Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename, const Float_t sumwgts, const Bool_t redophoid = false)
{
  Skimmer skimmer(indir, outdir, filename, sumwgts, redophoid);
  skimmer.EventLoop();
}
