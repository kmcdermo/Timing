#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter2D.cpp+"

void runTreePlotter2D(const TString & cutconfig, const TString & plotconfig, const TString & pdname, const TString & outfilename)
{
  TreePlotter2D plotter(cutconfig,plotconfig,pdname,outfilename);
  plotter.MakePlot();
}
