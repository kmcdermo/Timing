#include "TString.h"
#include "Common.cpp+"
#include "EntryListMaker.cpp+"

void runEntryListMaker(const TString & cutconfig, const TString & filename, const TString & grouplabel)
{
  EntryListMaker lister(cutconfig,filename,grouplabel);
  lister.MakeLists();
}
