#include "TString.h"
#include "Common.cpp+"
#include "FastSkimmer.cpp+"

void runFastSkimmer(const TString & cutflowconfig, const TString & pdname, const TString & inskimdir,
		    const TString & outfiletext, const Bool_t doskim = true, const TString & sampleconfig = "")
{
  FastSkimmer skimmer(cutflowconfig,pdname,inskimdir,outfiletext,doskim,sampleconfig);
  skimmer.MakeSkim();
}
