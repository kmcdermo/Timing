#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter2D.cpp+"

void runTreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		      const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
		      const TString & era, const TString & outfilename)
{
  TreePlotter2D plotter(infilename,insignalfilename,cutconfig,varwgtmapconfig,plotconfig,miscconfig,era,outfilename);
  plotter.MakeTreePlot2D();
}
