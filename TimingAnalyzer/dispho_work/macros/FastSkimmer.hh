#ifndef __FastSkimmer__
#define __FastSkimmer__

// ROOT inludes
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TList.h"
#include "TPaveText.h"

// STL includes
#include <iostream>
#include <fstream>
#include <map>

// Common include
#include "common/Common.hh"

class FastSkimmer
{
public:
  FastSkimmer(const TString & cutconfig, const TString & pdname, const TString & outfiletext);
  ~FastSkimmer();

  // Initialize
  void SetupConfig();
  void SetupLists();

  // Main call
  void MakeSkim();

  // Subroutines for plotting
  void MakeSkimFromTrees();
  void MergeSkims();
  
  // Meta data and extra info
  void MakeConfigPave();
  
private:
  // Settings
  const TString fCutConfig;
  const TString fPDName;
  const TString fOutFileText;

  // Output
  TFile * fOutFile;
  std::map<SampleType,TList*> ListMap;
  std::map<TString,TTree*> TreeMap;
  TPaveText * fConfigPave;
};

#endif
