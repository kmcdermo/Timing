#include "TString.h"
#include "Common.cpp+"
#include "SuperFastSkimmer.cpp+"

void runSuperFastSkimmer(const TString & cutflowconfig, const TString & infilename,
			 const Bool_t issignalfile, const TString & outfiletext)
{
  SuperFastSkimmer skimmer(cutflowconfig,infilename,issignalfile,outfiletext);
  skimmer.MakeSkims();
}
