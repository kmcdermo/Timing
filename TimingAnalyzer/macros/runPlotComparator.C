#include "TString.h"
#include "Common.cpp+"
#include "PlotComparator.cpp+"

void runPlotComparator(const TString & compareconfig, const TString & era, const TString & outfiletext)
{
  PlotComparator plotter(compareconfig,era,outfiletext);
  plotter.MakeComparisonPlot();
}
