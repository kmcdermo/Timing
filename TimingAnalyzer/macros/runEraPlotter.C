#include "TString.h"
#include "Common.cpp+"
#include "EraPlotter.cpp+"

void runEraPlotter(const TString & eraplotconfig, const TString & plotconfig, const TString & outfiletext)
{
  EraPlotter plotter(eraplotconfig,plotconfig,outfiletext);
  plotter.MakeEraPlot();
}
