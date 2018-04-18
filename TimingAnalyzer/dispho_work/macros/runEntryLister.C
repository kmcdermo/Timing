#include "TString.h"
#include "common/Common.cpp+"
#include "EntryLister.cpp+"

void runEntryLister(const TString & cutconfig, const TString & pdname, const TString outfiletext)
{
  EntryLister lister(cutconfig,pdname,outfiletext);
  lister.MakeList();
}
