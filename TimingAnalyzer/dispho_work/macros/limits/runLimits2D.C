#include "../Common.cpp+"
#include "Combine.cpp+"
#include "Limits2D.cpp+"

void runLimits2D(const TString & indir, const TString & infilename, const TString & limitconfig, const TString & era, const TString & outtext)
{
  Limits2D LimitPlotter(indir,infilename,limitconfig,era,outtext);
  LimitPlotter.MakeLimits2D();
}
