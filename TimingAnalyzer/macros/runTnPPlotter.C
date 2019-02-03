#include "TString.h"
#include "Common.cpp+"
#include "TnPPlotter.cpp+"

void runTnPPlotter(const TString & infilename, const Bool_t savemetadata, const TString & outfiletext)
{
  TnPPlotter plotter(infilename,savemetadata,outfiletext);
  plotter.MakeTnPPlots();
}
