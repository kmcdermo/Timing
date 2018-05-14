#include "TString.h"
#include "Common.cpp+"
#include "SignalSkimmer.cpp+"

void runSignalSkimmer(const TString & cutconfig, const TString outfiletext)
{
  SignalSkimmer skimmer(cutconfig,outfiletext);
  skimmer.MakeSkims();
}
