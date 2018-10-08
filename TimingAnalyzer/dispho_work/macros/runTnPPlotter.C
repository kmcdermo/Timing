#include "TString.h"
#include "Common.cpp+"
#include "TnPPloter.cpp+"

void runTnPPloter(const TString & infilename, const TString & outfiletext)
{
  TnPPloter plotter(infilename,outfiletext);
  plotter.MakeTnPPlots();
}
