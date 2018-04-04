#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & cutconfig, const TString & plotconfig, const Bool_t scalearea, const TString & outfiletext)
{
  TreePlotter plotter(cutconfig,plotconfig,scalearea,outfiletext);
  plotter.MakePlot();
}
