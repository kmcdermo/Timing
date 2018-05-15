#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter2D.cpp+"

void runTreePlotter2D(const TString & infilename, const TString & insignalfilename, 
		      const TString & cutconfig, const TString & plotconfig, const TString & outfilename)
{
  TreePlotter2D plotter(infilename,insignalfilename,cutconfig,plotconfig,outfilename);
  plotter.MakePlot();
}
