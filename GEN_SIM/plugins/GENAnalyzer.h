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
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// DataFormats
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/METReco/interface/GenMET.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TPRegexp.h"

inline bool sortByJetPt(const reco::GenJet & jet1, const reco::GenJet & jet2)
{
  return jet1.pt()>jet2.pt();
}

inline bool sortByMETPt(const reco::GenMET & met1, const reco::GenMET & met2)
{
  return met1.pt()>met2.pt();
}

class GENAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit GENAnalyzer(const edm::ParameterSet&);
  ~GENAnalyzer();

  void InitializeGenEvtBranches();

  void ClearGenJetsBranches();
  void ClearGenMETsBranches();


  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::EDGetTokenT<std::vector<reco::GenJet> >      genjetsToken;
  edm::EDGetTokenT<std::vector<reco::GenMET> >      genmetsToken;

  // output event level ntuple
  TTree* tree;

  // event info
  int event, run, lumi;  

  // Generator level info
  float genwgt;

  // gen jets
  int ngenjets;
  std::vector<float> genjetE, genjetpt, genjetphi, genjeteta;

  int ngenmets;
  std::vector<float> genMETpt, genMETphi, genMETsumEt;
};
