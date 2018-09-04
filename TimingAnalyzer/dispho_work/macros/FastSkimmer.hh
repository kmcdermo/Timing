#ifndef __FastSkimmer__
#define __FastSkimmer__

// ROOT inludes
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TEntryList.h"
#include "TList.h"
#include "TSystem.h"
#include "TPaveText.h"

// STL includes
#include <iostream>
#include <fstream>
#include <map>

// Common include
#include "Common.hh"

class FastSkimmer
{
public:
  FastSkimmer(const TString & cutflowconfig, const TString & pdname, const TString & outfiletext, 
	      const Bool_t doskim = true, const TString & sampleconfig = "");
  ~FastSkimmer();

  // Initialize
  void SetupCommon();
  void SetupLists();

  // Main call
  void MakeSkim();

  // Subroutines for skimming
  void MakeListFromTrees();
  void MakeMergedSkims();
  void MakeSkimsFromEntryLists(TFile *& TreeFile, std::map<TString,TTree*> & TreeMap, TList *& TreeList, 
			       TH1F *& OutHist, const TString & sample, const TString & treename);
  
  // Helper functions
  TH1F * SetupTotalCutFlowHist(const TString & sample);

  // Meta data and extra info
  void MakeConfigPave();
  
private:
  // Settings
  const TString fCutFlowConfig;
  const TString fPDName;
  const TString fOutFileText;
  const Bool_t  fDoSkim;
  const TString fSampleConfig;

  // tmp variables
  std::vector<TString> fSampleVec;

  // Output
  TFile * fOutFile;
  std::map<TString,std::map<TString,TEntryList*> > ListMapMap;
  TPaveText * fConfigPave;
};

#endif
