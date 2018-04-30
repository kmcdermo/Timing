#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & infilename, const TString & cutconfig, const TString & plotconfig, const Bool_t scalearea, const TString & outfiletext)
{
  TreePlotter plotter(infilename,cutconfig,plotconfig,scalearea,outfiletext);
  plotter.MakePlot();
}
