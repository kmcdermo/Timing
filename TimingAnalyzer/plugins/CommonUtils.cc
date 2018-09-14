#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

namespace oot
{
  /////////////////
  //             //
  // Object Prep //
  //             //
  /////////////////

  void ReadInTriggerNames(const std::string & inputPaths, std::vector<std::string> & pathNames, 
			  strBitMap & triggerBitMap)
  {
    if (Config::file_exists(inputPaths))
    {
      std::fstream pathStream;
      pathStream.open(inputPaths.c_str(),std::ios::in);
      std::string path;
      while (pathStream >> path)
      {
	if (path != "") 
	{
	  pathNames.emplace_back(path);
	  triggerBitMap[path] = false;
	}
      }
      pathStream.close();
    } // check to make sure text file exists
  }

  void ReadInFilterNames(const std::string & inputFilters, std::vector<std::string> & filterNames, 
			 trigObjVecMap & triggerObjectsByFilterMap)
  {
    if (Config::file_exists(inputFilters))
    {
      std::fstream filterStream;
      filterStream.open(inputFilters.c_str(),std::ios::in);
      std::string label;// instance, processName;
      while (filterStream >> label)
      {
	if (label != "") 
	{
	  filterNames.emplace_back(label);
	  triggerObjectsByFilterMap[label].clear();
	}
      }
      filterStream.close();
    } // check to make sure text file exists
  }

  void PrepNeutralinos(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, genPartVec& neutralinos)
  {
    if (genparticlesH.isValid())
    {
      int nNeutoPhGr = 0;
      for (const auto & genparticle : *genparticlesH) // loop over gen particles
      {
	if (nNeutoPhGr == 2) break;

	if (genparticle.pdgId() == 1000022 && genparticle.numberOfDaughters() == 2)
	{
	  if ((genparticle.daughter(0)->pdgId() == 22 && genparticle.daughter(1)->pdgId() == 1000039) ||
	      (genparticle.daughter(1)->pdgId() == 22 && genparticle.daughter(0)->pdgId() == 1000039)) 
	  {
	    nNeutoPhGr++;
	    neutralinos.emplace_back(genparticle);
	  } // end conditional over matching daughter ids
	} // end conditional over neutralino id
      } // end loop over gen particles

      std::sort(neutralinos.begin(),neutralinos.end(),oot::sortByPt);
    } // end check over valid
  }

  void PrepVPions(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, genPartVec& vPions)
  {
    if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
    {
      for (const auto & genparticle : *genparticlesH) // loop over gen particles
      {
	if (genparticle.pdgId() == 4900111 && genparticle.numberOfDaughters() == 2)
    	{
	  if (genparticle.daughter(0)->pdgId() == 22 && genparticle.daughter(1)->pdgId() == 22)
	  {
	    vPions.emplace_back(genparticle);
	  } // end check over both gen photons	
	} // end check over vPions
      } // end loop over gen particles

      std::sort(vPions.begin(),vPions.end(),oot::sortByPt);
    }
  }

  void PrepToys(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, genPartVec& toys)
  {
    if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
    {
      for (const auto & genparticle : *genparticlesH) // loop over gen particles
      {
	if (genparticle.pdgId() == 22)
	{
	  toys.emplace_back(genparticle);
	} // end check over photons
      } // end loop over gen particles

      std::sort(toys.begin(),toys.end(),oot::sortByPt);
    }
  }

  void PrepTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, 
		       const edm::Event & iEvent, strBitMap & triggerBitMap)
  {
    for (auto & triggerBitPair : triggerBitMap) 
    {
      triggerBitPair.second = false;
    }
    
    if (triggerResultsH.isValid())
    {
      const edm::TriggerNames &triggerNames = iEvent.triggerNames(*triggerResultsH);
      for (std::size_t itrig = 0; itrig < triggerNames.size(); itrig++)
      {
	std::string triggerName = triggerNames.triggerName(itrig);
      
	for (auto & triggerBitPair : triggerBitMap) 
	{
	  if (triggerName.find(triggerBitPair.first) != std::string::npos) triggerBitPair.second = triggerResultsH->accept(itrig);
	} // end loop over user path names
      } // end loop over trigger names
    } // end check over valid TriggerResults
  }
  
  void PrepTriggerObjects(const edm::Handle<edm::TriggerResults> & triggerResultsH,
			  const edm::Handle<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsH,
			  const edm::Event & iEvent, trigObjVecMap & triggerObjectsByFilterMap)
  {
    // clear first
    for (auto& triggerObjectsByFilterPair : triggerObjectsByFilterMap)
    {
      triggerObjectsByFilterPair.second.clear();
    }
    
    // store all the trigger objects needed to be checked later
    if (triggerObjectsH.isValid() && triggerResultsH.isValid())
    {
      const edm::TriggerNames &triggerNames = iEvent.triggerNames(*triggerResultsH);
      for (pat::TriggerObjectStandAlone triggerObject : *triggerObjectsH) 
      {
	triggerObject.unpackPathNames(triggerNames);
	triggerObject.unpackFilterLabels(iEvent, *triggerResultsH);
	for (auto& triggerObjectsByFilterPair : triggerObjectsByFilterMap)
	{	
	  if (triggerObject.hasFilterLabel(triggerObjectsByFilterPair.first)) triggerObjectsByFilterPair.second.emplace_back(triggerObject);
	} // end loop over user filter names
      } // end loop over trigger objects

      for (auto& triggerObjectsByFilterPair : triggerObjectsByFilterMap)
      {
	std::sort(triggerObjectsByFilterPair.second.begin(),triggerObjectsByFilterPair.second.end(),oot::sortByPt);
      }
    }
  }

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   std::vector<oot::Photon> & photons, const float rho,
		   const float phpTmin, const std::string & phIDmin)
  {
    // get VIDs of GED photons first (both GED ID and OOT ID), then OOT photons
    // put both GED and OOT photons in a common container, sorting by pT at the end
    oot::PrepPhotons(photonsH,photons,false,rho,phpTmin,phIDmin);
    oot::PrepPhotons(ootPhotonsH,photons,true,rho,phpTmin,phIDmin);

    std::sort(photons.begin(),photons.end(),oot::sortByPt);
  }

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   std::vector<oot::Photon> & photons, const bool isOOT,
		   const float rho, const float phpTmin, const std::string & phIDmin)
  {
    if (photonsH.isValid()) // standard handle check
    {
      for (const auto & photon : *photonsH)
      {
	if (photon.pt() < phpTmin) continue;

	idpVec idpairs;
	idpairs = {{"loose-ged",false}, {"medium-ged",false}, {"tight-ged",false}, {"loose-oot",false}, {"tight-oot",false}};
	oot::GetGEDPhoVID      (photon,idpairs);
	oot::GetOOTPhoVIDByHand(photon,idpairs,rho);
	
	bool isGoodID = true;
	if (phIDmin != "none")
	{
	  for (const auto & idpair : idpairs) 
	  {
	    if (idpair.first.find(phIDmin) != std::string::npos) // correct for GED or OOT!
	    {
	      if ((isOOT && idpair.first.find("oot")) || (!isOOT && idpair.first.find("ged")))
	      {
		if (!idpair.second) isGoodID = false;
		break;
	      }
	    }
	  }
	}
	if (!isGoodID) continue;

	photons.emplace_back(photon,isOOT);
	photons.back().photon_nc().setPhotonIDs(idpairs);
      } // end loop over photons
    } // isValid
  }

  void PrepRecHits(const EcalRecHitCollection * recHitsEB,
		   const EcalRecHitCollection * recHitsEE,
		   uiiumap & recHitMap, const float rhEmin)
  {
    int i = 0;
    for (const auto & recHit : *recHitsEB)
    {
      if (recHit.energy() > rhEmin)
      {
	recHitMap[recHit.detid().rawId()] = i++;
      }
    }
    for (const auto & recHit : *recHitsEE)
    {
      if (recHit.energy() > rhEmin)
      {
	recHitMap[recHit.detid().rawId()] = i++;
      }
    }
  }
    
  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin, 
		const float jetEtamax, const int jetID)
  {
    if (jetsH.isValid()) // standard handle check
    {
      for (const auto& jet : *jetsH)
      {
	if (jet.pt() < jetpTmin) continue;
	if (std::abs(jet.eta()) > jetEtamax) continue;
	if (oot::GetPFJetID(jet) < jetID) continue;

	jets.emplace_back(jet);
      }
      
      std::sort(jets.begin(),jets.end(),oot::sortByPt);
    }
  }  

  void PrepElectrons(const edm::Handle<std::vector<pat::Electron> > & electronsH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronVetoIdMapH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronLooseIdMapH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronMediumIdMapH, 
		     const edm::Handle<edm::ValueMap<bool> > & electronTightIdMapH, 
		     std::vector<pat::Electron> & electrons)
  {
    if (electronsH.isValid()) // standard handle check
    {
      const edm::ValueMap<bool> electronVetoIdMap   = *electronVetoIdMapH;
      const edm::ValueMap<bool> electronLooseIdMap  = *electronLooseIdMapH;
      const edm::ValueMap<bool> electronMediumIdMap = *electronMediumIdMapH;
      const edm::ValueMap<bool> electronTightIdMap  = *electronTightIdMapH;

      // create and initialize temp id-value vector
      std::vector<std::vector<pat::Electron::IdPair> > idpairs(electrons.size());
      for (size_t iel = 0; iel < idpairs.size(); iel++)
      {
	idpairs[iel].resize(4);
	idpairs[iel][0] = {"veto"  ,false};
	idpairs[iel][1] = {"loose" ,false};
	idpairs[iel][2] = {"medium",false};
	idpairs[iel][3] = {"tight" ,false};
      }

      int ielH = 0; // dumb counter because iterators only work with VID
      for (std::vector<pat::Electron>::const_iterator phiter = electronsH->begin(); phiter != electronsH->end(); ++phiter) // loop over electron vector
      {
	// Get the VID of the electron
	const edm::Ptr<pat::Electron> electronPtr(electronsH, phiter - electronsH->begin());
	
	// store VID in temp struct
	// veto < loose < medium < tight
	if (electronVetoIdMap  [electronPtr]) idpairs[ielH][0].second = true;
	if (electronLooseIdMap [electronPtr]) idpairs[ielH][1].second = true;
	if (electronMediumIdMap[electronPtr]) idpairs[ielH][2].second = true;
	if (electronTightIdMap [electronPtr]) idpairs[ielH][3].second = true;
	
	ielH++;
      }
      
      // set the ID-value for each electron in other collection
      for (size_t iel = 0; iel < electrons.size(); iel++)
      {
	electrons[iel].setElectronIDs(idpairs[iel]);
      }
      
      // now finally sort vector by pT
      std::sort(electrons.begin(),electrons.end(),oot::sortByPt);
    }
  }  

  ///////////////////
  //               //
  // Extra Pruning //
  //               //
  ///////////////////

  void PrunePhotons(std::vector<oot::Photon> & photons,
		    const EcalRecHitCollection * recHitsEB,
		    const EcalRecHitCollection * recHitsEE,
		    const float seedTimemin)
  {
    photons.erase(std::remove_if(photons.begin(),photons.end(),
				 [seedTimemin,&recHitsEB,&recHitsEE](const auto & photon)
				 {
				   const pat::Photon & pho = photon.photon();
				   const reco::SuperClusterRef& phosc = pho.superCluster().isNonnull() ? pho.superCluster() : pho.parentSuperCluster();
				   const DetId & seedDetId = phosc->seed()->seed(); // get seed detid
				   const EcalRecHitCollection * recHits = ((seedDetId.subdetId() == EcalBarrel) ? recHitsEB : recHitsEE); // which recHits to use
				   EcalRecHitCollection::const_iterator seedHit = recHits->find(seedDetId); // get the underlying rechit
				   const float seedTime = ((seedHit != recHits->end()) ? seedHit->time() : -9999.f);
				   return (seedTime < seedTimemin);
				 }),photons.end());
  }

  void PruneJets(std::vector<pat::Jet> & jets, const std::vector<oot::Photon> & photons,
		 const float dRmin)
  {
    if (photons.size() > 0)
    {
      // only clean out w.r.t. to leading photon... can do more later
      const auto & photon = photons[0].photon();

      // apply loose selection on photon 
      const float HoverE = photon.hadTowOverEm();
      const float Sieie  = photon.full5x5_sigmaIetaIeta();
      const float eta = std::abs(photon.superCluster()->eta());
      
      // cuts set to be looser than trigger values by .05 in H/E and 0.005 in Sieie
      if ( ((eta < Config::etaEBcutoff) && (HoverE < 0.25) && (Sieie < 0.019)) || ((eta >= Config::etaEBcutoff && eta < Config::etaEEmax) && (HoverE < 0.2) && (Sieie < 0.04)) )
      {
     	jets.erase(std::remove_if(jets.begin(),jets.end(),
				  [dRmin,&photon](const auto & jet)
				  {
				    return (reco::deltaR(jet,photon) < dRmin);
				  }),jets.end());
      }
    }
  }

  /////////////////////
  //                 //
  // Effective Areas //
  //                 //
  /////////////////////

  float GetChargedHadronEA(const float eta)
  {
    if      (eta <  1.0)                  return 0.0385;
    else if (eta >= 1.0   && eta < 1.479) return 0.0468;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0435;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0378;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0338;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.0314;
    else if (eta >= 2.4)                  return 0.0269;
    else                                  return 0.;
  }
  
  float GetNeutralHadronEA(const float eta) 
  {
    if      (eta <  1.0)                  return 0.0636;
    else if (eta >= 1.0   && eta < 1.479) return 0.1103;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0759;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0236;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0151;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.00007;
    else if (eta >= 2.4)                  return 0.0132;
    else                                  return 0.;
  }
  
  float GetGammaEA(const float eta) 
  {
    if      (eta <  1.0)                  return 0.1240;
    else if (eta >= 1.0   && eta < 1.479) return 0.1093;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0631;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0779;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0999;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.1155;
    else if (eta >= 2.4)                  return 0.1373;
    else                                  return 0.;
  }

  float GetEcalPFClEA(const float eta)
  {
    if      (eta < 0.8 ) return 0.19;
    else if (eta < 1.44) return 0.14;
    else                 return 0.f;
  }

  float GetHcalPFClEA(const float eta)
  {
    if      (eta < 0.8 ) return 0.089;
    else if (eta < 1.44) return 0.15;
    else                 return 0.f;
  }

  float GetTrackEA(const float eta)
  {
    if      (eta < 0.8 ) return 0.037;
    else if (eta < 1.44) return 0.031;
    else                 return 0.f;
  }

  ////////////////
  //            //
  // pT scaling //
  //            //
  ////////////////

  float GetNeutralHadronPtScale(const float eta, const float pt)
  {
    if      (eta <  Config::etaEBcutoff)                           return 0.0126*pt+0.000026*pt*pt;
    else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax) return 0.0119*pt+0.000025*pt*pt;
    else                                                           return 0.f;
  }

  float GetGammaPtScale(const float eta, const float pt)
  {
    if      (eta <  Config::etaEBcutoff)                           return 0.0035*pt;
    else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax) return 0.0040*pt;
    else                                                           return 0.f;
  }

  float GetEcalPFClPtScale(const float eta, const float pt)
  {
    if   (eta < 1.44) return 0.00092*pt;
    else              return 0.f;
  }

  float GetHcalPFClPtScale(const float eta, const float pt)
  {
    if   (eta < 1.44) return 0.0052*pt;
    else              return 0.f;
  }

  float GetTrackPtScale(const float eta, const float pt)
  {
    if   (eta < 1.44) return 0.00091*pt;
    else              return 0.f;
  }

  ////////////////////
  //                //
  // GED Photon VID //
  //                //
  ////////////////////

  void GetGEDPhoVID(const pat::Photon & photon, idpVec & idpairs)
  {
    idpairs[2].second = photon.photonID("cutBasedPhotonID-Fall17-94X-V1-tight");
    idpairs[1].second = photon.photonID("cutBasedPhotonID-Fall17-94X-V1-medium");
    idpairs[0].second = photon.photonID("cutBasedPhotonID-Fall17-94X-V1-loose");
  }

  void GetGEDPhoVIDByHand(const pat::Photon & photon, idpVec & idpairs, const float rho)
  {
    // needed for cuts
    const float eta = std::abs(photon.superCluster()->eta());
    const float pt  = photon.pt();
    
    // cut variables
    const float HoverE = photon.hadTowOverEm();
    const float Sieie  = photon.full5x5_sigmaIetaIeta();
    // Isolations are currently wrong! need to recompute them apparently : 
    // https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2#Selection_implementation_details
    // https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolationRun2#Recipe_for_accessing_PF_isol_AN1
    // https://github.com/cms-sw/cmssw/blob/master/RecoEgamma/PhotonIdentification/plugins/PhotonIDValueMapProducer.cc#L338-L395
    const float ChgHadIso = std::max(photon.chargedHadronIso() - (rho * oot::GetChargedHadronEA(eta))                                         ,0.f);
    const float NeuHadIso = std::max(photon.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(eta)) - (oot::GetNeutralHadronPtScale(eta,pt)),0.f);
    const float PhoIso    = std::max(photon.photonIso()        - (rho * oot::GetGammaEA        (eta)) - (oot::GetGammaPtScale        (eta,pt)),0.f);
    
    if (eta < Config::etaEBcutoff)
    {
      if      ((HoverE < 0.020) && (Sieie < 0.0103) && (ChgHadIso < 1.158) && (NeuHadIso < 1.267) && (PhoIso < 2.065)) 
      {
	idpairs[2].second = true;
	idpairs[1].second = true;
	idpairs[0].second = true;
      }
      else if ((HoverE < 0.035) && (Sieie < 0.0103) && (ChgHadIso < 1.416) && (NeuHadIso < 2.491) && (PhoIso < 2.952)) 
      {
	idpairs[2].second = false;
	idpairs[1].second = true;
	idpairs[0].second = true;
      }  
      else if ((HoverE < 0.105) && (Sieie < 0.0103) && (ChgHadIso < 2.839) && (NeuHadIso < 9.188) && (PhoIso < 2.956)) 
      {
	idpairs[2].second = false;
	idpairs[1].second = false;
	idpairs[0].second = true;
      }   
    }
    else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax)
    {
      if      ((HoverE < 0.025) && (Sieie < 0.0271) && (ChgHadIso < 0.575) && (NeuHadIso < 8.916) && (PhoIso < 3.272)) 
      {
	idpairs[2].second = true;
	idpairs[1].second = true;
	idpairs[0].second = true;
      }
      else if ((HoverE < 0.027) && (Sieie < 0.0271) && (ChgHadIso < 1.012) && (NeuHadIso < 9.131) && (PhoIso < 4.095)) 
      {
	idpairs[2].second = false;
	idpairs[1].second = true;
	idpairs[0].second = true;
      }   
      else if ((HoverE < 0.029) && (Sieie < 0.0276) && (ChgHadIso < 2.150) && (NeuHadIso < 10.471) && (PhoIso < 4.895)) 
      {
	idpairs[2].second = false;
	idpairs[1].second = false;
	idpairs[0].second = true;
      }   
    }
  }

  ///////////////////
  //               //
  // OOT Photon ID //
  //               //
  ///////////////////

  void GetOOTPhoVIDByHand(const pat::Photon & photon, idpVec& idpairs, const float rho)
  {
    // needed for cuts
    const float eta = std::abs(photon.superCluster()->eta());
    const float pt  = photon.pt();

    // cut variables
    const float HoverE = photon.hadTowOverEm();
    const float Sieie  = photon.full5x5_sigmaIetaIeta();
    const float EcalPFClIso = std::max(photon.ecalPFClusterIso() - (rho * oot::GetEcalPFClEA(eta)) - (oot::GetEcalPFClPtScale(eta,pt)),0.f);
    const float HcalPFClIso = std::max(photon.hcalPFClusterIso() - (rho * oot::GetHcalPFClEA(eta)) - (oot::GetHcalPFClPtScale(eta,pt)),0.f);
    const float TrkIso      = std::max(photon.trackIso()         - (rho * oot::GetTrackEA   (eta)) - (oot::GetTrackPtScale   (eta,pt)),0.f);

    if      ((HoverE < 0.0165) && (Sieie < 0.011) && (EcalPFClIso < 5.f) && (HcalPFClIso < 10.f) && (TrkIso < 5.5f))
    {
      idpairs[4].second = true;
      idpairs[3].second = true;
    }   
    else if ((HoverE < 0.185) && (Sieie < 0.0125) && (EcalPFClIso < 8.f) && (HcalPFClIso < 12.f) && (TrkIso < 8.5f)) 
    {
      idpairs[4].second = false;
      idpairs[3].second = true;
    }   
  }

  //////////////
  //          //
  // PFJet ID //
  //          //
  //////////////
  int GetPFJetID(const pat::Jet & jet) // https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2017
  {
    const float eta = std::abs(jet.eta());
    
    const float NHF  = jet.neutralHadronEnergyFraction();
    const float NEMF = jet.neutralEmEnergyFraction();
    const float CHF  = jet.chargedHadronEnergyFraction();
    const float CEMF = jet.chargedEmEnergyFraction();
    const float NHM  = jet.neutralMultiplicity();
    const float CHM  = jet.chargedMultiplicity();
    const float SHM  = jet.chargedMultiplicity()+jet.neutralMultiplicity();
    const float MUF  = jet.muonEnergyFraction();
    
    // 2 == TightLepVeto
    // 1 == Tight

    if (eta <= 2.4)
    {
      if      ((NHF < 0.90) && (NEMF < 0.90) && (SHM > 1) && (MUF < 0.80) && (CHF > 0) && (CHM > 0) && (CEMF < 0.80)) return 2;
      else if ((NHF < 0.90) && (NEMF < 0.90) && (SHM > 1) &&                 (CHF > 0) && (CHM > 0))                  return 1;
      else                                                                                                            return 0; 
    }
    else if (eta > 2.4 && eta <= 2.7)
    {
      if      ((NHF < 0.90) && (NEMF < 0.90) && (SHM > 1) && (MUF < 0.80)) return 2;
      else if ((NHF < 0.90) && (NEMF < 0.90) && (SHM > 1))                 return 1;
      else                                                                 return 0; 
    }
    else if (eta > 2.7 && eta <= 3.0)
    {
      if   ((NEMF > 0.02) && (NEMF < 0.99) && (NHM > 2)) return 1;
      else                                               return 0; 
    }
    else 
    {
      if   ((NEMF < 0.90) && (NHF > 0.02) && (NHM > 10)) return 1;
      else                                               return 0; 
    }

    return -1; // should not happen
  }

  ///////////////////////
  //                   //
  // Storing Functions //
  //                   //
  ///////////////////////
  void SplitPhotons(std::vector<oot::Photon>& photons, const int nmax)
  {
    std::vector<int> gedphos;
    std::vector<int> ootphos;

    int ipho = 0;
    for (const auto & photon : photons)
    {
      (!photon.isOOT() ? gedphos : ootphos).emplace_back(ipho++);
    }
    
    std::vector<oot::Photon> tmpphotons;

    const int ngedphos = gedphos.size();
    for (int i = 0; i < nmax; i++)
    {
      if (ngedphos > i) tmpphotons.emplace_back(photons[gedphos[i]]);
    }

    const int nootphos = ootphos.size();
    for (int i = 0; i < nmax; i++)
    {
      if (nootphos > i) tmpphotons.emplace_back(photons[ootphos[i]]);
    }

    photons.swap(tmpphotons);
  }

  void StoreOnlyPho(std::vector<oot::Photon>& photons, const int nmax, const bool isOOT)
  {
    std::vector<oot::Photon> tmpphotons;

    int ipho = 0;
    for (const auto & photon : photons)
    {
      if (ipho >= nmax) break;
      ipho++;

      if (photon.isOOT() == isOOT) tmpphotons.emplace_back(photon);
    }

    photons.swap(tmpphotons);
  }
};
