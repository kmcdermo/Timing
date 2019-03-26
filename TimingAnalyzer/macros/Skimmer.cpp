#include "Skimmer.hh"
#include "TROOT.h"

#include <iostream>

Skimmer::Skimmer(const TString & indir, const TString & outdir, const TString & filename, const TString & skimconfig)

  : fInDir(indir), fOutDir(outdir), fFileName(filename), fSkimConfig(skimconfig)
{
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

  ////////////////////////
  // Get all the inputs //
  ////////////////////////
  std::cout << "Setting up inputs for skim" << std::endl;

  // Set skim config options 
  Skimmer::SetupDefaults();
  Skimmer::SetupSkimConfig();

  // get detids if skim needs it
  if (fSkim == SkimType::DiXtal) Common::SetupDetIDs();

  // Get era info
  Common::SetupEras();

  // Get input file
  const TString infilename = Form("%s/%s", fInDir.Data(), fFileName.Data());
  fInFile = TFile::Open(infilename.Data());
  Common::CheckValidFile(fInFile,infilename);

  // Get input config tree
  const TString inconfigtreename = Form("%s/%s",Common::rootdir.Data(),Common::configtreename.Data());
  fInConfigTree = (TTree*)fInFile->Get(inconfigtreename.Data());
  Common::CheckValidTree(fInConfigTree,inconfigtreename,infilename);
  Skimmer::GetInConfig();

  // get sample weight from in config
  Skimmer::GetSampleWeight();

  // Get main input tree and initialize it
  const TString indisphotreename = Form("%s/%s",Common::rootdir.Data(),Common::disphotreename.Data());
  fInTree = (TTree*)fInFile->Get(indisphotreename.Data());
  Common::CheckValidTree(fInTree,indisphotreename,infilename);
  Skimmer::InitInTree();

  // Get the cut flow + event weight histogram --> set the wgtsum
  const TString inh_cutflowname = Form("%s/%s",Common::rootdir.Data(),Common::h_cutflowname.Data());
  fInCutFlow = (TH1F*)fInFile->Get(inh_cutflowname.Data());
  Common::CheckValidHist(fInCutFlow,inh_cutflowname,infilename);

  const TString inh_cutflow_wgtname = Form("%s/%s",Common::rootdir.Data(),Common::h_cutflow_wgtname.Data());
  fInCutFlowWgt = (TH1F*)fInFile->Get(inh_cutflow_wgtname.Data());
  Common::CheckValidHist(fInCutFlowWgt,inh_cutflow_wgtname,infilename);

  // Get PU weights input
  fPUWeights.clear();
  if (fIsMC)
  {
    fInPUWgtFile = TFile::Open(fPUWgtFileName);
    Common::CheckValidFile(fInPUWgtFile,fPUWgtFileName);

    const TString puhistname = Form("%s_%s",Common::puTrueHistName.Data(),Common::puwgtHistName.Data());
    fInPUWgtHist = (TH1F*)fInPUWgtFile->Get(puhistname.Data());
    Common::CheckValidHist(fInPUWgtHist,puhistname,fPUWgtFileName);

    Skimmer::GetPUWeights();
  }

  /////////////////////////
  // Set all the outputs //
  /////////////////////////
  std::cout << "Setting up output skim" << std::endl;

  // Make the output file, make trees, then init them
  fOutFile = TFile::Open(Form("%s/%s", fOutDir.Data(), fFileName.Data()),"recreate");
  fOutFile->cd();
  
  fOutConfigTree = new TTree(Common::configtreename.Data(),Common::configtreename.Data());
  fOutTree = new TTree(Common::disphotreename.Data(),Common::disphotreename.Data());

  // Init output info
  Skimmer::InitAndSetOutConfig();
  Skimmer::InitOutTree();
  Skimmer::InitOutCutFlowHists();
}

Skimmer::~Skimmer()
{
  fPUWeights.clear();
  if (fIsMC)
  {
    delete fInPUWgtHist;
    delete fInPUWgtFile;
  }

  //  delete fInCutFlowWgt;
  delete fInCutFlow;
  delete fInTree;
  delete fInConfigTree;
  delete fInFile;

  delete fOutCutFlowScl;
  delete fOutCutFlowWgt;
  delete fOutCutFlow;
  delete fOutTree;
  delete fOutConfigTree;
  delete fOutFile;
}

void Skimmer::EventLoop()
{
  // do loop over events, reading in branches as needed, skimming, filling output trees and hists
  const auto nEntries = fInTree->GetEntries();
  for (auto entry = 0U; entry < nEntries; entry++)
  {
    // dump status check
    if (entry%Common::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;

    // get event weight: no scaling by BR, xsec, lumi, etc.
    if (fIsMC) fInEvent.b_genwgt->GetEntry(entry);
    const auto wgt    = (fIsMC ? fInEvent.genwgt : 1.f);
    const auto evtwgt = fSampleWeight * wgt; // sample weight for data == 1

    // always fill to ensure no data was lost
    fOutCutFlow   ->Fill((cutLabels["PreSkim"]*1.f)-0.5f);
    fOutCutFlowWgt->Fill((cutLabels["PreSkim"]*1.f)-0.5f,wgt);
    fOutCutFlowScl->Fill((cutLabels["PreSkim"]*1.f)-0.5f,evtwgt);

    // perform skim: standard
    if (fSkim == SkimType::Standard) // do not apply skim selection on toy config
    {
      // leading photon skim section
      fInEvent.b_nphotons->GetEntry(entry);
      if (fInEvent.nphotons <= 0) continue;
      fOutCutFlow   ->Fill((cutLabels["nPhotons"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["nPhotons"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["nPhotons"]*1.f)-0.5f,evtwgt);
      
      fInPhos.front().b_isEB->GetEntry(entry);
      if (!fInPhos.front().isEB) continue;
      fOutCutFlow   ->Fill((cutLabels["ph0isEB"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["ph0isEB"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["ph0isEB"]*1.f)-0.5f,evtwgt);

      fInPhos.front().b_pt->GetEntry(entry);
      if (fInPhos.front().pt < 70.f) continue;
      fOutCutFlow   ->Fill((cutLabels["ph0pt70"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["ph0pt70"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["ph0pt70"]*1.f)-0.5f,evtwgt);

      // filter on MET Flags
      fInEvent.b_metPV->GetEntry(entry);
      fInEvent.b_metBeamHalo->GetEntry(entry);
      fInEvent.b_metHBHENoise->GetEntry(entry);
      fInEvent.b_metHBHEisoNoise->GetEntry(entry);
      fInEvent.b_metECALTP->GetEntry(entry);
      fInEvent.b_metPFMuon->GetEntry(entry);
      fInEvent.b_metPFChgHad->GetEntry(entry);
      // fInEvent.b_metECALCalib->GetEntry(entry);
      fInEvent.b_metECALBadCalib->GetEntry(entry);
      if (!fInEvent.metPV || !fInEvent.metBeamHalo || !fInEvent.metHBHENoise || !fInEvent.metHBHEisoNoise || 
	  // !fInEvent.metECALTP || !fInEvent.metPFMuon || !fInEvent.metPFChgHad || !fInEvent.metECALCalib) continue;
       	  !fInEvent.metECALTP || !fInEvent.metPFMuon || !fInEvent.metPFChgHad || !fInEvent.metECALBadCalib) continue;

      fInEvent.b_metEESC->GetEntry(entry);
      if (!fIsMC && !fInEvent.metEESC) continue;
      
      // fill cutflow for MET filters
      fOutCutFlow   ->Fill((cutLabels["METFlag"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["METFlag"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["METFlag"]*1.f)-0.5f,evtwgt);

      // fill photon list in standard fashion
      Skimmer::FillPhoListStandard();
    }
    else if (fSkim == SkimType::Zee)
    {
      // cut on HLT right away
      //      fInEvent.b_hltDiEle33MW->GetEntry(entry);
      
      //       if (!fInEvent.hltDiEle33MW) continue;
      fOutCutFlow   ->Fill((cutLabels["diEleHLT"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["diEleHLT"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["diEleHLT"]*1.f)-0.5f,evtwgt);

      // build list of "good electrons"
      std::vector<Int_t> good_phos;
      for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
      {
	auto & inpho = fInPhos[ipho];
	
	// 	inpho.b_pt->GetEntry(entry);
	// 	if (inpho.pt < 40.f) continue;

	inpho.b_hasPixSeed->GetEntry(entry);
	if (!inpho.hasPixSeed) continue;

	inpho.b_gedID->GetEntry(entry);
	if (inpho.gedID < 3) continue;

	inpho.b_isOOT->GetEntry(entry);
	if (inpho.isOOT) continue;

	good_phos.emplace_back(ipho);
      }
      
      // make sure have at least 1 good photon
      if (good_phos.size() < 1) continue;
      fOutCutFlow   ->Fill((cutLabels["goodPho1"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["goodPho1"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["goodPho1"]*1.f)-0.5f,evtwgt);

      // make sure have at least 2 good photons
      if (good_phos.size() < 2) continue;
      fOutCutFlow   ->Fill((cutLabels["goodPho2"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["goodPho2"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["goodPho2"]*1.f)-0.5f,evtwgt);

      // object for containing mass pairs
      std::vector<MassStruct> phopairs;

      // double loop over photons, make masses
      for (auto i = 0U; i < good_phos.size(); i++)
      {
	auto & pho1 = fInPhos[good_phos[i]];
	pho1.b_pt ->GetEntry(entry);
	pho1.b_eta->GetEntry(entry);
	pho1.b_phi->GetEntry(entry);
	pho1.b_E  ->GetEntry(entry);
	TLorentzVector pho1vec; pho1vec.SetPtEtaPhiE(pho1.pt, pho1.eta, pho1.phi, pho1.E);

	for (auto j = i+1; j < good_phos.size(); j++)
	{
	  auto & pho2 = fInPhos[good_phos[j]];
	  pho2.b_pt ->GetEntry(entry);
	  pho2.b_eta->GetEntry(entry);
	  pho2.b_phi->GetEntry(entry);
	  pho2.b_E  ->GetEntry(entry);
	  TLorentzVector pho2vec; pho2vec.SetPtEtaPhiE(pho2.pt, pho2.eta, pho2.phi, pho2.E);

	  // get invariant mass
	  pho1vec += pho2vec;
	  phopairs.emplace_back(good_phos[i],good_phos[j],pho1vec.M());
	}
      }

      // sort the mass structs
      std::sort(phopairs.begin(),phopairs.end(),
		[=](const auto & phopair1, const auto & phopair2)
		{
		  const auto diff1 = std::abs(phopair1.mass-Common::Zmass);
		  const auto diff2 = std::abs(phopair2.mass-Common::Zmass);
		  return diff1 < diff2;
		});
          
      // get best pair
      const auto & phopair = phopairs.front();
      
      // make sure within 30 GeV
      if ((phopair.mass < 60.f) || (phopair.mass > 150.f)) continue;
      fOutCutFlow   ->Fill((cutLabels["diPhoMZrange"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["diPhoMZrange"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["diPhoMZrange"]*1.f)-0.5f,evtwgt);

      // re-order photons based on pairs
      auto & pho1 = fInPhos[phopair.ipho1];
      auto & pho2 = fInPhos[phopair.ipho2];
      
      pho1.b_pt->GetEntry(entry);
      pho2.b_pt->GetEntry(entry);

      // now start to save them
      fPhoList.clear();
      fPhoList.emplace_back((pho1.pt > pho2.pt) ? phopair.ipho1 : phopair.ipho2);
      fPhoList.emplace_back((pho1.pt > pho2.pt) ? phopair.ipho2 : phopair.ipho1);
      for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
      {
	if      (phopair.ipho1 == ipho) continue;
	else if (phopair.ipho2 == ipho) continue;
	
	fPhoList.emplace_back(ipho);
      }
    } // end of ZeeSkim
    else if (fSkim == SkimType::DiXtal) // this is a hack selection, which mixes up seeds and photons --> do NOT use this for analysis
    {
      // get rechits
      fInRecHits.b_E->GetEntry(entry);
      fInRecHits.b_ID->GetEntry(entry);

      // loop over photons, getting pairs of rec hits that are most energetic and match!
      std::vector<DiXtalInfo> good_pairs;
      for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
      {	
	auto & inpho = fInPhos[ipho];

	// skip OOT for now
	inpho.b_isOOT->GetEntry(entry);
	if (inpho.isOOT) continue;
	
	inpho.b_smin->GetEntry(entry);
	inpho.b_smaj->GetEntry(entry);

	if (inpho.smin > 0.3) continue;
	if (inpho.smaj > 0.5) continue;
		
	// get pair of rechits that are good candidates : double loop, yo
	inpho.b_recHits->GetEntry(entry);

	const auto n = inpho.recHits->size();
	for (auto i = 0U; i < n; i++)
	{
	  Bool_t isGoodPair = false;

	  const auto rh_i = (*inpho.recHits)[i]; // position within event rec hits vector
	  const auto E_i  = (*fInRecHits.E) [rh_i];
	  const auto id_i = (*fInRecHits.ID)[rh_i];

	  for (auto j = i+1; j < n; j++)
	  {
	    const auto rh_j = (*inpho.recHits)[j]; // position within event rec hits vector
	    const auto E_j  = (*fInRecHits.E) [rh_j];
	    const auto id_j = (*fInRecHits.ID)[rh_j];

	    if (E_i > (1.2f * E_j)) break; // need to be within 20% of energy
	    if (Common::IsCrossNeighbor(id_i,id_j)) // neighboring crystals
	    {
	      good_pairs.emplace_back(rh_i,rh_j,ipho);
	      isGoodPair = true;
	      break;
	    } 
	  } // end inner double loop over rechits
	  if (isGoodPair) break;
	} // end outer double loop over rechits
      } // end loop over photons

      // skip if no pairs found
      if (good_pairs.size() == 0) continue;
      fOutCutFlow   ->Fill((cutLabels["goodDiXtal"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["goodDiXtal"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["goodDiXtal"]*1.f)-0.5f,evtwgt);

      // sort pairs by highest energy for E1
      std::sort(good_pairs.begin(),good_pairs.end(),
		[&](const auto & pair1, const auto & pair2)
		{
		  return ((*fInRecHits.E)[pair1.rh1] > (*fInRecHits.E)[pair2.rh2]);
		});

      // now do the unholiest of exercises... set seed ids of first and second photon to pair ids
      const auto & pair = good_pairs.front();
      fOutPhos[0].seed = pair.rh1;
      fOutPhos[1].seed = pair.rh2;

      // set pho list in standard fashion
      fPhoList.clear();
      fPhoList.emplace_back(pair.iph);
      fPhoList.emplace_back(pair.iph);
    }
    else if (fSkim == SkimType::AlwaysTrue) // no skim, just set photon list
    {
      Skimmer::FillPhoListStandard();
    }
    else if (fSkim == SkimType::AlwaysFalse) // just skip all events! testing purposes only...
    {
      continue;
    }
    else
    {
      std::cerr << "How did this happen?? Somehow SkimType Enum is not one that is specified... Exiting..."  << std::endl;
      exit(1);
    }

    // common skim params for MC
    if (fSkim != SkimType::AlwaysTrue)
    {
      // cut on crappy pu
      if (fIsMC)
      {
	fInEvent.b_genputrue->GetEntry(entry);
	if ((fInEvent.genputrue < 0) || (UInt_t(fInEvent.genputrue) >= fPUWeights.size())) continue;
      }

      // fill cutflow
      fOutCutFlow   ->Fill((cutLabels["badPU"]*1.f)-0.5f);
      fOutCutFlowWgt->Fill((cutLabels["badPU"]*1.f)-0.5f,wgt);
      fOutCutFlowScl->Fill((cutLabels["badPU"]*1.f)-0.5f,evtwgt);
    }
    
    // end of skim, now copy... dropping rechits
    if (fOutConfig.isGMSB) Skimmer::FillOutGMSBs(entry);
    if (fOutConfig.isHVDS) Skimmer::FillOutHVDSs(entry);
    if (fOutConfig.isToy)  Skimmer::FillOutToys(entry);
    Skimmer::FillOutEvent(entry,evtwgt);
    if (fSkim != SkimType::DiXtal) Skimmer::FillOutJets(entry);
    Skimmer::FillOutPhos(entry);
    if (fSkim != SkimType::DiXtal && fIsMC)
    {
      Skimmer::CorrectMET();
      Skimmer::ReorderJets();
    }

    // fill the tree
    fOutTree->Fill();
  } // end loop over events

  // write out the output!
  Common::Write(fOutFile,fOutCutFlow);
  Common::Write(fOutFile,fOutCutFlowWgt);
  Common::Write(fOutFile,fOutCutFlowScl);
  Common::Write(fOutFile,fOutConfigTree);
  Common::Write(fOutFile,fOutTree);
}

void Skimmer::FillOutGMSBs(const UInt_t entry)
{
  // get input branches
  for (auto igmsb = 0; igmsb < Common::nGMSBs; igmsb++)
  {
    auto & ingmsb = fInGMSBs[igmsb];
    ingmsb.b_genNmass->GetEntry(entry);
    ingmsb.b_genNE->GetEntry(entry);
    ingmsb.b_genNpt->GetEntry(entry);
    ingmsb.b_genNphi->GetEntry(entry);
    ingmsb.b_genNeta->GetEntry(entry);
    ingmsb.b_genNprodvx->GetEntry(entry);
    ingmsb.b_genNprodvy->GetEntry(entry);
    ingmsb.b_genNprodvz->GetEntry(entry);
    ingmsb.b_genNdecayvx->GetEntry(entry);
    ingmsb.b_genNdecayvy->GetEntry(entry);
    ingmsb.b_genNdecayvz->GetEntry(entry);
    ingmsb.b_genphE->GetEntry(entry);
    ingmsb.b_genphpt->GetEntry(entry);
    ingmsb.b_genphphi->GetEntry(entry);
    ingmsb.b_genpheta->GetEntry(entry);
    ingmsb.b_genphmatch->GetEntry(entry);
    ingmsb.b_gengrmass->GetEntry(entry);
    ingmsb.b_gengrE->GetEntry(entry);
    ingmsb.b_gengrpt->GetEntry(entry);
    ingmsb.b_gengrphi->GetEntry(entry);
    ingmsb.b_gengreta->GetEntry(entry);
  }

  // set output branches
  for (auto igmsb = 0; igmsb < Common::nGMSBs; igmsb++)
  {
    const auto & ingmsb = fInGMSBs[igmsb];
    auto & outgmsb = fOutGMSBs[igmsb];

    outgmsb.genNmass = ingmsb.genNmass;
    outgmsb.genNE = ingmsb.genNE;
    outgmsb.genNpt = ingmsb.genNpt;
    outgmsb.genNphi = ingmsb.genNphi;
    outgmsb.genNeta = ingmsb.genNeta;
    outgmsb.genNprodvx = ingmsb.genNprodvx;
    outgmsb.genNprodvy = ingmsb.genNprodvy;
    outgmsb.genNprodvz = ingmsb.genNprodvz;
    outgmsb.genNdecayvx = ingmsb.genNdecayvx;
    outgmsb.genNdecayvy = ingmsb.genNdecayvy;
    outgmsb.genNdecayvz = ingmsb.genNdecayvz;
    outgmsb.genphE = ingmsb.genphE;
    outgmsb.genphpt = ingmsb.genphpt;
    outgmsb.genphphi = ingmsb.genphphi;
    outgmsb.genpheta = ingmsb.genpheta;
    outgmsb.genphmatch = ingmsb.genphmatch;
    outgmsb.gengrmass = ingmsb.gengrmass;
    outgmsb.gengrE = ingmsb.gengrE;
    outgmsb.gengrpt = ingmsb.gengrpt;
    outgmsb.gengrphi = ingmsb.gengrphi;
    outgmsb.gengreta = ingmsb.gengreta;
  }
}

void Skimmer::FillOutHVDSs(const UInt_t entry)
{
  // get input branches
  for (auto ihvds = 0; ihvds < Common::nHVDSs; ihvds++)
  {
    auto & inhvds = fInHVDSs[ihvds];

    inhvds.b_genvPionmass->GetEntry(entry);
    inhvds.b_genvPionE->GetEntry(entry);
    inhvds.b_genvPionpt->GetEntry(entry);
    inhvds.b_genvPionphi->GetEntry(entry);
    inhvds.b_genvPioneta->GetEntry(entry);
    inhvds.b_genvPionprodvx->GetEntry(entry);
    inhvds.b_genvPionprodvy->GetEntry(entry);
    inhvds.b_genvPionprodvz->GetEntry(entry);
    inhvds.b_genvPiondecayvx->GetEntry(entry);
    inhvds.b_genvPiondecayvy->GetEntry(entry);
    inhvds.b_genvPiondecayvz->GetEntry(entry);
    inhvds.b_genHVph0E->GetEntry(entry);
    inhvds.b_genHVph0pt->GetEntry(entry);
    inhvds.b_genHVph0phi->GetEntry(entry);
    inhvds.b_genHVph0eta->GetEntry(entry);
    inhvds.b_genHVph0match->GetEntry(entry);
    inhvds.b_genHVph1E->GetEntry(entry);
    inhvds.b_genHVph1pt->GetEntry(entry);
    inhvds.b_genHVph1phi->GetEntry(entry);
    inhvds.b_genHVph1eta->GetEntry(entry);
    inhvds.b_genHVph1match->GetEntry(entry);
  }

  // set output branches
  for (auto ihvds = 0; ihvds < Common::nHVDSs; ihvds++)
  {
    const auto & inhvds = fInHVDSs[ihvds];
    auto & outhvds = fOutHVDSs[ihvds];

    outhvds.genvPionmass = inhvds.genvPionmass;
    outhvds.genvPionE = inhvds.genvPionE;
    outhvds.genvPionpt = inhvds.genvPionpt;
    outhvds.genvPionphi = inhvds.genvPionphi;
    outhvds.genvPioneta = inhvds.genvPioneta;
    outhvds.genvPionprodvx = inhvds.genvPionprodvx;
    outhvds.genvPionprodvy = inhvds.genvPionprodvy;
    outhvds.genvPionprodvz = inhvds.genvPionprodvz;
    outhvds.genvPiondecayvx = inhvds.genvPiondecayvx;
    outhvds.genvPiondecayvy = inhvds.genvPiondecayvy;
    outhvds.genvPiondecayvz = inhvds.genvPiondecayvz;
    outhvds.genHVph0E = inhvds.genHVph0E;
    outhvds.genHVph0pt = inhvds.genHVph0pt;
    outhvds.genHVph0phi = inhvds.genHVph0phi;
    outhvds.genHVph0eta = inhvds.genHVph0eta;
    outhvds.genHVph0match = inhvds.genHVph0match;
    outhvds.genHVph1E = inhvds.genHVph1E;
    outhvds.genHVph1pt = inhvds.genHVph1pt;
    outhvds.genHVph1phi = inhvds.genHVph1phi;
    outhvds.genHVph1eta = inhvds.genHVph1eta;
    outhvds.genHVph1match = inhvds.genHVph1match;
  }
}

void Skimmer::FillOutToys(const UInt_t entry)
{
  // get input branches
  for (auto itoy = 0; itoy < Common::nToys; itoy++)
  {
    auto & intoy = fInToys[itoy];

    intoy.b_genphE->GetEntry(entry);
    intoy.b_genphpt->GetEntry(entry);
    intoy.b_genphphi->GetEntry(entry);
    intoy.b_genpheta->GetEntry(entry);

    intoy.b_genphmatch->GetEntry(entry);
    intoy.b_genphmatch_ptres->GetEntry(entry);
    intoy.b_genphmatch_status->GetEntry(entry);
  }

  // set output branches
  for (auto itoy = 0; itoy < Common::nToys; itoy++)
  {
    const auto & intoy = fInToys[itoy];
    auto & outtoy = fOutToys[itoy];

    outtoy.genphE = intoy.genphE;
    outtoy.genphpt = intoy.genphpt;
    outtoy.genphphi = intoy.genphphi;
    outtoy.genpheta = intoy.genpheta;

    outtoy.genphmatch = intoy.genphmatch;
    outtoy.genphmatch_ptres = intoy.genphmatch_ptres;
    outtoy.genphmatch_status = intoy.genphmatch_status;
  }
}

void Skimmer::FillOutEvent(const UInt_t entry, const Float_t evtwgt)
{
  // get input branches
  fInEvent.b_run->GetEntry(entry);
  fInEvent.b_lumi->GetEntry(entry);
  fInEvent.b_event->GetEntry(entry);
  fInEvent.b_hltSignal->GetEntry(entry);
  fInEvent.b_hltRefPhoID->GetEntry(entry);
  fInEvent.b_hltRefDispID->GetEntry(entry);
  fInEvent.b_hltRefHT->GetEntry(entry);
  fInEvent.b_hltPho50->GetEntry(entry);
  fInEvent.b_hltPho200->GetEntry(entry);
  fInEvent.b_hltDiPho70->GetEntry(entry);
  fInEvent.b_hltDiPho3022M90->GetEntry(entry);
  fInEvent.b_hltDiPho30PV18PV->GetEntry(entry);
  fInEvent.b_hltEle32WPT->GetEntry(entry);
  fInEvent.b_hltDiEle33MW->GetEntry(entry);
  fInEvent.b_hltJet500->GetEntry(entry);
  fInEvent.b_nvtx->GetEntry(entry);
  fInEvent.b_vtxX->GetEntry(entry);
  fInEvent.b_vtxY->GetEntry(entry);
  fInEvent.b_vtxZ->GetEntry(entry);
  fInEvent.b_rho->GetEntry(entry);
  fInEvent.b_t1pfMETpt->GetEntry(entry);
  fInEvent.b_t1pfMETphi->GetEntry(entry);
  fInEvent.b_t1pfMETsumEt->GetEntry(entry);
  fInEvent.b_t1pfMETptUncorr->GetEntry(entry);
  fInEvent.b_t1pfMETphiUncorr->GetEntry(entry);
  fInEvent.b_t1pfMETsumEtUncorr->GetEntry(entry);
  fInEvent.b_njets->GetEntry(entry);
  fInEvent.b_nelLowL->GetEntry(entry);
  fInEvent.b_nelLowM->GetEntry(entry);
  fInEvent.b_nelLowT->GetEntry(entry);
  fInEvent.b_nelHighL->GetEntry(entry);
  fInEvent.b_nelHighM->GetEntry(entry);
  fInEvent.b_nelHighT->GetEntry(entry);
  fInEvent.b_nmuLowL->GetEntry(entry);
  fInEvent.b_nmuLowM->GetEntry(entry);
  fInEvent.b_nmuLowT->GetEntry(entry);
  fInEvent.b_nmuHighL->GetEntry(entry);
  fInEvent.b_nmuHighM->GetEntry(entry);
  fInEvent.b_nmuHighT->GetEntry(entry);
  fInEvent.b_nrechits->GetEntry(entry);
  fInEvent.b_nphotons->GetEntry(entry);

  // isMC only conditions
  if (fIsMC)
  {
    fInEvent.b_genwgt->GetEntry(entry);
    fInEvent.b_genx0->GetEntry(entry);
    fInEvent.b_geny0->GetEntry(entry);
    fInEvent.b_genz0->GetEntry(entry);
    fInEvent.b_gent0->GetEntry(entry);
    fInEvent.b_genpuobs->GetEntry(entry);
    fInEvent.b_genputrue->GetEntry(entry);
    fInEvent.b_genMETpt->GetEntry(entry);
    fInEvent.b_genMETphi->GetEntry(entry);

    if (fInConfig.isGMSB)
    {
      fInEvent.b_nNeutoPhGr->GetEntry(entry);
    }
    if (fInConfig.isHVDS)
    {
      fInEvent.b_nvPions->GetEntry(entry);
    }
    if (fInConfig.isToy)
    {
      fInEvent.b_nToyPhs->GetEntry(entry);
    }
  }

  // set output branches
  fOutEvent.run = fInEvent.run;
  fOutEvent.lumi = fInEvent.lumi;
  fOutEvent.event = fInEvent.event;
  fOutEvent.hltSignal = fInEvent.hltSignal;
  fOutEvent.hltRefPhoID = fInEvent.hltRefPhoID;
  fOutEvent.hltRefDispID = fInEvent.hltRefDispID;
  fOutEvent.hltRefHT = fInEvent.hltRefHT;
  fOutEvent.hltPho50 = fInEvent.hltPho50;
  fOutEvent.hltPho200 = fInEvent.hltPho200;
  fOutEvent.hltDiPho70 = fInEvent.hltDiPho70;
  fOutEvent.hltDiPho3022M90 = fInEvent.hltDiPho3022M90;
  fOutEvent.hltDiPho30PV18PV = fInEvent.hltDiPho30PV18PV;
  fOutEvent.hltEle32WPT = fInEvent.hltEle32WPT;
  fOutEvent.hltDiEle33MW = fInEvent.hltDiEle33MW;
  fOutEvent.hltJet500 = fInEvent.hltJet500;
  fOutEvent.nvtx = fInEvent.nvtx;
  fOutEvent.vtxX = fInEvent.vtxX;
  fOutEvent.vtxY = fInEvent.vtxY;
  fOutEvent.vtxZ = fInEvent.vtxZ;
  fOutEvent.rho = fInEvent.rho;
  fOutEvent.t1pfMETpt = fInEvent.t1pfMETpt;
  fOutEvent.t1pfMETphi = fInEvent.t1pfMETphi;
  fOutEvent.t1pfMETsumEt = fInEvent.t1pfMETsumEt;
  fOutEvent.t1pfMETptUncorr = fInEvent.t1pfMETptUncorr;
  fOutEvent.t1pfMETphiUncorr = fInEvent.t1pfMETphiUncorr;
  fOutEvent.t1pfMETsumEtUncorr = fInEvent.t1pfMETsumEtUncorr;
  fOutEvent.njets = fInEvent.njets;
  fOutEvent.nelLowL = fInEvent.nelLowL;
  fOutEvent.nelLowM = fInEvent.nelLowM;
  fOutEvent.nelLowT = fInEvent.nelLowT;
  fOutEvent.nelHighL = fInEvent.nelHighL;
  fOutEvent.nelHighM = fInEvent.nelHighM;
  fOutEvent.nelHighT = fInEvent.nelHighT;
  fOutEvent.nmuLowL = fInEvent.nmuLowL;
  fOutEvent.nmuLowM = fInEvent.nmuLowM;
  fOutEvent.nmuLowT = fInEvent.nmuLowT;
  fOutEvent.nmuHighL = fInEvent.nmuHighL;
  fOutEvent.nmuHighM = fInEvent.nmuHighM;
  fOutEvent.nmuHighT = fInEvent.nmuHighT;
  fOutEvent.nrechits = fInEvent.nrechits;
  fOutEvent.nphotons = fInEvent.nphotons;
  fOutEvent.evtwgt   = evtwgt;

  // isMC only branches
  if (fIsMC)
  {
    fOutEvent.genwgt = fInEvent.genwgt;
    fOutEvent.genx0 = fInEvent.genx0;
    fOutEvent.geny0 = fInEvent.geny0;
    fOutEvent.genz0 = fInEvent.genz0;
    fOutEvent.gent0 = fInEvent.gent0;
    fOutEvent.genpuobs = fInEvent.genpuobs;
    fOutEvent.genputrue = fInEvent.genputrue;
    fOutEvent.genMETpt = fInEvent.genMETpt;
    fOutEvent.genMETphi = fInEvent.genMETphi;

    if (fOutConfig.isGMSB)
    {
      fOutEvent.nNeutoPhGr = fInEvent.nNeutoPhGr;
    }
    if (fOutConfig.isHVDS)
    {
      fOutEvent.nvPions = fInEvent.nvPions;
    }
    if (fOutConfig.isToy)
    {
      fOutEvent.nToyPhs = fInEvent.nToyPhs;
    }

    // pileup weight!!
    fOutEvent.puwgt = fPUWeights[fInEvent.genputrue];
  }
}

void Skimmer::FillOutJets(const UInt_t entry)
{
  fInJets.b_E->GetEntry(entry);
  fInJets.b_pt->GetEntry(entry);
  fInJets.b_phi->GetEntry(entry);
  fInJets.b_eta->GetEntry(entry);
  fInJets.b_ID->GetEntry(entry);

  // fInJets.b_NHF->GetEntry(entry);
  // fInJets.b_NEMF->GetEntry(entry);
  // fInJets.b_CHF->GetEntry(entry);
  // fInJets.b_CEMF->GetEntry(entry);
  // fInJets.b_MUF->GetEntry(entry);
  // fInJets.b_NHM->GetEntry(entry);
  // fInJets.b_CHM->GetEntry(entry);

  const UInt_t nJets = fInJets.E->size();

  // resize outputs
  fOutJets.E_f.resize(nJets);
  fOutJets.pt_f.resize(nJets);
  fOutJets.phi_f.resize(nJets);
  fOutJets.eta_f.resize(nJets);
  fOutJets.ID_i.resize(nJets);

  // fOutJets.NHF_f.resize(nJets);
  // fOutJets.NEMF_f.resize(nJets);
  // fOutJets.CHF_f.resize(nJets);
  // fOutJets.CEMF_f.resize(nJets);
  // fOutJets.MUF_f.resize(nJets);
  // fOutJets.NHM_f.resize(nJets);
  // fOutJets.CHM_f.resize(nJets);

  // copy in: non-ideal (swap won't work, as we need the input values for MET corrections)
  for (auto ijet = 0U; ijet < nJets; ijet++)
  {
    fOutJets.E_f[ijet] = (*fInJets.E)[ijet];
    fOutJets.pt_f[ijet] = (*fInJets.pt)[ijet];
    fOutJets.phi_f[ijet] = (*fInJets.phi)[ijet];
    fOutJets.eta_f[ijet] = (*fInJets.eta)[ijet];
    fOutJets.ID_i[ijet] = (*fInJets.ID)[ijet];
    
    // fOutJets.NHF_f[ijet] = (*fInJets.NHF)[ijet];
    // fOutJets.NEMF_f[ijet] = (*fInJets.NEMF)[ijet];
    // fOutJets.CHF_f[ijet] = (*fInJets.CHF)[ijet];
    // fOutJets.CEMF_f[ijet] = (*fInJets.CEMF)[ijet];
    // fOutJets.MUF_f[ijet] = (*fInJets.MUF)[ijet];
    // fOutJets.NHM_f[ijet] = (*fInJets.NHM)[ijet];
    // fOutJets.CHM_f[ijet] = (*fInJets.CHM)[ijet];
  }

  // apply energy corrections, sort after MET correction!
  if (fIsMC)
  {
    // Read the JEC's and JER's
    if      (fJER == ECorr::Nominal) fInJets.b_smearSF    ->GetEntry(entry);
    else if (fJER == ECorr::Down)    fInJets.b_smearDownSF->GetEntry(entry);
    else if (fJER == ECorr::Up)      fInJets.b_smearUpSF  ->GetEntry(entry);

    if (fJEC == ECorr::Down || fJEC == ECorr::Up) fInJets.b_scaleRel->GetEntry(entry);
    
    // Apply JEC's and JER's
    for (auto ijet = 0U; ijet < nJets; ijet++)
    {
      // JER's Uncs --> Apply nominal smearing always!
      if (fJER == ECorr::Nominal)
      {
	fOutJets.E_f [ijet] *= (*fInJets.smearSF)[ijet];
	fOutJets.pt_f[ijet] *= (*fInJets.smearSF)[ijet];
      }
      else if (fJER == ECorr::Down)
      {
	fOutJets.E_f [ijet] *= (*fInJets.smearDownSF)[ijet];
	fOutJets.pt_f[ijet] *= (*fInJets.smearDownSF)[ijet];
      }
      else if (fJER == ECorr::Up)
      {
	fOutJets.E_f [ijet] *= (*fInJets.smearUpSF)[ijet];
	fOutJets.pt_f[ijet] *= (*fInJets.smearUpSF)[ijet];
      }
      
      // JEC's for Unc.
      if (fJEC == ECorr::Down)
      {
	fOutJets.E_f [ijet] *= (1.f - (*fInJets.scaleRel)[ijet]);
	fOutJets.pt_f[ijet] *= (1.f - (*fInJets.scaleRel)[ijet]);
      }
      else if (fJEC == ECorr::Up)
      {
	fOutJets.E_f [ijet] *= (1.f + (*fInJets.scaleRel)[ijet]);
	fOutJets.pt_f[ijet] *= (1.f + (*fInJets.scaleRel)[ijet]);
      }
    }
  }
}

void Skimmer::FillOutPhos(const UInt_t entry)
{  
  // get input photon branches
  for (auto ipho : fPhoList)
  {
    auto & inpho = fInPhos[ipho];
    
    inpho.b_E->GetEntry(entry);
    inpho.b_pt->GetEntry(entry);
    inpho.b_eta->GetEntry(entry);
    inpho.b_phi->GetEntry(entry);
    inpho.b_scE->GetEntry(entry);
    inpho.b_sceta->GetEntry(entry);
    inpho.b_scphi->GetEntry(entry);
    inpho.b_HoE->GetEntry(entry);
    inpho.b_r9->GetEntry(entry);
    inpho.b_ChgHadIso->GetEntry(entry);
    inpho.b_NeuHadIso->GetEntry(entry);
    inpho.b_PhoIso->GetEntry(entry);
    inpho.b_EcalPFClIso->GetEntry(entry);
    inpho.b_HcalPFClIso->GetEntry(entry);
    inpho.b_TrkIso->GetEntry(entry);
    // inpho.b_ChgHadIsoC->GetEntry(entry);
    // inpho.b_NeuHadIsoC->GetEntry(entry);
    // inpho.b_PhoIsoC->GetEntry(entry);
    // inpho.b_EcalPFClIsoC->GetEntry(entry);
    // inpho.b_HcalPFClIsoC->GetEntry(entry);
    // inpho.b_TrkIsoC->GetEntry(entry);
    inpho.b_sieie->GetEntry(entry);
    // inpho.b_sipip->GetEntry(entry);
    // inpho.b_sieip->GetEntry(entry);
    // inpho.b_e2x2->GetEntry(entry);
    // inpho.b_e3x3->GetEntry(entry);
    // inpho.b_e5x5->GetEntry(entry);
    inpho.b_smaj->GetEntry(entry);
    inpho.b_smin->GetEntry(entry);
    // inpho.b_alpha->GetEntry(entry);
    inpho.b_suisseX->GetEntry(entry);
    inpho.b_isOOT->GetEntry(entry);
    inpho.b_isEB->GetEntry(entry);
    inpho.b_isHLT->GetEntry(entry);
    inpho.b_isTrk->GetEntry(entry);
    inpho.b_passEleVeto->GetEntry(entry);
    inpho.b_hasPixSeed->GetEntry(entry);
    inpho.b_gedID->GetEntry(entry);
    inpho.b_ootID->GetEntry(entry);

    if (fInConfig.storeRecHits)
    {
      inpho.b_seed->GetEntry(entry);
      inpho.b_recHits->GetEntry(entry);
    }
    else
    {
      // inpho.b_seedX->GetEntry(entry);
      // inpho.b_seedY->GetEntry(entry);
      // inpho.b_seedZ->GetEntry(entry);
      inpho.b_seedE->GetEntry(entry);
      inpho.b_seedtime->GetEntry(entry);
      // inpho.b_seedtimeErr->GetEntry(entry);
      inpho.b_seedTOF->GetEntry(entry);
      inpho.b_seedID->GetEntry(entry);
      // inpho.b_seedisOOT->GetEntry(entry);
      inpho.b_seedisGS6->GetEntry(entry);
      inpho.b_seedisGS1->GetEntry(entry);
      inpho.b_seedped12->GetEntry(entry);
      inpho.b_seedped6->GetEntry(entry);
      inpho.b_seedped1->GetEntry(entry);
      inpho.b_seedpedrms12->GetEntry(entry);
      inpho.b_seedpedrms6->GetEntry(entry);
      inpho.b_seedpedrms1->GetEntry(entry);
    }
    
    if (fIsMC)
    {
      inpho.b_isGen->GetEntry(entry);
      if (fInConfig.isGMSB || fInConfig.isHVDS)
      {
	inpho.b_isSignal->GetEntry(entry);
      }

      if (fPhoSc == ECorr::Down || fPhoSc == ECorr::Up) inpho.b_scaleAbs->GetEntry(entry);
      if (fPhoSm == ECorr::Down || fPhoSm == ECorr::Up) inpho.b_smearAbs->GetEntry(entry);
    }
  }

  // get input recHits if needed
  if (fInConfig.storeRecHits)
  {
    // fInRecHits.b_X->GetEntry(entry);
    // fInRecHits.b_Y->GetEntry(entry);
    // fInRecHits.b_Z->GetEntry(entry);
    fInRecHits.b_E->GetEntry(entry);
    fInRecHits.b_time->GetEntry(entry);
    // fInRecHits.b_timeErr->GetEntry(entry);
    fInRecHits.b_TOF->GetEntry(entry);
    fInRecHits.b_ID->GetEntry(entry);
    // fInRecHits.b_isOOT->GetEntry(entry);
    fInRecHits.b_isGS6->GetEntry(entry);
    fInRecHits.b_isGS1->GetEntry(entry);
    fInRecHits.b_adcToGeV->GetEntry(entry);
    fInRecHits.b_ped12->GetEntry(entry);
    fInRecHits.b_ped6->GetEntry(entry);
    fInRecHits.b_ped1->GetEntry(entry);
    fInRecHits.b_pedrms12->GetEntry(entry);
    fInRecHits.b_pedrms6->GetEntry(entry);
    fInRecHits.b_pedrms1->GetEntry(entry);
  }

  // set output photon branches
  for (auto ipho = 0; ipho < fNOutPhos; ipho++) 
  {
    const auto & inpho = fInPhos[fPhoList[ipho]];
    auto & outpho = fOutPhos[ipho];

    outpho.E = inpho.E;
    outpho.pt = inpho.pt;
    outpho.eta = inpho.eta;
    outpho.phi = inpho.phi;
    outpho.scE = inpho.scE;
    outpho.sceta = inpho.sceta;
    outpho.scphi = inpho.scphi;
    outpho.HoE = inpho.HoE;
    outpho.r9 = inpho.r9;
    outpho.ChgHadIso = inpho.ChgHadIso;
    outpho.NeuHadIso = inpho.NeuHadIso;
    outpho.PhoIso = inpho.PhoIso;
    outpho.EcalPFClIso = inpho.EcalPFClIso;
    outpho.HcalPFClIso = inpho.HcalPFClIso;
    outpho.TrkIso = inpho.TrkIso;
    // outpho.ChgHadIsoC = inpho.ChgHadIsoC;
    // outpho.NeuHadIsoC = inpho.NeuHadIsoC;
    // outpho.PhoIsoC = inpho.PhoIsoC;
    // outpho.EcalPFClIsoC = inpho.EcalPFClIsoC;
    // outpho.HcalPFClIsoC = inpho.HcalPFClIsoC;
    // outpho.TrkIsoC = inpho.TrkIsoC;
    outpho.sieie = inpho.sieie;
    // outpho.sipip = inpho.sipip;
    // outpho.sieip = inpho.sieip;
    // outpho.e2x2 = inpho.e2x2;
    // outpho.e3x3 = inpho.e3x3;
    // outpho.e5x5 = inpho.e5x5;
    outpho.smaj = inpho.smaj;
    outpho.smin = inpho.smin;
    // outpho.alpha = inpho.alpha;
    outpho.suisseX = inpho.suisseX;
    outpho.isOOT = inpho.isOOT;
    outpho.isEB = inpho.isEB;
    outpho.isHLT = inpho.isHLT;
    outpho.isTrk = inpho.isTrk;
    outpho.passEleVeto = inpho.passEleVeto;
    outpho.hasPixSeed = inpho.hasPixSeed;
    outpho.gedID = inpho.gedID;
    outpho.ootID = inpho.ootID;

    if (fInConfig.storeRecHits)
    {
      // something unholy and utterly disgusting
      const auto seed = (fSkim != SkimType::DiXtal) ? inpho.seed : outpho.seed;

      // store seed info + derived types if seed exists
      if (seed >= 0)
      {
	// outpho.seedX = (*fInRecHits.X)[seed];
	// outpho.seedY = (*fInRecHits.Y)[seed];
	// outpho.seedZ = (*fInRecHits.Z)[seed];
	outpho.seedE = (*fInRecHits.E)[seed];

	outpho.seedtime    = (*fInRecHits.time)   [seed];
	// outpho.seedtimeErr = (*fInRecHits.timeErr)[seed];
	outpho.seedTOF     = (*fInRecHits.TOF)    [seed];

	// get trigger tower
	outpho.seedTT = Common::GetTriggerTower((*fInRecHits.ID)[seed]);
	
	// outpho.seedID    = (*fInRecHits.ID)[seed];
	// outpho.seedisOOT = (*fInRecHits.isOOT)[seed];

	outpho.seedisGS6 = (*fInRecHits.isGS6)[seed];
	outpho.seedisGS1 = (*fInRecHits.isGS1)[seed];
	outpho.seedadcToGeV = (*fInRecHits.adcToGeV)[seed];

	outpho.seedped12 = (*fInRecHits.ped12)[seed];
	outpho.seedped6  = (*fInRecHits.ped6) [seed];
	outpho.seedped1  = (*fInRecHits.ped1) [seed];
	outpho.seedpedrms12 = (*fInRecHits.pedrms12)[seed];
	outpho.seedpedrms6  = (*fInRecHits.pedrms6) [seed];
	outpho.seedpedrms1  = (*fInRecHits.pedrms1) [seed];

	// compute mean time, weighted time, AND
	// weighted time "TOF": really compute weighted time with TOF, then subtract this from weighted time to get "TOF"
	outpho.nrechits = 0;
	outpho.nrechitsLT120 = 0;
	outpho.meantime = 0.f;
	outpho.meantimeLT120 = 0.f;
	outpho.weightedtime = 0.f;
	outpho.weightedtimeLT120 = 0.f;
	outpho.weightedtimeTOF = 0.f;
	outpho.weightedtimeLT120TOF = 0.f;
	Float_t sumweights      = 0.f;
	Float_t sumweightsLT120 = 0.f;
	for (const auto irh : (*inpho.recHits))
	{
	  const Float_t rh_E = (*fInRecHits.E)   [irh];
	  const Float_t rh_T = (*fInRecHits.time)[irh];
	  const Float_t rh_A = (rh_E/(*fInRecHits.adcToGeV)[irh])/(*fInRecHits.pedrms12)[irh]; // amplitude normalized by pedestal noise
	  const Float_t inv_weight_2 = 1.f/(std::pow(Common::timefitN/rh_A,2)+2.f*std::pow(Common::timefitC,2));
	  const Float_t rh_TOF = (*fInRecHits.TOF)[irh];

	  outpho.nrechits++;
	  outpho.meantime += rh_T;

	  outpho.weightedtime    += rh_T*inv_weight_2;
	  outpho.weightedtimeTOF += (rh_T+rh_TOF)*inv_weight_2;
	  sumweights             += inv_weight_2;
	  
	  if (rh_E > 120.f) continue;

	  outpho.nrechitsLT120++;
	  outpho.meantimeLT120 += rh_T;

	  outpho.weightedtimeLT120    += rh_T*inv_weight_2;
	  outpho.weightedtimeLT120TOF += (rh_T+rh_TOF)*inv_weight_2;
	  sumweightsLT120             += inv_weight_2;
	}

	// set the derived types
	if   (outpho.nrechits > 0)
	{
	  outpho.meantime        /= outpho.nrechits;
	  outpho.weightedtime    /= sumweights;
	  outpho.weightedtimeTOF /= sumweights;

	  // hack for weighted time "TOF"
	  outpho.weightedtimeTOF -= outpho.weightedtime;
	}
	else 
	{
	  outpho.meantime        = -9999.f;
	  outpho.weightedtime    = -9999.f;
	  outpho.weightedtimeTOF = -9999.f*2.f;
	}

	if   (outpho.nrechitsLT120 > 0)
	{
	  outpho.meantimeLT120        /= outpho.nrechitsLT120;
	  outpho.weightedtimeLT120    /= sumweightsLT120;
	  outpho.weightedtimeLT120TOF /= sumweightsLT120;

	  // hack for weighted time "TOF"
	  outpho.weightedtimeLT120TOF -= outpho.weightedtimeLT120;
	}
	else
	{
	  outpho.meantimeLT120        = -9999.f;
	  outpho.weightedtimeLT120    = -9999.f;
	  outpho.weightedtimeLT120TOF = -9999.f*2.f;
	}
      } // end check that seed exists
      else
      {
	// outpho.seedX = -9999.f;
	// outpho.seedY = -9999.f;
	// outpho.seedZ = -9999.f;
	outpho.seedE = -9999.f;

	outpho.seedtime    = -9999.f;
	// outpho.seedtimeErr = -9999.f;
	outpho.seedTOF     = -9999.f;	

	// outpho.seedID    = 0;
	// outpho.seedisOOT = -1;
	outpho.seedTT    = -9999;

	outpho.seedisGS6 = -1;
	outpho.seedisGS1 = -1;
	outpho.seedadcToGeV = -9999.f;
	outpho.seedped12 = -9999.f;
	outpho.seedped6  = -9999.f;
	outpho.seedped1  = -9999.f;
	outpho.seedpedrms12 = -9999.f;
	outpho.seedpedrms6 = -9999.f;
	outpho.seedpedrms1 = -9999.f;

	outpho.nrechits             = -1;
	outpho.nrechitsLT120        = -1;
	outpho.meantime             = -9999.f;
	outpho.meantimeLT120        = -9999.f;
	outpho.weightedtime         = -9999.f;
	outpho.weightedtimeLT120    = -9999.f;
	outpho.weightedtimeTOF      = -9999.f*2.f;
	outpho.weightedtimeLT120TOF = -9999.f*2.f;
      }
    } // end check over store rechits
    else
    {
      // outpho.seedX = inpho.seedX;
      // outpho.seedY = inpho.seedY;
      // outpho.seedZ = inpho.seedZ;
      outpho.seedE = inpho.seedE;
      outpho.seedtime = inpho.seedtime;
      // outpho.seedtimeErr = inpho.seedtimeErr;
      outpho.seedTOF = inpho.seedTOF;
      // outpho.seedID = inpho.seedID;
      // outpho.seedisOOT = inpho.seedisOOT;
      outpho.seedTT = Common::GetTriggerTower(inpho.seedID);
      outpho.seedisGS6 = inpho.seedisGS1;
      outpho.seedisGS1 = inpho.seedisGS6;
      outpho.seedadcToGeV = inpho.seedadcToGeV;
      outpho.seedped12 = inpho.seedped12;
      outpho.seedped6 = inpho.seedped6;
      outpho.seedped1 = inpho.seedped1;
      outpho.seedpedrms12 = inpho.seedpedrms12;
      outpho.seedpedrms6 = inpho.seedpedrms6;
      outpho.seedpedrms1 = inpho.seedpedrms1;
    }
    
    if (fIsMC)
    {
      outpho.isGen = inpho.isGen;
      if (fOutConfig.isGMSB || fOutConfig.isHVDS)
      {
	outpho.isSignal = inpho.isSignal;
      }

      // apply photon scale uncs
      if (fPhoSc == ECorr::Down)
      {
	outpho.E  -= inpho.scaleAbs;
	outpho.pt -= inpho.scaleAbs;
      }
      else if (fPhoSc == ECorr::Up)
      {
	outpho.E  += inpho.scaleAbs;
	outpho.pt += inpho.scaleAbs;
      }
      
      // apply photon smear uncs
      if (fPhoSm == ECorr::Down)
      {
	outpho.E  -= inpho.smearAbs;
	outpho.pt -= inpho.smearAbs;
      }
      else if (fPhoSm == ECorr::Up)
      {
	outpho.E  += inpho.smearAbs;
	outpho.pt += inpho.smearAbs;
      }
    }
  }
}

void Skimmer::CorrectMET()
{
  // first, do jets
  if (fJER == ECorr::Down || fJER == ECorr::Up || fJER == ECorr::Nominal || fJEC == ECorr::Down || fJEC == ECorr::Up)
  {
    const UInt_t nJets = fOutJets.pt_f.size();
    for (auto ijet = 0U; ijet < nJets; ijet++)
    {
      const auto metpt  = fOutEvent.t1pfMETpt;
      const auto metphi = fOutEvent.t1pfMETphi;

      const auto ijetpt = (*fInJets.pt)[ijet];
      const auto ojetpt = fOutJets.pt_f[ijet];
      const auto jetphi = fOutJets.phi_f[ijet];

      const Float_t x = (metpt * std::cos(metphi)) + ((ijetpt - ojetpt) * std::cos(jetphi));
      const Float_t y = (metpt * std::sin(metphi)) + ((ijetpt - ojetpt) * std::sin(jetphi));

      fOutEvent.t1pfMETphi = Common::phi  (x,y);
      fOutEvent.t1pfMETpt  = Common::hypot(x,y);
    }
  }

  // then, do photons
  if (fPhoSc == ECorr::Down || fPhoSc == ECorr::Up || fPhoSm == ECorr::Down || fPhoSm == ECorr::Up)
  {
    for (auto ipho = 0; ipho < fNOutPhos; ipho++) 
    {
      const auto & inpho  = fInPhos[fPhoList[ipho]];
      const auto & outpho = fOutPhos[ipho];
      
      const auto metpt  = fOutEvent.t1pfMETpt;
      const auto metphi = fOutEvent.t1pfMETphi;
      
      const auto iphopt = inpho.pt;
      const auto ophopt = outpho.pt;
      const auto phophi = outpho.phi;
      
      const Float_t x = (metpt * std::cos(metphi)) + ((iphopt - ophopt) * std::cos(phophi));
      const Float_t y = (metpt * std::sin(metphi)) + ((iphopt - ophopt) * std::sin(phophi));
      
      fOutEvent.t1pfMETphi = Common::phi  (x,y);
      fOutEvent.t1pfMETpt  = Common::hypot(x,y);
    }
  }
}

void Skimmer::ReorderJets()
{
  /////////////////
  // sort by pt! //
  /////////////////
  
  // make list of indices, sort them by pt
  std::vector<UInt_t> ijets(fInJets.E->size());
  std::iota(ijets.begin(),ijets.end(),0);
  std::sort(ijets.begin(),ijets.end(),
	    [&](const auto ijet1, const auto ijet2)
	    {
	      return fOutJets.pt_f[ijet1]>fOutJets.pt_f[ijet2];
	    });
  
  // use reorder function to sort all the output
  Common::ReorderVector(fOutJets.E_f,ijets);
  Common::ReorderVector(fOutJets.pt_f,ijets);
  Common::ReorderVector(fOutJets.phi_f,ijets);
  Common::ReorderVector(fOutJets.eta_f,ijets);
  Common::ReorderVector(fOutJets.ID_i,ijets);
  
  // Common::ReorderVector(fOutJets.NHF_f,ijets);
  // Common::ReorderVector(fOutJets.NEMF_f,ijets);
  // Common::ReorderVector(fOutJets.CHF_f,ijets);
  // Common::ReorderVector(fOutJets.CEMF_f,ijets);
  // Common::ReorderVector(fOutJets.MUF_f,ijets);
  // Common::ReorderVector(fOutJets.NHM_f,ijets);
  // Common::ReorderVector(fOutJets.CHM_f,ijets);
}

void Skimmer::GetInConfig()
{
  // Get Input Config
  Skimmer::InitInConfigStrings();
  Skimmer::InitInConfigBranches();

  // read in first entry (will be the same for all entries in a given file)
  fInConfigTree->GetEntry(0);

  // set isMC
  fIsMC = (fInConfig.isGMSB || fInConfig.isHVDS || fInConfig.isBkgd || fInConfig.isToy || fInConfig.isADD);
}

void Skimmer::InitInConfigStrings()
{
  fInConfig.phIDmin = 0;
  fInConfig.phgoodIDmin = 0;
  fInConfig.inputPaths = 0;
  fInConfig.inputFilters = 0;
  fInConfig.inputFlags = 0;
}

void Skimmer::InitInConfigBranches()
{
  fInConfigTree->SetBranchAddress(fInConfig.s_blindSF.c_str(), &fInConfig.blindSF);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyBlindSF.c_str(), &fInConfig.applyBlindSF);
  fInConfigTree->SetBranchAddress(fInConfig.s_blindMET.c_str(), &fInConfig.blindMET);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyBlindMET.c_str(), &fInConfig.applyBlindMET);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetpTmin.c_str(), &fInConfig.jetpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetEtamax.c_str(), &fInConfig.jetEtamax);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetIDmin.c_str(), &fInConfig.jetIDmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_rhEmin.c_str(), &fInConfig.rhEmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_phpTmin.c_str(), &fInConfig.phpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_phIDmin.c_str(), &fInConfig.phIDmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_seedTimemin.c_str(), &fInConfig.seedTimemin);
  fInConfigTree->SetBranchAddress(fInConfig.s_nPhosmax.c_str(), &fInConfig.nPhosmax);
  fInConfigTree->SetBranchAddress(fInConfig.s_splitPho.c_str(), &fInConfig.splitPho);
  fInConfigTree->SetBranchAddress(fInConfig.s_onlyGED.c_str(), &fInConfig.onlyGED);
  fInConfigTree->SetBranchAddress(fInConfig.s_onlyOOT.c_str(), &fInConfig.onlyOOT);
  fInConfigTree->SetBranchAddress(fInConfig.s_ellowpTmin.c_str(), &fInConfig.ellowpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_elhighpTmin.c_str(), &fInConfig.elhighpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_mulowpTmin.c_str(), &fInConfig.mulowpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_muhighpTmin.c_str(), &fInConfig.muhighpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_storeRecHits.c_str(), &fInConfig.storeRecHits);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyTrigger.c_str(), &fInConfig.applyTrigger);
  fInConfigTree->SetBranchAddress(fInConfig.s_minHT.c_str(), &fInConfig.minHT);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyHT.c_str(), &fInConfig.applyHT);
  fInConfigTree->SetBranchAddress(fInConfig.s_phgoodpTmin.c_str(), &fInConfig.phgoodpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_phgoodIDmin.c_str(), &fInConfig.phgoodIDmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyPhGood.c_str(), &fInConfig.applyPhGood);
  fInConfigTree->SetBranchAddress(fInConfig.s_dRmin.c_str(), &fInConfig.dRmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_pTres.c_str(), &fInConfig.pTres);
  fInConfigTree->SetBranchAddress(fInConfig.s_genpTres.c_str(), &fInConfig.genpTres);
  fInConfigTree->SetBranchAddress(fInConfig.s_trackdRmin.c_str(), &fInConfig.trackdRmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_trackpTmin.c_str(), &fInConfig.trackpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_genjetdRmin.c_str(), &fInConfig.genjetdRmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_genjetpTfactor.c_str(), &fInConfig.genjetpTfactor);
  fInConfigTree->SetBranchAddress(fInConfig.s_leptondRmin.c_str(), &fInConfig.leptondRmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_smearjetEmin.c_str(), &fInConfig.smearjetEmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_inputPaths.c_str(), &fInConfig.inputPaths);
  fInConfigTree->SetBranchAddress(fInConfig.s_inputFilters.c_str(), &fInConfig.inputFilters);
  fInConfigTree->SetBranchAddress(fInConfig.s_inputFlags.c_str(), &fInConfig.inputFlags);
  fInConfigTree->SetBranchAddress(fInConfig.s_isGMSB.c_str(), &fInConfig.isGMSB);
  fInConfigTree->SetBranchAddress(fInConfig.s_isHVDS.c_str(), &fInConfig.isHVDS);
  fInConfigTree->SetBranchAddress(fInConfig.s_isBkgd.c_str(), &fInConfig.isBkgd);
  fInConfigTree->SetBranchAddress(fInConfig.s_isToy.c_str(), &fInConfig.isToy);
  fInConfigTree->SetBranchAddress(fInConfig.s_isADD.c_str(), &fInConfig.isADD);
  fInConfigTree->SetBranchAddress(fInConfig.s_xsec.c_str(), &fInConfig.xsec);
  fInConfigTree->SetBranchAddress(fInConfig.s_filterEff.c_str(), &fInConfig.filterEff);
  fInConfigTree->SetBranchAddress(fInConfig.s_BR.c_str(), &fInConfig.BR);
}

void Skimmer::InitInTree() 
{
  Skimmer::InitInStructs();
  Skimmer::InitInBranchVecs();
  Skimmer::InitInBranches();
}

void Skimmer::InitInStructs()
{
  if (fIsMC)
  {
    if (fInConfig.isGMSB)
    {
      fInGMSBs.clear(); 
      fInGMSBs.resize(Common::nGMSBs);
    }
    if (fInConfig.isHVDS)
    {
      fInHVDSs.clear(); 
      fInHVDSs.resize(Common::nHVDSs);
    }
    if (fInConfig.isToy)
    {
      fInToys.clear(); 
      fInToys.resize(Common::nToys);
    }
  }

  fInPhos.clear();
  fInPhos.resize(Common::nPhotons);
}

void Skimmer::InitInBranchVecs()
{
  fInJets.E = 0;
  fInJets.pt = 0;
  fInJets.phi = 0;
  fInJets.eta = 0;
  fInJets.ID = 0;

  // fInJets.NHF = 0;
  // fInJets.NEMF = 0;
  // fInJets.CHF = 0;
  // fInJets.CEMF = 0;
  // fInJets.MUF = 0;
  // fInJets.NHM = 0;
  // fInJets.CHM = 0;

  if (fIsMC)
  {
    if (fJEC == ECorr::Down || fJEC == ECorr::Up) fInJets.scaleRel = 0;
    if (fJER == ECorr::Nominal) fInJets.smearSF = 0;
    else if (fJER == ECorr::Down) fInJets.smearDownSF = 0;
    else if (fJER == ECorr::Up) fInJets.smearUpSF = 0;
    fInJets.isGen = 0;
  }

  if (fInConfig.storeRecHits) 
  {
    // fInRecHits.X = 0;
    // fInRecHits.Y = 0;
    // fInRecHits.Z = 0;
    fInRecHits.E = 0;
    fInRecHits.time = 0;
    // fInRecHits.timeErr = 0;
    fInRecHits.TOF = 0;
    fInRecHits.ID = 0;
    // fInRecHits.isOOT = 0;
    fInRecHits.isGS6 = 0;
    fInRecHits.isGS1 = 0;
    fInRecHits.adcToGeV = 0;
    fInRecHits.ped12 = 0;
    fInRecHits.ped6 = 0;
    fInRecHits.ped1 = 0;
    fInRecHits.pedrms12 = 0;
    fInRecHits.pedrms6 = 0;
    fInRecHits.pedrms1 = 0;

    for (auto ipho = 0; ipho < Common::nPhotons; ipho++) 
    {
      fInPhos[ipho].recHits = 0;
    }  
  }
}

void Skimmer::InitInBranches()
{
  if (fIsMC)
  {
    fInTree->SetBranchAddress(fInEvent.s_genwgt.c_str(), &fInEvent.genwgt, &fInEvent.b_genwgt);
    fInTree->SetBranchAddress(fInEvent.s_genx0.c_str(), &fInEvent.genx0, &fInEvent.b_genx0);
    fInTree->SetBranchAddress(fInEvent.s_geny0.c_str(), &fInEvent.geny0, &fInEvent.b_geny0);
    fInTree->SetBranchAddress(fInEvent.s_genz0.c_str(), &fInEvent.genz0, &fInEvent.b_genz0);
    fInTree->SetBranchAddress(fInEvent.s_gent0.c_str(), &fInEvent.gent0, &fInEvent.b_gent0);
    fInTree->SetBranchAddress(fInEvent.s_genpuobs.c_str(), &fInEvent.genpuobs, &fInEvent.b_genpuobs);
    fInTree->SetBranchAddress(fInEvent.s_genputrue.c_str(), &fInEvent.genputrue, &fInEvent.b_genputrue);
    fInTree->SetBranchAddress(fInEvent.s_genMETpt.c_str(), &fInEvent.genMETpt, &fInEvent.b_genMETpt);
    fInTree->SetBranchAddress(fInEvent.s_genMETphi.c_str(), &fInEvent.genMETphi, &fInEvent.b_genMETphi);

    if (fInConfig.isGMSB)
    {
      fInTree->SetBranchAddress(fInEvent.s_nNeutoPhGr.c_str(), &fInEvent.nNeutoPhGr, &fInEvent.b_nNeutoPhGr);
      for (auto igmsb = 0; igmsb < Common::nGMSBs; igmsb++) 
      {
	auto & gmsb = fInGMSBs[igmsb];
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNmass.c_str(),igmsb), &gmsb.genNmass, &gmsb.b_genNmass);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNE.c_str(),igmsb), &gmsb.genNE, &gmsb.b_genNE);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNpt.c_str(),igmsb), &gmsb.genNpt, &gmsb.b_genNpt);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNphi.c_str(),igmsb), &gmsb.genNphi, &gmsb.b_genNphi);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNeta.c_str(),igmsb), &gmsb.genNeta, &gmsb.b_genNeta);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNprodvx.c_str(),igmsb), &gmsb.genNprodvx, &gmsb.b_genNprodvx);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNprodvy.c_str(),igmsb), &gmsb.genNprodvy, &gmsb.b_genNprodvy);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNprodvz.c_str(),igmsb), &gmsb.genNprodvz, &gmsb.b_genNprodvz);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNdecayvx.c_str(),igmsb), &gmsb.genNdecayvx, &gmsb.b_genNdecayvx);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNdecayvy.c_str(),igmsb), &gmsb.genNdecayvy, &gmsb.b_genNdecayvy);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNdecayvz.c_str(),igmsb), &gmsb.genNdecayvz, &gmsb.b_genNdecayvz);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphE.c_str(),igmsb), &gmsb.genphE, &gmsb.b_genphE);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphpt.c_str(),igmsb), &gmsb.genphpt, &gmsb.b_genphpt);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphphi.c_str(),igmsb), &gmsb.genphphi, &gmsb.b_genphphi);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genpheta.c_str(),igmsb), &gmsb.genpheta, &gmsb.b_genpheta);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphmatch.c_str(),igmsb), &gmsb.genphmatch, &gmsb.b_genphmatch);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrmass.c_str(),igmsb), &gmsb.gengrmass, &gmsb.b_gengrmass);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrE.c_str(),igmsb), &gmsb.gengrE, &gmsb.b_gengrE);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrpt.c_str(),igmsb), &gmsb.gengrpt, &gmsb.b_gengrpt);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrphi.c_str(),igmsb), &gmsb.gengrphi, &gmsb.b_gengrphi);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengreta.c_str(),igmsb), &gmsb.gengreta, &gmsb.b_gengreta);
      } // end loop over neutralinos
    } // end block over gmsb

    if (fInConfig.isHVDS)
    {
      fInTree->SetBranchAddress(fInEvent.s_nvPions.c_str(), &fInEvent.nvPions, &fInEvent.b_nvPions);
      for (auto ihvds = 0; ihvds < Common::nHVDSs; ihvds++) 
      {
	auto & hvds = fInHVDSs[ihvds]; 
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionmass.c_str(),ihvds), &hvds.genvPionmass, &hvds.b_genvPionmass);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionE.c_str(),ihvds), &hvds.genvPionE, &hvds.b_genvPionE);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionpt.c_str(),ihvds), &hvds.genvPionpt, &hvds.b_genvPionpt);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionphi.c_str(),ihvds), &hvds.genvPionphi, &hvds.b_genvPionphi);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPioneta.c_str(),ihvds), &hvds.genvPioneta, &hvds.b_genvPioneta);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionprodvx.c_str(),ihvds), &hvds.genvPionprodvx, &hvds.b_genvPionprodvx);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionprodvy.c_str(),ihvds), &hvds.genvPionprodvy, &hvds.b_genvPionprodvy);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionprodvz.c_str(),ihvds), &hvds.genvPionprodvz, &hvds.b_genvPionprodvz);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPiondecayvx.c_str(),ihvds), &hvds.genvPiondecayvx, &hvds.b_genvPiondecayvx);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPiondecayvy.c_str(),ihvds), &hvds.genvPiondecayvy, &hvds.b_genvPiondecayvy);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPiondecayvz.c_str(),ihvds), &hvds.genvPiondecayvz, &hvds.b_genvPiondecayvz);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0E.c_str(),ihvds), &hvds.genHVph0E, &hvds.b_genHVph0E);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0pt.c_str(),ihvds), &hvds.genHVph0pt, &hvds.b_genHVph0pt);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0phi.c_str(),ihvds), &hvds.genHVph0phi, &hvds.b_genHVph0phi);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0eta.c_str(),ihvds), &hvds.genHVph0eta, &hvds.b_genHVph0eta);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0match.c_str(),ihvds), &hvds.genHVph0match, &hvds.b_genHVph0match);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1E.c_str(),ihvds), &hvds.genHVph1E, &hvds.b_genHVph1E);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1pt.c_str(),ihvds), &hvds.genHVph1pt, &hvds.b_genHVph1pt);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1phi.c_str(),ihvds), &hvds.genHVph1phi, &hvds.b_genHVph1phi);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1eta.c_str(),ihvds), &hvds.genHVph1eta, &hvds.b_genHVph1eta);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1match.c_str(),ihvds), &hvds.genHVph1match, &hvds.b_genHVph1match);
      } // end loop over nvpions 
    } // end block over hvds

    if (fInConfig.isToy)
    {
      fInTree->SetBranchAddress(fInEvent.s_nToyPhs.c_str(), &fInEvent.nToyPhs, &fInEvent.b_nToyPhs);
      for (auto itoy = 0; itoy < Common::nToys; itoy++) 
      {
	auto & toy = fInToys[itoy]; 
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphE.c_str(),itoy), &toy.genphE, &toy.b_genphE);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphpt.c_str(),itoy), &toy.genphpt, &toy.b_genphpt);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphphi.c_str(),itoy), &toy.genphphi, &toy.b_genphphi);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genpheta.c_str(),itoy), &toy.genpheta, &toy.b_genpheta);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphmatch.c_str(),itoy), &toy.genphmatch, &toy.b_genphmatch);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphmatch_ptres.c_str(),itoy), &toy.genphmatch_ptres, &toy.b_genphmatch_ptres);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphmatch_status.c_str(),itoy), &toy.genphmatch_status, &toy.b_genphmatch_status);
      } // end loop over toy phos
    } // end block over toyMC

  } // end block over isMC

  fInTree->SetBranchAddress(fInEvent.s_run.c_str(), &fInEvent.run, &fInEvent.b_run);
  fInTree->SetBranchAddress(fInEvent.s_lumi.c_str(), &fInEvent.lumi, &fInEvent.b_lumi);
  fInTree->SetBranchAddress(fInEvent.s_event.c_str(), &fInEvent.event, &fInEvent.b_event);

  fInTree->SetBranchAddress(fInEvent.s_hltSignal.c_str(), &fInEvent.hltSignal, &fInEvent.b_hltSignal);
  fInTree->SetBranchAddress(fInEvent.s_hltRefPhoID.c_str(), &fInEvent.hltRefPhoID, &fInEvent.b_hltRefPhoID);
  fInTree->SetBranchAddress(fInEvent.s_hltRefDispID.c_str(), &fInEvent.hltRefDispID, &fInEvent.b_hltRefDispID);
  fInTree->SetBranchAddress(fInEvent.s_hltRefHT.c_str(), &fInEvent.hltRefHT, &fInEvent.b_hltRefHT);
  fInTree->SetBranchAddress(fInEvent.s_hltPho50.c_str(), &fInEvent.hltPho50, &fInEvent.b_hltPho50);
  fInTree->SetBranchAddress(fInEvent.s_hltPho200.c_str(), &fInEvent.hltPho200, &fInEvent.b_hltPho200);
  fInTree->SetBranchAddress(fInEvent.s_hltDiPho70.c_str(), &fInEvent.hltDiPho70, &fInEvent.b_hltDiPho70);
  fInTree->SetBranchAddress(fInEvent.s_hltDiPho3022M90.c_str(), &fInEvent.hltDiPho3022M90, &fInEvent.b_hltDiPho3022M90);
  fInTree->SetBranchAddress(fInEvent.s_hltDiPho30PV18PV.c_str(), &fInEvent.hltDiPho30PV18PV, &fInEvent.b_hltDiPho30PV18PV);
  fInTree->SetBranchAddress(fInEvent.s_hltEle32WPT.c_str(), &fInEvent.hltEle32WPT, &fInEvent.b_hltEle32WPT);
  fInTree->SetBranchAddress(fInEvent.s_hltDiEle33MW.c_str(), &fInEvent.hltDiEle33MW, &fInEvent.b_hltDiEle33MW);
  fInTree->SetBranchAddress(fInEvent.s_hltJet500.c_str(), &fInEvent.hltJet500, &fInEvent.b_hltJet500);
  
  fInTree->SetBranchAddress(fInEvent.s_metPV.c_str(), &fInEvent.metPV, &fInEvent.b_metPV);
  fInTree->SetBranchAddress(fInEvent.s_metBeamHalo.c_str(), &fInEvent.metBeamHalo, &fInEvent.b_metBeamHalo);
  fInTree->SetBranchAddress(fInEvent.s_metHBHENoise.c_str(), &fInEvent.metHBHENoise, &fInEvent.b_metHBHENoise);
  fInTree->SetBranchAddress(fInEvent.s_metHBHEisoNoise.c_str(), &fInEvent.metHBHEisoNoise, &fInEvent.b_metHBHEisoNoise);
  fInTree->SetBranchAddress(fInEvent.s_metECALTP.c_str(), &fInEvent.metECALTP, &fInEvent.b_metECALTP);
  fInTree->SetBranchAddress(fInEvent.s_metPFMuon.c_str(), &fInEvent.metPFMuon, &fInEvent.b_metPFMuon);
  fInTree->SetBranchAddress(fInEvent.s_metPFChgHad.c_str(), &fInEvent.metPFChgHad, &fInEvent.b_metPFChgHad);
  fInTree->SetBranchAddress(fInEvent.s_metEESC.c_str(), &fInEvent.metEESC, &fInEvent.b_metEESC);
  // fInTree->SetBranchAddress(fInEvent.s_metECALCalib.c_str(), &fInEvent.metECALCalib, &fInEvent.b_metECALCalib);
  fInTree->SetBranchAddress(fInEvent.s_metECALBadCalib.c_str(), &fInEvent.metECALBadCalib, &fInEvent.b_metECALBadCalib);

  fInTree->SetBranchAddress(fInEvent.s_nvtx.c_str(), &fInEvent.nvtx, &fInEvent.b_nvtx);
  fInTree->SetBranchAddress(fInEvent.s_vtxX.c_str(), &fInEvent.vtxX, &fInEvent.b_vtxX);
  fInTree->SetBranchAddress(fInEvent.s_vtxY.c_str(), &fInEvent.vtxY, &fInEvent.b_vtxY);
  fInTree->SetBranchAddress(fInEvent.s_vtxZ.c_str(), &fInEvent.vtxZ, &fInEvent.b_vtxZ);
  fInTree->SetBranchAddress(fInEvent.s_rho.c_str(), &fInEvent.rho, &fInEvent.b_rho);

  fInTree->SetBranchAddress(fInEvent.s_t1pfMETpt.c_str(), &fInEvent.t1pfMETpt, &fInEvent.b_t1pfMETpt);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETphi.c_str(), &fInEvent.t1pfMETphi, &fInEvent.b_t1pfMETphi);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETsumEt.c_str(), &fInEvent.t1pfMETsumEt, &fInEvent.b_t1pfMETsumEt);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETptUncorr.c_str(), &fInEvent.t1pfMETptUncorr, &fInEvent.b_t1pfMETptUncorr);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETphiUncorr.c_str(), &fInEvent.t1pfMETphiUncorr, &fInEvent.b_t1pfMETphiUncorr);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETsumEtUncorr.c_str(), &fInEvent.t1pfMETsumEtUncorr, &fInEvent.b_t1pfMETsumEtUncorr);

  fInTree->SetBranchAddress(fInEvent.s_njets.c_str(), &fInEvent.njets, &fInEvent.b_njets);
  fInTree->SetBranchAddress(fInJets.s_E.c_str(), &fInJets.E, &fInJets.b_E);
  fInTree->SetBranchAddress(fInJets.s_pt.c_str(), &fInJets.pt, &fInJets.b_pt);
  fInTree->SetBranchAddress(fInJets.s_phi.c_str(), &fInJets.phi, &fInJets.b_phi);
  fInTree->SetBranchAddress(fInJets.s_eta.c_str(), &fInJets.eta, &fInJets.b_eta);
  fInTree->SetBranchAddress(fInJets.s_ID.c_str(), &fInJets.ID, &fInJets.b_ID);

  // fInTree->SetBranchAddress(fInJets.s_NHF.c_str(), &fInJets.NHF, &fInJets.b_NHF);
  // fInTree->SetBranchAddress(fInJets.s_NEMF.c_str(), &fInJets.NEMF, &fInJets.b_NEMF);
  // fInTree->SetBranchAddress(fInJets.s_CHF.c_str(), &fInJets.CHF, &fInJets.b_CHF);
  // fInTree->SetBranchAddress(fInJets.s_CEMF.c_str(), &fInJets.CEMF, &fInJets.b_CEMF);
  // fInTree->SetBranchAddress(fInJets.s_MUF.c_str(), &fInJets.MUF, &fInJets.b_MUF);
  // fInTree->SetBranchAddress(fInJets.s_NHM.c_str(), &fInJets.NHM, &fInJets.b_NHM);
  // fInTree->SetBranchAddress(fInJets.s_CHM.c_str(), &fInJets.CHM, &fInJets.b_CHM);

  if (fIsMC)
  {
    if (fJEC == ECorr::Down || fJEC == ECorr::Up) fInTree->SetBranchAddress(fInJets.s_scaleRel.c_str(), &fInJets.scaleRel, &fInJets.b_scaleRel);
    if (fJER == ECorr::Nominal) fInTree->SetBranchAddress(fInJets.s_smearSF.c_str(), &fInJets.smearSF, &fInJets.b_smearSF);
    else if (fJER == ECorr::Down) fInTree->SetBranchAddress(fInJets.s_smearDownSF.c_str(), &fInJets.smearDownSF, &fInJets.b_smearDownSF);
    else if (fJER == ECorr::Up) fInTree->SetBranchAddress(fInJets.s_smearUpSF.c_str(), &fInJets.smearUpSF, &fInJets.b_smearUpSF);
    fInTree->SetBranchAddress(fInJets.s_isGen.c_str(), &fInJets.isGen, &fInJets.b_isGen);
  }

  fInTree->SetBranchAddress(fInEvent.s_nelLowL.c_str(), &fInEvent.nelLowL, &fInEvent.b_nelLowL);
  fInTree->SetBranchAddress(fInEvent.s_nelLowM.c_str(), &fInEvent.nelLowM, &fInEvent.b_nelLowM);
  fInTree->SetBranchAddress(fInEvent.s_nelLowT.c_str(), &fInEvent.nelLowT, &fInEvent.b_nelLowT);
  fInTree->SetBranchAddress(fInEvent.s_nelHighL.c_str(), &fInEvent.nelHighL, &fInEvent.b_nelHighL);
  fInTree->SetBranchAddress(fInEvent.s_nelHighM.c_str(), &fInEvent.nelHighM, &fInEvent.b_nelHighM);
  fInTree->SetBranchAddress(fInEvent.s_nelHighT.c_str(), &fInEvent.nelHighT, &fInEvent.b_nelHighT);
  fInTree->SetBranchAddress(fInEvent.s_nmuLowL.c_str(), &fInEvent.nmuLowL, &fInEvent.b_nmuLowL);
  fInTree->SetBranchAddress(fInEvent.s_nmuLowM.c_str(), &fInEvent.nmuLowM, &fInEvent.b_nmuLowM);
  fInTree->SetBranchAddress(fInEvent.s_nmuLowT.c_str(), &fInEvent.nmuLowT, &fInEvent.b_nmuLowT);
  fInTree->SetBranchAddress(fInEvent.s_nmuHighL.c_str(), &fInEvent.nmuHighL, &fInEvent.b_nmuHighL);
  fInTree->SetBranchAddress(fInEvent.s_nmuHighM.c_str(), &fInEvent.nmuHighM, &fInEvent.b_nmuHighM);
  fInTree->SetBranchAddress(fInEvent.s_nmuHighT.c_str(), &fInEvent.nmuHighT, &fInEvent.b_nmuHighT);

  fInTree->SetBranchAddress(fInEvent.s_nrechits.c_str(), &fInEvent.nrechits, &fInEvent.b_nrechits);
  if (fInConfig.storeRecHits)
  {
    // fInTree->SetBranchAddress(fInRecHits.s_X.c_str(), &fInRecHits.X, &fInRecHits.b_X);
    // fInTree->SetBranchAddress(fInRecHits.s_Y.c_str(), &fInRecHits.Y, &fInRecHits.b_Y);
    // fInTree->SetBranchAddress(fInRecHits.s_Z.c_str(), &fInRecHits.Z, &fInRecHits.b_Z);
    fInTree->SetBranchAddress(fInRecHits.s_E.c_str(), &fInRecHits.E, &fInRecHits.b_E);
    fInTree->SetBranchAddress(fInRecHits.s_time.c_str(), &fInRecHits.time, &fInRecHits.b_time);
    // fInTree->SetBranchAddress(fInRecHits.s_timeErr.c_str(), &fInRecHits.timeErr, &fInRecHits.b_timeErr);
    fInTree->SetBranchAddress(fInRecHits.s_TOF.c_str(), &fInRecHits.TOF, &fInRecHits.b_TOF);
    fInTree->SetBranchAddress(fInRecHits.s_ID.c_str(), &fInRecHits.ID, &fInRecHits.b_ID);
    // fInTree->SetBranchAddress(fInRecHits.s_isOOT.c_str(), &fInRecHits.isOOT, &fInRecHits.b_isOOT);
    fInTree->SetBranchAddress(fInRecHits.s_isGS6.c_str(), &fInRecHits.isGS6, &fInRecHits.b_isGS6);
    fInTree->SetBranchAddress(fInRecHits.s_isGS1.c_str(), &fInRecHits.isGS1, &fInRecHits.b_isGS1);
    fInTree->SetBranchAddress(fInRecHits.s_adcToGeV.c_str(), &fInRecHits.adcToGeV, &fInRecHits.b_adcToGeV);
    fInTree->SetBranchAddress(fInRecHits.s_ped12.c_str(), &fInRecHits.ped12, &fInRecHits.b_ped12);
    fInTree->SetBranchAddress(fInRecHits.s_ped6.c_str(), &fInRecHits.ped6, &fInRecHits.b_ped6);
    fInTree->SetBranchAddress(fInRecHits.s_ped1.c_str(), &fInRecHits.ped1, &fInRecHits.b_ped1);
    fInTree->SetBranchAddress(fInRecHits.s_pedrms12.c_str(), &fInRecHits.pedrms12, &fInRecHits.b_pedrms12);
    fInTree->SetBranchAddress(fInRecHits.s_pedrms6.c_str(), &fInRecHits.pedrms6, &fInRecHits.b_pedrms6);
    fInTree->SetBranchAddress(fInRecHits.s_pedrms1.c_str(), &fInRecHits.pedrms1, &fInRecHits.b_pedrms1);
  }

  fInTree->SetBranchAddress(fInEvent.s_nphotons.c_str(), &fInEvent.nphotons, &fInEvent.b_nphotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++) 
  {
    auto & pho = fInPhos[ipho];
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_E.c_str(),ipho), &pho.E, &pho.b_E);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_pt.c_str(),ipho), &pho.pt, &pho.b_pt);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_eta.c_str(),ipho), &pho.eta, &pho.b_eta);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_phi.c_str(),ipho), &pho.phi, &pho.b_phi);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_scE.c_str(),ipho), &pho.scE, &pho.b_scE);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sceta.c_str(),ipho), &pho.sceta, &pho.b_sceta);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_scphi.c_str(),ipho), &pho.scphi, &pho.b_scphi);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_HoE.c_str(),ipho), &pho.HoE, &pho.b_HoE);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_r9.c_str(),ipho), &pho.r9, &pho.b_r9);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_ChgHadIso.c_str(),ipho), &pho.ChgHadIso, &pho.b_ChgHadIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_NeuHadIso.c_str(),ipho), &pho.NeuHadIso, &pho.b_NeuHadIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_PhoIso.c_str(),ipho), &pho.PhoIso, &pho.b_PhoIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_EcalPFClIso.c_str(),ipho), &pho.EcalPFClIso, &pho.b_EcalPFClIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_HcalPFClIso.c_str(),ipho), &pho.HcalPFClIso, &pho.b_HcalPFClIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_TrkIso.c_str(),ipho), &pho.TrkIso, &pho.b_TrkIso);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_ChgHadIsoC.c_str(),ipho), &pho.ChgHadIsoC, &pho.b_ChgHadIsoC);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_NeuHadIsoC.c_str(),ipho), &pho.NeuHadIsoC, &pho.b_NeuHadIsoC);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_PhoIsoC.c_str(),ipho), &pho.PhoIsoC, &pho.b_PhoIsoC);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_EcalPFClIsoC.c_str(),ipho), &pho.EcalPFClIsoC, &pho.b_EcalPFClIsoC);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_HcalPFClIsoC.c_str(),ipho), &pho.HcalPFClIsoC, &pho.b_HcalPFClIsoC);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_TrkIsoC.c_str(),ipho), &pho.TrkIsoC, &pho.b_TrkIsoC);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sieie.c_str(),ipho), &pho.sieie, &pho.b_sieie);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_sipip.c_str(),ipho), &pho.sipip, &pho.b_sipip);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_sieip.c_str(),ipho), &pho.sieip, &pho.b_sieip);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_e2x2.c_str(),ipho), &pho.e2x2, &pho.b_e2x2);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_e3x3.c_str(),ipho), &pho.e3x3, &pho.b_e3x3);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_e5x5.c_str(),ipho), &pho.e5x5, &pho.b_e5x5);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_smaj.c_str(),ipho), &pho.smaj, &pho.b_smaj);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_smin.c_str(),ipho), &pho.smin, &pho.b_smin);
    // fInTree->SetBranchAddress(Form("%s_%i",pho.s_alpha.c_str(),ipho), &pho.alpha, &pho.b_alpha);
    if (fInConfig.storeRecHits)
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seed.c_str(),ipho), &pho.seed, &pho.b_seed);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_recHits.c_str(),ipho), &pho.recHits, &pho.b_recHits);
    }
    else
    {
      // fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedX.c_str(),ipho), &pho.seedX, &pho.b_seedX);
      // fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedY.c_str(),ipho), &pho.seedY, &pho.b_seedY);
      // fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedZ.c_str(),ipho), &pho.seedZ, &pho.b_seedZ);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedE.c_str(),ipho), &pho.seedE, &pho.b_seedE);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedtime.c_str(),ipho), &pho.seedtime, &pho.b_seedtime);
      // fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedtimeErr.c_str(),ipho), &pho.seedtimeErr, &pho.b_seedtimeErr);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedTOF.c_str(),ipho), &pho.seedTOF, &pho.b_seedTOF);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedID.c_str(),ipho), &pho.seedID, &pho.b_seedID);
      // fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedisOOT.c_str(),ipho), &pho.seedisOOT, &pho.b_seedisOOT);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedisGS6.c_str(),ipho), &pho.seedisGS6, &pho.b_seedisGS6);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedisGS1.c_str(),ipho), &pho.seedisGS1, &pho.b_seedisGS1);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedadcToGeV.c_str(),ipho), &pho.seedadcToGeV, &pho.b_seedadcToGeV);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedped12.c_str(),ipho), &pho.seedped12, &pho.b_seedped12);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedped6.c_str(),ipho), &pho.seedped6, &pho.b_seedped6);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedped1.c_str(),ipho), &pho.seedped1, &pho.b_seedped1);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedpedrms12.c_str(),ipho), &pho.seedpedrms12, &pho.b_seedpedrms12);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedpedrms6.c_str(),ipho), &pho.seedpedrms6, &pho.b_seedpedrms6);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedpedrms1.c_str(),ipho), &pho.seedpedrms1, &pho.b_seedpedrms1);
    }
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_suisseX.c_str(),ipho), &pho.suisseX, &pho.b_suisseX);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isOOT.c_str(),ipho), &pho.isOOT, &pho.b_isOOT);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isEB.c_str(),ipho), &pho.isEB, &pho.b_isEB);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isHLT.c_str(),ipho), &pho.isHLT, &pho.b_isHLT);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isTrk.c_str(),ipho), &pho.isTrk, &pho.b_isTrk);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_passEleVeto.c_str(),ipho), &pho.passEleVeto, &pho.b_passEleVeto);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_hasPixSeed.c_str(),ipho), &pho.hasPixSeed, &pho.b_hasPixSeed);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_gedID.c_str(),ipho), &pho.gedID, &pho.b_gedID);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_ootID.c_str(),ipho), &pho.ootID, &pho.b_ootID);
    
    if (fIsMC)
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_isGen.c_str(),ipho), &pho.isGen, &pho.b_isGen);
      if (fInConfig.isGMSB || fInConfig.isHVDS)
      {
	fInTree->SetBranchAddress(Form("%s_%i",pho.s_isSignal.c_str(),ipho), &pho.isSignal, &pho.b_isSignal);
      }

      if (fPhoSc == ECorr::Down || fPhoSc == ECorr::Up) fInTree->SetBranchAddress(Form("%s_%i",pho.s_scaleAbs.c_str(),ipho), &pho.scaleAbs, &pho.b_scaleAbs);
      if (fPhoSm == ECorr::Down || fPhoSm == ECorr::Up) fInTree->SetBranchAddress(Form("%s_%i",pho.s_smearAbs.c_str(),ipho), &pho.smearAbs, &pho.b_smearAbs);
    }
  }
}

void Skimmer::InitAndSetOutConfig()
{
  // Make the branches
  fOutConfigTree->Branch(fOutConfig.s_blindSF.c_str(), &fOutConfig.blindSF);
  fOutConfigTree->Branch(fOutConfig.s_applyBlindSF.c_str(), &fOutConfig.applyBlindSF);
  fOutConfigTree->Branch(fOutConfig.s_blindMET.c_str(), &fOutConfig.blindMET);
  fOutConfigTree->Branch(fOutConfig.s_applyBlindMET.c_str(), &fOutConfig.applyBlindMET);
  fOutConfigTree->Branch(fOutConfig.s_jetpTmin.c_str(), &fOutConfig.jetpTmin);
  fOutConfigTree->Branch(fOutConfig.s_jetEtamax.c_str(), &fOutConfig.jetEtamax);
  fOutConfigTree->Branch(fOutConfig.s_jetIDmin.c_str(), &fOutConfig.jetIDmin);
  fOutConfigTree->Branch(fOutConfig.s_rhEmin.c_str(), &fOutConfig.rhEmin);
  fOutConfigTree->Branch(fOutConfig.s_phpTmin.c_str(), &fOutConfig.phpTmin);
  fOutConfigTree->Branch(fOutConfig.s_phIDmin.c_str(), &fOutConfig.phIDmin_s);
  fOutConfigTree->Branch(fOutConfig.s_seedTimemin.c_str(), &fOutConfig.seedTimemin);
  fOutConfigTree->Branch(fOutConfig.s_nPhosmax.c_str(), &fOutConfig.nPhosmax);
  fOutConfigTree->Branch(fOutConfig.s_splitPho.c_str(), &fOutConfig.splitPho);
  fOutConfigTree->Branch(fOutConfig.s_onlyGED.c_str(), &fOutConfig.onlyGED);
  fOutConfigTree->Branch(fOutConfig.s_onlyOOT.c_str(), &fOutConfig.onlyOOT);
  fOutConfigTree->Branch(fOutConfig.s_ellowpTmin.c_str(), &fOutConfig.ellowpTmin);
  fOutConfigTree->Branch(fOutConfig.s_elhighpTmin.c_str(), &fOutConfig.elhighpTmin);
  fOutConfigTree->Branch(fOutConfig.s_mulowpTmin.c_str(), &fOutConfig.mulowpTmin);
  fOutConfigTree->Branch(fOutConfig.s_muhighpTmin.c_str(), &fOutConfig.muhighpTmin);
  fOutConfigTree->Branch(fOutConfig.s_storeRecHits.c_str(), &fOutConfig.storeRecHits);
  fOutConfigTree->Branch(fOutConfig.s_applyTrigger.c_str(), &fOutConfig.applyTrigger);
  fOutConfigTree->Branch(fOutConfig.s_minHT.c_str(), &fOutConfig.minHT);
  fOutConfigTree->Branch(fOutConfig.s_applyHT.c_str(), &fOutConfig.applyHT);
  fOutConfigTree->Branch(fOutConfig.s_phgoodpTmin.c_str(), &fOutConfig.phgoodpTmin);
  fOutConfigTree->Branch(fOutConfig.s_phgoodIDmin.c_str(), &fOutConfig.phgoodIDmin_s);
  fOutConfigTree->Branch(fOutConfig.s_applyPhGood.c_str(), &fOutConfig.applyPhGood);
  fOutConfigTree->Branch(fOutConfig.s_dRmin.c_str(), &fOutConfig.dRmin);
  fOutConfigTree->Branch(fOutConfig.s_pTres.c_str(), &fOutConfig.pTres);
  fOutConfigTree->Branch(fOutConfig.s_genpTres.c_str(), &fOutConfig.genpTres);
  fOutConfigTree->Branch(fOutConfig.s_trackdRmin.c_str(), &fOutConfig.trackdRmin);
  fOutConfigTree->Branch(fOutConfig.s_trackpTmin.c_str(), &fOutConfig.trackpTmin);
  fOutConfigTree->Branch(fOutConfig.s_genjetdRmin.c_str(), &fOutConfig.genjetdRmin);
  fOutConfigTree->Branch(fOutConfig.s_genjetpTfactor.c_str(), &fOutConfig.genjetpTfactor);
  fOutConfigTree->Branch(fOutConfig.s_leptondRmin.c_str(), &fOutConfig.leptondRmin);
  fOutConfigTree->Branch(fOutConfig.s_smearjetEmin.c_str(), &fOutConfig.smearjetEmin);
  fOutConfigTree->Branch(fOutConfig.s_inputPaths.c_str(), &fOutConfig.inputPaths_s);
  fOutConfigTree->Branch(fOutConfig.s_inputFilters.c_str(), &fOutConfig.inputFilters_s);
  fOutConfigTree->Branch(fOutConfig.s_inputFlags.c_str(), &fOutConfig.inputFlags_s);
  fOutConfigTree->Branch(fOutConfig.s_isGMSB.c_str(), &fOutConfig.isGMSB);
  fOutConfigTree->Branch(fOutConfig.s_isHVDS.c_str(), &fOutConfig.isHVDS);
  fOutConfigTree->Branch(fOutConfig.s_isBkgd.c_str(), &fOutConfig.isBkgd);
  fOutConfigTree->Branch(fOutConfig.s_isToy.c_str(), &fOutConfig.isToy);
  fOutConfigTree->Branch(fOutConfig.s_isADD.c_str(), &fOutConfig.isADD);
  fOutConfigTree->Branch(fOutConfig.s_xsec.c_str(), &fOutConfig.xsec);
  fOutConfigTree->Branch(fOutConfig.s_filterEff.c_str(), &fOutConfig.filterEff);
  fOutConfigTree->Branch(fOutConfig.s_BR.c_str(), &fOutConfig.BR);

  // extra branches
  fOutConfigTree->Branch(fOutConfig.s_sumWgts.c_str(), &fOutConfig.sumWgts);
  fOutConfigTree->Branch(fOutConfig.s_sampleWeight.c_str(), &fOutConfig.sampleWeight);
  fOutConfigTree->Branch(fOutConfig.s_puWeights.c_str(), &fOutConfig.puWeights);

  // Now set the values of the branches
  fOutConfig.blindSF = fInConfig.blindSF;
  fOutConfig.applyBlindSF = fInConfig.applyBlindSF;
  fOutConfig.blindMET = fInConfig.blindMET;
  fOutConfig.applyBlindMET = fInConfig.applyBlindMET;
  fOutConfig.jetpTmin = fInConfig.jetpTmin;
  fOutConfig.jetEtamax = fInConfig.jetEtamax;
  fOutConfig.jetIDmin = fInConfig.jetIDmin;
  fOutConfig.rhEmin = fInConfig.rhEmin;
  fOutConfig.phpTmin = fInConfig.phpTmin;
  fOutConfig.phIDmin_s = fInConfig.phIDmin->c_str();
  fOutConfig.seedTimemin = fInConfig.seedTimemin;
  fOutConfig.nPhosmax = fInConfig.nPhosmax;
  fOutConfig.splitPho = fInConfig.splitPho;
  fOutConfig.onlyGED = fInConfig.onlyGED;
  fOutConfig.onlyOOT = fInConfig.onlyOOT;
  fOutConfig.ellowpTmin = fInConfig.ellowpTmin;
  fOutConfig.elhighpTmin = fInConfig.elhighpTmin;
  fOutConfig.mulowpTmin = fInConfig.mulowpTmin;
  fOutConfig.muhighpTmin = fInConfig.muhighpTmin;
  fOutConfig.storeRecHits = false; // drop these now!
  fOutConfig.applyTrigger = fInConfig.applyTrigger;
  fOutConfig.minHT = fInConfig.minHT;
  fOutConfig.applyHT = fInConfig.applyHT;
  fOutConfig.phgoodpTmin = fInConfig.phgoodpTmin;
  fOutConfig.phgoodIDmin_s = fInConfig.phgoodIDmin->c_str();
  fOutConfig.applyPhGood = fInConfig.applyPhGood;
  fOutConfig.dRmin = fInConfig.dRmin;
  fOutConfig.pTres = fInConfig.pTres;
  fOutConfig.genpTres = fInConfig.genpTres;
  fOutConfig.trackdRmin = fInConfig.trackdRmin;
  fOutConfig.trackpTmin = fInConfig.trackpTmin;
  fOutConfig.genjetdRmin = fInConfig.genjetdRmin;
  fOutConfig.genjetpTfactor = fInConfig.genjetpTfactor;
  fOutConfig.leptondRmin = fInConfig.leptondRmin;
  fOutConfig.smearjetEmin = fInConfig.smearjetEmin;
  fOutConfig.inputPaths_s = fInConfig.inputPaths->c_str();
  fOutConfig.inputFilters_s = fInConfig.inputFilters->c_str();
  fOutConfig.inputFlags_s = fInConfig.inputFlags->c_str();
  fOutConfig.isGMSB = fInConfig.isGMSB;
  fOutConfig.isHVDS = fInConfig.isHVDS;
  fOutConfig.isBkgd = fInConfig.isBkgd;
  fOutConfig.isToy = fInConfig.isToy;
  fOutConfig.isADD = fInConfig.isADD;
  fOutConfig.xsec = fInConfig.xsec;
  fOutConfig.filterEff = fInConfig.filterEff;
  fOutConfig.BR = fInConfig.BR;

  // include computed variables for ease of use
  fOutConfig.sumWgts = fSumWgts;
  fOutConfig.sampleWeight = fSampleWeight;
  fOutConfig.puWeights = fPUWeights;

  // and fill it once
  fOutConfigTree->Fill();
}

void Skimmer::InitOutTree()
{
  Skimmer::InitOutStructs();
  Skimmer::InitOutBranches();
}

void Skimmer::InitOutStructs()
{
  if (fIsMC)
  {
    if (fOutConfig.isGMSB)
    {
      fOutGMSBs.clear(); 
      fOutGMSBs.resize(Common::nGMSBs);
    }
    if (fOutConfig.isHVDS)
    {
      fOutHVDSs.clear(); 
      fOutHVDSs.resize(Common::nHVDSs);
    }
    if (fOutConfig.isToy)
    {
      fOutToys.clear(); 
      fOutToys.resize(Common::nToys);
    }
  }

  fOutPhos.clear();
  fOutPhos.resize(Common::nPhotons);
}

void Skimmer::InitOutBranches()
{
  if (fIsMC)
  {
    fOutTree->Branch(fOutEvent.s_genwgt.c_str(), &fOutEvent.genwgt);
    fOutTree->Branch(fOutEvent.s_puwgt.c_str(), &fOutEvent.puwgt);
    fOutTree->Branch(fOutEvent.s_genx0.c_str(), &fOutEvent.genx0);
    fOutTree->Branch(fOutEvent.s_geny0.c_str(), &fOutEvent.geny0);
    fOutTree->Branch(fOutEvent.s_genz0.c_str(), &fOutEvent.genz0);
    fOutTree->Branch(fOutEvent.s_gent0.c_str(), &fOutEvent.gent0);
    fOutTree->Branch(fOutEvent.s_genpuobs.c_str(), &fOutEvent.genpuobs);
    fOutTree->Branch(fOutEvent.s_genputrue.c_str(), &fOutEvent.genputrue);
    fOutTree->Branch(fOutEvent.s_genMETpt.c_str(), &fOutEvent.genMETpt);
    fOutTree->Branch(fOutEvent.s_genMETphi.c_str(), &fOutEvent.genMETphi);

    if (fOutConfig.isGMSB)
    {
      fOutTree->Branch(fOutEvent.s_nNeutoPhGr.c_str(), &fOutEvent.nNeutoPhGr);
      for (auto igmsb = 0; igmsb < Common::nGMSBs; igmsb++) 
      {
	auto & gmsb = fOutGMSBs[igmsb];
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNmass.c_str(),igmsb), &gmsb.genNmass);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNE.c_str(),igmsb), &gmsb.genNE);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNpt.c_str(),igmsb), &gmsb.genNpt);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNphi.c_str(),igmsb), &gmsb.genNphi);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNeta.c_str(),igmsb), &gmsb.genNeta);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNprodvx.c_str(),igmsb), &gmsb.genNprodvx);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNprodvy.c_str(),igmsb), &gmsb.genNprodvy);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNprodvz.c_str(),igmsb), &gmsb.genNprodvz);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNdecayvx.c_str(),igmsb), &gmsb.genNdecayvx);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNdecayvy.c_str(),igmsb), &gmsb.genNdecayvy);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNdecayvz.c_str(),igmsb), &gmsb.genNdecayvz);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphE.c_str(),igmsb), &gmsb.genphE);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphpt.c_str(),igmsb), &gmsb.genphpt);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphphi.c_str(),igmsb), &gmsb.genphphi);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genpheta.c_str(),igmsb), &gmsb.genpheta);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphmatch.c_str(),igmsb), &gmsb.genphmatch);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrmass.c_str(),igmsb), &gmsb.gengrmass);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrE.c_str(),igmsb), &gmsb.gengrE);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrpt.c_str(),igmsb), &gmsb.gengrpt);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrphi.c_str(),igmsb), &gmsb.gengrphi);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengreta.c_str(),igmsb), &gmsb.gengreta);
      } // end loop over neutralinos
    } // end block over gmsb

    if (fOutConfig.isHVDS)
    {
      fOutTree->Branch(fOutEvent.s_nvPions.c_str(), &fOutEvent.nvPions);
      for (auto ihvds = 0; ihvds < Common::nHVDSs; ihvds++) 
      {
	auto & hvds = fOutHVDSs[ihvds]; 
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionmass.c_str(),ihvds), &hvds.genvPionmass);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionE.c_str(),ihvds), &hvds.genvPionE);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionpt.c_str(),ihvds), &hvds.genvPionpt);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionphi.c_str(),ihvds), &hvds.genvPionphi);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPioneta.c_str(),ihvds), &hvds.genvPioneta);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionprodvx.c_str(),ihvds), &hvds.genvPionprodvx);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionprodvy.c_str(),ihvds), &hvds.genvPionprodvy);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionprodvz.c_str(),ihvds), &hvds.genvPionprodvz);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPiondecayvx.c_str(),ihvds), &hvds.genvPiondecayvx);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPiondecayvy.c_str(),ihvds), &hvds.genvPiondecayvy);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPiondecayvz.c_str(),ihvds), &hvds.genvPiondecayvz);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0E.c_str(),ihvds), &hvds.genHVph0E);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0pt.c_str(),ihvds), &hvds.genHVph0pt);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0phi.c_str(),ihvds), &hvds.genHVph0phi);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0eta.c_str(),ihvds), &hvds.genHVph0eta);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0match.c_str(),ihvds), &hvds.genHVph0match);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1E.c_str(),ihvds), &hvds.genHVph1E);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1pt.c_str(),ihvds), &hvds.genHVph1pt);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1phi.c_str(),ihvds), &hvds.genHVph1phi);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1eta.c_str(),ihvds), &hvds.genHVph1eta);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1match.c_str(),ihvds), &hvds.genHVph1match);
      } // end loop over nvpions 
    } // end block over hvds

    if (fOutConfig.isToy)
    {
      fOutTree->Branch(fOutEvent.s_nToyPhs.c_str(), &fOutEvent.nToyPhs);
      for (Int_t itoy = 0; itoy < Common::nToys; itoy++) 
      {
	auto & toy = fOutToys[itoy]; 
	fOutTree->Branch(Form("%s_%i",toy.s_genphE.c_str(),itoy), &toy.genphE);
	fOutTree->Branch(Form("%s_%i",toy.s_genphpt.c_str(),itoy), &toy.genphpt);
	fOutTree->Branch(Form("%s_%i",toy.s_genphphi.c_str(),itoy), &toy.genphphi);
	fOutTree->Branch(Form("%s_%i",toy.s_genpheta.c_str(),itoy), &toy.genpheta);
	fOutTree->Branch(Form("%s_%i",toy.s_genphmatch.c_str(),itoy), &toy.genphmatch);
	fOutTree->Branch(Form("%s_%i",toy.s_genphmatch_ptres.c_str(),itoy), &toy.genphmatch_ptres);
	fOutTree->Branch(Form("%s_%i",toy.s_genphmatch_status.c_str(),itoy), &toy.genphmatch_status);
      } // end loop over toy phos
    } // end block over toyMC

  } // end block over isMC

  fOutTree->Branch(fOutEvent.s_run.c_str(), &fOutEvent.run);
  fOutTree->Branch(fOutEvent.s_lumi.c_str(), &fOutEvent.lumi);
  fOutTree->Branch(fOutEvent.s_event.c_str(), &fOutEvent.event);

  fOutTree->Branch(fOutEvent.s_hltSignal.c_str(), &fOutEvent.hltSignal);
  fOutTree->Branch(fOutEvent.s_hltRefPhoID.c_str(), &fOutEvent.hltRefPhoID);
  fOutTree->Branch(fOutEvent.s_hltRefDispID.c_str(), &fOutEvent.hltRefDispID);
  fOutTree->Branch(fOutEvent.s_hltRefHT.c_str(), &fOutEvent.hltRefHT);
  fOutTree->Branch(fOutEvent.s_hltPho50.c_str(), &fOutEvent.hltPho50);
  fOutTree->Branch(fOutEvent.s_hltPho200.c_str(), &fOutEvent.hltPho200);
  fOutTree->Branch(fOutEvent.s_hltDiPho70.c_str(), &fOutEvent.hltDiPho70);
  fOutTree->Branch(fOutEvent.s_hltDiPho3022M90.c_str(), &fOutEvent.hltDiPho3022M90);
  fOutTree->Branch(fOutEvent.s_hltDiPho30PV18PV.c_str(), &fOutEvent.hltDiPho30PV18PV);
  fOutTree->Branch(fOutEvent.s_hltEle32WPT.c_str(), &fOutEvent.hltEle32WPT);
  fOutTree->Branch(fOutEvent.s_hltDiEle33MW.c_str(), &fOutEvent.hltDiEle33MW);
  fOutTree->Branch(fOutEvent.s_hltJet500.c_str(), &fOutEvent.hltJet500);

  fOutTree->Branch(fOutEvent.s_nvtx.c_str(), &fOutEvent.nvtx);
  fOutTree->Branch(fOutEvent.s_vtxX.c_str(), &fOutEvent.vtxX);
  fOutTree->Branch(fOutEvent.s_vtxY.c_str(), &fOutEvent.vtxY);
  fOutTree->Branch(fOutEvent.s_vtxZ.c_str(), &fOutEvent.vtxZ);
  fOutTree->Branch(fOutEvent.s_rho.c_str(), &fOutEvent.rho);

  fOutTree->Branch(fOutEvent.s_t1pfMETpt.c_str(), &fOutEvent.t1pfMETpt);
  fOutTree->Branch(fOutEvent.s_t1pfMETphi.c_str(), &fOutEvent.t1pfMETphi);
  fOutTree->Branch(fOutEvent.s_t1pfMETsumEt.c_str(), &fOutEvent.t1pfMETsumEt);
  fOutTree->Branch(fOutEvent.s_t1pfMETptUncorr.c_str(), &fOutEvent.t1pfMETptUncorr);
  fOutTree->Branch(fOutEvent.s_t1pfMETphiUncorr.c_str(), &fOutEvent.t1pfMETphiUncorr);
  fOutTree->Branch(fOutEvent.s_t1pfMETsumEtUncorr.c_str(), &fOutEvent.t1pfMETsumEtUncorr);

  fOutTree->Branch(fOutEvent.s_njets.c_str(), &fOutEvent.njets);
  if (fSkim != SkimType::DiXtal)
  {
    fOutTree->Branch(fOutJets.s_E.c_str(), &fOutJets.E_f);
    fOutTree->Branch(fOutJets.s_pt.c_str(), &fOutJets.pt_f);
    fOutTree->Branch(fOutJets.s_phi.c_str(), &fOutJets.phi_f);
    fOutTree->Branch(fOutJets.s_eta.c_str(), &fOutJets.eta_f);
    fOutTree->Branch(fOutJets.s_ID.c_str(), &fOutJets.ID_i);
    // fOutTree->Branch(fOutJets.s_NHF.c_str(), &fOutJets.NHF_f);
    // fOutTree->Branch(fOutJets.s_NEMF.c_str(), &fOutJets.NEMF_f);
    // fOutTree->Branch(fOutJets.s_CHF.c_str(), &fOutJets.CHF_f);
    // fOutTree->Branch(fOutJets.s_CEMF.c_str(), &fOutJets.CEMF_f);
    // fOutTree->Branch(fOutJets.s_MUF.c_str(), &fOutJets.MUF_f);
    // fOutTree->Branch(fOutJets.s_NHM.c_str(), &fOutJets.NHM_f);
    // fOutTree->Branch(fOutJets.s_CHM.c_str(), &fOutJets.CHM_f);
  }

  fOutTree->Branch(fOutEvent.s_nelLowL.c_str(), &fOutEvent.nelLowL);
  fOutTree->Branch(fOutEvent.s_nelLowM.c_str(), &fOutEvent.nelLowM);
  fOutTree->Branch(fOutEvent.s_nelLowT.c_str(), &fOutEvent.nelLowT);
  fOutTree->Branch(fOutEvent.s_nelHighL.c_str(), &fOutEvent.nelHighL);
  fOutTree->Branch(fOutEvent.s_nelHighM.c_str(), &fOutEvent.nelHighM);
  fOutTree->Branch(fOutEvent.s_nelHighT.c_str(), &fOutEvent.nelHighT);
  fOutTree->Branch(fOutEvent.s_nmuLowL.c_str(), &fOutEvent.nmuLowL);
  fOutTree->Branch(fOutEvent.s_nmuLowM.c_str(), &fOutEvent.nmuLowM);
  fOutTree->Branch(fOutEvent.s_nmuLowT.c_str(), &fOutEvent.nmuLowT);
  fOutTree->Branch(fOutEvent.s_nmuHighL.c_str(), &fOutEvent.nmuHighL);
  fOutTree->Branch(fOutEvent.s_nmuHighM.c_str(), &fOutEvent.nmuHighM);
  fOutTree->Branch(fOutEvent.s_nmuHighT.c_str(), &fOutEvent.nmuHighT);
  
  fOutTree->Branch(fOutEvent.s_nrechits.c_str(), &fOutEvent.nrechits);

  fOutTree->Branch(fOutEvent.s_nphotons.c_str(), &fOutEvent.nphotons);
  for (auto ipho = 0; ipho < fNOutPhos; ipho++) 
  {
    auto & pho = fOutPhos[ipho];
    fOutTree->Branch(Form("%s_%i",pho.s_E.c_str(),ipho), &pho.E);
    fOutTree->Branch(Form("%s_%i",pho.s_pt.c_str(),ipho), &pho.pt);
    fOutTree->Branch(Form("%s_%i",pho.s_eta.c_str(),ipho), &pho.eta);
    fOutTree->Branch(Form("%s_%i",pho.s_phi.c_str(),ipho), &pho.phi);
    fOutTree->Branch(Form("%s_%i",pho.s_scE.c_str(),ipho), &pho.scE);
    fOutTree->Branch(Form("%s_%i",pho.s_sceta.c_str(),ipho), &pho.sceta);
    fOutTree->Branch(Form("%s_%i",pho.s_scphi.c_str(),ipho), &pho.scphi);
    fOutTree->Branch(Form("%s_%i",pho.s_HoE.c_str(),ipho), &pho.HoE);
    fOutTree->Branch(Form("%s_%i",pho.s_r9.c_str(),ipho), &pho.r9);
    fOutTree->Branch(Form("%s_%i",pho.s_ChgHadIso.c_str(),ipho), &pho.ChgHadIso);
    fOutTree->Branch(Form("%s_%i",pho.s_NeuHadIso.c_str(),ipho), &pho.NeuHadIso);
    fOutTree->Branch(Form("%s_%i",pho.s_PhoIso.c_str(),ipho), &pho.PhoIso);
    fOutTree->Branch(Form("%s_%i",pho.s_EcalPFClIso.c_str(),ipho), &pho.EcalPFClIso);
    fOutTree->Branch(Form("%s_%i",pho.s_HcalPFClIso.c_str(),ipho), &pho.HcalPFClIso);
    fOutTree->Branch(Form("%s_%i",pho.s_TrkIso.c_str(),ipho), &pho.TrkIso);
    // fOutTree->Branch(Form("%s_%i",pho.s_ChgHadIsoC.c_str(),ipho), &pho.ChgHadIsoC);
    // fOutTree->Branch(Form("%s_%i",pho.s_NeuHadIsoC.c_str(),ipho), &pho.NeuHadIsoC);
    // fOutTree->Branch(Form("%s_%i",pho.s_PhoIsoC.c_str(),ipho), &pho.PhoIsoC);
    // fOutTree->Branch(Form("%s_%i",pho.s_EcalPFClIsoC.c_str(),ipho), &pho.EcalPFClIsoC);
    // fOutTree->Branch(Form("%s_%i",pho.s_HcalPFClIsoC.c_str(),ipho), &pho.HcalPFClIsoC);
    // fOutTree->Branch(Form("%s_%i",pho.s_TrkIsoC.c_str(),ipho), &pho.TrkIsoC);
    fOutTree->Branch(Form("%s_%i",pho.s_sieie.c_str(),ipho), &pho.sieie);
    // fOutTree->Branch(Form("%s_%i",pho.s_sipip.c_str(),ipho), &pho.sipip);
    // fOutTree->Branch(Form("%s_%i",pho.s_sieip.c_str(),ipho), &pho.sieip);
    // fOutTree->Branch(Form("%s_%i",pho.s_e2x2.c_str(),ipho), &pho.e2x2);
    // fOutTree->Branch(Form("%s_%i",pho.s_e3x3.c_str(),ipho), &pho.e3x3);
    // fOutTree->Branch(Form("%s_%i",pho.s_e5x5.c_str(),ipho), &pho.e5x5);
    fOutTree->Branch(Form("%s_%i",pho.s_smaj.c_str(),ipho), &pho.smaj);
    fOutTree->Branch(Form("%s_%i",pho.s_smin.c_str(),ipho), &pho.smin);
    // fOutTree->Branch(Form("%s_%i",pho.s_alpha.c_str(),ipho), &pho.alpha);
    fOutTree->Branch(Form("%s_%i",pho.s_suisseX.c_str(),ipho), &pho.suisseX);
    // fOutTree->Branch(Form("%s_%i",pho.s_seedX.c_str(),ipho), &pho.seedX);
    // fOutTree->Branch(Form("%s_%i",pho.s_seedY.c_str(),ipho), &pho.seedY);
    // fOutTree->Branch(Form("%s_%i",pho.s_seedZ.c_str(),ipho), &pho.seedZ);
    fOutTree->Branch(Form("%s_%i",pho.s_seedE.c_str(),ipho), &pho.seedE);
    fOutTree->Branch(Form("%s_%i",pho.s_seedtime.c_str(),ipho), &pho.seedtime);
    // fOutTree->Branch(Form("%s_%i",pho.s_seedtimeErr.c_str(),ipho), &pho.seedtimeErr);
    fOutTree->Branch(Form("%s_%i",pho.s_seedTOF.c_str(),ipho), &pho.seedTOF);
    // fOutTree->Branch(Form("%s_%i",pho.s_seedID.c_str(),ipho), &pho.seedID);
    // fOutTree->Branch(Form("%s_%i",pho.s_seedisOOT.c_str(),ipho), &pho.seedisOOT);
    fOutTree->Branch(Form("%s_%i",pho.s_seedisGS6.c_str(),ipho), &pho.seedisGS6);
    fOutTree->Branch(Form("%s_%i",pho.s_seedisGS1.c_str(),ipho), &pho.seedisGS1);
    fOutTree->Branch(Form("%s_%i",pho.s_seedadcToGeV.c_str(),ipho), &pho.seedadcToGeV);
    fOutTree->Branch(Form("%s_%i",pho.s_seedped12.c_str(),ipho), &pho.seedped12);
    fOutTree->Branch(Form("%s_%i",pho.s_seedped6.c_str(),ipho), &pho.seedped6);
    fOutTree->Branch(Form("%s_%i",pho.s_seedped1.c_str(),ipho), &pho.seedped1);
    fOutTree->Branch(Form("%s_%i",pho.s_seedpedrms12.c_str(),ipho), &pho.seedpedrms12);
    fOutTree->Branch(Form("%s_%i",pho.s_seedpedrms6.c_str(),ipho), &pho.seedpedrms6);
    fOutTree->Branch(Form("%s_%i",pho.s_seedpedrms1.c_str(),ipho), &pho.seedpedrms1);
    fOutTree->Branch(Form("%s_%i",pho.s_isOOT.c_str(),ipho), &pho.isOOT);
    fOutTree->Branch(Form("%s_%i",pho.s_isEB.c_str(),ipho), &pho.isEB);
    fOutTree->Branch(Form("%s_%i",pho.s_isHLT.c_str(),ipho), &pho.isHLT);
    fOutTree->Branch(Form("%s_%i",pho.s_isTrk.c_str(),ipho), &pho.isTrk);
    fOutTree->Branch(Form("%s_%i",pho.s_passEleVeto.c_str(),ipho), &pho.passEleVeto);
    fOutTree->Branch(Form("%s_%i",pho.s_hasPixSeed.c_str(),ipho), &pho.hasPixSeed);
    fOutTree->Branch(Form("%s_%i",pho.s_gedID.c_str(),ipho), &pho.gedID);
    fOutTree->Branch(Form("%s_%i",pho.s_ootID.c_str(),ipho), &pho.ootID);

    if (fIsMC)
    {
      fOutTree->Branch(Form("%s_%i",pho.s_isGen.c_str(),ipho), &pho.isGen);
      if (fOutConfig.isGMSB || fOutConfig.isHVDS)
      {
	fOutTree->Branch(Form("%s_%i",pho.s_isSignal.c_str(),ipho), &pho.isSignal);
      }
    }

    // Derived types
    fOutTree->Branch(Form("%s_%i",pho.s_seedTT.c_str(),ipho), &pho.seedTT);

    if (fInConfig.storeRecHits)
    {
      fOutTree->Branch(Form("%s_%i",pho.s_nrechits.c_str(),ipho), &pho.nrechits);
      fOutTree->Branch(Form("%s_%i",pho.s_nrechitsLT120.c_str(),ipho), &pho.nrechitsLT120);
      fOutTree->Branch(Form("%s_%i",pho.s_meantime.c_str(),ipho), &pho.meantime);
      fOutTree->Branch(Form("%s_%i",pho.s_meantimeLT120.c_str(),ipho), &pho.meantimeLT120);
      fOutTree->Branch(Form("%s_%i",pho.s_weightedtime.c_str(),ipho), &pho.weightedtime);
      fOutTree->Branch(Form("%s_%i",pho.s_weightedtimeLT120.c_str(),ipho), &pho.weightedtimeLT120);
      fOutTree->Branch(Form("%s_%i",pho.s_weightedtimeTOF.c_str(),ipho), &pho.weightedtimeTOF);
      fOutTree->Branch(Form("%s_%i",pho.s_weightedtimeLT120TOF.c_str(),ipho), &pho.weightedtimeLT120TOF);
    }
  } // end loop over nPhotons

  // add event weight
  fOutTree->Branch(fOutEvent.s_evtwgt.c_str(), &fOutEvent.evtwgt);
} 

void Skimmer::InitOutCutFlowHists()
{
  Skimmer::InitOutCutFlowHist(fInCutFlow,fOutCutFlow,Common::h_cutflowname);
  Skimmer::InitOutCutFlowHist(fInCutFlowWgt,fOutCutFlowWgt,Common::h_cutflow_wgtname);
  Skimmer::InitOutCutFlowHist(fInCutFlowWgt,fOutCutFlowScl,Common::h_cutflow_scaledname);

  // rescale input of scaled histogram by sample weight!
  fOutCutFlowScl->Scale(fSampleWeight);
  fOutCutFlowScl->SetTitle("Scaled");
  fOutCutFlowScl->GetYaxis()->SetTitle("nEvents");
}

void Skimmer::InitOutCutFlowHist(const TH1F * inh_cutflow, TH1F *& outh_cutflow, const TString & histname)
{
  // get cut flow labels
  const auto inNbinsX = inh_cutflow->GetNbinsX();
  for (auto ibin = 1; ibin <= inNbinsX; ibin++)
  {
    cutLabels[inh_cutflow->GetXaxis()->GetBinLabel(ibin)] = ibin;
  }
  auto inNbinsX_new = inNbinsX;

  // always add safety counter
  cutLabels["PreSkim"] = ++inNbinsX_new;

  // labels for each skim
  if (fSkim == SkimType::Standard)
  {
    cutLabels["nPhotons"] = ++inNbinsX_new;
    cutLabels["ph0isEB"] = ++inNbinsX_new;
    cutLabels["ph0pt70"] = ++inNbinsX_new;
    cutLabels["METFlag"] = ++inNbinsX_new;
  }
  else if (fSkim == SkimType::Zee)
  {
    cutLabels["diEleHLT"] = ++inNbinsX_new;
    cutLabels["goodPho1"] = ++inNbinsX_new;
    cutLabels["goodPho2"] = ++inNbinsX_new;
    cutLabels["diPhoMZrange"] = ++inNbinsX_new;
  }
  else if (fSkim == SkimType::DiXtal)
  {
    cutLabels["goodDiXtal"] = ++inNbinsX_new;
  }
  else
  {
    std::cerr << "How did this happen?? Somehow, fSkim was not setup! Exiting..." << std::endl;
    exit(1);
  }

  // common skim
  cutLabels["badPU"] = ++inNbinsX_new;

  // make new cut flow
  outh_cutflow = new TH1F(Form("%s",histname.Data()),inh_cutflow->GetTitle(),cutLabels.size(),0,cutLabels.size());
  outh_cutflow->Sumw2();

  for (const auto & cutlabel : cutLabels)
  {
    const auto ibin = cutlabel.second;

    outh_cutflow->GetXaxis()->SetBinLabel(ibin,cutlabel.first.c_str());

    if (ibin > inNbinsX) continue;

    outh_cutflow->SetBinContent(ibin,inh_cutflow->GetBinContent(ibin));
    outh_cutflow->SetBinError(ibin,inh_cutflow->GetBinError(ibin));
  }
  outh_cutflow->GetYaxis()->SetTitle(inh_cutflow->GetYaxis()->GetTitle());
}

void Skimmer::GetSampleWeight()
{
  // include normalization to lumi!!! ( do we need to multiply by * fInConfig.BR)
  fSampleWeight = (fIsMC ? fInConfig.xsec * fInConfig.filterEff * Common::EraMap["Full"].lumi * Common::invfbToinvpb / fSumWgts : 1.f); 
}

void Skimmer::GetPUWeights()
{
  for (auto ibin = 1; ibin <= fInPUWgtHist->GetNbinsX(); ibin++)
  {
    fPUWeights.emplace_back(fInPUWgtHist->GetBinContent(ibin));
  }
}

void Skimmer::FillPhoListStandard()
{      
  fPhoList.clear();
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    fPhoList.emplace_back(ipho);
  }
}

///////////////////////////
// Configure the skimmer //
///////////////////////////

void Skimmer::SetupDefaults()
{
  fSkim = SkimType::Standard;
  fSumWgts = 1.f;
  fJEC = ECorr::Nominal;
  fJER = ECorr::Nominal;
  fPhoSc = ECorr::Nominal;
  fPhoSm = ECorr::Nominal;
}

void Skimmer::SetupSkimConfig()
{
  std::cout << "Reading skim config..." << std::endl;

  std::ifstream infile(Form("%s",fSkimConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("skim_type=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"skim_type=");
      Skimmer::SetupSkimType(str);
    }
    else if (str.find("sum_wgts=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"sum_wgts=");
      fSumWgts = Common::Atof(str);
    }
    else if (str.find("puwgt_filename=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"puwgt_filename=");
      fPUWgtFileName = str;
    }
    else if (str.find("jet_scale=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"jet_scale=");
      Skimmer::SetupEnergyCorrection(str,fJEC,"jet_scale");
    }
    else if (str.find("jet_resol=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"jet_resol=");
      Skimmer::SetupEnergyCorrection(str,fJER,"jet_resol");
    }
    else if (str.find("pho_scale=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"pho_scale=");
      Skimmer::SetupEnergyCorrection(str,fPhoSc,"pho_scale");
    }
    else if (str.find("pho_smear=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"pho_smear=");
      Skimmer::SetupEnergyCorrection(str,fPhoSm,"pho_smear");
    }
    else
    {
      std::cerr << "Specified a non-known option: " << str.c_str() << " ...Exiting..." << std::endl;
      exit(1);
    }
  }

  // reduce output of DiXtal
  fNOutPhos = (fSkim == SkimType::DiXtal ? 2 : Common::nPhotons);
}

void Skimmer::SetupSkimType(const TString & skim_type)
{
  if      (skim_type.EqualTo("Standard"   ,TString::kExact)) fSkim = SkimType::Standard;
  else if (skim_type.EqualTo("Zee"        ,TString::kExact)) fSkim = SkimType::Zee;
  else if (skim_type.EqualTo("DiXtal"     ,TString::kExact)) fSkim = SkimType::DiXtal;
  else if (skim_type.EqualTo("AlwaysTrue" ,TString::kExact)) fSkim = SkimType::AlwaysTrue;
  else if (skim_type.EqualTo("AlwaysFalse",TString::kExact)) fSkim = SkimType::AlwaysFalse;
  else
  {
    std::cerr << skim_type.Data() << " is not a valid skim selection! Exiting..." << std::endl;
    exit(1);
  }
}

void Skimmer::SetupEnergyCorrection(const TString & str, ECorr & ecorr, const TString & text)
{
  if      (str.EqualTo("Nominal",TString::kExact)) ecorr = ECorr::Nominal;
  else if (str.EqualTo("Down"   ,TString::kExact)) ecorr = ECorr::Down;
  else if (str.EqualTo("Up"     ,TString::kExact)) ecorr = ECorr::Up;
  else
  {
    std::cerr << str.Data() << " is not a valid " << text.Data() << " selection! Exiting..." << std::endl;
    exit(1);
  }
}
