#include "../Common.cpp+"
#include "Limits1D.cpp+"

void runLimits1D(const TString & indir, const TString & infilename, const TString & outtext)
{
  Limits1D LimitPlotter(indir,infilename,outtext);
  LimitPlotter.MakeLimits1D();
}
