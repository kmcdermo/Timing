#include "TString.h"
#include "Common.cpp+"
#include "CRtoSRPlotter.cpp+"

void runCRtoSRPlotter(const TString & crtosrconfig, const TString & era, const TString & outfiletext)
{
  CRtoSRPlotter plotter(crtosrconfig,era,outfiletext);
  plotter.MakeCRtoSRPlot();
}
