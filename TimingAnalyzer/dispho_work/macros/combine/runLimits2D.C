#include "../Common.cpp+"
#include "Combine.cpp+"
#include "Limits2D.cpp+"

void runLimits2D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext)
{
  Limits2D LimitPlotter(indir,infilename,doobserved,outtext);
  LimitPlotter.MakeLimits2D();
}
