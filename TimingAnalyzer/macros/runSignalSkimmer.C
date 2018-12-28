#include "TString.h"
#include "Common.cpp+"
#include "SignalSkimmer.cpp+"

void runSignalSkimmer(const TString & cutflowconfig, const TString & inskimdir, const TString & outfiletext)
{
  SignalSkimmer skimmer(cutflowconfig,inskimdir,outfiletext);
  skimmer.MakeSkims();
}
