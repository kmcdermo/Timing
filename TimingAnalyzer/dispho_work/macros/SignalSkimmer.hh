#ifndef __SignalSkimmer__
#define __SignalSkimmer__

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

class SignalSkimmer
{
public:
  SignalSkimmer(const TString & cutflowconfig, const TString & inskimdir, const TString & outfiletext);
  ~SignalSkimmer();

  // Initialize
  void SetupCommon();

  // Main call
  void MakeSkims();

  // Subroutines for skimming
  void MakeSkimsFromTrees();
  
  // Meta data and extra info
  void MakeConfigPave();

  // Helper Functions
  void InitListMap(std::map<TString,TEntryList*> & listmap, const TString & signal);
  
private:
  // Settings
  const TString fCutFlowConfig;
  const TString fInSkimDir;
  const TString fOutFileText;

  // Output
  TFile * fOutFile;
  TPaveText * fConfigPave;
};

#endif
