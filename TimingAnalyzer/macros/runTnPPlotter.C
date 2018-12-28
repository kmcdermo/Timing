#include "TString.h"
#include "Common.cpp+"
#include "TnPPlotter.cpp+"

void runTnPPlotter(const TString & infilename, const TString & outfiletext)
{
  TnPPlotter plotter(infilename,outfiletext);
  plotter.MakeTnPPlots();
}
