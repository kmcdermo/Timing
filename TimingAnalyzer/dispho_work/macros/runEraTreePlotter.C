#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"
#include "EraTreePlotter.cpp+"

void runEraTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		       const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
		       const TString & era, const TString & outfiletext)
{
  EraTreePlotter plotter(infilename,insignalfilename,cutconfig,varwgtmapconfig,plotconfig,miscconfig,era,outfiletext);
  plotter.MakeEraTreePlot();
}
