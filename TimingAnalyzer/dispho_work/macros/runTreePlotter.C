#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		    const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
		    const TString & outfiletext)
{
  TreePlotter plotter(infilename,insignalfilename,cutconfig,varwgtmapconfig,plotconfig,miscconfig,outfiletext);
  plotter.MakeTreePlot();
}
