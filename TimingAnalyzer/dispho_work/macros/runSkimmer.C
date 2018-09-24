#include "TString.h"
#include "Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename,
		const Float_t sumwgts, const TString & skimtype = "Standard", const TString & puwgtfilename = "")
{
  Skimmer skimmer(indir, outdir, filename, sumwgts, skimtype, puwgtfilename);
  skimmer.EventLoop();
}
