#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & cutconfig, const TString & plotconfig, const TString & pdname, const Bool_t scalearea, const TString & outfiletext)
{
  TreePlotter plotter(cutconfig,plotconfig,pdname,scalearea,outfiletext);
  plotter.MakePlot();
}
