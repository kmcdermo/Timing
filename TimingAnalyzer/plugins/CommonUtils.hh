#ifndef __TimingCommonUtils__
#define __TimingCommonUtils__

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
//                      //
// Standard Definitions //
//                      //
//////////////////////////
#include "DataFormats/Math/interface/deltaR.h"

namespace Config
{
  // Useful math constants
  constexpr float PI    = 3.14159265358979323846;
  constexpr float TWOPI = 2.0*PI;

  // other useful constants
  constexpr float sol = 29.9792458; // speed of light in cm/ns

  // ECAL size
  constexpr float etaEBcutoff = 1.479;
  constexpr float etaEBmax = 1.4442;
  constexpr float etaEEmin = 1.566;
  constexpr float etaEEmax = 2.5;

  // nObjects
  constexpr int nGMSBs = 2;
  constexpr int nHVDSs = 4;
  constexpr int nToys  = 2;
  constexpr int nPhotons = 4;
  constexpr int nJets = 10;
  
  // trigger name related strings
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

  // trigger filter related strings
  static const std::string L1Trigger = "hltL1sSingleEGNonIsoOrWithJetAndTauNoPS";
  static const std::string L1toHLTFilter = "hltEGL1SingleEGNonIsoOrWithJetAndTauNoPSFilter";  
  static const std::string ETFilter = "hltEG60EtFilter";
  static const std::string PhoIDLastFilter = "hltEG60R9Id90CaloIdLIsoLHollowTrackIsoFilter";
  static const std::string DispIDFilter = "hltEG60R9Id90CaloIdLIsoLDisplacedIdFilter";

  // MET Filter flag names
  static const std::string PVFlag = "Flag_goodVertices";
  static const std::string BeamHaloFlag = "Flag_globalSuperTightHalo2016Filter";
  static const std::string HBHENoiseFlag = "Flag_HBHENoiseFilter";
  static const std::string HBHEisoNoiseFlag = "Flag_HBHENoiseIsoFilter";
  static const std::string ECALTPFlag = "Flag_EcalDeadCellTriggerPrimitiveFilter";
  static const std::string PFMuonFlag = "Flag_BadPFMuonFilter";
  static const std::string PFChgHadFlag = "Flag_BadChargedCandidateFilter";
  static const std::string EESCFlag = "Flag_eeBadScFilter";
  static const std::string ECALCalibFlag = "Flag_ecalBadCalibFilter";

  // VID names
  static const std::string GEDPhotonLooseVID = "cutBasedPhotonID-Fall17-94X-V1-loose";
  static const std::string GEDPhotonMediumVID = "cutBasedPhotonID-Fall17-94X-V1-medium";
  static const std::string GEDPhotonTightVID = "cutBasedPhotonID-Fall17-94X-V1-tight";
  static const std::string OOTPhotonLooseVID = "cutBasedPhotonID-Fall17-94X-OOT-V1-loose";
  static const std::string OOTPhotonTightVID = "cutBasedPhotonID-Fall17-94X-OOT-V1-tight";
  static const std::string ElectronLooseVID = "cutBasedElectronID-Fall17-94X-V1-loose";
  static const std::string ElectronMediumVID = "cutBasedElectronID-Fall17-94X-V1-medium";
  static const std::string ElectronTightVID = "cutBasedElectronID-Fall17-94X-V1-tight";

  // inline math functions
  inline float rad2  (const float x, const float y, const float z = 0.f){return x*x + y*y + z*z;}
  inline float hypo  (const float x, const float y, const float z = 0.f){return std::sqrt(Config::rad2(x,y,z));}
  inline float phi   (const float x, const float y){return std::atan2(y,x);}
  inline float theta (const float r, const float z){return std::atan2(r,z);}
  inline float eta   (const float x, const float y, const float z)
  {
    return -1.0f*std::log(std::tan(Config::theta(Config::hypo(x,y),z)/2.f));
  }

  // check to see if file exists
  inline bool file_exists(const std::string & filename){std::fstream input(filename.c_str()); return (bool)input;}
};

//////////////////
//              //
// OOT TypeDefs //
//              //
//////////////////
typedef std::tuple<std::size_t, std::size_t, double> triple;
typedef std::vector<triple> triplevec;

///////////////////
//               //
// RecHit ID Map //
//               //
///////////////////
typedef std::unordered_map<uint32_t,int> uiiumap;

////////////////////////
//                    //
// Object Definitions //
//                    //
////////////////////////
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

///////////////
//           //
// VID Pairs //
//           //
///////////////
typedef std::vector<pat::Photon::IdPair> idpVec;

/////////////////////////////
//                         //
// Trigger Object Typedefs //
//                         //
/////////////////////////////
typedef std::map<std::string,bool> strBitMap;
typedef std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > trigObjVecMap;

/////////////////////////////////
//                             //
// Namespace for OOT Utilities //
//                             //
/////////////////////////////////

namespace oot
{
  // photon corrected pt
  inline float GetPhotonPt(const pat::Photon & photon){return (photon.pt()*(photon.userFloat("ecalEnergyPostCorr")/photon.energy()));}

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

  // object prep
  void ReadInTriggerNames(const std::string & inputPaths, std::vector<std::string> & pathNames, 
			  strBitMap & triggerBits);
  void ReadInFilterNames(const std::string & inputFilters, std::vector<std::string> & filterNames, 
			 trigObjVecMap & triggerObjectsByFilter);
  void PrepNeutralinos(const edm::Handle<std::vector<reco::GenParticle> >& genparticlesH, std::vector<reco::GenParticle> & neutralinos);
  void PrepVPions(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, std::vector<reco::GenParticle> & vPions);
  void PrepToys(const edm::Handle<std::vector<reco::GenParticle> >& genparticlesH, std::vector<reco::GenParticle> & toys);
  void PrepTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, 
		       const edm::Event & iEvent, strBitMap & triggerBitMap);
  void PrepTriggerObjects(const edm::Handle<edm::TriggerResults> & triggerResultsH,
			  const edm::Handle<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsH,
			  const edm::Event & iEvent, trigObjVecMap & triggerObjectsByFilterMap);
  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin = 0.f, 
		const float jetEtamax = 100.f, const int jetID = -1);
  void PrepRecHits(const EcalRecHitCollection * recHitsEB, 
		   const EcalRecHitCollection * recHitsEE,
		   uiiumap & recHitMap, const float rhEmin = 0.f);
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH, 
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   std::vector<pat::Photon> & photons, const float rho,
		   const float phpTmin = 0.f, const std::string & phIDmin = "none");
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH,
		   std::vector<pat::Photon> & photons, const bool isOOT,
		   const float rho, const float phpTmin = 0.f, const std::string & phIDmin = "none");
  void PrunePhotons(std::vector<pat::Photon> & photons,
		    const EcalRecHitCollection * recHitsEB,
		    const EcalRecHitCollection * recHitsEE,
		    const float seedTimemin = -10000.f);
  void PruneJets(std::vector<pat::Jet> & jets, const std::vector<pat::Photon> & photons, 
		 const int nPhosmax = 0, const float dRmin = 100.f);

  // object ID
  float GetChargedHadronEA(const float eta);
  float GetNeutralHadronEA(const float eta);
  float GetGammaEA(const float eta);
  float GetEcalPFClEA(const float eta);
  float GetHcalPFClEA(const float eta);
  float GetTrackEA(const float eta);
  float GetNeutralHadronPtScale(const float eta, const float pt);
  float GetGammaPtScale(const float eta, const float pt);
  float GetEcalPFClPtScale(const float eta, const float pt);
  float GetHcalPFClPtScale(const float eta, const float pt);
  float GetTrackPtScale(const float eta, const float pt);
  void GetGEDPhoVID(const pat::Photon & photon, idpVec& idpairs);
  void GetGEDPhoVIDByHand(const pat::Photon & photon, idpVec& idpairs);
  void GetOOTPhoVID(const pat::Photon & photon, idpVec& idpairs);
  void GetOOTPhoVIDByHand(const pat::Photon & photon, idpVec& idpairs, const float rho);
  int GetPFJetID(const pat::Jet & jet);

  // photon storage
  void SplitPhotons(std::vector<pat::Photon> & photons, const int nmax);
  void StoreOnlyPho(std::vector<pat::Photon> & photons, const int nmax, const bool isOOT);

  ///////////////////////////
  //                       //
  // Lepton Prep Functions //
  //                       //
  ///////////////////////////
  
  template <typename Lep>
  void PrepLeptons(const edm::Handle<std::vector<Lep> > & lepsH, std::vector<Lep> & leps,
		   const std::vector<pat::Photon> & photons, const float leppTmin = 0.f, 
		   const float lepdRmin = 100.f)
  {
    for (const auto & lep : *lepsH)
    {
      if (lep.pt() < leppTmin) continue;
      
      bool isMatched = false; // consider dR matching to photons only
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

  ////////////////////////
  //                    //
  // Matching Functions //
  //                    //
  ////////////////////////

  template <typename Obj>
  void HLTToObjectMatching(const trigObjVecMap & triggerObjectsByFilterMap, strBitMap & isHLTMatched, 
			   const Obj & obj, const float pTres = 1.f, const float dRmin = 100.f)
  {
    for (const auto & triggerObjectsByFilterPair : triggerObjectsByFilterMap)
    {
      const auto & filterName = triggerObjectsByFilterPair.first;
      const bool isL1T = (filterName == Config::L1Trigger.c_str());

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
    if (tracksH.isValid())
    {
      for (const auto & track : *tracksH)
      {
	if (track.pt() < trackpTmin) continue;
	if (reco::deltaR(obj,track) < trackdRmin)
	{
	  return true;
	} // end check over deltaR
      } // end loop over tracks
    } // end check over valid tracks
    return false;
  }

  template <typename Obj>
  bool GenToObjectMatching(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, const Obj & obj,
			   const float pTres = 1.f, const float dRmin = 100.f)
  {
    if (genparticlesH.isValid()) // make sure gen particles exist
    {
      for (const auto & genpart : *genparticlesH)
      {
	if (genpart.pdgId() != 22 || !genpart.isPromptFinalState()) continue;

	if (genpart.pt() < ((1.f-pTres) * obj.pt())) continue;
	if (genpart.pt() > ((1.f+pTres) * obj.pt())) continue;
	
	const float dR = reco::deltaR(obj,genpart);
	if (dR < dRmin) 
	{
	  return true;
	} // end check over dRmin
      } // end loop over gen particles
    } // end check over gen particles exist
    return false;      
  } 
};

///////////////////////
//                   //
// DetID Definitions //
//                   //
///////////////////////
#include "DataFormats/DetId/interface/DetId.h"

typedef std::vector<std::pair<DetId,float> > DetIdPairVec;

#endif
