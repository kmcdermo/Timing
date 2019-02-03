#include "TString.h"
#include "Common.cpp+"
#include "CRtoSRPlotter.cpp+"

void runCRtoSRPlotter(const TString & crtosrconfig, const TString & era, 
		      const Bool_t savemetadata, const TString & outfiletext)
{
  CRtoSRPlotter plotter(crtosrconfig,era,savemetadata,outfiletext);
  plotter.MakeCRtoSRPlot();
}
