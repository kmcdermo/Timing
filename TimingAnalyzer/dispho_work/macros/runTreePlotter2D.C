#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter2D.cpp+"

void runTreePlotter2D(const TString & cutconfig, const TString & plotconfig, const TString & outfilename)
{
  TreePlotter2D plotter(cutconfig,plotconfig,outfilename);
  plotter.MakePlot();
}
