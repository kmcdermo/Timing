#include "Common.cpp+"
#include "ABCD.cpp+"
#include "CombinePreparer.cpp+"

void runCombinePreparer(const TString & infilename, const TString & bininfoname,
			const TString & ratioinfoname, const TString & binratioinfoname,
			const TString & systfilename, const TString & wsname, 
			const TString & datacardname, const Bool_t blinddata,
			const Bool_t savemetadata, const TString & wsfilename)
{
  CombinePreparer Preparer(infilename,bininfoname,ratioinfoname,binratioinfoname,systfilename,
			   wsname,datacardname,blinddata,savemetadata,wsfilename);
  Preparer.PrepareCombine();
}
