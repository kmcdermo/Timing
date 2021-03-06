#include "TString.h"
#include "Common.cpp+"
#include "TreePlotter.cpp+"
#include "RescalePlotter.cpp+"

void runRescalePlotter(const TString & infilename, const TString & rescaleconfig, const TString & plotconfig,
		       const TString & miscconfig, const TString & era, const Bool_t savemetadata, const TString & outfiletext)
{
  RescalePlotter plotter(infilename,rescaleconfig,plotconfig,miscconfig,era,savemetadata,outfiletext);
  plotter.MakeRescaledPlot();
}
