#include "TString.h"
#include "Common.cpp+"
#include "SignalSkimmer.cpp+"

void runSignalSkimmer(const TString & cutflowconfig, const TString & outfiletext)
{
  SignalSkimmer skimmer(cutflowconfig,outfiletext);
  skimmer.MakeSkims();
}
