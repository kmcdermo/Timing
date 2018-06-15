#include "TString.h"
#include "Common.cpp+"
#include "FastSkimmer.cpp+"

void runFastSkimmer(const TString & cutflowconfig, const TString & pdname, const TString outfiletext, 
		    const Bool_t doskim = true, const TString & sampleconfig = "")
{
  FastSkimmer skimmer(cutflowconfig,pdname,outfiletext,doskim,sampleconfig);
  skimmer.MakeSkim();
}
