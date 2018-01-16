#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter2D.cpp+"

void runTreePlotter2D(const TString & commoncut, const TString & text, const TString & inconfig)
{
  TreePlotter2D plotter(commoncut,text,inconfig);
  plotter.MakePlot();
}

