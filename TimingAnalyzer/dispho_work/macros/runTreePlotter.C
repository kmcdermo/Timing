#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		    const TString & plotconfig, const TString & miscconfig, const TString & outfiletext)
{
  TreePlotter plotter(infilename,insignalfilename,cutconfig,plotconfig,miscconfig,outfiletext);
  plotter.MakePlot();
}
