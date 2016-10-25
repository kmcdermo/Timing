// basic C++ headers
#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <tuple>

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// HLT info
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// DataFormats
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

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

typedef std::vector<std::pair<DetId,float> > DetIdPairVec;

class OOTRecHits : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit OOTRecHits(const edm::ParameterSet&);
  ~OOTRecHits();
  
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
  
  // recHit cuts
  bool   applyrhEcut;
  double rhEcut;

  // Photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > photonsToken;

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitsReducedEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsReducedEETAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsFullEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsFullEETAG;

  // output ntuple
  // tree
  TTree* countingtree;

  // event info
  int event, run, lumi;  

  // rec hit info EB
  int nReducedEB, nR2FMatchedEB, nReducedOOTEB, nR2FMatchedOOTEB;
  int nFullEB   , nF2RMatchedEB, nFullOOTEB   , nF2RMatchedOOTEB;

  // rec hit info EE
  int nReducedEE, nR2FMatchedEE, nReducedOOTEE, nR2FMatchedOOTEE;
  int nFullEE   , nF2RMatchedEE, nFullOOTEE   , nF2RMatchedOOTEE;
};

