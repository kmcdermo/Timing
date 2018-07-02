#include "Skimmer.hh"
#include "TROOT.h"

#include <iostream>

Skimmer::Skimmer(const TString & indir, const TString & outdir, const TString & filename, 
		 const Float_t sumwgts, const TString & puwgtfilename, const Bool_t redophoid) :
  fInDir(indir), fOutDir(outdir), fFileName(filename), 
  fSumWgts(sumwgts), fPUWgtFileName(puwgtfilename), fRedoPhoID(redophoid)
{
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

  ////////////////////////
  // Get all the inputs //
  ////////////////////////
  std::cout << "Setting up inputs for skim" << std::endl;

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
  Common::CheckValidTH1F(fInCutFlow,inh_cutflowname,infilename);

  // const TString inh_cutflow_wgtname = Form("%s/%s_wgt",Common::rootdir.Data(),Common::h_cutflow_wgtname.Data());
  // fInCutFlowWgt = (TH1F*)fInFile->Get(inh_cutflow_wgtname.Data());
  // Common::CheckValidTH1F(fInCutFlowWgt,inh_cutflow_wgtname,infilename);

  // Get PU weights input
  fPUWeights.clear();
  if (fIsMC)
  {
    const Bool_t useOld = (fPUWgtFileName == "");

    const TString pufilename = (useOld ? Form("%s/%s/%s.root",Common::eosDir.Data(),Common::baseDir.Data(),Common::puwgtFileName.Data()) : Form("%s",fPUWgtFileName.Data()));
    fInPUWgtFile = TFile::Open(pufilename.Data());
    Common::CheckValidFile(fInPUWgtFile,pufilename);

    const TString puhistname = (useOld ? Form("%s",Common::puwgtHistName.Data()) : Form("%s_%s",Common::puTrueHistName.Data(),Common::puwgtHistName.Data()));
    fInPUWgtHist = (TH1F*)fInPUWgtFile->Get(puhistname.Data());
    Common::CheckValidTH1F(fInPUWgtHist,puhistname,pufilename);

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

  //  delete fOutCutFlowScl;
  //  delete fOutCutFlowWgt;
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

    // perform skim
    if (!fOutConfig.isToy) // do not apply skim selection on toy config
    {
      // leading photon skim section
      fInEvent.b_nphotons->GetEntry(entry);
      if (fInEvent.nphotons <= 0) continue;
      fOutCutFlow->Fill((cutLabels["nPhotons"]*1.f)-0.5f,wgt);
      //      fOutCutFlow   ->Fill((cutLabels["nPhotons"]*1.f)-0.5f);
      //      fOutCutFlowWgt->Fill((cutLabels["nPhotons"]*1.f)-0.5f,wgt);
      //      fOutCutFlowScl->Fill((cutLabels["nPhotons"]*1.f)-0.5f,evtwgt);
      
      fInPhos[0].b_isEB->GetEntry(entry);
      if (!fInPhos[0].isEB) continue;
      fOutCutFlow->Fill((cutLabels["ph0isEB"]*1.f)-0.5f,wgt);
      //      fOutCutFlow   ->Fill((cutLabels["ph0isEB"]*1.f)-0.5f);
      //      fOutCutFlowWgt->Fill((cutLabels["ph0isEB"]*1.f)-0.5f,wgt);
      //      fOutCutFlowScl->Fill((cutLabels["ph0isEB"]*1.f)-0.5f,evtwgt);

      fInPhos[0].b_pt->GetEntry(entry);
      if (fInPhos[0].pt < 70.f) continue;
      fOutCutFlow->Fill((cutLabels["ph0pt70"]*1.f)-0.5f,wgt);
      //      fOutCutFlow   ->Fill((cutLabels["ph0pt70"]*1.f)-0.5f);
      //      fOutCutFlowWgt->Fill((cutLabels["ph0pt70"]*1.f)-0.5f,wgt);
      //      fOutCutFlowScl->Fill((cutLabels["ph0pt70"]*1.f)-0.5f,evtwgt);

      // filter on MET Flags
      fInEvent.b_metPV->GetEntry(entry);
      fInEvent.b_metBeamHalo->GetEntry(entry);
      fInEvent.b_metHBHENoise->GetEntry(entry);
      fInEvent.b_metHBHEisoNoise->GetEntry(entry);
      fInEvent.b_metECALTP->GetEntry(entry);
      fInEvent.b_metPFMuon->GetEntry(entry);
      fInEvent.b_metPFChgHad->GetEntry(entry);
      if (!fInEvent.metPV || !fInEvent.metBeamHalo || !fInEvent.metHBHENoise || !fInEvent.metHBHEisoNoise || 
       	  !fInEvent.metECALTP || !fInEvent.metPFMuon || !fInEvent.metPFChgHad) continue;

      fInEvent.b_metECALCalib->GetEntry(entry);
      if (!fOutConfig.isGMSB && !fInEvent.metECALCalib) continue;

      fInEvent.b_metEESC->GetEntry(entry);
      if (!fIsMC && !fInEvent.metEESC) continue;
      
      // fill cutflow for MET filters
      fOutCutFlow->Fill((cutLabels["METFlag"]*1.f)-0.5f,wgt);
      //      fOutCutFlow   ->Fill((cutLabels["METFlag"]*1.f)-0.5f);
      //      fOutCutFlowWgt->Fill((cutLabels["METFlag"]*1.f)-0.5f,wgt);
      //      fOutCutFlowScl->Fill((cutLabels["METFlag"]*1.f)-0.5f,evtwgt);
      
      // cut on crappy pu
      if (fIsMC) 
      {
	fInEvent.b_genputrue->GetEntry(entry);
	if ((fInEvent.genputrue < 0) || (UInt_t(fInEvent.genputrue) >= fPUWeights.size())) continue;
      }
      
      // fill cutflow
      fOutCutFlow->Fill((cutLabels["badPU"]*1.f)-0.5f,wgt);
      //      fOutCutFlow   ->Fill((cutLabels["badPU"]*1.f)-0.5f);
      //      fOutCutFlowWgt->Fill((cutLabels["badPU"]*1.f)-0.5f,wgt);
      //      fOutCutFlowScl->Fill((cutLabels["badPU"]*1.f)-0.5f,evtwgt);
    }

    // end of skim, now copy... dropping rechits
    if (fOutConfig.isGMSB) Skimmer::FillOutGMSBs(entry);
    if (fOutConfig.isHVDS) Skimmer::FillOutHVDSs(entry);
    if (fOutConfig.isToy)  Skimmer::FillOutToys(entry);
    Skimmer::FillOutEvent(entry,evtwgt);
    Skimmer::FillOutJets(entry);
    Skimmer::FillOutPhos(entry);

    // fill the tree
    fOutTree->Fill();
  } // end loop over events

  // write out the output!
  fOutFile->cd();
  fOutCutFlow->Write();
  //  fOutCutFlowWgt->Write();
  //  fOutCutFlowScl->Write();
  fOutConfigTree->Write();
  fOutTree->Write();
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
  fInEvent.b_hltDiEle33MW->GetEntry(entry);
  fInEvent.b_hltDiEle27WPT->GetEntry(entry);
  fInEvent.b_hltJet500->GetEntry(entry);
  fInEvent.b_nvtx->GetEntry(entry);
  fInEvent.b_vtxX->GetEntry(entry);
  fInEvent.b_vtxY->GetEntry(entry);
  fInEvent.b_vtxZ->GetEntry(entry);
  fInEvent.b_rho->GetEntry(entry);
  fInEvent.b_t1pfMETpt->GetEntry(entry);
  fInEvent.b_t1pfMETphi->GetEntry(entry);
  fInEvent.b_t1pfMETsumEt->GetEntry(entry);
  fInEvent.b_njets->GetEntry(entry);
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
  fOutEvent.hltDiEle33MW = fInEvent.hltDiEle33MW;
  fOutEvent.hltDiEle27WPT = fInEvent.hltDiEle27WPT;
  fOutEvent.hltJet500 = fInEvent.hltJet500;
  fOutEvent.nvtx = fInEvent.nvtx;
  fOutEvent.vtxX = fInEvent.vtxX;
  fOutEvent.vtxY = fInEvent.vtxY;
  fOutEvent.vtxZ = fInEvent.vtxZ;
  fOutEvent.rho = fInEvent.rho;
  fOutEvent.t1pfMETpt = fInEvent.t1pfMETpt;
  fOutEvent.t1pfMETphi = fInEvent.t1pfMETphi;
  fOutEvent.t1pfMETsumEt = fInEvent.t1pfMETsumEt;
  fOutEvent.njets = fInEvent.njets;
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

  fOutJets.E_f.swap( (*fInJets.E) );
  fOutJets.pt_f.swap( (*fInJets.pt) );
  fOutJets.phi_f.swap( (*fInJets.phi) );
  fOutJets.eta_f.swap( (*fInJets.eta) );
  fOutJets.ID_i.swap( (*fInJets.ID) );

  // fInJets.b_NHF->GetEntry(entry);
  // fInJets.b_NEMF->GetEntry(entry);
  // fInJets.b_CHF->GetEntry(entry);
  // fInJets.b_CEMF->GetEntry(entry);
  // fInJets.b_MUF->GetEntry(entry);
  // fInJets.b_NHM->GetEntry(entry);
  // fInJets.b_CHM->GetEntry(entry);

  // fOutJets.NHF_f.swap( (*fInJets.NHF) );
  // fOutJets.NEMF_f.swap( (*fInJets.NEMF) );
  // fOutJets.CHF_f.swap( (*fInJets.CHF) );
  // fOutJets.CEMF_f.swap( (*fInJets.CEMF) );
  // fOutJets.MUF_f.swap( (*fInJets.MUF) );
  // fOutJets.NHM_f.swap( (*fInJets.NHM) );
  // fOutJets.CHM_f.swap( (*fInJets.CHM) );
}

void Skimmer::FillOutPhos(const UInt_t entry)
{  
  // get input photon branches
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++) 
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
    inpho.b_sipip->GetEntry(entry);
    inpho.b_sieip->GetEntry(entry);
    inpho.b_e2x2->GetEntry(entry);
    inpho.b_e3x3->GetEntry(entry);
    inpho.b_e5x5->GetEntry(entry);
    inpho.b_smaj->GetEntry(entry);
    inpho.b_smin->GetEntry(entry);
    inpho.b_alpha->GetEntry(entry);
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
      inpho.b_seedtime->GetEntry(entry);
      inpho.b_seedE   ->GetEntry(entry);
      inpho.b_seedID  ->GetEntry(entry);
    }
    
    if (fIsMC)
    {
      inpho.b_isGen->GetEntry(entry);
      if (fInConfig.isGMSB || fInConfig.isHVDS)
      {
	inpho.b_isSignal->GetEntry(entry);
      }
    }
  }

  // get input recHits if needed
  if (fInConfig.storeRecHits)
  {
    fInRecHits.b_eta->GetEntry(entry);
    fInRecHits.b_phi->GetEntry(entry);
    fInRecHits.b_E->GetEntry(entry);
    fInRecHits.b_time->GetEntry(entry);
    fInRecHits.b_OOT->GetEntry(entry);
    fInRecHits.b_ID->GetEntry(entry);
  }

  // set output photon branches
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++) 
  {
    const auto & inpho = fInPhos[ipho];
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
    outpho.sipip = inpho.sipip;
    outpho.sieip = inpho.sieip;
    outpho.e2x2 = inpho.e2x2;
    outpho.e3x3 = inpho.e3x3;
    outpho.e5x5 = inpho.e5x5;
    outpho.smaj = inpho.smaj;
    outpho.smin = inpho.smin;
    outpho.alpha = inpho.alpha;
    outpho.suisseX = inpho.suisseX;
    outpho.isOOT = inpho.isOOT;
    outpho.isEB = inpho.isEB;
    outpho.isHLT = inpho.isHLT;
    outpho.isTrk = inpho.isTrk;
    outpho.passEleVeto = inpho.passEleVeto;
    outpho.hasPixSeed = inpho.hasPixSeed;

    if (fRedoPhoID)
    {
      inpho.gedID >= 0 ? Skimmer::GetGEDPhoVID(outpho) : -1;
      inpho.ootID >= 0 ? Skimmer::GetOOTPhoVID(outpho) : -1;
    }
    else
    {
      outpho.gedID = inpho.gedID;
      outpho.ootID = inpho.ootID;
    }

    if (fInConfig.storeRecHits)
    {
      if (inpho.seed >= 0)
      {
	outpho.seedtime = (*fInRecHits.time)[inpho.seed];
	outpho.seedE    = (*fInRecHits.E)   [inpho.seed];
	outpho.seedID   = (*fInRecHits.ID)  [inpho.seed];
      }
      else
      {
	outpho.seedtime = -9999.f;
	outpho.seedE    = -9999.f;
	outpho.seedID   = 0;
      }
    }
    else
    {
      outpho.seedtime = inpho.seedtime;
      outpho.seedE    = inpho.seedE;
      outpho.seedID   = inpho.seedID;
    }
    
    if (fIsMC)
    {
      outpho.isGen = inpho.isGen;
      if (fOutConfig.isGMSB || fOutConfig.isHVDS)
      {
	outpho.isSignal = inpho.isSignal;
      }
    }
  }
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
  fInConfigTree->SetBranchAddress(fInConfig.s_splitPho.c_str(), &fInConfig.splitPho);
  fInConfigTree->SetBranchAddress(fInConfig.s_onlyGED.c_str(), &fInConfig.onlyGED);
  fInConfigTree->SetBranchAddress(fInConfig.s_onlyOOT.c_str(), &fInConfig.onlyOOT);
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

  if (fInConfig.storeRecHits) 
  {
    fInRecHits.eta = 0;
    fInRecHits.phi = 0;
    fInRecHits.E = 0;
    fInRecHits.time = 0;
    fInRecHits.OOT = 0;
    fInRecHits.ID = 0;

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
  fInTree->SetBranchAddress(fInEvent.s_hltDiEle33MW.c_str(), &fInEvent.hltDiEle33MW, &fInEvent.b_hltDiEle33MW);
  fInTree->SetBranchAddress(fInEvent.s_hltDiEle27WPT.c_str(), &fInEvent.hltDiEle27WPT, &fInEvent.b_hltDiEle27WPT);
  fInTree->SetBranchAddress(fInEvent.s_hltJet500.c_str(), &fInEvent.hltJet500, &fInEvent.b_hltJet500);
  
  fInTree->SetBranchAddress(fInEvent.s_metPV.c_str(), &fInEvent.metPV, &fInEvent.b_metPV);
  fInTree->SetBranchAddress(fInEvent.s_metBeamHalo.c_str(), &fInEvent.metBeamHalo, &fInEvent.b_metBeamHalo);
  fInTree->SetBranchAddress(fInEvent.s_metHBHENoise.c_str(), &fInEvent.metHBHENoise, &fInEvent.b_metHBHENoise);
  fInTree->SetBranchAddress(fInEvent.s_metHBHEisoNoise.c_str(), &fInEvent.metHBHEisoNoise, &fInEvent.b_metHBHEisoNoise);
  fInTree->SetBranchAddress(fInEvent.s_metECALTP.c_str(), &fInEvent.metECALTP, &fInEvent.b_metECALTP);
  fInTree->SetBranchAddress(fInEvent.s_metPFMuon.c_str(), &fInEvent.metPFMuon, &fInEvent.b_metPFMuon);
  fInTree->SetBranchAddress(fInEvent.s_metPFChgHad.c_str(), &fInEvent.metPFChgHad, &fInEvent.b_metPFChgHad);
  fInTree->SetBranchAddress(fInEvent.s_metEESC.c_str(), &fInEvent.metEESC, &fInEvent.b_metEESC);
  fInTree->SetBranchAddress(fInEvent.s_metECALCalib.c_str(), &fInEvent.metECALCalib, &fInEvent.b_metECALCalib);

  fInTree->SetBranchAddress(fInEvent.s_nvtx.c_str(), &fInEvent.nvtx, &fInEvent.b_nvtx);
  fInTree->SetBranchAddress(fInEvent.s_vtxX.c_str(), &fInEvent.vtxX, &fInEvent.b_vtxX);
  fInTree->SetBranchAddress(fInEvent.s_vtxY.c_str(), &fInEvent.vtxY, &fInEvent.b_vtxY);
  fInTree->SetBranchAddress(fInEvent.s_vtxZ.c_str(), &fInEvent.vtxZ, &fInEvent.b_vtxZ);
  fInTree->SetBranchAddress(fInEvent.s_rho.c_str(), &fInEvent.rho, &fInEvent.b_rho);

  fInTree->SetBranchAddress(fInEvent.s_t1pfMETpt.c_str(), &fInEvent.t1pfMETpt, &fInEvent.b_t1pfMETpt);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETphi.c_str(), &fInEvent.t1pfMETphi, &fInEvent.b_t1pfMETphi);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETsumEt.c_str(), &fInEvent.t1pfMETsumEt, &fInEvent.b_t1pfMETsumEt);

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

  fInTree->SetBranchAddress(fInEvent.s_nrechits.c_str(), &fInEvent.nrechits, &fInEvent.b_nrechits);
  if (fInConfig.storeRecHits)
  {
    fInTree->SetBranchAddress(fInRecHits.s_eta.c_str(), &fInRecHits.eta, &fInRecHits.b_eta);
    fInTree->SetBranchAddress(fInRecHits.s_phi.c_str(), &fInRecHits.phi, &fInRecHits.b_phi);
    fInTree->SetBranchAddress(fInRecHits.s_E.c_str(), &fInRecHits.E, &fInRecHits.b_E);
    fInTree->SetBranchAddress(fInRecHits.s_time.c_str(), &fInRecHits.time, &fInRecHits.b_time);
    fInTree->SetBranchAddress(fInRecHits.s_OOT.c_str(), &fInRecHits.OOT, &fInRecHits.b_OOT);
    fInTree->SetBranchAddress(fInRecHits.s_ID.c_str(), &fInRecHits.ID, &fInRecHits.b_ID);
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
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sipip.c_str(),ipho), &pho.sipip, &pho.b_sipip);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sieip.c_str(),ipho), &pho.sieip, &pho.b_sieip);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_e2x2.c_str(),ipho), &pho.e2x2, &pho.b_e2x2);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_e3x3.c_str(),ipho), &pho.e3x3, &pho.b_e3x3);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_e5x5.c_str(),ipho), &pho.e5x5, &pho.b_e5x5);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_smaj.c_str(),ipho), &pho.smaj, &pho.b_smaj);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_smin.c_str(),ipho), &pho.smin, &pho.b_smin);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_alpha.c_str(),ipho), &pho.alpha, &pho.b_alpha);
    if (fInConfig.storeRecHits)
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seed.c_str(),ipho), &pho.seed, &pho.b_seed);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_recHits.c_str(),ipho), &pho.recHits, &pho.b_recHits);
    }
    else
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedtime.c_str(),ipho), &pho.seedtime, &pho.b_seedtime);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedE.c_str(),ipho), &pho.seedE, &pho.b_seedE);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedID.c_str(),ipho), &pho.seedID, &pho.b_seedID);
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
  fOutConfigTree->Branch(fOutConfig.s_splitPho.c_str(), &fOutConfig.splitPho);
  fOutConfigTree->Branch(fOutConfig.s_onlyGED.c_str(), &fOutConfig.onlyGED);
  fOutConfigTree->Branch(fOutConfig.s_onlyOOT.c_str(), &fOutConfig.onlyOOT);
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
  fOutConfig.splitPho = fInConfig.splitPho;
  fOutConfig.onlyGED = fInConfig.onlyGED;
  fOutConfig.onlyOOT = fInConfig.onlyOOT;
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
  fOutTree->Branch(fOutEvent.s_hltDiEle33MW.c_str(), &fOutEvent.hltDiEle33MW);
  fOutTree->Branch(fOutEvent.s_hltDiEle27WPT.c_str(), &fOutEvent.hltDiEle27WPT);
  fOutTree->Branch(fOutEvent.s_hltJet500.c_str(), &fOutEvent.hltJet500);

  fOutTree->Branch(fOutEvent.s_nvtx.c_str(), &fOutEvent.nvtx);
  fOutTree->Branch(fOutEvent.s_vtxX.c_str(), &fOutEvent.vtxX);
  fOutTree->Branch(fOutEvent.s_vtxY.c_str(), &fOutEvent.vtxY);
  fOutTree->Branch(fOutEvent.s_vtxZ.c_str(), &fOutEvent.vtxZ);
  fOutTree->Branch(fOutEvent.s_rho.c_str(), &fOutEvent.rho);

  fOutTree->Branch(fOutEvent.s_t1pfMETpt.c_str(), &fOutEvent.t1pfMETpt);
  fOutTree->Branch(fOutEvent.s_t1pfMETphi.c_str(), &fOutEvent.t1pfMETphi);
  fOutTree->Branch(fOutEvent.s_t1pfMETsumEt.c_str(), &fOutEvent.t1pfMETsumEt);

  fOutTree->Branch(fOutEvent.s_njets.c_str(), &fOutEvent.njets);
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

  fOutTree->Branch(fOutEvent.s_nrechits.c_str(), &fOutEvent.nrechits);

  fOutTree->Branch(fOutEvent.s_nphotons.c_str(), &fOutEvent.nphotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++) 
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
    fOutTree->Branch(Form("%s_%i",pho.s_sipip.c_str(),ipho), &pho.sipip);
    fOutTree->Branch(Form("%s_%i",pho.s_sieip.c_str(),ipho), &pho.sieip);
    fOutTree->Branch(Form("%s_%i",pho.s_e2x2.c_str(),ipho), &pho.e2x2);
    fOutTree->Branch(Form("%s_%i",pho.s_e3x3.c_str(),ipho), &pho.e3x3);
    fOutTree->Branch(Form("%s_%i",pho.s_e5x5.c_str(),ipho), &pho.e5x5);
    fOutTree->Branch(Form("%s_%i",pho.s_smaj.c_str(),ipho), &pho.smaj);
    fOutTree->Branch(Form("%s_%i",pho.s_smin.c_str(),ipho), &pho.smin);
    fOutTree->Branch(Form("%s_%i",pho.s_alpha.c_str(),ipho), &pho.alpha);
    fOutTree->Branch(Form("%s_%i",pho.s_suisseX.c_str(),ipho), &pho.suisseX);
    fOutTree->Branch(Form("%s_%i",pho.s_seedtime.c_str(),ipho), &pho.seedtime);
    fOutTree->Branch(Form("%s_%i",pho.s_seedE.c_str(),ipho), &pho.seedE);
    fOutTree->Branch(Form("%s_%i",pho.s_seedID.c_str(),ipho), &pho.seedID);;
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

    // add event weight
    fOutTree->Branch(fOutEvent.s_evtwgt.c_str(), &fOutEvent.evtwgt);
  }
} 

void Skimmer::InitOutCutFlowHists()
{
  Skimmer::InitOutCutFlowHist(fInCutFlow,fOutCutFlow,"");
  //  Skimmer::InitOutCutFlowHist(fInCutFlowWgt,fOutCutFlowWgt,"_wgt");
  //  Skimmer::InitOutCutFlowHist(fInCutFlowWgt,fOutCutFlowScl,"_scaled");
  //  fOutCutFlowScl->Scale(fSampleWeight);
  //  fOutCutFlowScl->SetTitle("Scaled");
  //  fOutCutFlowScl->GetYaxis()->SetTitle("nEvents");
}

void Skimmer::InitOutCutFlowHist(const TH1F * inh_cutflow, TH1F *& outh_cutflow, const TString & label)
{
  // get cut flow labels
  const auto inNbinsX = inh_cutflow->GetNbinsX();
  for (auto ibin = 1; ibin <= inNbinsX; ibin++)
  {
    cutLabels[inh_cutflow->GetXaxis()->GetBinLabel(ibin)] = ibin;
  }
  auto inNbinsX_new = inNbinsX;
  cutLabels["nPhotons"] = ++inNbinsX_new;
  cutLabels["ph0isEB"] = ++inNbinsX_new;
  cutLabels["ph0pt70"] = ++inNbinsX_new;
  cutLabels["METFlag"] = ++inNbinsX_new;
  cutLabels["badPU"] = ++inNbinsX_new;

  // make new cut flow
  outh_cutflow = new TH1F(Form("%s%s",Common::h_cutflowname.Data(),label.Data()),inh_cutflow->GetTitle(),cutLabels.size(),0,cutLabels.size());
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
  fSampleWeight = (fIsMC ? fInConfig.xsec * fInConfig.filterEff * Common::lumi * Common::invfbToinvpb / fSumWgts : 1.f); 
}

void Skimmer::GetPUWeights()
{
  for (auto ibin = 1; ibin <= fInPUWgtHist->GetNbinsX(); ibin++)
  {
    fPUWeights.emplace_back(fInPUWgtHist->GetBinContent(ibin));
  }
}

/////////////////////
//                 //
// Effective Areas //
//                 //
/////////////////////

Float_t Skimmer::GetChargedHadronEA(const Float_t eta)
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
  
Float_t Skimmer::GetNeutralHadronEA(const Float_t eta) 
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
  
Float_t Skimmer::GetGammaEA(const Float_t eta) 
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

Float_t Skimmer::GetEcalPFClEA(const Float_t eta)
{
  if   (eta < Common::etaEBcutoff) return 0.167;
  else                             return 0.f;
}

Float_t Skimmer::GetHcalPFClEA(const Float_t eta)
{
  if   (eta < Common::etaEBcutoff) return 0.108;
  else                             return 0.f;
}

Float_t Skimmer::GetTrackEA(const Float_t eta)
{
  if   (eta < Common::etaEBcutoff) return 0.113;
  else                             return 0.f;
}

////////////////
//            //
// pT scaling //
//            //
////////////////

Float_t Skimmer::GetNeutralHadronPtScale(const Float_t eta, const Float_t pt)
{
  if      (eta <  Common::etaEBcutoff)                           return 0.0126*pt+0.000026*pt*pt;
  else if (eta >= Common::etaEBcutoff && eta < Common::etaEEmax) return 0.0119*pt+0.000025*pt*pt;
  else                                                           return 0.f;
}

Float_t Skimmer::GetGammaPtScale(const Float_t eta, const Float_t pt)
{
  if      (eta <  Common::etaEBcutoff)                           return 0.0035*pt;
  else if (eta >= Common::etaEBcutoff && eta < Common::etaEEmax) return 0.0040*pt;
  else                                                           return 0.f;
}

Float_t Skimmer::GetEcalPFClPtScale(const Float_t eta, const Float_t pt)
{
  if   (eta <  Common::etaEBcutoff) return 0.0028*pt;
  else                              return 0.f;
}

Float_t Skimmer::GetHcalPFClPtScale(const Float_t eta, const Float_t pt)
{
  if   (eta <  Common::etaEBcutoff) return 0.0087*pt;
  else                              return 0.f;
}

Float_t Skimmer::GetTrackPtScale(const Float_t eta, const Float_t pt)
{
  if   (eta <  Common::etaEBcutoff) return 0.0056*pt;
  else                              return 0.f;
}

////////////////////
//                //
// GED Photon VID //
//                //
////////////////////

Int_t Skimmer::GetGEDPhoVID(const Pho & outpho)
{
  // needed for cuts
  const Float_t eta = std::abs(outpho.sceta);
  const Float_t pt  = outpho.pt;
  
  // cut variables
  const Float_t HoverE    = outpho.HoE;
  const Float_t Sieie     = outpho.sieie;
  const Float_t ChgHadIso = std::max(outpho.ChgHadIso - (fOutEvent.rho * Skimmer::GetChargedHadronEA(eta))                                             ,0.f);
  const Float_t NeuHadIso = std::max(outpho.NeuHadIso - (fOutEvent.rho * Skimmer::GetNeutralHadronEA(eta)) - (Skimmer::GetNeutralHadronPtScale(eta,pt)),0.f);
  const Float_t PhoIso    = std::max(outpho.PhoIso    - (fOutEvent.rho * Skimmer::GetGammaEA        (eta)) - (Skimmer::GetGammaPtScale        (eta,pt)),0.f);
  
  if (eta < Common::etaEBcutoff)
  {
    if      ((HoverE < 0.020) && (Sieie < 0.0103) && (ChgHadIso < 1.158) && (NeuHadIso < 1.267) && (PhoIso < 2.065)) 
    {
      return 3;
    }
    else if ((HoverE < 0.035) && (Sieie < 0.0103) && (ChgHadIso < 1.416) && (NeuHadIso < 2.491) && (PhoIso < 2.952)) 
    {
      return 2;
    }   
    else if ((HoverE < 0.105) && (Sieie < 0.0103) && (ChgHadIso < 2.839) && (NeuHadIso < 9.188) && (PhoIso < 2.956)) 
    {
      return 1;
    } 
    else
    {
      return 0;
    }
  }
  else if (eta >= Common::etaEBcutoff && eta < Common::etaEEmax)
  {
    if      ((HoverE < 0.025) && (Sieie < 0.0271) && (ChgHadIso < 0.575) && (NeuHadIso < 8.916) && (PhoIso < 3.272)) 
    {
      return 3;
    }
    else if ((HoverE < 0.027) && (Sieie < 0.0271) && (ChgHadIso < 1.012) && (NeuHadIso < 9.131) && (PhoIso < 4.095)) 
    {
      return 2;
    }   
    else if ((HoverE < 0.029) && (Sieie < 0.0276) && (ChgHadIso < 2.150) && (NeuHadIso < 10.471) && (PhoIso < 4.895)) 
    {
      return 1;
    }   
    else
    {
      return 0;
    }
  }
  else
  {
    return -1;
  }
}

///////////////////
//               //
// OOT Photon ID //
//               //
///////////////////

Int_t Skimmer::GetOOTPhoVID(const Pho & outpho)
{
  // needed for cuts
  const Float_t eta = std::abs(outpho.sceta);
  const Float_t pt  = outpho.pt;
  
  // cut variables
  const Float_t HoverE      = outpho.HoE;
  const Float_t Sieie       = outpho.sieie;
  const Float_t EcalPFClIso = std::max(outpho.EcalPFClIso - (fOutEvent.rho * Skimmer::GetEcalPFClEA(eta)) - (Skimmer::GetEcalPFClPtScale(eta,pt)),0.f);
  const Float_t HcalPFClIso = std::max(outpho.HcalPFClIso - (fOutEvent.rho * Skimmer::GetHcalPFClEA(eta)) - (Skimmer::GetHcalPFClPtScale(eta,pt)),0.f);
  const Float_t TrkIso      = std::max(outpho.TrkIso      - (fOutEvent.rho * Skimmer::GetTrackEA   (eta)) - (Skimmer::GetTrackPtScale   (eta,pt)),0.f);
  
  if (eta < Common::etaEBcutoff)
  {
    if      ((HoverE < 0.020) && (Sieie < 0.0103) && (EcalPFClIso < 2.f) && (HcalPFClIso < 5.f) && (TrkIso < 3.f)) 
    {
      return 3;
    }   
    else if ((HoverE < 0.105) && (Sieie < 0.0103) && (EcalPFClIso < 5.f) && (HcalPFClIso < 10.f) && (TrkIso < 6.f)) 
    {
      return 1;
    }   
    else
    {
      return 0;
    }
  }
  else if (eta >= Common::etaEBcutoff && eta < Common::etaEEmax)
  {
    if      ((HoverE < 0.025) && (Sieie < 0.0271) && (EcalPFClIso < 2.f) && (HcalPFClIso < 5.f) && (TrkIso < 3.f)) 
    {
      return 3;
    }   
    else if ((HoverE < 0.029) && (Sieie < 0.0276) && (EcalPFClIso < 5.f) && (HcalPFClIso < 10.f) && (TrkIso < 6.f)) 
    {
      return 1;
    }   
    else
    {
      return 0;
    }
  }
  else
  {
    return -1;
  }
}
