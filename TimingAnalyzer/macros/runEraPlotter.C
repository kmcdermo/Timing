#include "TString.h"
#include "Common.cpp+"
#include "EraPlotter.cpp+"

void runEraPlotter(const TString & eraplotconfig, const TString & plotconfig, const Bool_t savemetadata, const TString & outfiletext)
{
  EraPlotter plotter(eraplotconfig,plotconfig,savemetadata,outfiletext);
  plotter.MakeEraPlot();
}
