#ifndef __TimingCommonUtils__
#define __TimingCommonUtils__

////////////////////
// Common Headers //
////////////////////

// include files for data types
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/Math/interface/deltaR.h"

// basic C++ types
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <map>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <algorithm>
#include <memory>
#include <tuple>
#include <random>

//////////////////////////
// Standard Definitions //
//////////////////////////

namespace Config
{
  ///////////////////////////
  // Useful Math Constants //
  ///////////////////////////

  constexpr float PI    = 3.14159265358979323846;
  constexpr float TWOPI = 2.0*PI;

  ///////////////////////
  // Physics Constants //
  ///////////////////////

  constexpr float sol = 29.9792458; // speed of light in cm/ns

  ///////////////
  // ECAL Size //
  ///////////////

  constexpr float etaEBcutoff = 1.479;
  constexpr float etaEBmax = 1.4442;
  constexpr float etaEEmin = 1.566;
  constexpr float etaEEmax = 2.5;

  //////////////
  // nObjects //
  //////////////

  constexpr int nGMSBs = 2;
  constexpr int nHVDSs = 4;
  constexpr int nToys  = 2;
  constexpr int nPhotons = 4;
  constexpr int nJets = 10;
  
  //////////////////////////////////
  // Trigger Name Related Strings //
  //////////////////////////////////

  static const std::string SignalPath = "HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_PFHT350MinPFJet15_v";
  static const std::string RefPhoIDPath = "HLT_Photon60_R9Id90_CaloIdL_IsoL_v";
  static const std::string RefDispIDPath = "HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_v";
  static const std::string RefHTPath = "HLT_PFHT350MinPFJet15_v";
  static const std::string Pho50Path = "HLT_Photon50_v";
  static const std::string Pho200Path = "HLT_Photon200_v";
  static const std::string DiPho70Path = "HLT_DoublePhoton70_v";
  static const std::string DiPho3022M90Path = "HLT_Diphoton30_22_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90_v";
  static const std::string DiPho30PV18PVPath = "HLT_Diphoton30PV_18PV_R9Id_AND_IsoCaloId_AND_HE_R9Id_PixelVeto_Mass55_v";
  static const std::string Ele32WPTPath = "HLT_Ele32_WPTight_Gsf_L1DoubleEG_v";
  static const std::string DiEle33MWPath = "HLT_DoubleEle33_CaloIdL_MW_v";
  static const std::string Jet500Path = "HLT_PFJet500_v";

  ////////////////////////////////////
  // Trigger Filter Related Strings //
  ////////////////////////////////////

  static const std::string L1Trigger = "hltL1sSingleEGNonIsoOrWithJetAndTauNoPS";
  static const std::string L1toHLTFilter = "hltEGL1SingleEGNonIsoOrWithJetAndTauNoPSFilter";  
  static const std::string ETFilter = "hltEG60EtFilter";
  static const std::string PhoIDLastFilter = "hltEG60R9Id90CaloIdLIsoLHollowTrackIsoFilter";
  static const std::string DispIDFilter = "hltEG60R9Id90CaloIdLIsoLDisplacedIdFilter";

  ///////////////////////////
  // MET Filter Flag Names //
  ///////////////////////////

  static const std::string PVFlag = "Flag_goodVertices";
  static const std::string BeamHaloFlag = "Flag_globalSuperTightHalo2016Filter";
  static const std::string HBHENoiseFlag = "Flag_HBHENoiseFilter";
  static const std::string HBHEisoNoiseFlag = "Flag_HBHENoiseIsoFilter";
  static const std::string ECALTPFlag = "Flag_EcalDeadCellTriggerPrimitiveFilter";
  static const std::string PFMuonFlag = "Flag_BadPFMuonFilter";
  static const std::string PFChgHadFlag = "Flag_BadChargedCandidateFilter";
  static const std::string EESCFlag = "Flag_eeBadScFilter";
  static const std::string ECALCalibFlag = "Flag_ecalBadCalibFilter";

  ///////////////
  // VID Names //
  ///////////////

  static const std::string NoVID = "NONE";
  static const std::string GEDPhotonLooseVID = "cutBasedPhotonID-Fall17-94X-V1-loose";
  static const std::string GEDPhotonMediumVID = "cutBasedPhotonID-Fall17-94X-V1-medium";
  static const std::string GEDPhotonTightVID = "cutBasedPhotonID-Fall17-94X-V1-tight";
  static const std::string OOTPhotonLooseVID = "cutBasedPhotonID-Fall17-94X-OOT-V1-loose";
  static const std::string OOTPhotonTightVID = "cutBasedPhotonID-Fall17-94X-OOT-V1-tight";
  static const std::string ElectronLooseVID = "cutBasedElectronID-Fall17-94X-V1-loose";
  static const std::string ElectronMediumVID = "cutBasedElectronID-Fall17-94X-V1-medium";
  static const std::string ElectronTightVID = "cutBasedElectronID-Fall17-94X-V1-tight";

  ///////////////////////////////
  // Photon Internal VID Names //
  ///////////////////////////////

  static const std::string EmptyVID = "none";  
  static const std::string LooseGED = "loose-ged"; 
  static const std::string MediumGED = "medium-ged"; 
  static const std::string TightGED = "tight-ged"; 
  static const std::string LooseOOT = "loose-oot";
  static const std::string TightOOT = "tight-oot";

  //////////////////////
  // User Float Names //
  //////////////////////

  static const std::string JetID = "jetID";
  static const std::string IsOOT = "isOOT";
  static const std::string OOTMETPt = "ootMETpt";
  static const std::string OOTMETPhi = "ootMETphi";
  static const std::string OOTMETSumEt = "ootMETsumEt";
  static const std::string EcalEnergyPostCorr = "ecalEnergyPostCorr";
  static const std::string EnergyScaleDown = "energyScaleDown";
  static const std::string EnergyScaleUp = "energyScaleUp";
  static const std::string EnergySigmaDown = "energySigmaDown";
  static const std::string EnergySigmaUp = "energySigmaUp";

  ///////////////////////////
  // Inline Math Functions //
  ///////////////////////////

  inline float rad2  (const float x, const float y, const float z = 0.f){return x*x + y*y + z*z;}
  inline float hypo  (const float x, const float y, const float z = 0.f){return std::sqrt(Config::rad2(x,y,z));}
  inline float phi   (const float x, const float y){return std::atan2(y,x);}
  inline float theta (const float r, const float z){return std::atan2(r,z);}
  inline float eta   (const float r, const float z){return -1.0f*std::log(std::tan(Config::theta(r,z)/2.f));}
  inline float eta   (const float x, const float y, const float z){return Config::eta(Config::hypo(x,y),z);}

  ////////////////////
  // Misc Functions //
  ////////////////////

  inline bool file_exists(const std::string & filename){std::fstream input(filename.c_str()); return (bool)input;}
};

/////////////////////
// Common Typedefs //
/////////////////////

typedef std::unordered_map<uint32_t,int> uiiumap; // RecHit ID Map
typedef std::map<std::string,bool> strBitMap; // Trigger Bit Map

/////////////////////////////////
// Namespace for OOT Utilities //
/////////////////////////////////

namespace oot
{
  ////////////////////////////////
  // Special Struct For Photons //
  ////////////////////////////////

  struct ReducedPhoton
  {
    ReducedPhoton() {}
    ReducedPhoton(const size_t idx, const bool isOOT, const bool toRemove)
      : idx(idx), isOOT(isOOT), toRemove(toRemove) {}

    size_t idx;
    bool isOOT;
    bool toRemove;
  };

  //////////////////////////////
  // Special Photon Functions //
  //////////////////////////////

  // photon corrected et, pt
  inline float GetCorrFactor(const pat::Photon & photon){return photon.userFloat(Config::EcalEnergyPostCorr)/photon.energy();}
  inline float GetPhotonEt(const pat::Photon & photon){return photon.et()*oot::GetCorrFactor(photon);}
  inline float GetPhotonPt(const pat::Photon & photon){return photon.pt()*oot::GetCorrFactor(photon);}

  // sort by photon corrected pt
  inline void SortPhotonsByPt(std::vector<pat::Photon> & photons)
  {
    std::sort(photons.begin(),photons.end(),
	      [](const auto & photon1, const auto & photon2)
	      {
		return oot::GetPhotonPt(photon1) > oot::GetPhotonPt(photon2);
	      });
  }

  // sort by pt template
  const auto sortByPt = [](const auto & obj1, const auto & obj2) {return obj1.pt() > obj2.pt();};

  ///////////////////////////
  // Object Prep Functions //
  ///////////////////////////

  void ReadInTriggerNames(const std::string & inputPaths, std::vector<std::string> & pathNames, 
			  strBitMap & triggerBits);
  void ReadInFilterNames(const std::string & inputFilters, std::vector<std::string> & filterNames, 
			 std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilter);
  void PrepNeutralinos(const edm::Handle<std::vector<reco::GenParticle> >& genparticlesH, std::vector<reco::GenParticle> & neutralinos);
  void PrepVPions(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, std::vector<reco::GenParticle> & vPions);
  void PrepToys(const edm::Handle<std::vector<reco::GenParticle> >& genparticlesH, std::vector<reco::GenParticle> & toys);
  void PrepTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, 
		       const edm::Event & iEvent, strBitMap & triggerBitMap);
  void PrepTriggerObjects(const edm::Handle<edm::TriggerResults> & triggerResultsH,
			  const edm::Handle<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsH,
			  const edm::Event & iEvent, std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilterMap);
  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin = 0.f, 
		const float jetEtamax = 100.f, const int jetID = -1);
  void PrepRecHits(const EcalRecHitCollection * recHitsEB, 
		   const EcalRecHitCollection * recHitsEE,
		   uiiumap & recHitMap, const float rhEmin = 0.f);

  //////////////////////////////////
  // Photon (+MET) Prep Functions //
  //////////////////////////////////

  void PrepPhotonsCorrectMET(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH, 
			     const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
			     std::vector<pat::Photon> & photons, pat::MET & t1pfMET, const float rho,
			     const float dRmin = 0.f, const float phpTmin = 0.f, const std::string & phIDmin = Config::EmptyVID);
  void FindOverlapPhotons(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH,
			  const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
			  std::vector<oot::ReducedPhoton> & reducedPhotons, const float dRmin);
  void MergePhotons(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH,
		    const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH, 
		    const std::vector<oot::ReducedPhoton> & reducedPhotons,
		    std::vector<pat::Photon> & photons, const float rho,
		    const float phpTmin, const std::string & phIDmin);
  void CorrectMET(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH,
		  const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		  const std::vector<ReducedPhoton> & reducedPhotons,
		  pat::MET & t1pfMET);

  ///////////////////////
  // Pruning Functions //
  ///////////////////////

  void PrunePhotons(std::vector<pat::Photon> & photons,
		    const EcalRecHitCollection * recHitsEB,
		    const EcalRecHitCollection * recHitsEE,
		    const float seedTimemin = -10000.f);
  void PruneJets(std::vector<pat::Jet> & jets, const std::vector<pat::Photon> & photons, 
		 const int nPhosmax = 0, const float dRmin = 100.f);

  //////////////////////////////
  // Effective Area Functions //
  //////////////////////////////

  float GetChargedHadronEA(const float eta);
  float GetNeutralHadronEA(const float eta);
  float GetGammaEA(const float eta);
  float GetEcalPFClEA(const float eta);
  float GetHcalPFClEA(const float eta);
  float GetTrackEA(const float eta);

  //////////////////////////
  // pT Scaling Functions //
  //////////////////////////

  float GetNeutralHadronPtScale(const float eta, const float pt);
  float GetGammaPtScale(const float eta, const float pt);
  float GetEcalPFClPtScale(const float eta, const float pt);
  float GetHcalPFClPtScale(const float eta, const float pt);
  float GetTrackPtScale(const float eta, const float pt);

  //////////////////////////
  // Object VID Functions //
  //////////////////////////

  void GetGEDPhoVID(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs);
  void GetGEDPhoVIDByHand(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs);
  void GetOOTPhoVID(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs);
  void GetOOTPhoVIDByHand(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs, const float rho);
  int GetPFJetID(const pat::Jet & jet);

  //////////////////////////////
  // Photon Storage Functions //
  //////////////////////////////

  void SplitPhotons(std::vector<pat::Photon> & photons, const int nmax);
  void StoreOnlyPho(std::vector<pat::Photon> & photons, const int nmax, const bool isOOT);

  /////////////////////
  // Debug Functions //
  /////////////////////

  void DumpPhoton(const pat::Photon & photon, const bool isOOT,
		  const EcalRecHitCollection * recHitsEB, 
		  const EcalRecHitCollection * recHitsEE);

  ///////////////////////////
  // Lepton Prep Functions //
  ///////////////////////////

  template <typename Lep>
  void PrepLeptons(const edm::Handle<std::vector<Lep> > & lepsH, std::vector<Lep> & leps,
		   const std::vector<pat::Photon> & photons, const float leppTmin = 0.f, 
		   const float lepdRmin = 100.f)
  {
    for (const auto & lep : *lepsH)
    {
      if (lep.pt() < leppTmin) continue;
      
      auto isMatched = false; // consider dR matching to photons only
      for (const auto & photon : photons)
      {
	if (reco::deltaR(lep,photon) < lepdRmin)
	{
	  isMatched = true;
	  break;
	}
      }

      if (isMatched) continue;

      // emplace back
      leps.emplace_back(lep);
    }

    // sort by pt for good measure
    std::sort(leps.begin(),leps.end(),sortByPt);
  }

  /////////////////////////////////
  // Template Matching Functions //
  /////////////////////////////////

  template <typename Obj>
  void HLTToObjectMatching(const std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilterMap, 
			   strBitMap & isHLTMatched, const Obj & obj, const float pTres = 1.f, const float dRmin = 100.f)
  {
    for (const auto & triggerObjectsByFilterPair : triggerObjectsByFilterMap)
    {
      const auto & filterName = triggerObjectsByFilterPair.first;
      const auto isL1T = (filterName == Config::L1Trigger.c_str());

      for (const auto & triggerObject : triggerObjectsByFilterPair.second)
      {
	if (!isL1T)
	{
	  if (triggerObject.pt() < ((1.f-pTres) * obj.pt())) continue;
	  if (triggerObject.pt() > ((1.f+pTres) * obj.pt())) continue;
	}
	if (reco::deltaR(obj,triggerObject) < dRmin)
	{
	  isHLTMatched[filterName] = true; 
	  break;
	} // end check deltaR
      } // end loop over trigger objects 
    } // end loop over filter names
  }

  template <typename Obj>
  bool TrackToObjectMatching(const edm::Handle<std::vector<reco::Track> > & tracksH, const Obj & obj, 
			     const float trackpTmin = 0.f, const float trackdRmin = 100.f)
  {
    for (const auto & track : *tracksH)
    {
      if (track.pt() < trackpTmin) continue;
      if (reco::deltaR(obj,track) < trackdRmin) return true;
    } // end loop over tracks
    return false;
  }

  // obj1 and obj2 come from the same object (i.e. obj1 = photon, obj2 = SC of photon)
  // obj11 provides pT info, obj2 provides position info
  // in fact, they can be the same object if momentum and position info good for single object
  template <typename Obj1, typename Obj2>
  bool GenPhotonToObjectMatching(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, 
				 const Obj1 & obj1, const Obj2 & obj2,
				 const float pTres = 1.f, const float dRmin = 100.f)
  {
    for (const auto & genpart : *genparticlesH)
    {
      // ensure gen particle is a photon from hard scatter
      if (genpart.pdgId() != 22 || !genpart.isPromptFinalState()) continue;
      
      // ensure pts are reasonable
      if (genpart.pt() < ((1.f-pTres) * obj1.pt())) continue;
      if (genpart.pt() > ((1.f+pTres) * obj1.pt())) continue;
      
      // compute eta/phi direction along line connecting object to gen particle
      const auto dx = obj2.x()-genpart.vx();
      const auto dy = obj2.y()-genpart.vy();
      const auto dz = obj2.z()-genpart.vz();
      const auto phi = Config::phi(dx,dy);
      const auto eta = Config::eta(dx,dy,dz);

      // compare direction (momentum) of gen particle to direction of line connecting gen particle vertex to obj2
      if (reco::deltaR(eta,phi,genpart.eta(),genpart.phi()) < dRmin) return true;
    } // end loop over gen particles
    return false;      
  } 

  ////////////////////
  // Misc Functions //
  ////////////////////

  template <typename T>
  inline bool BadHandle(const T & objH, const std::string & desc)
  {
    if (objH.isValid()) return false;
    else
    {
      std::cerr << "Bad handle for " << desc.c_str() << "! Skipping this event..." << std::endl;
      return true;
    }
  }

  inline void InitializeBitMap(const std::vector<std::string> & keys, strBitMap & bitmap)
  {
    for (const auto & key : keys) bitmap[key] = false;
  }
};

#endif
