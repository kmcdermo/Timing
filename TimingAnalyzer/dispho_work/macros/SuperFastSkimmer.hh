#ifndef __SuperFastSkimmer__
#define __SuperFastSkimmer__

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

class SuperFastSkimmer
{
public:
  SuperFastSkimmer(const TString & cutflowconfig, const TString & infilename,
		   const Boo_t issignalfile, const TString & outfiletext);
  ~SuperFastSkimmer();

  // Initialize
  void SetupCommon();

  // Main call
  void MakeSkims();

  // Subroutines for skimming
  void MakeSkimsFromTrees();
  
  // Meta data and extra info
  void MakeConfigPave();

  // Helper Functions
  void InitListMap(std::map<TString,TEntryList*> & listmap, const TString & sample);
  
private:
  // Settings
  const TString fCutFlowConfig;
  const TString fInFileName;
  const Bool_t  fIsSignalFile;
  const TString fOutFileText;

  // Output
  TFile * fOutFile;
  TPaveText * fConfigPave;
};

#endif
