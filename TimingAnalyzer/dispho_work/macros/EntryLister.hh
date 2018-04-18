#ifndef __EntryLister__
#define __EntryLister__

// ROOT inludes
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TList.h"
#include "TEntryList.h"
#include "TPaveText.h"

// STL includes
#include <iostream>
#include <fstream>

// Common include
#include "common/Common.hh"

class EntryLister
{
public:
  EntryLister(const TString & cutconfig, const TString & pdname, const TString & outfiletext);
  ~EntryLister();

  // Initialize
  void SetupConfig();

  // Main call
  void MakeList();

  // Subroutines
  void MakeListFromTrees();
  void MakeConfigPave();
  
private:
  // Settings
  const TString fCutConfig;
  const TString fPDName;
  const TString fOutFileText;

  // Output
  TFile * fOutFile;
  TPaveText * fConfigPave;
};

#endif
