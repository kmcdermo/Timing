
      // Check for gen level match
      if (isMC) 
      {
	if (isGMSB)
	{
	  if      (iph == genph1match && iph != genph2match) phmatch[iph] = 1; // matched to photon from leading neutralino 
	  else if (iph == genph2match && iph != genph1match) phmatch[iph] = 2; // matched to photon from subleading neutralino
	  else if (iph == genph1match && iph == genph2match) phmatch[iph] = 3; // matched to both photons from both neutralinos (probably will never happen)
	  else                                               phmatch[iph] = 0; // no corresponding match
	} 
	if (isHVDS) 
	{
	  // reminder genHVph(1?2)match is a vector of size nvPions --> stores the index of reco photon that a gen photon from a dark pion is matched to
	  // since we can have many dipho pairs, store just if reco photon is matched to dark pion decay
	  // so meaning of 1,2, and 3 change

	  bool tmpph1match = false;
	  bool tmpph2match = false;
	  for (unsigned int jph = 0; jph < genHVph1match.size(); jph++)
	  {
	    if (genHVph1match[jph] == iph)
	    { 
	      tmpph1match = true;
	      break;
	    }
	  } // end loop over genHVph1matches
	  for (unsigned int jph = 0; jph < genHVph2match.size(); jph++)
	  {
	    if (genHVph2match[jph] == iph) 
	    {
	      tmpph2match = true;
	      break;
	    }
	  } // end loop over genHVph2matches

	  if      (tmpph1match && !tmpph2match) phmatch[iph] = 1; // 1 means "I am matched to some photon inside genHVph1match and not to any photon in genHVph2match"
	  else if (tmpph2match && !tmpph1match) phmatch[iph] = 2; // 2 means "I am matched to some photon inside genHVph2match and not to any photon in genHVph1match"
	  else if (tmpph1match &&  tmpph2match) phmatch[iph] = 3; // 3 means "I am matched to some photon inside genHVph1match and some photon in genHVph2match" (i.e. the generator photons merged into a single reco photon)
	  else                                  phmatch[iph] = 0; // no corresponding match
	} // end block over HVDS
	phIsGenMatched[iph] = oot::GenToObjectMatching(*phiter,genparticlesH,pTres,dRmin);
      }











  /////////////
  //         //
  // MC INFO //
  //         //
  /////////////
  if (isMC) 
  {
    ///////////////////////
    //                   //
    // Event weight info //
    //                   //
    ///////////////////////
    DisPho::InitializeGenEvtBranches();
    if (genevtInfoH.isValid()) {genwgt = genevtInfoH->weight();}

    /////////////////////
    //                 //
    // Gen pileup info //
    //                 //
    /////////////////////
    DisPho::InitializeGenPUBranches();
    if (pileupInfoH.isValid()) // standard check for pileup
    {
      for (std::vector<PileupSummaryInfo>::const_iterator puiter = pileupInfoH->begin(); puiter != pileupInfoH->end(); ++puiter) 
      {
	if (puiter->getBunchCrossing() == 0) 
	{
	  genpuobs  = puiter->getPU_NumInteractions();
	  genputrue = puiter->getTrueNumInteractions();
	} // end check over correct BX
      } // end loop over PU
    } // end check over pileup

    ///////////////////////
    //                   //
    // Gen particle info //
    //                   //
    ///////////////////////
    if (isGMSB) 
    {
      DisPho::InitializeGMSBBranches();
      if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
      {
	// Dump gen particle pdgIds
	if (dumpIds) DisPho::DumpGenIds(genparticlesH); 
	
	nNeutralino = 0;
	nNeutoPhGr  = 0;
	std::vector<reco::GenParticle> neutralinos;
	for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
        {
	  if (nNeutoPhGr == 2) break;

	  if (gpiter->pdgId() == 1000022 && gpiter->numberOfDaughters() == 2)
	  {
	    nNeutralino++;
	    
	    if ((gpiter->daughter(0)->pdgId() == 22 && gpiter->daughter(1)->pdgId() == 1000039) ||
		(gpiter->daughter(1)->pdgId() == 22 && gpiter->daughter(0)->pdgId() == 1000039)) 
	    {
	      nNeutoPhGr++;
	      neutralinos.push_back((*gpiter));
	    } // end conditional over matching daughter ids
	  } // end conditional over neutralino id
	} // end loop over gen particles

	std::sort(neutralinos.begin(),neutralinos.end(),oot::sortByPt);

	nNeutoPhGr = 0; // reuse
	for (std::vector<reco::GenParticle>::const_iterator gpiter = neutralinos.begin(); gpiter != neutralinos.end(); ++gpiter) // loop over neutralinos
	{
	  nNeutoPhGr++;
	  if (nNeutoPhGr == 1)
	  { 
	    // set neutralino parameters
	    genN1mass = gpiter->mass();
	    genN1E    = gpiter->energy();
	    genN1pt   = gpiter->pt();
	    genN1phi  = gpiter->phi();
	    genN1eta  = gpiter->eta();
	    
	    // neutralino production vertex
	    genN1prodvx = gpiter->vx();
	    genN1prodvy = gpiter->vy();
	    genN1prodvz = gpiter->vz();
	    
	    // neutralino decay vertex (same for both daughters unless really screwed up)
	    genN1decayvx = gpiter->daughter(0)->vx();
	    genN1decayvy = gpiter->daughter(0)->vy();
	    genN1decayvz = gpiter->daughter(0)->vz();
	    
	    // set photon daughter stuff
	    int phdaughter = (gpiter->daughter(0)->pdgId() == 22)?0:1;
	    
	    genph1E    = gpiter->daughter(phdaughter)->energy();
	    genph1pt   = gpiter->daughter(phdaughter)->pt();
	    genph1phi  = gpiter->daughter(phdaughter)->phi();
	    genph1eta  = gpiter->daughter(phdaughter)->eta();
	    
	    // check for a reco match!
	    if (photonsH.isValid()) // standard check
	    {
	      int   iph   = 0;
	      float mindR = dRmin;
	      for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	      {
		if (std::abs(phiter->pt()-genph1pt)/genph1pt < pTres)
		{
		  const float delR = deltaR(genph1phi,genph1eta,phiter->phi(),phiter->eta());
		  if (delR < mindR) 
		  {
		    mindR = delR;
		    genph1match = iph; 
		  } // end check over deltaR
		} // end check over pt resolution
		iph++;
	      } // end loop over reco photons
	    } // end check for reco match

	    // set gravitino daughter stuff
	    int grdaughter = (gpiter->daughter(0)->pdgId() == 1000039)?0:1;

	    gengr1mass = gpiter->daughter(grdaughter)->mass();
	    gengr1E    = gpiter->daughter(grdaughter)->energy();
	    gengr1pt   = gpiter->daughter(grdaughter)->pt();
	    gengr1phi  = gpiter->daughter(grdaughter)->phi();
	    gengr1eta  = gpiter->daughter(grdaughter)->eta();
	  } // end block over first matched neutralino -> photon + gravitino
       	  else if (nNeutoPhGr == 2)
	  {
	    // set neutralino parameters
	    genN2mass = gpiter->mass();
	    genN2E    = gpiter->energy();
	    genN2pt   = gpiter->pt();
	    genN2phi  = gpiter->phi();
	    genN2eta  = gpiter->eta();
	    
	    // neutralino production vertex
	    genN2prodvx = gpiter->vx();
	    genN2prodvy = gpiter->vy();
	    genN2prodvz = gpiter->vz();
	    
	    // neutralino decay vertex (same for both daughters unless really screwed up)
	    genN2decayvx = gpiter->daughter(0)->vx();
	    genN2decayvy = gpiter->daughter(0)->vy();
	    genN2decayvz = gpiter->daughter(0)->vz();

	    // set photon daughter stuff
	    int phdaughter = (gpiter->daughter(0)->pdgId() == 22)?0:1;
		
	    genph2E    = gpiter->daughter(phdaughter)->energy();
	    genph2pt   = gpiter->daughter(phdaughter)->pt();
	    genph2phi  = gpiter->daughter(phdaughter)->phi();
	    genph2eta  = gpiter->daughter(phdaughter)->eta();
	      
	    // check for a reco match!
	    if (photonsH.isValid()) // standard check
	    {
	      int   iph   = 0;
	      float mindR = dRmin;
	      for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	      {
		if (std::abs(phiter->pt()-genph2pt)/genph2pt < pTres)
		{
		  const float delR = deltaR(genph2phi,genph2eta,phiter->phi(),phiter->eta());
		  if (delR < mindR) 
		  {
		    mindR = delR;
		    genph2match = iph;
		  } // end check over deltaR
		} // end check over pt resolution
		iph++;
	      } // end loop over reco photons
	    } // end check for reco match
		
	    // set gravitino daughter stuff
	    int grdaughter = (gpiter->daughter(0)->pdgId() == 1000039)?0:1;

	    gengr2mass = gpiter->daughter(grdaughter)->mass();
	    gengr2E    = gpiter->daughter(grdaughter)->energy();
	    gengr2pt   = gpiter->daughter(grdaughter)->pt();
	    gengr2phi  = gpiter->daughter(grdaughter)->phi();
	    gengr2eta  = gpiter->daughter(grdaughter)->eta();
	  } // end block over second matched neutralino -> photon + gravitino
	} // end loop over good neutralinos
      } // end check for gen particles
    } // end check over isGMSB
 
    if (isHVDS) 
    {
      DisPho::ClearHVDSBranches();
      if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
      {
	if (dumpIds) DisPho::DumpGenIds(genparticlesH); 

	DisPho::InitializeHVDSBranches();
	std::vector<reco::GenParticle> vPions;
    	for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
        {
    	  if (gpiter->pdgId() == 4900111 && gpiter->numberOfDaughters() == 2)
    	  {
	    if (gpiter->daughter(0)->pdgId() == 22 && gpiter->daughter(1)->pdgId() == 22)
	    {
	      nvPions++;
	      vPions.push_back((*gpiter));
	    } // end check over both gen photons	
	  } // end check over vPions
	} // end loop over gen particles

	std::sort(vPions.begin(),vPions.end(),oot::sortByPt);

	for (std::vector<reco::GenParticle>::const_iterator gpiter = vPions.begin(); gpiter != vPions.end(); ++gpiter)
	{
	  // set neutralino parameters
	  genvPionmass.push_back(gpiter->mass());
	  genvPionE   .push_back(gpiter->energy());
	  genvPionpt  .push_back(gpiter->pt());
	  genvPionphi .push_back(gpiter->phi());
	  genvPioneta .push_back(gpiter->eta());
	  
	  // vPion production vertex
	  genvPionprodvx.push_back(gpiter->vx());
	  genvPionprodvy.push_back(gpiter->vy());
	  genvPionprodvz.push_back(gpiter->vz());
	  
	  // vPion decay vertex (same for both daughters unless really screwed up)
	  genvPiondecayvx.push_back(gpiter->daughter(0)->vx());
	  genvPiondecayvy.push_back(gpiter->daughter(0)->vy());
	  genvPiondecayvz.push_back(gpiter->daughter(0)->vz());
	  
	  int leading    = (gpiter->daughter(0)->pt()>gpiter->daughter(1)->pt())?0:1;
	  int subleading = (gpiter->daughter(0)->pt()>gpiter->daughter(1)->pt())?1:0;
	  
	  genHVph1E  .push_back(gpiter->daughter(leading)->energy());
	  genHVph1pt .push_back(gpiter->daughter(leading)->pt());
	  genHVph1phi.push_back(gpiter->daughter(leading)->phi());
	  genHVph1eta.push_back(gpiter->daughter(leading)->eta());
	  
	  genHVph2E  .push_back(gpiter->daughter(subleading)->energy());
	  genHVph2pt .push_back(gpiter->daughter(subleading)->pt());
	  genHVph2phi.push_back(gpiter->daughter(subleading)->phi());
	  genHVph2eta.push_back(gpiter->daughter(subleading)->eta());
	  
	  // check for a reco match!
	  if (photonsH.isValid()) // standard check
	  {
	    int   tmpph1 = -9999, tmpph2 = -9999;
	    int   iph = 0;
	    float mindR1 = dRmin, mindR2 = dRmin; // at least this much
	    for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	    {
	      const float tmppt  = phiter->pt();
	      const float tmpphi = phiter->phi();
	      const float tmpeta = phiter->eta();
		  
	      // check photon 1
	      if (std::abs(tmppt-genHVph1pt.back())/genHVph1pt.back() < pTres)
	      {
		const float delR = deltaR(genHVph1phi.back(),genHVph1eta.back(),tmpphi,tmpeta);
		if (delR < mindR1) 
		{
		  mindR1 = delR;
		  tmpph1 = iph;
		} // end check over deltaR
	      } // end check over pt resolution
	      
	      // check photon 2
	      if (std::abs(tmppt-genHVph2pt.back())/genHVph2pt.back() < pTres)
	      {
		const float delR = deltaR(genHVph2phi.back(),genHVph2eta.back(),tmpphi,tmpeta);
		if (delR < mindR2) 
		{
		  mindR2 = delR;
		  tmpph2 = iph;
		} // end check over deltaR
	      } // end check over pt resolution
	      
	      // now update iph
	      iph++;
	    } // end loop over reco photons
	    
	    // now save tmp photon iphs
	    genHVph1match.push_back(tmpph1);
	    genHVph2match.push_back(tmpph2);
	  } // end check for reco match
	} // end loop over good vPions
      } // end check over valid gen particles
    } // end check over isHVDS
  } // end block over isMC


void DisPho::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void DisPho::InitializeGenPUBranches()
{
  genpuobs = -9999; genputrue = -9999;
}

void DisPho::InitializeGMSBBranches()
{
  // Gen particle info
  nNeutralino = -9999;
  nNeutoPhGr  = -9999;

  genN1mass = -9999.f; genN1E = -9999.f; genN1pt = -9999.f; genN1phi = -9999.f; genN1eta = -9999.f;
  genN1prodvx = -9999.f; genN1prodvy = -9999.f; genN1prodvz = -9999.f;
  genN1decayvx = -9999.f; genN1decayvy = -9999.f; genN1decayvz = -9999.f;
  genph1E = -9999.f; genph1pt = -9999.f; genph1phi = -9999.f; genph1eta = -9999.f;
  genph1match = -9999;
  gengr1mass = -9999.f; gengr1E = -9999.f; gengr1pt = -9999.f; gengr1phi = -9999.f; gengr1eta = -9999.f;

  genN2mass = -9999.f; genN2E = -9999.f; genN2pt = -9999.f; genN2phi = -9999.f; genN2eta = -9999.f;
  genN2prodvx = -9999.f; genN2prodvy = -9999.f; genN2prodvz = -9999.f;
  genN2decayvx = -9999.f; genN2decayvy = -9999.f; genN2decayvz = -9999.f;
  genph2E = -9999.f; genph2pt = -9999.f; genph2phi = -9999.f; genph2eta = -9999.f;
  genph2match = -9999;
  gengr2mass = -9999.f; gengr2E = -9999.f; gengr2pt = -9999.f; gengr2phi = -9999.f; gengr2eta = -9999.f;
}

void DisPho::ClearHVDSBranches()
{
  nvPions = -9999;
  
  genvPionmass.clear();
  genvPionE   .clear();
  genvPionpt  .clear();
  genvPionphi .clear();
  genvPioneta .clear();
  
  genvPionprodvx.clear();
  genvPionprodvy.clear();
  genvPionprodvz.clear();
  
  genvPiondecayvx.clear();
  genvPiondecayvy.clear();
  genvPiondecayvz.clear();
  
  genHVph1E  .clear();
  genHVph1pt .clear();
  genHVph1phi.clear();
  genHVph1eta.clear();
  
  genHVph2E  .clear();
  genHVph2pt .clear();
  genHVph2phi.clear();
  genHVph2eta.clear();
  
  genHVph1match.clear();
  genHVph2match.clear();
}

void DisPho::InitializeHVDSBranches()
{
  nvPions = 0;
}

void DisPho::ClearGenJetBranches()
{
  ngenjets = -9999;

  genjetmatch.clear();

  genjetE.clear();
  genjetpt.clear();
  genjetphi.clear();
  genjeteta.clear();
}

void DisPho::InitializeGenJetBranches()
{
  genjetmatch.resize(ngenjets);

  genjetE.resize(ngenjets);
  genjetpt.resize(ngenjets);
  genjetphi.resize(ngenjets);
  genjeteta.resize(ngenjets);

  for (int igjet = 0; igjet < ngenjets; igjet++)
  {
    genjetmatch[igjet] = -9999;

    genjetE  [igjet] = -9999.f;
    genjetpt [igjet] = -9999.f;
    genjetphi[igjet] = -9999.f;
    genjeteta[igjet] = -9999.f;
  }
}



  if (isMC)
  {
    // Generator inf
    tree->Branch("genwgt"               , &genwgt               , "genwgt/F");
    tree->Branch("genpuobs"             , &genpuobs             , "genpuobs/I");
    tree->Branch("genputrue"            , &genputrue            , "genputrue/I");
  }

  if (isGMSB)
  {
    // Gen particle info
    tree->Branch("nNeutralino"          , &nNeutralino          , "nNeutralino/I");
    tree->Branch("nNeutoPhGr"           , &nNeutoPhGr           , "nNeutoPhGr/I");

    tree->Branch("genN1mass"            , &genN1mass            , "genN1mass/F");
    tree->Branch("genN1E"               , &genN1E               , "genN1E/F");
    tree->Branch("genN1pt"              , &genN1pt              , "genN1pt/F");
    tree->Branch("genN1phi"             , &genN1phi             , "genN1phi/F");
    tree->Branch("genN1eta"             , &genN1eta             , "genN1eta/F");
    tree->Branch("genN1prodvx"          , &genN1prodvx          , "genN1prodvx/F");
    tree->Branch("genN1prodvy"          , &genN1prodvy          , "genN1prodvy/F");
    tree->Branch("genN1prodvz"          , &genN1prodvz          , "genN1prodvz/F");
    tree->Branch("genN1decayvx"         , &genN1decayvx         , "genN1decayvx/F");
    tree->Branch("genN1decayvy"         , &genN1decayvy         , "genN1decayvy/F");
    tree->Branch("genN1decayvz"         , &genN1decayvz         , "genN1decayvz/F");
    tree->Branch("genph1E"              , &genph1E              , "genph1E/F");
    tree->Branch("genph1pt"             , &genph1pt             , "genph1pt/F");
    tree->Branch("genph1phi"            , &genph1phi            , "genph1phi/F");
    tree->Branch("genph1eta"            , &genph1eta            , "genph1eta/F");
    tree->Branch("genph1match"          , &genph1match          , "genph1match/I");
    tree->Branch("gengr1mass"           , &gengr1mass           , "gengr1mass/F");
    tree->Branch("gengr1E"              , &gengr1E              , "gengr1E/F");
    tree->Branch("gengr1pt"             , &gengr1pt             , "gengr1pt/F");
    tree->Branch("gengr1phi"            , &gengr1phi            , "gengr1phi/F");
    tree->Branch("gengr1eta"            , &gengr1eta            , "gengr1eta/F");

    tree->Branch("genN2mass"            , &genN2mass            , "genN2mass/F");
    tree->Branch("genN2E"               , &genN2E               , "genN2E/F");
    tree->Branch("genN2pt"              , &genN2pt              , "genN2pt/F");
    tree->Branch("genN2phi"             , &genN2phi             , "genN2phi/F");
    tree->Branch("genN2eta"             , &genN2eta             , "genN2eta/F");
    tree->Branch("genN2prodvx"          , &genN2prodvx          , "genN2prodvx/F");
    tree->Branch("genN2prodvy"          , &genN2prodvy          , "genN2prodvy/F");
    tree->Branch("genN2prodvz"          , &genN2prodvz          , "genN2prodvz/F");
    tree->Branch("genN2decayvx"         , &genN2decayvx         , "genN2decayvx/F");
    tree->Branch("genN2decayvy"         , &genN2decayvy         , "genN2decayvy/F");
    tree->Branch("genN2decayvz"         , &genN2decayvz         , "genN2decayvz/F");
    tree->Branch("genph2E"              , &genph2E              , "genph2E/F");
    tree->Branch("genph2pt"             , &genph2pt             , "genph2pt/F");
    tree->Branch("genph2phi"            , &genph2phi            , "genph2phi/F");
    tree->Branch("genph2eta"            , &genph2eta            , "genph2eta/F");
    tree->Branch("genph2match"          , &genph2match          , "genph2match/I");
    tree->Branch("gengr2mass"           , &gengr2mass           , "gengr2mass/F");
    tree->Branch("gengr2E"              , &gengr2E              , "gengr2E/F");
    tree->Branch("gengr2pt"             , &gengr2pt             , "gengr2pt/F");
    tree->Branch("gengr2phi"            , &gengr2phi            , "gengr2phi/F");
    tree->Branch("gengr2eta"            , &gengr2eta            , "gengr2eta/F");
  
    // GenJet Info
    tree->Branch("ngenjets"             , &ngenjets             , "ngenjets/I");
    tree->Branch("genjetmatch"          , &genjetmatch);
    tree->Branch("genjetE"              , &genjetE);
    tree->Branch("genjetpt"             , &genjetpt);
    tree->Branch("genjetphi"            , &genjetphi);
    tree->Branch("genjeteta"            , &genjeteta);
  }

  // HVDS info
  if (isHVDS)
  {
    tree->Branch("nvPions"              , &nvPions             , "nvPions/I");

    tree->Branch("genvPionprodvx"       , &genvPionprodvx);
    tree->Branch("genvPionprodvy"       , &genvPionprodvy);
    tree->Branch("genvPionprodvz"       , &genvPionprodvz);

    tree->Branch("genvPiondecayvx"      , &genvPiondecayvx);
    tree->Branch("genvPiondecayvy"      , &genvPiondecayvy);
    tree->Branch("genvPiondecayvz"      , &genvPiondecayvz);

    tree->Branch("genvPionmass"         , &genvPionmass);
    tree->Branch("genvPionE"            , &genvPionE);   
    tree->Branch("genvPionpt"           , &genvPionpt);
    tree->Branch("genvPionphi"          , &genvPionphi);
    tree->Branch("genvPioneta"          , &genvPioneta);

    tree->Branch("genHVph1E"            , &genHVph1E);   
    tree->Branch("genHVph1pt"           , &genHVph1pt);
    tree->Branch("genHVph1phi"          , &genHVph1phi);
    tree->Branch("genHVph1eta"          , &genHVph1eta);

    tree->Branch("genHVph2E"            , &genHVph2E);   
    tree->Branch("genHVph2pt"           , &genHVph2pt);
    tree->Branch("genHVph2phi"          , &genHVph2phi);
    tree->Branch("genHVph2eta"          , &genHVph2eta);

    tree->Branch("genHVph1match"        , &genHVph1match);
    tree->Branch("genHVph2match"        , &genHVph2match);
  }


  if (isGMSB || isHVDS) tree->Branch("phmatch", &phmatch);
  if (isMC) tree->Branch("phIsGenMatched", &phIsGenMatched);
