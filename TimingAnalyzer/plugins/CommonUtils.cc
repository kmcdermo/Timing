#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

namespace oot
{
  ///////////////////////////
  // Object Prep Functions //
  ///////////////////////////

  void ReadInTriggerNames(const std::string & inputPaths, std::vector<std::string> & pathNames, 
			  strBitMap & triggerBitMap)
  {
    if (Config::file_exists(inputPaths))
    {
      std::ifstream pathStream(inputPaths.c_str(),std::ios::in);
      std::string path;
      while (pathStream >> path)
      {
	if (path != "") 
	{
	  pathNames.emplace_back(path);
	  triggerBitMap[path] = false;
	}
      }
    } // check to make sure text file exists
  }

  void ReadInFilterNames(const std::string & inputFilters, std::vector<std::string> & filterNames, 
			 std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilterMap)
  {
    if (Config::file_exists(inputFilters))
    {
      std::ifstream filterStream(inputFilters.c_str(),std::ios::in);
      std::string label;// instance, processName;
      while (filterStream >> label)
      {
	if (label != "") 
	{
	  filterNames.emplace_back(label);
	  triggerObjectsByFilterMap[label].clear();
	}
      }
    } // check to make sure text file exists
  }

  void PrepNeutralinos(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, std::vector<reco::GenParticle> & neutralinos)
  {
    auto nNeutoPhGr = 0;
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
  }

  void PrepVPions(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, std::vector<reco::GenParticle> & vPions)
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

  void PrepToys(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH, std::vector<reco::GenParticle> & toys)
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

  void PrepTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, 
		       const edm::Event & iEvent, strBitMap & triggerBitMap)
  {
    for (auto & triggerBitPair : triggerBitMap) 
    {
      triggerBitPair.second = false;
    }
    
    const auto & triggerNames = iEvent.triggerNames(*triggerResultsH);
    for (auto itrig = 0U; itrig < triggerNames.size(); itrig++)
    {
      const auto & triggerName = triggerNames.triggerName(itrig);
      
      for (auto & triggerBitPair : triggerBitMap) 
      {
	if (triggerName.find(triggerBitPair.first) != std::string::npos) triggerBitPair.second = triggerResultsH->accept(itrig);
      } // end loop over user path names
    } // end loop over trigger names
  }
  
  void PrepTriggerObjects(const edm::Handle<edm::TriggerResults> & triggerResultsH,
			  const edm::Handle<std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsH,
			  const edm::Event & iEvent, std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > & triggerObjectsByFilterMap)
  {
    // clear first
    for (auto & triggerObjectsByFilterPair : triggerObjectsByFilterMap)
    {
      triggerObjectsByFilterPair.second.clear();
    }
    
    // store all the trigger objects needed to be checked later
    const auto & triggerNames = iEvent.triggerNames(*triggerResultsH);
    for (pat::TriggerObjectStandAlone triggerObject : *triggerObjectsH) 
    {
      triggerObject.unpackPathNames(triggerNames);
      triggerObject.unpackFilterLabels(iEvent, *triggerResultsH);
      for (auto & triggerObjectsByFilterPair : triggerObjectsByFilterMap)
      {	
	if (triggerObject.hasFilterLabel(triggerObjectsByFilterPair.first)) triggerObjectsByFilterPair.second.emplace_back(triggerObject);
      } // end loop over user filter names
    } // end loop over trigger objects

    for (auto & triggerObjectsByFilterPair : triggerObjectsByFilterMap)
    {
      std::sort(triggerObjectsByFilterPair.second.begin(),triggerObjectsByFilterPair.second.end(),oot::sortByPt);
    }
  }

  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin, 
		const float jetEtamax, const int jetIDmin)
  {
    for (const auto & jet : *jetsH)
    {
      if (jet.pt() < jetpTmin) continue;
      if (std::abs(jet.eta()) > jetEtamax) continue;
      
      const auto jetID = oot::GetPFJetID(jet);
      if (jetID < jetIDmin) continue;

      // save the jets, and then store the ID
      jets.emplace_back(jet);
      jets.back().addUserInt(Config::JetID,jetID);
      
      std::sort(jets.begin(),jets.end(),oot::sortByPt);
    }
  }  

  void PrepRecHits(const EcalRecHitCollection * recHitsEB,
		   const EcalRecHitCollection * recHitsEE,
		   uiiumap & recHitMap, const float rhEmin)
  {
    auto i = 0;
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
    
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  //                                             Photon Cross-Cleaning and MET Corrections                                                          //
  //                                                                                                                                                //
  // The photon cross-cleaning is applied to remove overlapping photon objects from the two photon collections (GED and OOT).                       //
  // If photond in one collection overlaps with photons in the other collection, the one with higher pT is kept, and the other photons are dropped. //
  //                                                                                                                                                //
  // Since OOT photons are not used in the computation of the MET, we have to apply the appropriate correction to the MET when an OOT photon is     //
  // either completely unmatched or is matched to a GED photon and is higher in pT than the GED photon.                                             //
  //                                                                                                                                                //
  // Namely, we __subtract__ the 4-vector of any added OOT photons, and __add__ the 4-vector of any dropped GED photons.                            //
  //                                                                                                                                                //
  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void PrepPhotonsCorrectMET(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH, 
			     const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
			     std::vector<pat::Photon> & photons, pat::MET & t1pfMET, const float rho, 
			     const float dRmin, const float phpTmin, const std::string & phIDmin)
  {
    // container for which photons to keep/drop
    std::vector<oot::ReducedPhoton> reducedPhotons;

    ///////////////////
    // Find overlaps //
    ///////////////////

    oot::FindOverlapPhotons(gedPhotonsH,ootPhotonsH,reducedPhotons,dRmin);

    ///////////////////////////////
    // Produce merged collection //
    ///////////////////////////////

    oot::MergePhotons(gedPhotonsH,ootPhotonsH,reducedPhotons,photons,rho,phpTmin,phIDmin);

    /////////////////////
    // Correct the MET //
    /////////////////////

    oot::CorrectMET(gedPhotonsH,ootPhotonsH,reducedPhotons,t1pfMET);
  }

  void FindOverlapPhotons(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH,
			  const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
			  std::vector<oot::ReducedPhoton> & reducedPhotons, const float dRmin)
  {
    ///////////////////////////////////////////////////
    // First, store all photons in single collection //
    ///////////////////////////////////////////////////

    // loop over all GED photons
    const auto nGED = gedPhotonsH->size();
    for (auto iGED = 0U; iGED < nGED; iGED++)
    {
      reducedPhotons.emplace_back(iGED,false,false);
    }

    // loop over all OOT photons
    const auto nOOT = ootPhotonsH->size();
    for (auto iOOT = 0U; iOOT < nOOT; iOOT++)
    {
      reducedPhotons.emplace_back(iOOT,true ,false);
    }

    ///////////////////////////////////
    // Second, sort the vector by pT //
    ///////////////////////////////////
    
    std::sort(reducedPhotons.begin(),reducedPhotons.end(),
	      [&](const auto & i_reducedPhoton, const auto & j_reducedPhoton)
	      {
		// tmps
		const auto i_idx = i_reducedPhoton.idx;
		const auto j_idx = j_reducedPhoton.idx;

		const auto i_isOOT = i_reducedPhoton.isOOT;
		const auto j_isOOT = j_reducedPhoton.isOOT;

		const auto & i_photon = (i_isOOT ? (*ootPhotonsH)[i_idx] : (*gedPhotonsH)[i_idx]);
		const auto & j_photon = (j_isOOT ? (*ootPhotonsH)[j_idx] : (*gedPhotonsH)[j_idx]);

		return oot::GetPhotonPt(i_photon) > oot::GetPhotonPt(j_photon);
	      });

    //////////////////////////////////////////////////////////
    // Mark for removal overlaps from different collections //
    //////////////////////////////////////////////////////////

    // loop over full merged collection
    const auto nPHO = reducedPhotons.size();
    for (auto iPHO = 0U; iPHO < nPHO; iPHO++)
    {
      // get tmps for 1st photon
      const auto & i_reducedPhoton = reducedPhotons[iPHO];
      const auto i_idx = i_reducedPhoton.idx;
      
      // skip if already marked for removal!
      if (i_reducedPhoton.toRemove) continue;

      // get remainder of tmps for 1st photon
      const auto i_isOOT = i_reducedPhoton.isOOT;
      const auto & i_photon = (i_isOOT ? (*ootPhotonsH)[i_idx] : (*gedPhotonsH)[i_idx]);

      // only check those that are lower in pT
      for (auto jPHO = iPHO+1; jPHO < nPHO; jPHO++)
      {
	// get tmps for 2nd photon
	auto & j_reducedPhoton = reducedPhotons[jPHO];
	const auto j_idx = j_reducedPhoton.idx;

	// skip if already marked for removal!
	if (j_reducedPhoton.toRemove) continue;

	// skip if from the same collection
	const auto j_isOOT = j_reducedPhoton.isOOT;
	if (i_isOOT == j_isOOT) continue;

	// get 2nd photon
	const auto & j_photon = (j_isOOT ? (*ootPhotonsH)[j_idx] : (*gedPhotonsH)[j_idx]);
	
	// if photons overlap, mark lower pT one for removal
	if (reco::deltaR(i_photon,j_photon) < dRmin)
	{
	  j_reducedPhoton.toRemove = true;
	} // end check over dR
      } // end loop over lower pT photons
    } // end loop over all photons
  }

  void MergePhotons(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH,
		    const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH, 
		    const std::vector<oot::ReducedPhoton> & reducedPhotons,
		    std::vector<pat::Photon> & photons, const float rho,
		    const float phpTmin, const std::string & phIDmin)
  {
    // loop over all photons, check if it passes cuts, then save it
    for (const auto & reducedPhoton : reducedPhotons)
    {
      // get tmps
      const auto idx = reducedPhoton.idx;
      const auto isOOT = reducedPhoton.isOOT;

      // get the photon
      const auto & photon = (isOOT ? (*ootPhotonsH)[idx] : (*gedPhotonsH)[idx]);

      // check to ensure it was not marked for removal!
      if (reducedPhoton.toRemove) continue;

      // cut on low pt
      if (oot::GetPhotonPt(photon) < phpTmin) continue;
      
      // store the GED and OOT VID
      std::vector<pat::Photon::IdPair> idpairs = {{Config::LooseGED,false}, {Config::MediumGED,false}, {Config::TightGED,false},
						  {Config::LooseOOT,false}, {Config::TightOOT,false}};

      oot::GetGEDPhoVID(photon,idpairs);      
      if (isOOT) oot::GetOOTPhoVID      (photon,idpairs);
      else       oot::GetOOTPhoVIDByHand(photon,idpairs,rho);

      // skip bad ID'ed photons
      if (phIDmin != Config::EmptyVID)
      {
	auto isGoodID = true;
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
	if (!isGoodID) continue;
      } // end check for bad ID

      // save it in the final vector!
      photons.emplace_back(photon);

      // and then modify it!
      auto & tmpphoton = photons.back();
      tmpphoton.setPhotonIDs(idpairs);
      tmpphoton.addUserData<bool>(Config::IsOOT,isOOT);
    } // end loop over photons
  }

  void CorrectMET(const edm::Handle<std::vector<pat::Photon> > & gedPhotonsH,
		  const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		  const std::vector<ReducedPhoton> & reducedPhotons,
		  pat::MET & t1pfMET)
  {
    ////////////////////////////////
    // Compute Correction Factors //
    ////////////////////////////////

    // inputs to MET correction
    auto t1pfMETpx    = t1pfMET.px();
    auto t1pfMETpy    = t1pfMET.py();
    auto t1pfMETsumEt = t1pfMET.sumEt();

    // loop over all photons, correct for DROPPED GED and STORED OOT
    for (const auto & reducedPhoton : reducedPhotons)
    {
      // get tmps
      const auto idx = reducedPhoton.idx;
      const auto isOOT = reducedPhoton.isOOT;
      const auto toRemove = reducedPhoton.toRemove;

      // get the photon
      const auto & photon = (isOOT ? (*ootPhotonsH)[idx] : (*gedPhotonsH)[idx]);

      // apply met corrections as needed
      if (isOOT && !toRemove) // add the OOT photons if being stored!
      {
	// get inputs
	const auto pt  = oot::GetPhotonPt(photon);
	const auto phi = photon.phi();

	// set MET
	t1pfMETpx -= pt*std::cos(phi);
	t1pfMETpy -= pt*std::sin(phi);
	
	t1pfMETsumEt += oot::GetPhotonEt(photon);
      }
      else if (!isOOT && toRemove) // subtract the GED photons if being dropped!
      {
	// get inputs
	const auto pt  = oot::GetPhotonPt(photon);
	const auto phi = photon.phi();

	// set MET
	t1pfMETpx += pt*std::cos(phi);
	t1pfMETpy += pt*std::sin(phi);

	t1pfMETsumEt -= oot::GetPhotonEt(photon);
      }
    } // end loop over all photons
    
    ////////////////////
    // Set MET Object //
    ////////////////////

    t1pfMET.addUserFloat(Config::OOTMETPt,Config::hypo(t1pfMETpx,t1pfMETpy));
    t1pfMET.addUserFloat(Config::OOTMETPhi,Config::phi(t1pfMETpx,t1pfMETpy));
    t1pfMET.addUserFloat(Config::OOTMETSumEt,t1pfMETsumEt);
  }

  ///////////////////////
  // Pruning Functions //
  ///////////////////////

  void PrunePhotons(std::vector<pat::Photon> & photons,
		    const EcalRecHitCollection * recHitsEB,
		    const EcalRecHitCollection * recHitsEE,
		    const float seedTimemin)
  {
    photons.erase(std::remove_if(photons.begin(),photons.end(),
				 [seedTimemin,&recHitsEB,&recHitsEE](const auto & photon)
				 {
				   const auto & phosc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
				   const auto & seedDetId = phosc->seed()->seed(); // get seed detid
				   const auto recHits = ((seedDetId.subdetId() == EcalSubdetector::EcalBarrel) ? recHitsEB : recHitsEE); // which recHits to use
				   const auto seedHit = recHits->find(seedDetId); // get the underlying rechit
				   const auto seedTime = ((seedHit != recHits->end()) ? seedHit->time() : -9999.f);
				   return (seedTime < seedTimemin);
				 }),photons.end());
  }

  void PruneJets(std::vector<pat::Jet> & jets, const std::vector<pat::Photon> & photons,
		 const int nPhosmax, const float dRmin)
  {
    // clean out w.r.t. to nMaxPhos
    const auto nPhos = std::min<int>(photons.size(),nPhosmax);

    // loop over at most the leading photons and clean...
    for (auto ipho = 0; ipho < nPhos; ipho++)
    {
      const auto & photon = photons[ipho];

      // apply loose selection on photon 
      const auto HoverE = photon.hadTowOverEm();
      const auto Sieie  = photon.full5x5_sigmaIetaIeta();
      const auto eta    = std::abs(photon.superCluster()->eta());
      
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

  ///////////////////////////////
  // Effective Areas Functions //
  ///////////////////////////////

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

  //////////////////////////
  // pT Scaling Functions //
  //////////////////////////

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

  //////////////////////////
  // Object VID Functions //
  //////////////////////////

  void GetGEDPhoVID(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs)
  {
    idpairs[2].second = photon.photonID(Config::GEDPhotonTightVID);
    idpairs[1].second = photon.photonID(Config::GEDPhotonMediumVID);
    idpairs[0].second = photon.photonID(Config::GEDPhotonLooseVID);
  }

  void GetGEDPhoVIDByHand(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs, const float rho)
  {
    // needed for cuts
    const auto eta = std::abs(photon.superCluster()->eta());
    const auto pt  = photon.pt();
    
    // cut variables
    const auto HoverE = photon.hadTowOverEm();
    const auto Sieie  = photon.full5x5_sigmaIetaIeta();
    // Isolations are currently wrong! need to recompute them apparently : 
    // https://twiki.cern.ch/twiki/bin/view/CMS/CutBasedPhotonIdentificationRun2#Selection_implementation_details
    // https://twiki.cern.ch/twiki/bin/view/CMS/EgammaPFBasedIsolationRun2#Recipe_for_accessing_PF_isol_AN1
    // https://github.com/cms-sw/cmssw/blob/master/RecoEgamma/PhotonIdentification/plugins/PhotonIDValueMapProducer.cc#L338-L395
    const auto ChgHadIso = std::max(photon.chargedHadronIso() - (rho * oot::GetChargedHadronEA(eta))                                         ,0.f);
    const auto NeuHadIso = std::max(photon.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(eta)) - (oot::GetNeutralHadronPtScale(eta,pt)),0.f);
    const auto PhoIso    = std::max(photon.photonIso()        - (rho * oot::GetGammaEA        (eta)) - (oot::GetGammaPtScale        (eta,pt)),0.f);
    
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

  void GetOOTPhoVID(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs)
  {
    idpairs[4].second = photon.photonID(Config::OOTPhotonTightVID);
    idpairs[3].second = photon.photonID(Config::OOTPhotonLooseVID);
  }

  void GetOOTPhoVIDByHand(const pat::Photon & photon, std::vector<pat::Photon::IdPair> & idpairs, const float rho)
  {
    // needed for cuts
    const auto eta = std::abs(photon.superCluster()->eta());
    const auto pt  = photon.pt();

    // cut variables
    const auto HoverE = photon.hadTowOverEm();
    const auto Sieie  = photon.full5x5_sigmaIetaIeta();
    const auto EcalPFClIso = std::max(photon.ecalPFClusterIso() - (rho * oot::GetEcalPFClEA(eta)) - (oot::GetEcalPFClPtScale(eta,pt)),0.f);
    const auto HcalPFClIso = std::max(photon.hcalPFClusterIso() - (rho * oot::GetHcalPFClEA(eta)) - (oot::GetHcalPFClPtScale(eta,pt)),0.f);
    const auto TrkIso      = std::max(photon.trackIso()         - (rho * oot::GetTrackEA   (eta)) - (oot::GetTrackPtScale   (eta,pt)),0.f);

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

  int GetPFJetID(const pat::Jet & jet) // https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2017
  {
    const auto eta = std::abs(jet.eta());
    
    const auto NHF  = jet.neutralHadronEnergyFraction();
    const auto NEMF = jet.neutralEmEnergyFraction();
    const auto CHF  = jet.chargedHadronEnergyFraction();
    const auto CEMF = jet.chargedEmEnergyFraction();
    const auto NHM  = jet.neutralMultiplicity();
    const auto CHM  = jet.chargedMultiplicity();
    const auto SHM  = jet.chargedMultiplicity()+jet.neutralMultiplicity();
    const auto MUF  = jet.muonEnergyFraction();
    
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
  // Storing Functions //
  ///////////////////////

  void SplitPhotons(std::vector<pat::Photon> & photons, const int nmax)
  {
    std::vector<int> gedphos;
    std::vector<int> ootphos;

    auto ipho = 0;
    for (const auto & photon : photons)
    {
      (!*(photon.userData<bool>(Config::IsOOT)) ? gedphos : ootphos).emplace_back(ipho++);
    }
    
    std::vector<pat::Photon> tmpphotons;

    const int ngedphos = gedphos.size();
    for (auto i = 0; i < nmax; i++)
    {
      if (ngedphos > i) tmpphotons.emplace_back(photons[gedphos[i]]);
    }

    const int nootphos = ootphos.size();
    for (auto i = 0; i < nmax; i++)
    {
      if (nootphos > i) tmpphotons.emplace_back(photons[ootphos[i]]);
    }

    photons.swap(tmpphotons);
  }

  void StoreOnlyPho(std::vector<pat::Photon> & photons, const int nmax, const bool isOOT)
  {
    std::vector<pat::Photon> tmpphotons;

    auto ipho = 0;
    for (const auto & photon : photons)
    {
      if (ipho >= nmax) break;
      ipho++;

      if (*(photon.userData<bool>(Config::IsOOT)) == isOOT) tmpphotons.emplace_back(photon);
    }

    photons.swap(tmpphotons);
  }

  /////////////////////
  // Debug Functions //
  /////////////////////

  void DumpPhoton(const pat::Photon & photon, const bool isOOT,
		  const EcalRecHitCollection * recHitsEB,
		  const EcalRecHitCollection * recHitsEE)
  {
    std::cout << (isOOT ? "OOT" : "GED") << " Photon Info -->"
	      << " pT: " << std::setprecision(3) << oot::GetPhotonPt(photon) 
	      << " phi: " << std::setprecision(3) << photon.phi() 
	      << " eta: " << std::setprecision(3) << photon.eta() << std::endl;
    
    const auto & phosc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
    const auto & seedDetId = phosc->seed()->seed(); // get seed detid
    const auto isEB = (seedDetId.subdetId() == EcalSubdetector::EcalBarrel);
    
    std::cout << " isEB: " << isEB << " seedId: " << seedDetId.rawId() << std::endl;
    
    const auto recHits = (isEB ? recHitsEB : recHitsEE); // get rechits
    const auto & hitsAndFractions = phosc->hitsAndFractions(); // get vector of detids
    
    // loop over all rec hits in SC
    for (auto hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr)
    {
      const auto & recHitDetId = hafitr->first; // get detid of crystal
      const auto & recHit = recHits->find(recHitDetId);
      const auto recHitId = recHitDetId.rawId();
      
      // standard check
      if (recHit != recHits->end())
      {
	if (recHit->energy() < 1) continue;

	std::cout << "   rhId: " << recHitId;
	if (isEB)
	{
	  const EBDetId recHitEB(recHitId);
	  std::cout << " ieta: " << recHitEB.ieta() << " iphi: " << recHitEB.iphi();
	}
	else
        {
	  const EEDetId recHitEE(recHitId);
	  std::cout << " ix: " << recHitEE.ix() << " iy: " << recHitEE.iy();
	}
	std::cout << " isOOT: " << recHit->checkFlag(EcalRecHit::kOutOfTime)
		  << " energy: " << std::setprecision(3) << recHit->energy() 
		  << " time: " << std::setprecision(3) << recHit->time() 
		  << std::endl;
      }
    } // end loop over hits and fractions
  }
};

