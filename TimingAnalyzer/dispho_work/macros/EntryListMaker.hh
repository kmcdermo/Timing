#ifndef __EntryListMaker__
#define __EntryListMaker__

// ROOT inludes
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TString.h"
#include "TEntryList.h"
#include "TPaveText.h"

// STL includes
#include <iostream>
#include <fstream>
#include <map>
#include <vector>

// Common include
#include "Common.hh"

class EntryListMaker
{
public:
  EntryListMaker(const TString & cutflowconfig, const TString & filename, const TString & grouplabel);
  ~EntryListMaker();

  // Main call
  void MakeLists();

  // Subroutines for list making
  void MakeListsFromTrees();
  
  // Meta data and extra info
  void MakeConfigPave();

  // Helper Functions
  void InitListMap(std::map<TString,TEntryList*> & listmap, const TString & name);
  
private:
  // Settings
  const TString fCutFlowConfig;
  const TString fFileName;
  const TString fGroupLabel;

  // I/O
  TFile * fFile;

  // Output
  TPaveText * fConfigPave;
};

#endif
