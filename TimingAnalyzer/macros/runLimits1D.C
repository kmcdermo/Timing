#include "Common.cpp+"
#include "Combine.cpp+"
#include "Limits1D.cpp+"

void runLimits1D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & era, const TString & outtext)
{
  Limits1D LimitPlotter(indir,infilename,doobserved,era,outtext);
  LimitPlotter.MakeLimits1D();
}
