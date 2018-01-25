#include "TString.h"
#include "common/Common.cpp+"
#include "TreePlotter.cpp+"

void runTreePlotter(const TString & var, const TString & commoncut, const TString & bkgdcut, const TString & signcut, const TString & datacut,
		    const TString & text, const Int_t nbinsx, const Float_t xlow, const Float_t xhigh, const Bool_t islogx, const Bool_t islogy, 
		    const TString & title, const TString & xtitle, const TString & ytitle, const TString & delim)
{
  TreePlotter plotter(var,commoncut,bkgdcut,signcut,datacut,text,nbinsx,xlow,xhigh,islogx,islogy,title,xtitle,ytitle,delim);
  plotter.MakePlot();
}

