#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		    const TString & plotsignals, const TString & plotconfig, const TString & miscplotconfig, const TString & outfiletext)
{
  TreePlotter plotter(infilename,insignalfilename,cutconfig,plotsignals,plotconfig,miscplotconfig,outfiletext);
  plotter.MakePlot();
}
