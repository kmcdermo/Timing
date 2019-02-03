#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		    const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
		    const TString & era, const Bool_t savemetadata, const TString & outfiletext)
{
  TreePlotter plotter(infilename,insignalfilename,cutconfig,varwgtmapconfig,plotconfig,miscconfig,era,savemetadata,outfiletext);
  plotter.MakeTreePlot();
}
