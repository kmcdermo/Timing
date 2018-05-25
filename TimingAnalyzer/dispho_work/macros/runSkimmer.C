#include "TString.h"
#include "Common.cpp+"
#include "Skimmer.cpp+"

void runSkimmer(const TString & indir, const TString & outdir, const TString & filename, 
		const Float_t sumwgts, const TString & puwgtfile = "", const Bool_t redophoid = false)
{
  Skimmer skimmer(indir, outdir, filename, sumwgts, puwgtfile, redophoid);
  skimmer.EventLoop();
}
