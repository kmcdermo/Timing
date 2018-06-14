#include "TString.h"
#include "Common.cpp+"
#include "FastSkimmer.cpp+"

void runFastSkimmer(const TString & cutflowconfig, const TString & pdname, const TString outfiletext, 
		    const TString & sampleconfig = "", const Bool_t doskim = true)
{
  FastSkimmer skimmer(cutflowconfig,pdname,outfiletext,doskim);
  skimmer.MakeSkim();
}
