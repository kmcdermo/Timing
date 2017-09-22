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
#include <algorithm>
#include <memory>
#include <tuple>

//////////////////////////
//                      //
// Standard Definitions //
//                      //
//////////////////////////

namespace Config
{
  constexpr float PI    = 3.14159265358979323846;
  constexpr float TWOPI = 2.0*PI;

  inline float rad2  (const float x, const float y){return x*x + y*y;}
  inline float phi   (const float x, const float y){return std::atan2(y,x);}
  inline float theta (const float r, const float z){return std::atan2(r,z);}
  inline float eta   (const float x, const float y, const float z)
  {
    return -1.0f*std::log(std::tan(Config::theta(std::sqrt(Config::rad2(x,y)),z)/2.f));
  }
  inline float deltaR(const float phi1, const float eta1, const float phi2, const float eta2)
  {
    if   (std::abs(phi2-phi1)<Config::PI) 
    {
      return std::sqrt(Config::rad2(eta2-eta1,phi2-phi1));
    }
    else 
    {
      if (phi2-phi1>0.f) 
      {
	return std::sqrt(Config::rad2(eta2-eta1,  Config::TWOPI-(phi2-phi1) ));
      }
      else // technically, the last sign flip is unnecessary here
      {
	return std::sqrt(Config::rad2(eta2-eta1,-(Config::TWOPI+(phi2-phi1))));
      }
    }
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
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

namespace oot
{
  // special ootPhoton class
  class Photon
  {
  public: 
    Photon(const pat::Photon & photon, const bool isOOT) : photon_(std::move(photon)), isOOT_(isOOT) {}
    ~Photon() {}

    const pat::Photon& photon() const {return photon_;} 
    pat::Photon& photon_nc() {return photon_;} 
    bool isOOT() const {return isOOT_;}

    float pt() const {return photon_.pt();}
    float phi() const {return photon_.phi();}
    float eta() const {return photon_.eta();}

  private:
    pat::Photon photon_;
    bool isOOT_;
  };

  // sort by pt template
  const auto sortByPt = [](const auto& obj1, const auto& obj2) {return obj1.pt() > obj2.pt();};

  // sort by closest to z-mass
  inline bool minimizeByZmass(const triple& pair1, const triple& pair2)
  {
    return std::get<2>(pair1)<std::get<2>(pair2);
  }

  void ReadInTriggerNames(const std::string & inputPaths, std::vector<std::string> & pathNames, 
			  std::vector<bool> & triggerBits);
  void ReadInFilterNames(const std::string & inputFilters, std::vector<std::string> & filterNames, 
			 std::vector<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilter);
  void PrepTriggerObjects(const edm::Handle<edm::TriggerResults> & triggerResultsH,
			  const edm::Handle<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsH,
			  const edm::Event& iEvent, const std::vector<std::string> & filterNames, 
			  std::vector<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilter);
  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin = 0.f, const int jetID = -1);
  void PrepRecHits(const EcalRecHitCollection * recHitsEB, 
		   const EcalRecHitCollection * recHitsEE,
		   uiiumap & recHitMap);
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonLooseIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonMediumIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonTightIdMapH,
		   std::vector<oot::Photon> & photons, const bool isOOT, const float phpTmin = 0.f);
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonLooseIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonMediumIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonTightIdMapH,
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   const edm::Handle<edm::ValueMap<bool> > & ootPhotonLooseIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & ootPhotonMediumIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & ootPhotonTightIdMapH,
		   std::vector<oot::Photon> & photons, const float phpTmin = 0.f);
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   std::vector<oot::Photon> & photons, const float phpTmin = 0.f);
  void PrepTrigger(std::vector<bool> & triggerBits);
  void PrepElectrons(const edm::Handle<std::vector<pat::Electron> > & electronsH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronVetoIdMapH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronLooseIdMapH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronMediumIdMapH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronTightIdMapH, 
		     std::vector<pat::Electron> & electrons);
  float GetChargedHadronEA(const float eta);
  float GetNeutralHadronEA(const float eta);
  float GetGammaEA(const float eta);
  int PassHoE   (const float eta, const float HoE);
  int PassSieie (const float eta, const float Sieie);
  int PassChgIso(const float eta, const float ChgIso);
  int PassNeuIso(const float eta, const float NeuIso, const float pt);
  int PassPhIso (const float eta, const float PhIso,  const float pt);
  int GetPFJetID(const pat::Jet & jet);

  ////////////////////////
  //                    //
  // Matching Functions //
  //                    //
  ////////////////////////

  // templates MUST be in header if included elsewhere
  template <typename VB, typename Obj>
  void HLTToObjectMatching(const std::vector<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilter, 
			   VB& isHLTMatched, const Obj& obj, const int iobj, const float pTres, const float dRmin)
  {
    for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
    {
      for (std::size_t iobject = 0; iobject < triggerObjectsByFilter[ifilter].size(); iobject++)
      {
	const pat::TriggerObjectStandAlone & triggerObject = triggerObjectsByFilter[ifilter][iobject];
	if (std::abs(triggerObject.pt()-obj.pt())/obj.pt() < pTres)
        {
	  if (Config::deltaR(obj.phi(),obj.eta(),triggerObject.phi(),triggerObject.eta()) < dRmin)
	  {
	    isHLTMatched[iobj][ifilter] = true;
	  }
	}
      }
    }
  }

  template <typename Obj>
  bool GenToObjectMatching(const Obj& obj, const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH,
			   const float pTres, const float dRmin)
  {
    if (genparticlesH.isValid()) // make sure gen particles exist
    {
      for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter)
      {
	if (gpiter->pdgId() == 22 && gpiter->isPromptFinalState())
	{
	  if (std::abs(gpiter->pt()-obj.pt())/obj.pt() < pTres)
	  {
	    const float dR = Config::deltaR(obj.phi(),obj.eta(),gpiter->phi(),gpiter->eta());
	    if (dR < dRmin) 
	    {
	      return true;
	    } // end check over dRmin
	  } // end check over pT resolution
	} // end check over gen particle status 
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
