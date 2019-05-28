#ifndef __Common__
#define __Common__

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TH1D.h"
#include "TH2F.h"
#include "TGraphAsymmErrors.h"
#include "TF1.h"
#include "TEntryList.h"
#include "TMath.h"
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

// ECAL Enums
enum ECAL {EB, EM, EP, NONE};

// Sample Enums
enum SampleGroup {isData, isBkgd, isSignal, isToy};

// Variable Enums
enum Variable {X, Y};

// ECAL DetID
struct DetIDStruct
{
  DetIDStruct() {}
  DetIDStruct(const Int_t i1, const Int_t i2, const Int_t TT, const ECAL & ecal) 
    : i1(i1), i2(i2), TT(TT), ecal(ecal) {}
  
  Int_t i1; // EB: iphi, EE: ix
  Int_t i2; // EB: ieta, EE: iy
  Int_t TT; // trigger tower
  ECAL ecal; // EB, EM, EP
};

// Era Struct
struct EraStruct
{
  EraStruct() {}
  EraStruct(const UInt_t startRun, const UInt_t endRun, const Float_t lumi)
    : startRun(startRun), endRun(endRun), lumi(lumi) {}
  
  UInt_t  startRun;
  UInt_t  endRun;
  Float_t lumi;
};

// Block Struct
struct BlockStruct
{
  BlockStruct() {}
  BlockStruct(const Float_t xlow, const Float_t xup, 
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
  // numerical constants
  constexpr float    PI    = 3.14159265358979323846f;
  constexpr float TwoPI    = 6.28318530717958647692f;
  constexpr float SqrtPI   = 1.77245385090551602729f;
  constexpr float PIOver2  = Common::PI / 2.0f;
  constexpr float PIOver4  = Common::PI / 4.0f;
  constexpr float PI3Over4 = 3.0f * Common::PI / 4.0f;
  constexpr float InvPI    = 1.0f / Common::PI;
  constexpr float RadToDeg = 180.0f / Common::PI;
  constexpr float DegToRad = Common::PI / 180.0f;
  constexpr float sol      = 29.9792458; // speed of light in cm/ns
  constexpr float Sqrt2    = 1.414213562373095048807;
  constexpr float InvSqrt2 = 1.f / Common::Sqrt2;

  // ECAL info
  constexpr Float_t etaEBcutoff = 1.479;
  constexpr Float_t etaEEmax    = 2.5;
  constexpr Float_t radEB       = 129.f;
  constexpr Float_t zEE         = 314.f;
  extern std::map<UInt_t,DetIDStruct> DetIDMap;
  static const TString DetIDConfig   = "ecal_config/reducedinfo_detids.txt";
  static const TString DetIDConfigEB = "ecal_config/fullinfo_detids_EB.txt";
  static const TString DetIDConfigEE = "ecal_config/fullinfo_detids_EE.txt";
  ECAL GetECALEnum(const TString & ecal);
  void SetupDetIDs();
  void SetupDetIDsEB();
  void SetupDetIDsEE();
  Int_t WrapIPhi(const Int_t iphi);
  Bool_t IsCrossNeighbor(const UInt_t detid1, const UInt_t detid2);
  Bool_t IsWithinRadius(const UInt_t detid1, const UInt_t detid2, const Int_t radius);
  Int_t GetTriggerTower(const UInt_t detid);

  // Time constants: local resolution from same towers
  constexpr Float_t timefitN_Data = 27.1;   // ns
  constexpr Float_t timefitC_Data = 0.0904; // ns
  constexpr Float_t timefitN_MC   = 28.6;   // ns
  constexpr Float_t timefitC_MC   = 0.1120; // ns

  // Physics info
  constexpr Float_t helix = 0.0114; // 0.3 * 3.8 T / (100 cm / m), 0.3 = sol m/ns --> constant in radius of curv.
  constexpr Float_t Zmass = 91.1876;

  // generic computation functions
  inline Float_t phi(const Float_t x, const Float_t y){return TMath::ATan2(y,x);}
  inline Float_t uneta(const Float_t eta){return std::tan(2.f*std::atan(std::exp(-1.f*eta)));}
  inline Float_t hypot(const Float_t x, const Float_t y, const Float_t z = 0.f){return std::sqrt(std::pow(x,2.f)+std::pow(y,2.f)+std::pow(z,2.f));}

  // input
  constexpr    Float_t invfbToinvpb   = 1000.f; // conversion for the world's dumbest unit
  static const TString extraText      = "Preliminary";
  static const TString eosPreFix      = "root://eoscms.cern.ch";
  static const TString eosDir         = "/eos/cms/store/group/phys_exotica/displacedPhotons/nTuples/2017/analysis";
  static const TString baseDir        = "skims";
  static const TString calibDir       = "calib";
  static const TString tupleFileName  = "tree.root";
  static const TString rootdir        = "tree";
  static const TString configtreename = "configtree";
  static const TString disphotreename = "disphotree";
  static const TString h_cutflowname  = "h_cutflow";
  static const TString pavename       = "Config";

  static const TString h_cutflow_wgtname    = Common::h_cutflowname+"_wgt";
  static const TString h_cutflow_scaledname = Common::h_cutflowname+"_scaled";

  static const Int_t   nPUBins        = 150;
  static const TString dataPUFileName = "datapu";
  static const TString dataPUHistName = "pileup";
  static const TString puObsHistName  = "h_genpuobs";
  static const TString puTrueHistName = "h_genputrue";
  static const TString puwgtFileName  = "puweights";
  static const TString puwgtHistName  = "PUWeightsHist";

  // VID SF info
  static const TString gedVIDSFFileName = "SF_GEDTightAbsSmajSminVLSieie94X.root";
  static const TString ootVIDSFFileName = "SF_OOTTightAbsSmajSminVLSieie94X.root";
  static const TString vidSFHistName    = "EGamma_SF2D";

  // isTrk SF info
  static const TString isTrkBoolSFFileName = "SF_isTrk_281118.root";
  static const TString isTrkBoolSFHistName = "EB_Hist";

  // time and met template info
  static const TString templateFileName  = "met_vs_time_templates_exclusive_1pho.root";
  static const TString templateSuffix    = "Templates";
  static const TString templateXHistName = "Data_Hist_Plotted_Time_for_MET_0_to_100";
  static const TString templateYHistName = "Data_Hist_Plotted_MET_for_Time_m1_to_1";

  // Sample Information
  extern TString PrimaryDataset;
  extern std::map<TString,EraStruct> EraMap;
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
  extern std::map<TString,TString> XVarMap;
  extern std::map<TString,TString> YVarMap;
  extern std::map<TString,TString> CutWgtMap;
  extern std::vector<std::pair<TString,TString> > CutFlowPairVec;

  // special name for Bkgd Hist
  static const TString BkgdSampleName = "Bkgd";
  static const TString BkgdHistName   = Common::BkgdSampleName+"_Hist";
  
  // special name for EWK histogram
  static const TString EWKSampleName = "EWK";
  static const TString EWKHistName   = Common::EWKSampleName+"_Hist";

  // Sample setup functions
  void SetupPrimaryDataset(const TString & pdname);
  void SetupEras();
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
  void SetWhichVar(const TString & var, Variable & Var);
  void SetVar(const TString & var, const Variable Var);
  void SetVarMod(const TString & varmod, const Variable Var, const SampleGroup sample);
  void SetupCuts(const TString & cutconfig);
  void SetupCutFlow(const TString & cutflowconfig);
  void SetupEraCuts(const TString & era);
  void SetupWeights();
  void SetupEraWeights(const TString & era);
  void RemoveGroup(const SampleGroup isGroup);
  void KeepOnlySamples(const std::vector<TString> & samplevec);
  void KeepOnlySignals();

  // Setup hists
  void SetupBins(std::string & str, std::vector<Double_t> & bins, Bool_t & var_bins);
  void SetupBinLabels(std::string & str, std::vector<TString> & binlabels);
  void SetupScale(const std::string & str, Bool_t & scale);

  // Setup blinding regions and/or regions to count
  void SetupBlockRanges(const std::string & str, std::vector<BlockStruct> & block);

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
  static const TString outIntText = "_integrals";
  static const TString outFitText = "_fitinfo";
  static const std::vector<TString> extensions = {"png","pdf"};
  static const TString outTextExt = "log";

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

  // extra pad info
  constexpr Float_t merged_margin = 0.04;
  constexpr Float_t bottom_margin = 0.35;

  // color vector for special overplotting
  static const std::vector<Color_t> ColorVec = {kBlue,kRed+1,kGreen+1,kMagenta,kOrange+1,kAzure+10,kYellow+3,kPink-9,kYellow-7,kViolet-1,kTeal+6}; 

  // Hack to get combine to converge
  Float_t ApplyCombineSF(const TString & sample);
   
  // String formatting
  std::string RemoveDelim(std::string tmp, const std::string & delim){return tmp.erase(tmp.find(delim),delim.length());}
  TString ReplaceDelimWithSpace(TString tmp, const TString & delim){return tmp.ReplaceAll(delim," ");}
  TString ReplaceDotWithP(TString tmp){return tmp.ReplaceAll(".","p");}
  TString ReplaceSlashWithUnderscore(TString tmp){return tmp.ReplaceAll("/","_");}
  TString ReplaceSpaceWithUnderscore(TString tmp){return tmp.ReplaceAll(" ","_");}
  TString ReplaceMinusWithM(TString tmp){return tmp.ReplaceAll("-","m");}
  TString PrintBool(const Bool_t b){return (b ? "true" : "false");}
  Float_t Atof(const std::string & str){return std::atof(str.c_str());}

  // Scale hists/graphs up or down
  void Scale(TH2F *& hist, const Bool_t isUp, const Bool_t varBinsX, const Bool_t varBinsY);
  void Scale(TH1F *& hist, const Bool_t isUp);
  void Scale(TH1D *& hist, const Bool_t isUp);
  void Scale(TGraphAsymmErrors *& graph, const std::vector<Double_t> & bins, const Bool_t isUp);

  // Check inputs
  void CheckValidFile(const TFile * file, const TString & filename);
  Bool_t isGoodFile(const TFile * file);
  void CheckValidTree(const TTree * tree, const TString & treename, const TString & filename);
  template <typename T>
  void CheckValidHist(const T * hist, const TString & histname, const TString & filename)
  {
    if (hist == (T*) NULL) // check if valid hist
    {
      std::cerr << "Input Hist is bad pointer: " << histname.Data() << " in input file: " << filename.Data() << " ...exiting..." << std::endl;
      exit(1);
    }
  }
  void CheckValidF1(const TF1 * f1, const TString & f1name, const TString & filename);
  void CheckValidEntryList(const TEntryList * entrylist, const TString & entrylistname, const TString & filename);
  void CheckIfConfigEmpty(const TString & param, const TString & label);

  // Check samples
  Bool_t IsCR(const TString & sample)
  {
    return (sample.Contains("GJets",TString::kExact) || sample.Contains("QCD",TString::kExact));
  }
  Bool_t IsBkgd(const TString & sample)
  {
    return (sample.Contains(Common::BkgdSampleName,TString::kExact));
  }
  Bool_t IsEWK(const TString & sample)
  {
    return (sample.Contains(Common::EWKSampleName,TString::kExact));
  }

  // may actually want an invalid tree/file...
  Bool_t IsNullFile(const TString & filename);
  Bool_t IsNullTree(const TTree * tree);
  template <typename T>
  Bool_t IsNullHist(const T * hist)
  {
    return (hist == (T*) NULL);
  }
  Bool_t IsNullF1(const TF1 * f1);

  // check for negative bins and set to zero if so
  void CheckNegativeBins(std::map<TString,TH1F*> & HistMap);
  void CheckNegativeBins(std::map<TString,TH2F*> & HistMap);

  // saving config info
  void AddEraInfoToPave(TPaveText *& outpave,const TString & era);
  void AddTextFromInputConfig(TPaveText *& outpave, const TString & label, const TString & configname);
  void AddTextFromInputPave(TPaveText *& outpave, TFile *& infile);
  void AddPaddingToPave(TPaveText *& outpave, const Int_t lines);

  // extra textfile config info
  void AddPaddingToFile(std::ofstream & file, const Int_t lines);

  // function to save multiple canvas images
  void SaveAs(TCanvas *& canv, const TString & label);

  // ROOT Formatting
  void CMSLumi(TCanvas * canv, const Int_t iPosX = 10, const TString & era = "Full");
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

  ////////////////////////////
  // Other common functions //
  ////////////////////////////

  // common function for deleting map pairs that are dynamically allocated
  template <typename T>
  void DeleteMap(T & Map)
  {
    for (auto & Pair : Map) delete Pair.second;
    Map.clear();
  }

  template <typename T>
  void DeleteVec(T & Vec)
  {
    for (auto & Obj : Vec) delete Obj;
  }

  // write to output file
  template <typename T>
  void Write(TFile * file, T & Obj)
  {
    file->cd();
    Obj->Write(Obj->GetName(),TObject::kWriteDelete);
  }

  template <typename T>
  void WriteVec(TFile * file, T & Vec)
  {
    for (auto & Obj : Vec) Common::Write(file,Obj);
  }

  template <typename T>
  void WriteMap(TFile * file, T & Map)
  {
    for (auto & Pair : Map) Common::Write(file,Pair.second);
  }

  // write to output file with explicit label
  template <typename T>
  void Write(TFile * file, T & Obj, const TString & label)
  {
    file->cd();
    Obj->Write(label.Data(),TObject::kWriteDelete);
  }

  // reorder a vector by a vector of indices
  template <class T>
  void ReorderVector(std::vector<T>& vals, std::vector<UInt_t> idxs)
  {
    UInt_t i, j, k;
    T t;
    for (i = 0; i < vals.size(); i++)
    {
      if (i != idxs[i])
      {
	t = vals[i];
	k = i;
	while (i != (j = idxs[k]))
	{
	  // every move places a value in it's final location
	  vals[k] = vals[j];
	  idxs[k] = k;
	  k = j;
	}
	vals[k] = t;
	idxs[k] = k;
      }
    }
  }
};

#endif
