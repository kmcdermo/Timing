#include "TString.h"
#include "Common.cpp+"
#include "PlotComparator.cpp+"

void runPlotComparator(const TString & compareconfig, const TString & era,
		       const Bool_t savemetadata, const TString & outfiletext)
{
  PlotComparator plotter(compareconfig,era,savemetadata,outfiletext);
  plotter.MakeComparisonPlot();
}
