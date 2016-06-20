#include <memory>
#include <vector>
#include <iostream>

// Framework
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/ValueMap.h"

// Electrons
#include "DataFormats/PatCandidates/interface/Electron.h"

class PFCleaner : public edm::stream::EDProducer<> {
public:
  explicit PFCleaner(const edm::ParameterSet&);
  ~PFCleaner();
        
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
    
private:
  virtual void beginJob();
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endJob();
        
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  const edm::EDGetTokenT<std::vector<pat::Electron> > electronsToken;

  const edm::EDGetTokenT<edm::ValueMap<bool> > electronVetoIdMapToken;
  const edm::EDGetTokenT<edm::ValueMap<bool> > electronLooseIdMapToken;
  const edm::EDGetTokenT<edm::ValueMap<bool> > electronMediumIdMapToken;
  const edm::EDGetTokenT<edm::ValueMap<bool> > electronTightIdMapToken;
  const edm::EDGetTokenT<edm::ValueMap<bool> > electronHeepIdMapToken;
};

PFCleaner::PFCleaner(const edm::ParameterSet& iConfig): 
  electronsToken           (consumes<std::vector<pat::Electron> > (iConfig.getParameter<edm::InputTag>("electrons"))),
  electronVetoIdMapToken   (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidveto"))),
  electronLooseIdMapToken  (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidloose"))),
  electronMediumIdMapToken (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidmedium"))),
  electronTightIdMapToken  (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidtight"))),
  electronHeepIdMapToken   (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidheep")))
{
  produces<pat::ElectronRefVector>("vetoelectrons");
  produces<pat::ElectronRefVector>("looseelectrons");
  produces<pat::ElectronRefVector>("mediumelectrons");
  produces<pat::ElectronRefVector>("tightelectrons");
  produces<pat::ElectronRefVector>("heepelectrons");
}

PFCleaner::~PFCleaner() {}

void PFCleaner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  edm::Handle<std::vector<pat::Electron> > electronsH;
  iEvent.getByToken(electronsToken, electronsH);

  edm::Handle<edm::ValueMap<bool> > electronVetoIdH;
  iEvent.getByToken(electronVetoIdMapToken, electronVetoIdH);

  edm::Handle<edm::ValueMap<bool> > electronLooseIdH;
  iEvent.getByToken(electronLooseIdMapToken, electronLooseIdH);

  edm::Handle<edm::ValueMap<bool> > electronMediumIdH;
  iEvent.getByToken(electronMediumIdMapToken, electronMediumIdH);

  edm::Handle<edm::ValueMap<bool> > electronTightIdH;
  iEvent.getByToken(electronTightIdMapToken, electronTightIdH);

  edm::Handle<edm::ValueMap<bool> > electronHeepIdH;
  iEvent.getByToken(electronHeepIdMapToken, electronHeepIdH);
    
  std::auto_ptr<pat::ElectronRefVector> outputvetoelectrons(new pat::ElectronRefVector);
  std::auto_ptr<pat::ElectronRefVector> outputlooseelectrons(new pat::ElectronRefVector);
  std::auto_ptr<pat::ElectronRefVector> outputmediumelectrons(new pat::ElectronRefVector);
  std::auto_ptr<pat::ElectronRefVector> outputtightelectrons(new pat::ElectronRefVector);
  std::auto_ptr<pat::ElectronRefVector> outputheepelectrons(new pat::ElectronRefVector);

  //electron info https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2
  for (std::vector<pat::Electron>::const_iterator electrons_iter = electronsH->begin(); electrons_iter != electronsH->end(); ++electrons_iter) {
    const edm::Ptr<pat::Electron> electronPtr(electronsH, electrons_iter - electronsH->begin());

    // cuts!
    bool passeskincuts  = (electrons_iter->pt() > 10 && fabs(electrons_iter->superCluster()->eta()) < 2.5); 
    bool passesvetoid   = (*electronVetoIdH)[electronPtr];
    bool passeslooseid  = (*electronLooseIdH)[electronPtr];
    bool passesmediumid = (*electronMediumIdH)[electronPtr];
    bool passestightid  = (*electronTightIdH)[electronPtr];
    bool passesheepid   = (*electronHeepIdH)[electronPtr];
    
    if (passeskincuts && passesvetoid) 
      outputvetoelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));
    
    if (passeskincuts && passeslooseid) 
      outputlooseelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));
    
    if (passeskincuts && passesmediumid) 
	outputmediumelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));
    
    if (passeskincuts && passestightid) 
      outputtightelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));
    
    if (passeskincuts && passesheepid)
	outputheepelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));
  }

  iEvent.put(outputvetoelectrons,   "vetoelectrons");
  iEvent.put(outputlooseelectrons,  "looseelectrons");
  iEvent.put(outputmediumelectrons, "mediumelectrons");
  iEvent.put(outputtightelectrons,  "tightelectrons");
  iEvent.put(outputheepelectrons,   "heepelectrons");
}

void PFCleaner::beginJob() {}

void PFCleaner::endJob() {}

void PFCleaner::beginRun(edm::Run const&, edm::EventSetup const&) {}

void PFCleaner::endRun(edm::Run const&, edm::EventSetup const&) {}

void PFCleaner::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}

void PFCleaner::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {}

void PFCleaner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(PFCleaner);
