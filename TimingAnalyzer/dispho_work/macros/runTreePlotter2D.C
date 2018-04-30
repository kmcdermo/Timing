#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter2D.cpp+"

void runTreePlotter2D(const TString & infilename, const TString & cutconfig, const TString & plotconfig, const TString & outfilename)
{
  TreePlotter2D plotter(infilename,cutconfig,plotconfig,outfilename);
  plotter.MakePlot();
}
