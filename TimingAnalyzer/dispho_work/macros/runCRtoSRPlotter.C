#include "TString.h"
#include "Common.cpp+"
#include "CRtoSRPlotter.cpp+"

void runCRtoSRPlotter(const TString & crtosrconfig, const TString & plotconfig, const TString & outfiletext)
{
  CRtoSRPlotter plotter(crtosrconfig,plotconfig,outfiletext);
  plotter.MakeCRtoSRPlot();
}
