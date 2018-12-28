#include "TString.h"
#include "Common.cpp+"
#include "SigEffPlotter.cpp+"

void runSigEffPlotter(const TString & infilename, const TString & outtext)
{
  SigEffPlotter plotter(infilename,outtext);
  plotter.MakeSigEffPlot();
}
