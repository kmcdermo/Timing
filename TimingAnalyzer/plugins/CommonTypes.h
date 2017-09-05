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

////////////////////////
//                    //
// Object Definitions //
//                    //
////////////////////////
#include "FWCore/Framework/interface/ESHandle.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

namespace oot
{
  struct Photon
  {
    pat::Photon photon_;
    bool isOOT_;

    Photon(const pat::Photon & photon, const bool isOOT) : photon_(std::move(photon)), isOOT_(isOOT) {}
    float pt() const {return photon_.pt();}
  };

  // sort by pt template
  const auto sortByPt = [](const auto& obj1, const auto& obj2) {return obj1.pt() > obj2.pt();};

  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin = 0.f);

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   std::vector<oot::Photon> & photons, const float phpTmin = 0.f);
  float GetChargedHadronEA(const float eta);
  float GetNeutralHadronEA(const float eta);
  float GetGammaEA(const float eta);
};

///////////////////////
//                   //
// DetID Definitions //
//                   //
///////////////////////
#include "DataFormats/DetId/interface/DetId.h"

typedef std::vector<std::pair<DetId,float> > DetIdPairVec;
typedef std::unordered_map<uint32_t,int> uiiumap;
