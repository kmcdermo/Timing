#include "TString.h"
#include "Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename,
		const Float_t sumwgts, const TString & puwgtfilename = "", const TString & skimtype = "Standard")
{
  Skimmer skimmer(indir, outdir, filename, sumwgts, puwgtfilename, skimtype);
  skimmer.EventLoop();
}
