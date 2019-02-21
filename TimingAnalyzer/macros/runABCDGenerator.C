#include "Common.cpp+"
#include "ABCD.cpp+"
#include "ABCDGenerator.cpp+"

void runABCDGenerator(const TString & infilename, const TString & bininfoname,
		      const TString & ratioinfoname, const TString & binratioinfoname)
{
  ABCDGenerator Generator(infilename,bininfoname,ratioinfoname,binratioinfoname);
  Generator.ProduceABCDConfig();
}
