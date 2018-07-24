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
  extern std::map<TString,SampleGroup> BkgdGroupMap;
  extern std::map<TString,TString> SignalGroupMap;
  extern std::map<TString,vector<TString> > SignalSubGroupMap;
  extern std::map<TString,TString> TreeNameMap;
  extern std::map<TString,TString> HistNameMap;
  extern std::map<TString,TString> BkgdHistNameMap;
  extern std::map<TString,TString> SampleCutFlowHistNameMap;
  extern std::map<TString,TString> GroupCutFlowHistNameMap;
  extern std::map<TString,TString> SignalCutFlowHistNameMap;
  extern std::map<TString,ColorStruct> SignalSubGroupColorMap;
  extern std::map<TString,Color_t> ColorMap;
  extern std::map<TString,TString> LabelMap; 
  extern std::map<TString,TString> CutWgtMap;
  extern std::map<TString,TString> VarWgtMap;
  extern std::vector<std::pair<TString,TString> > CutFlowPairVec;

  // special name for EWK histogram
  static const TString EWKSampleName = "EWK";
  static const TString EWKHistName   = Common::EWKSampleName+"_Hist";

  // Sample setup functions
  void SetupPrimaryDataset(const TString & pdname);
  void SetupSamples();
  void SetupSignalSamples();
  void SetupGroups();
  void SetupBkgdGroups();
  void SetupSignalGroups();
  void SetupSignalSubGroups();
  void SetupTreeNames();
  void SetupHistNames();
  void SetupBkgdHistNames();
  void SetupSampleCutFlowHistNames();
  void SetupGroupCutFlowHistNames();
  void SetupSignalCutFlowHistNames();
  void SetupSignalSubGroupColors();
  void SetupColors();
  void SetupLabels();
  void SetupCuts(const TString & cutconfig);
  void SetupCutFlow(const TString & cutflowconfig);
  void SetupVarWgts(const TString & varwgtconfig);
  void SetupWeights();
  void KeepOnlySamples(const std::vector<TString> & samplevec);
  void KeepOnlySignals();

  // Setup hists
  void SetupBins(std::string & str, std::vector<Double_t> & bins, Bool_t & var_bins);
  void SetupBinLabels(std::string & str, std::vector<TString> & binlabels);
  void SetupBlinding(const std::string & str, std::vector<BlindStruct> & blinding);
  void SetupScale(const std::string & str, Bool_t & scale);

  // Misc setup
  void SetupVarBinsBool(const TString & label, const TString & plotconfig, Bool_t & var_bins);
  void SetupBool(const std::string & str, Bool_t & setting);
  void SetupWhichSamples(const TString & sampleconfig, std::vector<TString> & samplevec);
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

  // output info
  static const std::vector<TString> extensions = {"png","pdf","eps"};

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
  TString PrintBool(const Bool_t b){return (b ? "true" : "false");}

  // Scale hists/graphs up or down
  void Scale(TH2F *& hist, const Bool_t isUp, const Bool_t varBinsX, const Bool_t varBinsY);
  void Scale(TH1F *& hist, const Bool_t isUp);
  void Scale(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins, const Bool_t isUp);

  // Check inputs
  void CheckValidFile(const TFile * file, const TString & fname);
  void CheckValidTree(const TTree * tree, const TString & tname, const TString & fname);
  template <typename T>
  void CheckValidHist(const T * plot, const TString & pname, const TString & fname)
  {
    if (plot == (T*) NULL) // check if valid plot
    {
      std::cerr << "Input Hist is bad pointer: " << pname.Data() << " in input file: " << fname.Data() 
		<< " ...exiting..." << std::endl;
      exit(1);
    }
  }

  // Check samples
  Bool_t IsCR(const TString & sample)
  {
    return (sample.Contains("GJets",TString::kExact) || sample.Contains("QCD",TString::kExact));
  }

  Bool_t IsEWK(const TString & sample)
  {
    return (sample.Contains("EWK",TString::kExact));
  }

  // may actually want an invalid tree/file...
  Bool_t IsNullFile(const TString & filename);
  Bool_t IsNullTree(const TTree * tree);

  // saving config info
  void AddTextFromInputConfig(TPaveText *& outpave, const TString & label, const TString & configname);
  void AddTextFromInputPave(TPaveText *& outpave, TFile *& infile);
  void AddPaddingToPave(TPaveText *& outpave, const Int_t lines);

  // extra textfile config info
  void AddPaddingToFile(std::ofstream & file, const Int_t lines);

  // function to save multiple canvas inmages
  void SaveAs(TCanvas *& canv, const TString & label);

  // ROOT Formatting
  void CMSLumi(TCanvas * canv, const Int_t iPosX = 10);
  void SetTDRStyle(TStyle * tdrStyle);
  
  ///////////////////////////////////////////////////////////
  // Template functions for setting up signal region stuff //
  ///////////////////////////////////////////////////////////

  template <typename T>
  void SetupCRHists(const TString & CR, TFile *& infile, std::map<TString,T*> & HistMap, std::map<TString,T*> & HistMapTmp)
  {
    std::cout << "Setting up CR hists for: " << CR.Data() << std::endl;
    
    // tmp variable
    const TString infilename = infile->GetName();
    
    // Get Data
    HistMap[CR] = (T*)infile->Get(Form("%s",Common::HistNameMap["Data"].Data()));
    Common::CheckValidHist(HistMap[CR],Common::HistNameMap["Data"],infilename);
    HistMap[CR]->SetName(Form("%s_CR_%s",CR.Data(),HistMap[CR]->GetName()));
    
    // Get Bkgd MC Histograms
    for (const auto & BkgdHistNamePair : Common::BkgdHistNameMap)
    {
      const auto & sample   = BkgdHistNamePair.first;
      const auto & histname = BkgdHistNamePair.second;
      
      auto & hist = HistMapTmp[Form("%s_CR_%s",CR.Data(),sample.Data())];
      
      hist = (T*)infile->Get(Form("%s",histname.Data()));
      Common::CheckValidHist(hist,histname,infilename);
      hist->SetName(Form("%s_CR_%s",CR.Data(),hist->GetName()));
    }
  }

  template <typename T>
  void SetupSRMCHists(TFile *& infile, std::map<TString,T*> & HistMapTmp)
  {
    std::cout << "Setting up SR MC Hists..." << std::endl;
    
    // tmp variable
    const TString & infilename = infile->GetName();

    // Get Bkgd MC Histograms
    for (const auto & BkgdHistNamePair : Common::BkgdHistNameMap)
    {
      const auto & sample   = BkgdHistNamePair.first;
      const auto & histname = BkgdHistNamePair.second;
      
      if (!Common::IsCR(sample)) continue;
      
      auto & hist = HistMapTmp[Form("SR_%s",sample.Data())];
      
      hist = (T*)infile->Get(Form("%s",histname.Data()));
      Common::CheckValidHist(hist,histname,infilename);
      hist->SetName(Form("SR_%s",hist->GetName()));
    }
  }

  template <typename T>
  Float_t GetKFactor(const TString & CR, const T & HistMap, const T & HistMapTmp, const Bool_t VarBins)
  {
    std::cout << "Computing k-factor for: " << CR.Data() << std::endl;
    
    const Float_t numer = HistMap.at(CR)->Integral(VarBins?"width":"");
    
    Float_t denom = 0.f;
    for (const auto & HistPair : HistMapTmp)
    {
      const auto & key  = HistPair.first;
      const auto & hist = HistPair.second;
      
      if (key.Contains("SR",TString::kExact)) continue; // skip SR plots
      if (!key.Contains(Form("%s_CR",CR.Data()),TString::kExact)) continue; // skip other CR
      
      // add up all bkgd MC in CR
      denom += hist->Integral(VarBins?"width":"");
    }
    
    return (numer / denom);
  }

  template <typename T>
  void ShapeCRHist(const TString & CR, T & HistMap, const T & HistMapTmp)
  {
    std::cout << Form("Reshaping %s data hist by subtracting MC in SR (excluding %s MC)...",CR.Data(),CR.Data()) << std::endl;

    for (const auto & HistPair : HistMapTmp)
    {
      const auto & key  = HistPair.first;
      const auto & hist = HistPair.second;
      
      if (key.Contains("SR",TString::kExact)) continue; // skip SR plots
      if (!key.Contains(Form("%s_CR",CR.Data()),TString::kExact)) continue; // skip other CR
      if (key.Contains(Form("%s_CR_%s",CR.Data(),CR.Data()),TString::kExact)) continue; // skip MC Hist in CR
      
      // subtract from data every MC that is NOT CR MC
      HistMap[CR]->Add(hist,-1.f);
    }
  }
  
  template <typename T>
  Float_t GetTransferFactor(const TString & CR, const T & HistMapTmp, const Bool_t VarBins, const Float_t kFactor)
  {
    std::cout << "Computing x-factor for: " << CR.Data() << std::endl;

    const Float_t cr_int = HistMapTmp.at(Form("%s_CR_%s",CR.Data(),CR.Data()))->Integral(VarBins?"width":"");
    const Float_t sr_int = HistMapTmp.at(Form("SR_%s",CR.Data()))             ->Integral(VarBins?"width":"");
    
    return (sr_int / (kFactor * cr_int));
  }

  template <typename T>
  void GetSRPredFromCR(const TString & CR, T & HistMap, const T & HistMapTmp, const Bool_t VarBins,
		       Float_t & kFactor, Float_t & xFactor)
  {  
    std::cout << "Constructing SR prediction from data for: " << CR.Data() << std::endl;
    
    // Create CR Data/MC k-factors
    kFactor = Common::GetKFactor(CR,HistMap,HistMapTmp,VarBins);
    
    // Fix CR Data shapes by subtracting away non CR MC
    Common::ShapeCRHist(CR,HistMap,HistMapTmp);
    
    // Create CRtoSR MC x-factors    
    xFactor = Common::GetTransferFactor(CR,HistMapTmp,VarBins,kFactor);
    
    // Scale Data CR by MC XFs --> yields SR prediction for each CR
    HistMap[CR]->Scale(xFactor);
  }

  template <typename T>
  void GetSRPredFromCRs(T & HistMap, const T & HistMapTmp, const Bool_t VarBins,
			std::map<TString,Float_t> & KFMap, std::map<TString,Float_t> & XFMap)
  {
    std::cout << "Constructing SR prediction from data in each CR..." << std::endl;

    for (const auto & BkgdGroupPair : Common::BkgdGroupMap)
    {
      const auto & sample = BkgdGroupPair.first;

      if (!Common::IsCR(sample)) continue;

      const auto & CR = sample;
      GetSRPredFromCR(CR,HistMap,HistMapTmp,VarBins,KFMap[CR],XFMap[CR]);
    }
  }
};

#endif
