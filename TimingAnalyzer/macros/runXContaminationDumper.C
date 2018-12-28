#include "TString.h"
#include "Common.cpp+"
#include "XContaminationDumper.cpp+"

void runXContaminationDumper(const TString & infilename, const TString & xcontdumpconfig, 
			     const TString & plotconfig, const TString & era, const TString & outfiletext)
{
  XContaminationDumper dumper(infilename,xcontdumpconfig,plotconfig,era,outfiletext);
  dumper.MakeContaminationDump();
}
