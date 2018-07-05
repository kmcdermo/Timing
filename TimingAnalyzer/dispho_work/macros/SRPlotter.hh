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
  void SetupHists();
  void SetupCRHists(TFile *& file, const TString & CR);

  // main calls!
  void MakeSRPlot();
  void ScaleCRtoSR();
  void MakeConfigPave();
  void DeleteMemory();

  // helper functions
  Float_t GetKFactor(const TString & CR);
  Float_t GetScaleFactor(const TString & CR, const Float_t kFactor);

  // Meta data and extra info
  void DumpFactors(const TString & filename);
  void DumpIntegrals(const TString & CR, TFile *& file, std::ofstream & dumpfile);

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
  std::map<TString,Float_t> fCRSFMap;
};

#endif
