#ifndef __Common__
#define __Common__

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TROOT.h"
#include "TStyle.h"
#include "TString.h"
#include "TColor.h"
#include "TPaveText.h"
#include "TText.h"

// STL includes
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <utility>
#include <algorithm>
#include <sys/stat.h>

// Sample Enums
enum SampleGroup {isData, isBkgd, isSignal, isToy};

// Blind Struct
struct BlindStruct
{
  BlindStruct() {}
  BlindStruct(const Float_t xlow, const Float_t xup, 
	      const Float_t ylow = std::numeric_limits<float>::lowest(), 
	      const Float_t yup = std::numeric_limits<float>::max())
    : xlow(xlow), xup(xup), ylow(ylow), yup(yup) {} 

  Float_t xlow;
  Float_t xup;
  Float_t ylow;
  Float_t yup;
};

// Color Struct
struct ColorStruct
{
  ColorStruct() {}
  ColorStruct(const Color_t & color, const TString & increment) 
    : color(color), increment(increment) {}

  Color_t color;
  TString increment;
};

// Configuration parameters
namespace Common
{
  // basics
  constexpr Float_t lumi = 41.53; // CMS lumi : https://hypernews.cern.ch/HyperNews/CMS/get/luminosity/761/1.html
  constexpr Float_t invfbToinvpb = 1000.f; // conversion for the world's dumbest unit
  static const TString extraText = "Preliminary";

  // ECAL info
  constexpr Float_t etaEBcutoff = 1.479;
  constexpr Float_t etaEEmax    = 2.5;

  // input
  static const TString eosDir         = "/eos/cms/store/user/kmcdermo/nTuples";
  static const TString baseDir        = "skims/2017/rereco_v4";
  static const TString tupleFileName  = "tree.root";
  static const TString rootdir        = "tree";
  static const TString configtreename = "configtree";
  static const TString disphotreename = "disphotree";
  static const TString h_cutflowname  = "h_cutflow";
  static const TString pavename       = "Config";

  static const Int_t   nPUBins        = 150;
  static const TString dataPUFileName = "datapu";
  static const TString dataPUHistName = "pileup";
  static const TString puObsHistName  = "h_genpuobs";
  static const TString puTrueHistName = "h_genputrue";
  static const TString puwgtFileName  = "puweights";
  static const TString puwgtHistName  = "PUWeightsHist";

  // Sample Information
  extern TString PrimaryDataset;
  extern std::map<TString,TString> SampleMap;
  extern std::map<TString,SampleGroup> GroupMap;
  extern std::map<TString,TString> SignalGroupMap;
  extern std::map<TString,vector<TString> > SignalSubGroupMap;
  extern std::map<TString,TString> TreeNameMap;
  extern std::map<TString,TString> HistNameMap;
  extern std::map<TString,TString> SampleCutFlowHistNameMap;
  extern std::map<TString,TString> GroupCutFlowHistNameMap;
  extern std::map<TString,TString> SignalCutFlowHistNameMap;
  extern std::map<TString,ColorStruct> SignalSubGroupColorMap;
  extern std::map<TString,Color_t> ColorMap;
  extern std::map<TString,TString> LabelMap; 
  extern std::map<TString,TString> CutMap;
  extern std::vector<std::pair<TString,TString> > CutFlowPairVec;

  // Sample setup functions
  void SetupPrimaryDataset(const TString & pdname);
  void SetupSamples();
  void SetupGroups();
  void SetupSignalSamples();
  void SetupSignalGroups();
  void SetupSignalSubGroups();
  void SetupTreeNames();
  void SetupHistNames();
  void SetupSampleCutFlowHistNames();
  void SetupGroupCutFlowHistNames();
  void SetupSignalCutFlowHistNames();
  void SetupSignalSubGroupColors();
  void SetupColors();
  void SetupLabels();
  void SetupCuts(const TString & cutconfig);
  void SetupCutFlow(const TString & cutflowconfig);
  void KeepOnlySignals();

  // Setup hists
  void SetupBins(std::string & str, std::vector<Double_t> & bins, Bool_t & var_bins);
  void SetupBinLabels(std::string & str, std::vector<TString> & binlabels);
  void SetupBlinding(const std::string & str, std::vector<BlindStruct> & blinding);
  void SetupScale(const std::string & str, Bool_t & scale);

  // Misc setup
  void SetupBool(const std::string & str, Bool_t & setting);
  void SetupWhichSignals(const std::string & str, std::vector<TString> & signalvec);

  // cutflow histograms
  TH1F * SetupOutCutFlowHist(const TH1F * inhist, const TString & outname, std::map<TString,Int_t> & binlabels);
  
  // skim input
  constexpr UInt_t nEvCheck = 10000;
  constexpr Int_t nGMSBs = 2;
  constexpr Int_t nHVDSs = 4;
  constexpr Int_t nToys = 2;
  constexpr Int_t nJets = 4;
  constexpr Int_t nPhotons = 4;

  // plotting style
  constexpr Float_t TitleSize    = 0.035;
  constexpr Float_t TitleXOffset = 1.1;
  constexpr Float_t TitleYOffset = 1.1;
  constexpr Float_t LabelOffset  = 0.007;
  constexpr Float_t LabelSize    = 0.03;
  constexpr Float_t TickLength   = 0.03;
  constexpr Float_t TitleFF      = 1.4; 
  constexpr Int_t   Ndivisions   = 505;
  
  // up = upper pad, lp = lower pad
  constexpr Float_t left_up   = 0.0;
  constexpr Float_t bottom_up = 0.3;
  constexpr Float_t right_up  = 1.0;
  constexpr Float_t top_up    = 0.99;
  constexpr Float_t height_up = top_up - bottom_up;

  constexpr Float_t left_lp   = left_up;
  constexpr Float_t bottom_lp = 0.05;
  constexpr Float_t right_lp  = right_up;
  constexpr Float_t top_lp    = bottom_up;
  constexpr Float_t height_lp = top_lp - bottom_lp;

  // String formatting
  std::string RemoveDelim(std::string tmp, const std::string & delim){return tmp.erase(tmp.find(delim),delim.length());}
  TString ReplaceDelimWithSpace(TString tmp, const TString & delim){return tmp.ReplaceAll(delim," ");}
  TString ReplaceSlashWithUnderscore(TString tmp){return tmp.ReplaceAll("/","_");}
  TString ReplaceSpaceWithUnderscore(TString tmp){return tmp.ReplaceAll(" ","_");}

  // Weight for sample
  TString WeightString(const TString & sample)
  {
    // Get the appropriate weight 
    return (Common::GroupMap[sample] != isData ? "evtwgt * puwgt" : "1.0");
  }
  
  // Scale hists/graphs up or down
  void Scale(TH2F *& hist, const Bool_t isUp, const Bool_t varBinsX, const Bool_t varBinsY);
  void Scale(TH1F *& hist, const Bool_t isUp);
  void Scale(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins, const Bool_t isUp);

  // Check inputs
  void CheckValidFile(const TFile * file, const TString & fname);
  void CheckValidTree(const TTree * tree, const TString & tname, const TString & fname);
  void CheckValidTH1F(const TH1F * plot, const TString & pname, const TString & fname);
  void CheckValidTH1D(const TH1D * plot, const TString & pname, const TString & fname);
  void CheckValidTH2F(const TH2F * plot, const TString & pname, const TString & fname);

  // may actually want an invalid tree/file...
  Bool_t IsNullFile(const TString & filename);
  Bool_t IsNullTree(const TTree * tree);

  // saving config info
  void AddTextFromInputConfig(TPaveText *& outpave, const TString & label, const TString & configname);
  void AddTextFromInputPave(TPaveText *& outpave, TFile *& infile);

  // ROOT Formatting
  void CMSLumi(TCanvas * canv, const Int_t iPosX = 10);
  void SetTDRStyle(TStyle * tdrStyle);
};

#endif
