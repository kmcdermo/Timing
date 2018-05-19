#include "../Common.cpp+"
#include "Combine.cpp+"
#include "Limits1D.cpp+"

void runLimits1D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext)
{
  Limits1D LimitPlotter(indir,infilename,doobserved,outtext);
  LimitPlotter.MakeLimits1D();
}
