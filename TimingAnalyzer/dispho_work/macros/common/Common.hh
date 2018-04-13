#ifndef __Common__
#define __Common__

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"

// STL includes
#include <map>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

// Sample Enums
enum SampleType  {Data, GMSB, HVDS, QCD, GJets, DYLL, DiPho, ToyMC};
enum SampleGroup {isData, isBkgd, isSignal, isToy};

// Configuration parameters
namespace Config
{
  // basics
  constexpr Float_t lumi = 41.37; // CMS lumi : golden for ReReco
  constexpr Float_t invfbToinvpb = 1000.f; // conversion for the world's dumbest unit
  static const TString extraText = "Preliminary";

  // ECAL info
  constexpr Float_t etaEBcutoff = 1.479;
  constexpr Float_t etaEEmax    = 2.5;

  // input
  static const TString eosDir         = "/eos/cms/store/user/kmcdermo";
  static const TString baseDir        = "nTuples/skims/2017/rereco_v3";
  static const TString tupleFileName  = "tree.root";
  static const TString puwgtFileName  = "puweights";
  static const TString puwgtHistName  = "PUWeightsHist";
  static const TString rootdir        = "tree/";
  static const TString configtreename = "configtree";
  static const TString disphotreename = "disphotree";
  static const TString h_cutflowname  = "h_cutflow";

  // Sample Information
  extern TString                          PrimaryDataset;
  extern std::map<TString,SampleType>     SampleMap;
  extern std::map<SampleType,SampleGroup> GroupMap;
  extern std::map<SampleType,TString>     HistNameMap;
  extern std::map<SampleType,Color_t>     ColorMap;
  extern std::map<SampleType,TString>     LabelMap;
  extern std::map<SampleType,TString>     CutMap;

  // Sample setup functions
  void SetupPrimaryDataset(const TString & pdname);
  void SetupSamples();
  void SetupGroups();
  void SetupHistNames();
  void SetupColors();
  void SetupLabels();
  void SetupCuts(const TString & cutconfig);

  // Setup hists
  void SetupBins(std::string & str, std::vector<Double_t> & bins, Bool_t & var_bins);
  void SetupScale(const std::string & str, Bool_t & scale);

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

  // Weight for sample
  TString WeightString(const TString & input, const SampleType sample)
  {
    // Get the appropriate weight 
    TString weight = (Config::GroupMap[sample] != isData ? "evtwgt * puwgt" : "1.0");
    if (sample == DYLL)
    {
      if      (input.Contains("base")) weight += " * 0.4982";
      else if (input.Contains("ext"))  weight += " * 0.5018";
      else    
      {
	std::cerr << "Somehow you specified a DYLL sample that does not exist: " << input.Data() << " ...exiting..." << std::endl;
	exit(1);
      }
    }

    return weight;
  }

  // Check inputs
  void CheckValidFile(const TFile * file, const TString & fname);
  void CheckValidTree(const TTree * tree, const TString & tname, const TString & fname);
  void CheckValidTH1F(const TH1F * plot, const TString & pname, const TString & fname);
  void CheckValidTH2F(const TH2F * plot, const TString & pname, const TString & fname);

  // ROOT Formatting
  void CMSLumi(TCanvas * canv, const Int_t iPosX = 10);
  void SetTDRStyle(TStyle * tdrStyle);
};

#endif
