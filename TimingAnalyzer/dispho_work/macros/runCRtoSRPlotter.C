#include "TString.h"
#include "Common.cpp+"
#include "CRtoSRPlotter.cpp+"

void runCRtoSRPlotter(const TString & crtosrconfig, const TString & outfiletext)
{
  CRtoSRPlotter plotter(crtosrconfig,outfiletext);
  plotter.MakeCRtoSRPlot();
}
