#ifndef __SRPlotter__
#define __SRPlotter__

#include "Common.hh"
#include "TreePlotter.hh"

class SRPlotter : TreePlotter 
{
public:
  SRPlotter(const TString & srplotconfig, const TString & miscconfig, const TString & outfiletext);
  ~SRPlotter() {}

  // setup functions
  void SetupConfig();
  void SetupSRPlotConfig();

  // main calls!
  void MakePlots();
  void MakeSRPlot();
  void MakeScaledCRPlot(const TString & CR, TFile *& infile);
  void DeleteInputFiles();

  // SR-plot main functions
  void SetupHists();

  // CR-only functions
  void SetupCROnlyHists(const TString & CR, TFile *& infile);
  void ScaleCRByKFOnly(const TString & CR);
  
  // Shared main calls
  void CommonPlotter(const TString & outfiletext);
  void MakeConfigPave();
  void DeleteMemory(const Bool_t deleteSRHists);

  // Meta data and extra info
  void DumpFactors(const TString & outfilename);
  void DumpIntegrals(const TString & CR, TFile *& infile, std::ofstream & dumpfile);

private:
  const TString fSRPlotConfig;
  const TString fMiscConfig;
  const TString fOutFileText;

  // input sr plot config
  TString fGJetsFileName;
  TString fQCDFileName;
  TString fSRFileName;
  TString fPlotConfig;

  // input files
  TFile * fGJetsFile;
  TFile * fQCDFile;
  TFile * fSRFile;

  // input extra hists
  std::map<TString,TH1F*> HistMap;

  // factor maps
  std::map<TString,Float_t> fCRKFMap;
  std::map<TString,Float_t> fCRXFMap;
};

#endif
