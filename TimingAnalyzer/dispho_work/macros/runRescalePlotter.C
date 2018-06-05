#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"
#include "RescalePlotter.cpp+"

void runRescalePlotter(const TString & infilename, const TString & rescaleconfig,
		       const TString & miscconfig, const TString & outfiletext)
{
  RescalePlotter plotter(infilename,rescaleconfig,miscconfig,outfiletext);
  plotter.MakeRescaledPlot();
}
