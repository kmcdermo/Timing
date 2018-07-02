#include "TString.h"
#include "Common.cpp+"
#include "VarWeighter.cpp+"

void runVarWeighter(const TString & varwgtconfig)
{
  VarWeighter weighter(varwgtconfig);
  weighter.MakeVarWeights();
}
