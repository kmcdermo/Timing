#include <memory>
#include <vector>
#include <iostream>

#include <TRandom3.h>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/EgammaCandidates/interface/Conversion.h"
#include "DataFormats/GsfTrackReco/interface/GsfTrack.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "RecoEgamma/EgammaTools/interface/ConversionTools.h"

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

        bool randomConeOverlaps(double, double, double, std::vector<pat::Jet>);

        //livia 
        bool isPassingPhotonHighPtID(double, double, double, double, double, double, double);

        const edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;
        const edm::EDGetTokenT<edm::View<reco::Candidate> > pfcandsToken;
        const edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;
        const edm::EDGetTokenT<std::vector<pat::Muon> > muonsToken;
        const edm::EDGetTokenT<std::vector<pat::Tau> >  tausToken;
        const edm::EDGetTokenT<std::vector<pat::Electron> > electronsToken;
        const edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;

        const edm::EDGetTokenT<edm::ValueMap<bool> > electronVetoIdMapToken;
        const edm::EDGetTokenT<edm::ValueMap<bool> > electronLooseIdMapToken;
        const edm::EDGetTokenT<edm::ValueMap<bool> > electronTightIdMapToken;
        const edm::EDGetTokenT<edm::ValueMap<bool> > electronHeepIdMapToken;

        const edm::EDGetTokenT<edm::ValueMap<bool> > photonLooseIdMapToken;
        const edm::EDGetTokenT<edm::ValueMap<bool> > photonMediumIdMapToken;
        const edm::EDGetTokenT<double>  rhoToken;

        //livia 
        const edm::EDGetTokenT<edm::ValueMap<float> > photonsieieToken;
        const edm::EDGetTokenT<edm::ValueMap<float> > photonPHisoToken;
        const edm::EDGetTokenT<edm::ValueMap<float> > photonCHisoToken;
 
  
        bool userandomphi;
};

PFCleaner::PFCleaner(const edm::ParameterSet& iConfig): 

    verticesToken            (consumes<std::vector<reco::Vertex> > (iConfig.getParameter<edm::InputTag>("vertices"))),
    pfcandsToken             (consumes<edm::View<reco::Candidate> > (iConfig.getParameter<edm::InputTag>("pfcands"))),
    jetsToken                (consumes<std::vector<pat::Jet> > (iConfig.getParameter<edm::InputTag>("jets"))),
    muonsToken               (consumes<std::vector<pat::Muon> > (iConfig.getParameter<edm::InputTag>("muons"))), 
    tausToken                (consumes<std::vector<pat::Tau> >  (iConfig.getParameter<edm::InputTag>("taus"))),
    electronsToken           (consumes<std::vector<pat::Electron> > (iConfig.getParameter<edm::InputTag>("electrons"))),
    photonsToken             (consumes<std::vector<pat::Photon> > (iConfig.getParameter<edm::InputTag>("photons"))),
    electronVetoIdMapToken   (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidveto"))),
    electronLooseIdMapToken  (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidloose"))),
    electronTightIdMapToken  (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidtight"))),
    electronHeepIdMapToken   (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("electronidheep"))),
    photonLooseIdMapToken    (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("photonidloose"))),
    photonMediumIdMapToken    (consumes<edm::ValueMap<bool> >(iConfig.getParameter<edm::InputTag>("photonidmedium"))),
    rhoToken                 (consumes<double>(iConfig.getParameter<edm::InputTag>("rho"))),
    //livia
    photonsieieToken         (consumes<edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("photonsieie"))), 
    photonPHisoToken         (consumes<edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("photonphiso"))), 
    photonCHisoToken         (consumes<edm::ValueMap<float> > (iConfig.getParameter<edm::InputTag>("photonchiso"))),   
    userandomphi             (iConfig.existsAs<bool>("userandomphiforRC") ? iConfig.getParameter<bool>("userandomphiforRC") : false)
{
    produces<pat::MuonRefVector>("muons");
    produces<pat::MuonRefVector>("tightmuons");
    produces<pat::MuonRefVector>("highptmuons");

    produces<pat::TauRefVector>("taus");

    produces<pat::ElectronRefVector>("electrons");
    produces<pat::ElectronRefVector>("looseelectrons");
    produces<pat::ElectronRefVector>("tightelectrons");
    produces<pat::ElectronRefVector>("heepelectrons");

    produces<pat::PhotonRefVector>("photons");
    produces<pat::PhotonRefVector>("loosephotons");
    produces<pat::PhotonRefVector>("mediumphotons");
    produces<pat::PhotonRefVector>("tightphotons");

    produces<edm::ValueMap<float> >("rndgammaiso");
    produces<edm::ValueMap<float> >("rndchhadiso");
    produces<edm::ValueMap<bool> >("photonHighPtId");
}


PFCleaner::~PFCleaner() {
}

void PFCleaner::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
    using namespace edm;
    using namespace reco;
    using namespace std;

    Handle<std::vector<reco::Vertex> > verticesH;
    iEvent.getByToken(verticesToken, verticesH);

    Handle<edm::View<reco::Candidate> > pfcandsH;
    iEvent.getByToken(pfcandsToken, pfcandsH);

    Handle<std::vector<pat::Jet> > jetsH;
    iEvent.getByToken(jetsToken, jetsH);

    Handle<std::vector<pat::Muon> > muonsH;
    iEvent.getByToken(muonsToken, muonsH);

    Handle<std::vector<pat::Electron> > electronsH;
    iEvent.getByToken(electronsToken, electronsH);

    Handle<std::vector<pat::Tau> > tausH;
    iEvent.getByToken(tausToken, tausH);

    Handle<std::vector<pat::Photon> > photonsH;
    iEvent.getByToken(photonsToken, photonsH);

    Handle<edm::ValueMap<bool> > electronVetoIdH;
    iEvent.getByToken(electronVetoIdMapToken, electronVetoIdH);

    Handle<edm::ValueMap<bool> > electronLooseIdH;
    iEvent.getByToken(electronLooseIdMapToken, electronLooseIdH);

    Handle<edm::ValueMap<bool> > electronTightIdH;
    iEvent.getByToken(electronTightIdMapToken, electronTightIdH);

    Handle<edm::ValueMap<bool> > electronHeepIdH;
    iEvent.getByToken(electronHeepIdMapToken, electronHeepIdH);

    Handle<edm::ValueMap<bool> > photonLooseIdH;
    iEvent.getByToken(photonLooseIdMapToken, photonLooseIdH);

    Handle<edm::ValueMap<bool> > photonMediumIdH;
    iEvent.getByToken(photonMediumIdMapToken, photonMediumIdH);

    Handle<edm::ValueMap<float> > photonsieieH;
    iEvent.getByToken(photonsieieToken, photonsieieH);

    Handle<edm::ValueMap<float> > photonPHisoH;
    iEvent.getByToken(photonPHisoToken, photonPHisoH);

    Handle<edm::ValueMap<float> > photonCHisoH;
    iEvent.getByToken(photonCHisoToken, photonCHisoH);
    
    Handle<double> rhoH;
    iEvent.getByToken(rhoToken, rhoH);
    double rho = *rhoH;
    
    std::auto_ptr<pat::MuonRefVector> outputmuons(new pat::MuonRefVector);
    std::auto_ptr<pat::MuonRefVector> outputtightmuons(new pat::MuonRefVector);
    std::auto_ptr<pat::MuonRefVector> outputhighptmuons(new pat::MuonRefVector);
    std::auto_ptr<pat::ElectronRefVector> outputelectrons(new pat::ElectronRefVector);
    std::auto_ptr<pat::ElectronRefVector> outputvetoelectrons(new pat::ElectronRefVector);
    std::auto_ptr<pat::ElectronRefVector> outputlooseelectrons(new pat::ElectronRefVector);
    std::auto_ptr<pat::ElectronRefVector> outputtightelectrons(new pat::ElectronRefVector);
    std::auto_ptr<pat::ElectronRefVector> outputheepelectrons(new pat::ElectronRefVector);
    std::auto_ptr<pat::PhotonRefVector> outputphotons(new pat::PhotonRefVector);
    std::auto_ptr<pat::PhotonRefVector> outputloosephotons(new pat::PhotonRefVector);
    std::auto_ptr<pat::PhotonRefVector> outputmediumphotons(new pat::PhotonRefVector);
    std::auto_ptr<pat::PhotonRefVector> outputtightphotons(new pat::PhotonRefVector);
    std::auto_ptr<edm::ValueMap<float> > outputgammaisomap(new ValueMap<float>());
    std::auto_ptr<edm::ValueMap<float> > outputchhadisomap(new ValueMap<float>());
    std::auto_ptr<pat::TauRefVector> outputtaus(new pat::TauRefVector);
    //livia
    std::auto_ptr<edm::ValueMap<bool> > outputphotonhighptidmap(new ValueMap<bool>());

    TRandom3 rand;

    //    std::cout<<"new event "<<iEvent.id().event()<<std::endl;

    //muon info https://twiki.cern.ch/twiki/bin/viewauth/CMS/SWGuideMuonIdRun2
    for (vector<pat::Muon>::const_iterator muons_iter = muonsH->begin(); muons_iter != muonsH->end(); ++muons_iter) {
        if (verticesH->size() == 0) continue;

        bool passeskincuts = (muons_iter->pt() > 10 && fabs(muons_iter->eta()) < 2.4);
        float isoval       = muons_iter->pfIsolationR04().sumNeutralHadronEt;
        isoval += muons_iter->pfIsolationR04().sumPhotonEt;
        isoval -= 0.5*muons_iter->pfIsolationR04().sumPUPt;
        if (isoval < 0.) isoval = 0.;
        isoval += muons_iter->pfIsolationR04().sumChargedHadronPt;
        isoval /= muons_iter->pt();

        if (passeskincuts) {
	  if (muon::isLooseMuon(*muons_iter) && isoval < 0.2) 
	    outputmuons->push_back(pat::MuonRef(muonsH, muons_iter - muonsH->begin()));
	  if (muon::isTightMuon(*muons_iter, *(verticesH->begin())) && isoval < 0.12) 
	    outputtightmuons->push_back(pat::MuonRef(muonsH, muons_iter - muonsH->begin()));
	  if (muon::isHighPtMuon(*muons_iter, *(verticesH->begin())) && isoval < 0.12)
	    outputhighptmuons->push_back(pat::MuonRef(muonsH, muons_iter - muonsH->begin()));
	}
    }


    //electron info https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedElectronIdentificationRun2
    for (vector<pat::Electron>::const_iterator electrons_iter = electronsH->begin(); electrons_iter != electronsH->end(); ++electrons_iter) {

        const Ptr<pat::Electron> electronPtr(electronsH, electrons_iter - electronsH->begin());

        bool passeskincuts  = (electrons_iter->pt() > 10 && fabs(electrons_iter->superCluster()->eta()) < 2.5);
        bool passesvetoid   = (*electronVetoIdH)[electronPtr];
        bool passeslooseid  = (*electronLooseIdH)[electronPtr];
        bool passestightid  = (*electronTightIdH)[electronPtr];
        bool passesheepid   = (*electronHeepIdH)[electronPtr];

	//cout<<"electron pt "<<electrons_iter->pt()<<" eta "<<electrons_iter->eta()<<" dEta "<<electrons_iter->deltaEtaSuperClusterTrackAtVtx()<<" dPhi "<<electrons_iter->deltaPhiSuperClusterTrackAtVtx()<<" sigmaIetaIeta "<<electrons_iter->full5x5_sigmaIetaIeta()<<" HoverE "<<electrons_iter->hadronicOverEm()<<" dxy "<<electrons_iter->gsfTrack()->dxy(verticesH->at(0).position())<<" dZ "<<electrons_iter->gsfTrack()->dz(verticesH->at(0).position())<<" ooemoop "<<std::abs(1.0 - electrons_iter->eSuperClusterOverP())*1.0/electrons_iter->ecalEnergy()<<" EoverP "<<electrons_iter->eSuperClusterOverP()<<" ecalEnergy "<<electrons_iter->ecalEnergy()<<" chIso "<<electrons_iter->pfIsolationVariables().sumChargedHadronPt<<" nhIso "<<electrons_iter->pfIsolationVariables().sumNeutralHadronEt<<" emIso "<<electrons_iter->pfIsolationVariables().sumPhotonEt<<" rho "<<rho<<" missing hits "<<electrons_iter->gsfTrack()->hitPattern().numberOfHits(reco::HitPattern::MISSING_INNER_HITS)<<endl;

        if (passeskincuts && passesvetoid) 
	  outputelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));

        if (passeskincuts && passeslooseid) 
	  outputlooseelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));

        if (passeskincuts && passestightid) 
	  outputtightelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));

	if(passeskincuts &&  passesheepid)
	  outputheepelectrons->push_back(pat::ElectronRef(electronsH, electrons_iter - electronsH->begin()));
	
    }
    
    // taus
    for (vector<pat::Tau>::const_iterator taus_iter = tausH->begin(); taus_iter != tausH->end(); ++taus_iter) {
        if (verticesH->size() == 0) continue;
	// clean tau candidates from identified muons and electrons (loosely identified)
	bool skiptau = false;
        for (std::size_t j = 0; j < outputmuons->size(); j++) {
          if (deltaR(outputmuons->at(j)->eta(), outputmuons->at(j)->phi(), taus_iter->eta(), taus_iter->phi()) < 0.4) skiptau = true;
        }
        for (std::size_t j = 0; j < outputelectrons->size(); j++) {
          if (deltaR(outputelectrons->at(j)->eta(), outputelectrons->at(j)->phi(), taus_iter->eta(), taus_iter->phi()) < 0.4) skiptau = true;
        }
	
	// apply loose id and store vector
	if (taus_iter->pt() > 18 &&
            fabs(taus_iter->eta()) < 2.3 &&
            taus_iter->tauID("decayModeFinding") > 0.5 &&
            taus_iter->tauID("byCombinedIsolationDeltaBetaCorrRaw3Hits") < 5 && !skiptau){
	  outputtaus->push_back(pat::TauRef(tausH, taus_iter - tausH->begin()));
	}
	else continue;
    }


    // photon https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2
    std::vector<float> rndgammaiso;
    std::vector<float> rndchhadiso;
    std::vector<bool> photonidhighpt;//livia

    for (vector<pat::Photon>::const_iterator photons_iter = photonsH->begin(); photons_iter != photonsH->end(); ++photons_iter) {

        float gaisoval = 0.;
        float chisoval = 0.;
        bool passesphotonidhighpt = false;

        double rndphi = photons_iter->phi() + M_PI/2.0;
        if (userandomphi) {
            rndphi = rand.Uniform(-M_PI, M_PI);
            while (randomConeOverlaps(rndphi, photons_iter->eta(), photons_iter->phi(), *jetsH)) 
	      rndphi = rand.Uniform(-M_PI, M_PI);
        }

        for(size_t i = 0; i < pfcandsH->size(); i++) {
            const auto& pfcand = pfcandsH->ptrAt(i);
            if (    pfcand->pdgId()  ==  22 && deltaR(photons_iter->eta(), rndphi, pfcand->eta(), pfcand->phi()) <= 0.3) 
	      gaisoval += pfcand->pt();
            if (abs(pfcand->pdgId()) == 211 && deltaR(photons_iter->eta(), rndphi, pfcand->eta(), pfcand->phi()) <= 0.3) 
	      chisoval += pfcand->pt();
        }
        rndgammaiso.push_back(gaisoval);
        rndchhadiso.push_back(chisoval);

        //livia
        const Ptr<pat::Photon> photonPtr(photonsH, photons_iter - photonsH->begin());
        double photonsieie=(*photonsieieH)[photonPtr];
        double photonphiso=(*photonPHisoH)[photonPtr];
        double photonchiso=(*photonCHisoH)[photonPtr];
        double photonhoe =  photons_iter->hadTowOverEm();

        passesphotonidhighpt = isPassingPhotonHighPtID(photons_iter->pt(), photons_iter->superCluster()->eta(), photonhoe, photonchiso, photonphiso, photonsieie, rho);
        photonidhighpt.push_back(passesphotonidhighpt);


        if (fabs(photons_iter->superCluster()->eta()) > 2.5 || photons_iter->pt() < 15) continue;
        if (photons_iter->r9() > 0.8 || photons_iter->chargedHadronIso() < 20. || photons_iter->chargedHadronIso() < photons_iter->pt()*0.3) 
	  outputloosephotons->push_back(pat::PhotonRef(photonsH, photons_iter - photonsH->begin())); 

        bool passeslooseid = (*photonLooseIdH)[photonPtr];
        bool passesmediumid = (*photonMediumIdH)[photonPtr];

        if ((passeslooseid || passesphotonidhighpt) && photons_iter->passElectronVeto()) {

            if (passeslooseid) 
	      outputphotons->push_back(pat::PhotonRef(photonsH, photons_iter - photonsH->begin()));

	    if (passesmediumid)
	      outputmediumphotons->push_back(pat::PhotonRef(photonsH, photons_iter - photonsH->begin()));

            if (photons_iter->pt() > 120) 
	      outputtightphotons->push_back(pat::PhotonRef(photonsH, photons_iter - photonsH->begin()));
        }
    }


    edm::ValueMap<float>::Filler gafiller(*outputgammaisomap);
    gafiller.insert(photonsH, rndgammaiso.begin(), rndgammaiso.end());
    gafiller.fill();

    edm::ValueMap<float>::Filler chfiller(*outputchhadisomap);
    chfiller.insert(photonsH, rndchhadiso.begin(), rndchhadiso.end());
    chfiller.fill();

    //livia
    
    edm::ValueMap<bool>::Filler photonhighptidfiller(*outputphotonhighptidmap);
    photonhighptidfiller.insert(photonsH, photonidhighpt.begin(), photonidhighpt.end());
    photonhighptidfiller.fill();
    
    iEvent.put(outputmuons, "muons");
    iEvent.put(outputtightmuons, "tightmuons");
    iEvent.put(outputhighptmuons, "highptmuons");

    iEvent.put(outputelectrons, "electrons");
    iEvent.put(outputtightelectrons, "tightelectrons");
    iEvent.put(outputheepelectrons, "heepelectrons");

    iEvent.put(outputphotons, "photons");
    iEvent.put(outputtightphotons, "tightphotons");
    iEvent.put(outputloosephotons, "loosephotons");
    iEvent.put(outputphotonhighptidmap, "photonHighPtId");

    iEvent.put(outputgammaisomap, "rndgammaiso");
    iEvent.put(outputchhadisomap, "rndchhadiso");

    iEvent.put(outputtaus, "taus");

}

void PFCleaner::beginJob() {
}

void PFCleaner::endJob() {
}

void PFCleaner::beginRun(edm::Run const&, edm::EventSetup const&) {
}

void PFCleaner::endRun(edm::Run const&, edm::EventSetup const&) {
}

void PFCleaner::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {
}

void PFCleaner::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) {
}

void PFCleaner::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
    edm::ParameterSetDescription desc;
    desc.setUnknown();
    descriptions.addDefault(desc);
}

bool PFCleaner::randomConeOverlaps(double randomphi, double photoneta, double photonphi, std::vector<pat::Jet> jets) {
    if (reco::deltaR(photoneta, randomphi, photoneta, photonphi) < 0.4) return true;
    for (std::size_t i = 0; i < jets.size(); i++) {
        if (jets[i].pt() > 30. && reco::deltaR(photoneta, randomphi, jets[i].eta(), jets[i].phi()) < 0.4) return true;
    }
    return false;
}

bool PFCleaner::isPassingPhotonHighPtID(double pt, double eta, double hoe, double chiso, double phiso, double sieie, double rho ){

    double chisoCUT;
    double phisoCUT;
    double sieieCUT;
    double hoeCUT;
    double alpha;
    double k;
    double EA=0;
    double newphiso;
    int isPassing = false;
    if(abs(eta)<1.4442){
        chisoCUT= 5;
        sieieCUT=0.0105;
        hoeCUT=0.05;
        phisoCUT=2.75;
        alpha=2.5;
        k=0.0045;
        if(abs(eta)<0.9)EA=0.17;
        if(abs(eta)>0.9&&abs(eta)<1.4442)EA=0.14;
    }
    newphiso = alpha+phiso-rho*EA-k*pt;
    if(newphiso < phisoCUT && chiso < chisoCUT && sieie < sieieCUT && hoe < hoeCUT)isPassing =true;
    
    return isPassing;
}

DEFINE_FWK_MODULE(PFCleaner);
