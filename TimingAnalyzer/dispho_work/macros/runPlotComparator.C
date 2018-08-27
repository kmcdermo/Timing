#include "TString.h"
#include "Common.cpp+"
#include "PlotComparator.cpp+"

void runPlotComparator(const TString & compareconfig, const TString & outfiletext)
{
  PlotComparator plotter(compareconfig,outfiletext);
  plotter.MakeComparisonPlot();
}
