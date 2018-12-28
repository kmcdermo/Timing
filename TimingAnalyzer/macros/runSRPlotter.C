#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"
#include "SRPlotter.cpp+"

void runSRPlotter(const TString & srplotconfig, const TString & miscconfig, const TString & era, const TString & outfiletext)
{
  SRPlotter plotter(srplotconfig,miscconfig,era,outfiletext);
  plotter.MakePlots();
}
