#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter2D.cpp+"
#include "EraTreePlotter2D.cpp+"

void runEraTreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
			 const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
			 const TString & era, const TString & outfiletext)
{
  EraTreePlotter2D plotter(infilename,insignalfilename,cutconfig,varwgtmapconfig,plotconfig,miscconfig,era,outfiletext);
  plotter.MakeEraTreePlot2D();
}
