// basic C++ headers
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <map>
#include <unordered_map>
#include <cmath>
#include <algorithm>

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// DetIds and Ecal stuff
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

// EGamma Tools
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

// Common types
#include "CommonTypes.h"

class OOTRecHits : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit OOTRecHits(const edm::ParameterSet&);
  ~OOTRecHits();
  
  void PhotonRecHits(edm::Handle<std::vector<reco::Photon> > &, 
		     EcalClusterLazyTools *&, EcalClusterLazyTools *&,
		     const CaloSubdetectorGeometry *&, const CaloSubdetectorGeometry *&);
  void InitializePhotonBranches();
  void InitializeFullRecHitBranches();
  void InitializeReducedRecHitBranches();

  void ReducedToFullEB(EcalClusterLazyTools *&, EcalClusterLazyTools *&);
  void ReducedToFullEE(EcalClusterLazyTools *&, EcalClusterLazyTools *&);
  void FullToReducedEB(EcalClusterLazyTools *&, EcalClusterLazyTools *&);
  void FullToReducedEE(EcalClusterLazyTools *&, EcalClusterLazyTools *&);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  
  // photon rec hit analysis
  const bool doPhRhs;
  // delta R cuts
  const bool applydelRcut;
  const double delRcut;

  // counting analysis
  const bool doCount;
  // recHit cuts
  const bool applyrhEcut;
  const double rhEcut;

  // Photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > photonsToken;

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitsReducedEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsReducedEETAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsFullEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsFullEETAG;

  // photon ntuple
  // phrhtree;
  TTree * phrhtree;
  float phE, phpt, phphi, pheta;
  float phscE, phscphi, phsceta;

  // full hits
  int phnfrhs;
  std::vector<float> phfrhEs, phfrhphis, phfrhetas, phfrhdelRs, phfrhtimes;
  std::vector<int>   phfrhIDs, phfrhOOTs;
  int phfseedpos;

  // reduced hits
  int phnrrhs;
  std::vector<float> phrrhEs, phrrhphis, phrrhetas, phrrhdelRs, phrrhtimes;
  std::vector<int>   phrrhIDs, phrrhOOTs;
  int phrseedpos;

  // output ntuple
  // countingtree
  TTree * countingtree;

  // event info
  int event, run, lumi;  
  int nphotons;
  
  // rec hit info EB
  int nReducedEB, nR2FMatchedEB, nReducedOOTEB, nR2FMatchedOOTEB;
  int nFullEB   , nF2RMatchedEB, nFullOOTEB   , nF2RMatchedOOTEB;

  // rec hit info EE
  int nReducedEE, nR2FMatchedEE, nReducedOOTEE, nR2FMatchedOOTEE;
  int nFullEE   , nF2RMatchedEE, nFullOOTEE   , nF2RMatchedOOTEE;
};

