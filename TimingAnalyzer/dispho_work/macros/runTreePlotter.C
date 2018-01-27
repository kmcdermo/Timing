#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & cutconfig, const TString & plotconfig, const TString & outfiletext)
{
  TreePlotter plotter(cutconfig,plotconfig,outfiletext);
  plotter.MakePlot();
}
