#include "TString.h"
#include "Common.cpp+"
#include "VarWeighter.cpp+"

void runVarWeighter(const TString & varwgtconfig, const Bool_t savemetadata)
{
  VarWeighter weighter(varwgtconfig,savemetadata);
  weighter.MakeVarWeights();
}
