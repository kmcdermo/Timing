#ifndef __PhotonDump__
#define __PhotonDump__

// basic C++ headers
#include <iostream>
#include <fstream>
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

// HLT + Trigger info
#include "FWCore/Common/interface/TriggerNames.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

// DetIds 
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

// Ecal RecHits
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

// Supercluster info
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"

// EGamma Tools
#include "RecoEcal/EgammaCoreTools/interface/EcalTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TPRegexp.h"

// Common types
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

class PhotonDump : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit PhotonDump(const edm::ParameterSet&);
  ~PhotonDump();

  void InitializeTriggerBranches();

  void InitializeGenEvtBranches();

  void InitializeGenPUBranches();

  void DumpGenIds(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH);

  void InitializeGMSBBranches();
  
  void ClearHVDSBranches();
  void InitializeHVDSBranches();

  void InitializePVBranches();

  void InitializeMETBranches();

  void ClearGenJetBranches();
  void InitializeGenJetBranches();

  void ClearJetBranches();
  void InitializeJetBranches();

  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();
  void DumpVIDs(const pat::Photon &, const int, const float);
  void InitializeRecoRecHitBranches(const int iph);
  void DumpRecHitInfo(const int, const DetIdPairVec &, const EcalRecHitCollection *&);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  // dR matching criteria
  const float dRmin;
  const float pTres;

  // triggers
  const std::string inputPaths;
  std::vector<std::string> pathNames;
  const std::string inputFilters;
  std::vector<std::string> filterNames;
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  const edm::InputTag triggerObjectsTag;
  edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsToken;
  std::vector<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsByFilter; // first index is filter label, second is trigger objects

  // vertices
  const edm::InputTag verticesTag;
  edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;

  // rhos
  const edm::InputTag rhosTag;
  edm::EDGetTokenT<double> rhosToken;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;

  // photons + ids
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;
  const edm::InputTag photonLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonLooseIdMapToken;
  const edm::InputTag photonMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonMediumIdMapToken;
  const edm::InputTag photonTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonTightIdMapToken;

  // ootPhotons + ids
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  const edm::InputTag ootPhotonLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > ootPhotonLooseIdMapToken;
  const edm::InputTag ootPhotonMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > ootPhotonMediumIdMapToken;
  const edm::InputTag ootPhotonTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > ootPhotonTightIdMapToken;

  // ECAL RecHits
  const bool dumpRHs;
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

  // Gen Particles and MC info
  const bool isGMSB;
  const bool isHVDS;
  const bool isBkg;
  const bool dumpIds;
  bool isMC;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::EDGetTokenT<std::vector<reco::GenJet> >      genjetsToken;

  // output event level ntuple
  TTree* tree;

  // event info
  unsigned long int event;
  unsigned int run, lumi;  

  // trigger info
  std::vector<bool> triggerBits;

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // Generator level info
  float genwgt;
  int genpuobs, genputrue;

  // Gen particle info: GMSB
  int nNeutralino, nNeutoPhGr;

  float genN1mass, genN1E, genN1pt, genN1phi, genN1eta;
  float genN1prodvx, genN1prodvy, genN1prodvz;
  float genN1decayvx, genN1decayvy, genN1decayvz;
  float genph1E, genph1pt, genph1phi, genph1eta;
  int genph1match;
  float gengr1mass, gengr1E, gengr1pt, gengr1phi, gengr1eta;

  float genN2mass, genN2E, genN2pt, genN2phi, genN2eta;
  float genN2prodvx, genN2prodvy, genN2prodvz;
  float genN2decayvx, genN2decayvy, genN2decayvz;
  float genph2E, genph2pt, genph2phi, genph2eta;
  int genph2match;
  float gengr2mass, gengr2E, gengr2pt, gengr2phi, gengr2eta;

  // Gen particle info: HVDS
  int nvPions;
  
  std::vector<float> genvPionmass, genvPionE, genvPionpt, genvPionphi, genvPioneta;
  std::vector<float> genvPionprodvx, genvPionprodvy, genvPionprodvz;
  std::vector<float> genvPiondecayvx, genvPiondecayvy, genvPiondecayvz;
  
  std::vector<float> genHVph1E, genHVph1pt, genHVph1phi, genHVph1eta;
  std::vector<float> genHVph2E, genHVph2pt, genHVph2phi, genHVph2eta;
  std::vector<int> genHVph1match, genHVph2match;

  // gen jets
  int ngenjets;
  std::vector<int> genjetmatch;
  std::vector<float> genjetE, genjetpt, genjetphi, genjeteta;

  // MET
  float t1pfMETpt,       t1pfMETphi,       t1pfMETsumEt;
  float t1pfMETuncorpt,  t1pfMETuncorphi,  t1pfMETuncorsumEt;
  float t1pfMETcalopt,   t1pfMETcalophi,   t1pfMETcalosumEt;
  float t1pfMETgenMETpt, t1pfMETgenMETphi, t1pfMETgenMETsumEt;

  // jets
  int njets;
  std::vector<int> jetmatch;
  std::vector<float> jetE, jetpt, jetphi, jeteta;

  // photon info
  int nphotons;
  std::vector<int>   phisOOT;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<int>   phmatch;
  std::vector<bool>  phIsGenMatched;
  std::vector<float> phHoE, phr9, phChgIso, phNeuIso, phIso, phsuisseX;
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin, phalpha;
  std::vector<int>   phVID, phHoE_b, phsieie_b, phChgIso_b, phNeuIso_b, phIso_b;
  std::vector<std::vector<int> > phIsHLTMatched; // first index is iph, second is for filter, true/false

  // supercluster info 
  std::vector<float> phscE, phsceta, phscphi;

  // seed info
  std::vector<int> phseedpos;
  
  // all rec hit info
  std::vector<int> phnrh;
  std::vector<std::vector<float> > phrheta, phrhphi, phrhE, phrhtime;
  std::vector<std::vector<int> > phrhID;
  std::vector<std::vector<int> > phrhOOT;
};

#endif
