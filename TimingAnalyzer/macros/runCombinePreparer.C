#include "Common.cpp+"
#include "ABCD.cpp+"
#include "CombinePreparer.cpp+"

void runCombinePreparer(const TString & infilename, const TString & bininfoname,
			const TString & ratioinfoname, const TString & binratioinfoname,
			const TString & systfilename, const TString & datacardname,
			const Bool_t blinddata, const Bool_t includesystematics)
{
  CombinePreparer Preparer(infilename,bininfoname,ratioinfoname,binratioinfoname,
			   systfilename,datacardname,blinddata,includesystematics);
  Preparer.PrepareCombine();
}
