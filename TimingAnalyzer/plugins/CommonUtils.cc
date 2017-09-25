#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

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
      for (const auto & genpart : *genparticlesH) // loop over gen particles
      {
	if (nNeutoPhGr == 2) break;

	if (genpart.pdgId() == 1000022 && genpart.numberOfDaughters() == 2)
	{
	  if ((genpart.daughter(0)->pdgId() == 22 && genpart.daughter(1)->pdgId() == 1000039) ||
	      (genpart.daughter(1)->pdgId() == 22 && genpart.daughter(0)->pdgId() == 1000039)) 
	  {
	    nNeutoPhGr++;
	    neutralinos.emplace_back(genpart);
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
      for (const auto & genpart : *genparticlesH) // loop over gen particles
      {
	if (genpart.pdgId() == 4900111 && genpart.numberOfDaughters() == 2)
    	{
	  if (genpart.daughter(0)->pdgId() == 22 && genpart.daughter(1)->pdgId() == 22)
	  {
	    vPions.emplace_back(genpart);
	  } // end check over both gen photons	
	} // end check over vPions
      } // end loop over gen particles

      std::sort(vPions.begin(),vPions.end(),oot::sortByPt);
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
	  if (triggerName.find(triggerBitPair.first)) triggerBitPair.second = triggerResultsH->accept(itrig);
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
		   const edm::Handle<edm::ValueMap<bool> > & photonLooseIdMapH,
		   const edm::Handle<edm::ValueMap<bool> > & photonMediumIdMapH,
		   const edm::Handle<edm::ValueMap<bool> > & photonTightIdMapH,
		   std::vector<oot::Photon> & photons, const bool isOOT, const float phpTmin)
  {
    if (photonsH.isValid()) // standard handle check
    {
      const edm::ValueMap<bool> photonLooseIdMap  = *photonLooseIdMapH;
      const edm::ValueMap<bool> photonMediumIdMap = *photonMediumIdMapH;
      const edm::ValueMap<bool> photonTightIdMap  = *photonTightIdMapH;

      for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter)
      {
	if (phiter->pt() >= phpTmin) 
	{
	  // Initialize Id Pair
	  std::vector<pat::Photon::IdPair> idpair = {{"loose",false}, {"medium",false}, {"tight",false}};

	  // Get the VID of the photon
	  const edm::Ptr<pat::Photon> photonPtr(photonsH, phiter - photonsH->begin());
	  
	  // store VID in temp struct
	  // loose > medium > tight
	  if (photonLooseIdMap [photonPtr]) idpair[0].second = true;
	  if (photonMediumIdMap[photonPtr]) idpair[1].second = true;
	  if (photonTightIdMap [photonPtr]) idpair[2].second = true;

	  // set VID/isOOT of photon
	  photons.emplace_back(*phiter,isOOT);
	  photons.back().photon_nc().setPhotonIDs(idpair);
	} // pt check
      } // end loop over photons
    } // isValid
  }

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonLooseIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonMediumIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & photonTightIdMapH,
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   const edm::Handle<edm::ValueMap<bool> > & ootPhotonLooseIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & ootPhotonMediumIdMapH, 
		   const edm::Handle<edm::ValueMap<bool> > & ootPhotonTightIdMapH,
		   std::vector<oot::Photon> & photons, const float phpTmin, const std::string & phIDmin)
  {
    oot::PrepPhotons(photonsH,photonLooseIdMapH,photonMediumIdMapH,photonTightIdMapH,photons,false,phpTmin);
    oot::PrepPhotons(ootPhotonsH,ootPhotonLooseIdMapH,ootPhotonMediumIdMapH,ootPhotonTightIdMapH,photons,true,phpTmin);

    // final sorting
    if (phIDmin != "")
    {
      photons.erase(std::remove_if(photons.begin(),photons.end(),[&](const oot::Photon & photon){return !photon.photon().photonID(phIDmin);}),photons.end());
    }
    std::sort(photons.begin(),photons.end(),oot::sortByPt);
  }  

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
		   std::vector<oot::Photon> & photons, const float phpTmin)
  {
    if (photonsH.isValid()) // standard handle check
    {
      for (const auto & photon : *photonsH)
      {
	if (photon.pt() >= phpTmin) 
	{
	  photons.emplace_back(photon,false);
	}
      }
    }

    if (ootPhotonsH.isValid()) // standard handle check
    {
      for (const auto & photon : *ootPhotonsH)
      {
	if (photon.pt() >= phpTmin) 
	{
	  photons.emplace_back(photon,true);
	}
      }
    }

    std::sort(photons.begin(),photons.end(),oot::sortByPt);
  }

  void PrepRecHits(const EcalRecHitCollection * recHitsEB,
		   const EcalRecHitCollection * recHitsEE,
		   uiiumap & recHitMap, const float rhEmin)
  {
    int i = 0;
    for (const auto & recHit : *recHitsEB)
    {
      if (recHit.energy() > rhEmin) recHitMap[recHit.detid().rawId()] = i++;
    }
    for (const auto & recHit : *recHitsEE)
    {
      if (recHit.energy() > rhEmin) recHitMap[recHit.detid().rawId()] = i++;
    }
  }

  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, 
		std::vector<pat::Jet> & jets, const float jetpTmin, const int jetID)
  {
    if (jetsH.isValid()) // standard handle check
    {
      for (const auto& jet : *jetsH)
      {
	if (jet.pt() > jetpTmin) 
	{
	  if (oot::GetPFJetID(jet) >= jetID) jets.emplace_back(jet);
	}
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

  /////////////////////
  //                 //
  // Effective Areas //
  //                 //
  /////////////////////

  float GetChargedHadronEA(const float eta)
  {
    if      (eta <  1.0)                  return 0.0360;
    else if (eta >= 1.0   && eta < 1.479) return 0.0377;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0306;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0283;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0254;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.0217;
    else if (eta >= 2.4)                  return 0.0167;
    else                                  return 0.;
  }
  
  float GetNeutralHadronEA(const float eta) 
  {
    if      (eta <  1.0)                  return 0.0597;
    else if (eta >= 1.0   && eta < 1.479) return 0.0807;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0629;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.0197;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.0184;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.0284;
    else if (eta >= 2.4)                  return 0.0591;
    else                                  return 0.;
  }
  
  float GetGammaEA(const float eta) 
  {
    if      (eta <  1.0)                  return 0.1210;
    else if (eta >= 1.0   && eta < 1.479) return 0.1107;
    else if (eta >= 1.479 && eta < 2.0  ) return 0.0699;
    else if (eta >= 2.0   && eta < 2.2  ) return 0.1056;
    else if (eta >= 2.2   && eta < 2.3  ) return 0.1457;
    else if (eta >= 2.3   && eta < 2.4  ) return 0.1719;
    else if (eta >= 2.4)                  return 0.1998;
    else                                  return 0.;
  }

  ////////////////
  //            //
  // VID Checks //
  //            //
  ////////////////

  int PassHoE(const float eta, const float HoE)
  {  
    if (eta <= 1.479) // 1.4442
    {
      if      (HoE < 0.0269) return 3; 
      else if (HoE < 0.0396) return 2; 
      else if (HoE < 0.0597) return 1; 
      else                   return 0;
    }
    else if (eta > 1.479 && eta < 2.5)
    {
      if      (HoE < 0.0213) return 3; 
      else if (HoE < 0.0219) return 2; 
      else if (HoE < 0.0481) return 1; 
      else                   return 0;
    }
    else                     return 0;
  }

  int PassSieie(const float eta, const float Sieie)
  { 
    if (eta <= 1.479)
    {
      if      (Sieie < 0.00994) return 3; 
      else if (Sieie < 0.01022) return 2; 
      else if (Sieie < 0.01031) return 1; 
      else                      return 0;
    }
    else if (eta > 1.479 && eta < 2.5)
    {
      if      (Sieie < 0.03000) return 3; 
      else if (Sieie < 0.03001) return 2; 
      else if (Sieie < 0.03013) return 1; 
      else                      return 0;
    }
    else                        return 0;
  }
  
  int PassChgIso(const float eta, const float ChgIso)
  { 
    if (eta < 1.479)
    {
      if      (ChgIso < 0.202) return 3; 
      else if (ChgIso < 0.441) return 2; 
      else if (ChgIso < 1.295) return 1; 
      else                     return 0;
    }
    else if (eta > 1.479 && eta < 2.5)
    {
      if      (ChgIso < 0.034) return 3; 
      else if (ChgIso < 0.442) return 2; 
      else if (ChgIso < 1.011) return 1; 
      else                     return 0;
    }
    else                       return 0;
  }

  int PassNeuIso(const float eta, const float NeuIso, const float pt)
  { 
    if (eta < 1.479)
    {
      const float ptdep = 0.0148*pt+0.000017*pt*pt;
      if      (NeuIso < (0.264 +ptdep)) return 3; 
      else if (NeuIso < (2.725 +ptdep)) return 2; 
      else if (NeuIso < (10.910+ptdep)) return 1; 
      else                              return 0;
    }
    else if (eta > 1.479 && eta < 2.5)
    {
      const float ptdep = 0.0163*pt+0.000014*pt*pt;
      if      (NeuIso < (0.586 +ptdep)) return 3; 
      else if (NeuIso < (1.715 +ptdep)) return 2; 
      else if (NeuIso < (5.931 +ptdep)) return 1; 
      else                              return 0;
    }
    else                                return 0;
  }

  int PassPhIso(const float eta, const float PhIso, const float pt)
  { 
    if (eta < 1.479)
    {
      const float ptdep = 0.0047*pt;
      if      (PhIso < (2.362+ptdep)) return 3; 
      else if (PhIso < (2.571+ptdep)) return 2; 
      else if (PhIso < (3.630+ptdep)) return 1; 
      else                            return 0;
    }
    else if (eta > 1.479 && eta < 2.5)
    {
      const float ptdep = 0.0034*pt;
      if      (PhIso < (2.617+ptdep)) return 3; 
      else if (PhIso < (3.863+ptdep)) return 2; 
      else if (PhIso < (6.641+ptdep)) return 1; 
      else                            return 0;
    }
    else                              return 0;
  }

  //////////////
  //          //
  // PFJet ID //
  //          //
  //////////////
  int GetPFJetID(const pat::Jet & jet) //https://twiki.cern.ch/twiki/bin/view/CMS/JetID13TeVRun2016
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
    
    if (eta <= 2.4)
    {
      if      ((NHF < 0.90) && (NEMF < 0.90) && (CHF > 0) && (CEMF < 0.90) && (CHM > 0) && (SHM > 1) && (MUF < 0.8)) return 3;
      else if ((NHF < 0.90) && (NEMF < 0.90) && (CHF > 0) && (CEMF < 0.99) && (CHM > 0) && (SHM > 1))                return 2;
      else if ((NHF < 0.99) && (NEMF < 0.99) && (CHF > 0) && (CEMF < 0.99) && (CHM > 0) && (SHM > 1))                return 1;
      else                                                                                                           return 0; 
    }
    else if (eta > 2.4 && eta <= 2.7)
    {
      if      ((NHF < 0.90) && (NEMF < 0.90) && (SHM > 1) && (MUF < 0.8)) return 3;
      else if ((NHF < 0.90) && (NEMF < 0.90) && (SHM > 1))                return 2;
      else if ((NHF < 0.99) && (NEMF < 0.99) && (SHM > 1))                return 1;
      else                                                                return 0; 
    }
    else if (eta > 2.7 && eta <= 3.0)
    {
      if      ((NHF < 0.98) && (NEMF > 0.01) && (NHM > 2)) return 2;
      else if ((NHF < 0.98) && (NEMF > 0.01) && (NHM > 2)) return 1;
      else                                                 return 0; 
    }
    else 
    {
      if      ((NEMF < 0.90) && (NHM > 10)) return 2;
      else if ((NEMF < 0.90) && (NHM > 10)) return 1;
      else                                  return 0; 
    }
  }
};
