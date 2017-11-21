#include "Skimmer.hh"
#include "TROOT.h"

Skimmer::Skimmer(const TString & sample, const Bool_t isMC) : fSample(sample), fIsMC(isMC)
{
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

  const TString indir = "/afs/cern.ch/work/k/kmcdermo/public";

  // Get input
  const TString filename = Form("%s/input/%i/%s/%s/%s", indir.Data(), Config::year, (fIsMC?"MC":"DATA"), fSample.Data(), Config::nTupleName.Data());
  fInFile  = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  // Get main tree and initialize everything
  const TString treename = "tree/tree";
  fInTree = (TTree*)fInFile->Get(treename.Data());
  CheckValidTree(fInTree,treename,filename);
  Skimmer::InitTree();

  // Get config tree, initialize it, and set it to read the first entry!
  const TString configtreename = "tree/configtree";
  fConfigTree = (TTree*)fInFile->Get(configtreename.Data());
  CheckValidTree(fConfigTree,configtreename,filename);
  Skimmer::InitAndReadConfigTree();

  // Get the cut flow + event weight histogram
  const TString histname = "tree/h_cutflow";
  fCutFlow = (TH1F*)fInFile->Get(histname.Data());
  CheckValidTH1F(fCutFlow,histname,filename);

  // Set Output Stuff
  fOutDir = Form("%s/%i/%s/%s", Config::outdir.Data(), Config::year, (fIsMC?"MC":"DATA"), fSample.Data());
  MakeOutDir(fOutDir);
  fOutFile = new TFile(Form("%s/%s",fOutDir.Data(),Config::AnOutName.Data()),"UPDATE");
  fColor = (fIsMC?Config::ColorMap[fSample]:kBlack);
}

Skimmer::~Skimmer()
{
  delete fInTree;
  delete fInFile;
  delete fOutFile;
}

void Skimmer::EventLoop()
{
  // do loop over events, filling histos
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    // read in tree
    fInTree->GetEntry(entry);

    // dump status check
    if (entry%Config::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;

  } // end loop over events
}



void Skimmer::InitAndReadConfigTree()
{
  Skimmer::InitConfigStrings();
  Skimmer::InitConfigBranches();

  // read in first entry (will be the same for all entries in a given file
  fConfigTree->GetEntry(0);
}

void Skimmer::InitConfigStrings()
{
  phIDmin = 0;
  phgoodIDmin = 0;
  inputPaths = 0;
  inputFilters = 0;
}

void Skimmer::InitConfigBranches()
{
  fConfigTree->SetBranchAddress("blindSF", &blindSF, &b_blindSF);
  fConfigTree->SetBranchAddress("applyBlindSF", &applyBlindSF, &b_applyBlindSF);
  fConfigTree->SetBranchAddress("blindMET", &blindMET, &b_blindMET);
  fConfigTree->SetBranchAddress("applyBlindMET", &applyBlindMET, &b_applyBlindMET);
  fConfigTree->SetBranchAddress("jetpTmin", &jetpTmin, &b_jetpTmin);
  fConfigTree->SetBranchAddress("jetIDmin", &jetIDmin, &b_jetIDmin);
  fConfigTree->SetBranchAddress("rhEmin", &rhEmin, &b_rhEmin);
  fConfigTree->SetBranchAddress("phpTmin", &phpTmin, &b_phpTmin);
  fConfigTree->SetBranchAddress("phIDmin", &phIDmin, &b_phIDmin);
  fConfigTree->SetBranchAddress("seedTimemin", &seedTimemin, &b_seedTimemin);
  fConfigTree->SetBranchAddress("splitPho", &splitPho, &b_splitPho);
  fConfigTree->SetBranchAddress("onlyGED", &onlyGED, &b_onlyGED);
  fConfigTree->SetBranchAddress("onlyOOT", &onlyOOT, &b_onlyOOT);
  fConfigTree->SetBranchAddress("applyTrigger", &applyTrigger, &b_applyTrigger);
  fConfigTree->SetBranchAddress("minHT", &minHT, &b_minHT);
  fConfigTree->SetBranchAddress("applyHT", &applyHT, &b_applyHT);
  fConfigTree->SetBranchAddress("phgoodpTmin", &phgoodpTmin, &b_phgoodpTmin);
  fConfigTree->SetBranchAddress("phgoodIDmin", &phgoodIDmin, &b_phgoodIDmin);
  fConfigTree->SetBranchAddress("applyPhGood", &applyPhGood, &b_applyPhGood);
  fConfigTree->SetBranchAddress("dRmin", &dRmin, &b_dRmin);
  fConfigTree->SetBranchAddress("pTres", &pTres, &b_pTres);
  fConfigTree->SetBranchAddress("trackdRmin", &trackdRmin, &b_trackdRmin);
  fConfigTree->SetBranchAddress("trackpTmin", &trackpTmin, &b_trackpTmin);
  fConfigTree->SetBranchAddress("inputPaths", &inputPaths, &b_inputPaths);
  fConfigTree->SetBranchAddress("inputFilters", &inputFilters, &b_inputFilters);

  if (fIsMC)
  {
    fConfigTree->SetBranchAddress("isGMSB", &isGMSB, &b_isGMSB);
    fConfigTree->SetBranchAddress("isHVDS", &isHVDS, &b_isHVDS);
    fConfigTree->SetBranchAddress("isBkgd", &isBkgd, &b_isBkgd);
    fConfigTree->SetBranchAddress("xsec", &xsec, &b_xsec);
    fConfigTree->SetBranchAddress("filterEff", &filterEff, &b_filterEff);
  }
}



















void Skimmer::InitTree() 
{
  Skimmer::InitStructs();
  if (Config::readRecHits) Skimmer::InitBranchVecs();
  Skimmer::InitBranches();
}

void Skimmer::InitStructs()
{
  if (fIsMC)
  {
    if (fIsGMSB)
    {
      gmsbs.clear(); 
      gmsbs.resize(Config::nGMSBs);
    }
    if (fIsHVDS)
    {
      hvdss.clear(); 
      hvdss.resize(Config::nHVDSs);
    }
  }

  jets.clear();
  jets.resize(Config::nJets);

  phos.clear();
  phos.resize(Config::nTotalPhotons);
}

void Skimmer::InitBranchVecs()
{
  rheta = 0;
  rhphi = 0;
  rhE = 0;
  rhtime = 0;
  rhOOT = 0;
  rhID = 0;

  for (Int_t ipho = 0; ipho < Config::nTotalPhotons; ipho++) 
  {
    phos[ipho].recHits = 0;
  }  
}

void Skimmer::InitBranches()
{
  if (fIsMC)
  {
    fInTree->SetBranchAddress("genwgt", &genwgt, &b_genwgt);
    fInTree->SetBranchAddress("genpuobs", &genpuobs, &b_genpuobs);
    fInTree->SetBranchAddress("genputrue", &genputrue, &b_genputrue);
    
    if (fIsGMSB)
    {
      fInTree->SetBranchAddress("nNeutoPhGr", &nNeutoPhGr, &b_nNeutoPhGr);
      for (Int_t igmsb = 0; igmsb < Config::nGMSBs; igmsb++) 
      {
	auto & gmsb = gmsbs[igmsb]; 
	fInTree->SetBranchAddress(Form("genNmass_%i",igmsb), &gmsb.genNmass, &gmsb.b_genNmass);
	fInTree->SetBranchAddress(Form("genNE_%i",igmsb), &gmsb.genNE, &gmsb.b_genNE);
	fInTree->SetBranchAddress(Form("genNpt_%i",igmsb), &gmsb.genNpt, &gmsb.b_genNpt);
	fInTree->SetBranchAddress(Form("genNphi_%i",igmsb), &gmsb.genNphi, &gmsb.b_genNphi);
	fInTree->SetBranchAddress(Form("genNeta_%i",igmsb), &gmsb.genNeta, &gmsb.b_genNeta);
	fInTree->SetBranchAddress(Form("genNprodvx_%i",igmsb), &gmsb.genNprodvx, &gmsb.b_genNprodvx);
	fInTree->SetBranchAddress(Form("genNprodvy_%i",igmsb), &gmsb.genNprodvy, &gmsb.b_genNprodvy);
	fInTree->SetBranchAddress(Form("genNprodvz_%i",igmsb), &gmsb.genNprodvz, &gmsb.b_genNprodvz);
	fInTree->SetBranchAddress(Form("genNdecayvx_%i",igmsb), &gmsb.genNdecayvx, &gmsb.b_genNdecayvx);
	fInTree->SetBranchAddress(Form("genNdecayvy_%i",igmsb), &gmsb.genNdecayvy, &gmsb.b_genNdecayvy);
	fInTree->SetBranchAddress(Form("genNdecayvz_%i",igmsb), &gmsb.genNdecayvz, &gmsb.b_genNdecayvz);
	fInTree->SetBranchAddress(Form("genphE_%i",igmsb), &gmsb.genphE, &gmsb.b_genphE);
	fInTree->SetBranchAddress(Form("genphpt_%i",igmsb), &gmsb.genphpt, &gmsb.b_genphpt);
	fInTree->SetBranchAddress(Form("genphphi_%i",igmsb), &gmsb.genphphi, &gmsb.b_genphphi);
	fInTree->SetBranchAddress(Form("genpheta_%i",igmsb), &gmsb.genpheta, &gmsb.b_genpheta);
	fInTree->SetBranchAddress(Form("genphmatch_%i",igmsb), &gmsb.genphmatch, &gmsb.b_genphmatch);
	fInTree->SetBranchAddress(Form("gengrmass_%i",igmsb), &gmsb.gengrmass, &gmsb.b_gengrmass);
	fInTree->SetBranchAddress(Form("gengrE_%i",igmsb), &gmsb.gengrE, &gmsb.b_gengrE);
	fInTree->SetBranchAddress(Form("gengrpt_%i",igmsb), &gmsb.gengrpt, &gmsb.b_gengrpt);
	fInTree->SetBranchAddress(Form("gengrphi_%i",igmsb), &gmsb.gengrphi, &gmsb.b_gengrphi);
	fInTree->SetBranchAddress(Form("gengreta_%i",igmsb), &gmsb.gengreta, &gmsb.b_gengreta);
      } // end loop over neutralinos
    } // end block over gmsb

    if (fIsHVDS)
    {
      fInTree->SetBranchAddress("nvPions", &nvPions, &b_nvPions);
      for (Int_t ihvds = 0; ihvds < Config::nHVDSs; ihvds++) 
      {
	auto & hvds = hvdss[ihvds]; 
	fInTree->SetBranchAddress(Form("genvPionmass_%i",ihvds), &hvds.genvPionmass, &hvds.b_genvPionmass);
	fInTree->SetBranchAddress(Form("genvPionE_%i",ihvds), &hvds.genvPionE, &hvds.b_genvPionE);
	fInTree->SetBranchAddress(Form("genvPionpt_%i",ihvds), &hvds.genvPionpt, &hvds.b_genvPionpt);
	fInTree->SetBranchAddress(Form("genvPionphi_%i",ihvds), &hvds.genvPionphi, &hvds.b_genvPionphi);
	fInTree->SetBranchAddress(Form("genvPioneta_%i",ihvds), &hvds.genvPioneta, &hvds.b_genvPioneta);
	fInTree->SetBranchAddress(Form("genvPionprodvx_%i",ihvds), &hvds.genvPionprodvx, &hvds.b_genvPionprodvx);
	fInTree->SetBranchAddress(Form("genvPionprodvy_%i",ihvds), &hvds.genvPionprodvy, &hvds.b_genvPionprodvy);
	fInTree->SetBranchAddress(Form("genvPionprodvz_%i",ihvds), &hvds.genvPionprodvz, &hvds.b_genvPionprodvz);
	fInTree->SetBranchAddress(Form("genvPiondecayvx_%i",ihvds), &hvds.genvPiondecayvx, &hvds.b_genvPiondecayvx);
	fInTree->SetBranchAddress(Form("genvPiondecayvy_%i",ihvds), &hvds.genvPiondecayvy, &hvds.b_genvPiondecayvy);
	fInTree->SetBranchAddress(Form("genvPiondecayvz_%i",ihvds), &hvds.genvPiondecayvz, &hvds.b_genvPiondecayvz);
	fInTree->SetBranchAddress(Form("genHVph0E_%i",ihvds), &hvds.genHVph0E, &hvds.b_genHVph0E);
	fInTree->SetBranchAddress(Form("genHVph0pt_%i",ihvds), &hvds.genHVph0pt, &hvds.b_genHVph0pt);
	fInTree->SetBranchAddress(Form("genHVph0phi_%i",ihvds), &hvds.genHVph0phi, &hvds.b_genHVph0phi);
	fInTree->SetBranchAddress(Form("genHVph0eta_%i",ihvds), &hvds.genHVph0eta, &hvds.b_genHVph0eta);
	fInTree->SetBranchAddress(Form("genHVph0match_%i",ihvds), &hvds.genHVph0match, &hvds.b_genHVph0match);
	fInTree->SetBranchAddress(Form("genHVph1E_%i",ihvds), &hvds.genHVph1E, &hvds.b_genHVph1E);
	fInTree->SetBranchAddress(Form("genHVph1pt_%i",ihvds), &hvds.genHVph1pt, &hvds.b_genHVph1pt);
	fInTree->SetBranchAddress(Form("genHVph1phi_%i",ihvds), &hvds.genHVph1phi, &hvds.b_genHVph1phi);
	fInTree->SetBranchAddress(Form("genHVph1eta_%i",ihvds), &hvds.genHVph1eta, &hvds.b_genHVph1eta);
	fInTree->SetBranchAddress(Form("genHVph1match_%i",ihvds), &hvds.genHVph1match, &hvds.b_genHVph1match);
      } // end loop over nvpions 
    } // end block over hvds
  } // end block over isMC

  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("hltDisPho", &hltDisPho, &b_hltDisPho);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
  fInTree->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
  fInTree->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
  fInTree->SetBranchAddress("rho", &rho, &b_rho);
  fInTree->SetBranchAddress("t1pfMETpt", &t1pfMETpt, &b_t1pfMETpt);
  fInTree->SetBranchAddress("t1pfMETphi", &t1pfMETphi, &b_t1pfMETphi);
  fInTree->SetBranchAddress("t1pfMETsumEt", &t1pfMETsumEt, &b_t1pfMETsumEt);
  fInTree->SetBranchAddress("jetHT", &jetHT, &b_jetHT);

  fInTree->SetBranchAddress("njets", &njets, &b_njets);
  for (Int_t ijet = 0; ijet < Config::nJets; ijet++) 
  {
    auto & jet = jets[ijet];
    fInTree->SetBranchAddress(Form("jetE_%i",ijet), &jet.E, &jet.b_E);    
    fInTree->SetBranchAddress(Form("jetpt_%i",ijet), &jet.pt, &jet.b_pt);    
    fInTree->SetBranchAddress(Form("jetphi_%i",ijet), &jet.phi, &jet.b_phi);    
    fInTree->SetBranchAddress(Form("jeteta_%i",ijet), &jet.eta, &jet.b_eta);    
  }

  fInTree->SetBranchAddress("nrechits", &nrechits, &b_nrechits);
  if (Config::readRecHits)
  {
    fInTree->SetBranchAddress("rheta", &rheta, &b_rheta);
    fInTree->SetBranchAddress("rhphi", &rhphi, &b_rhphi);
    fInTree->SetBranchAddress("rhE", &rhE, &b_rhE);
    fInTree->SetBranchAddress("rhtime", &rhtime, &b_rhtime);
    fInTree->SetBranchAddress("rhOOT", &rhOOT, &b_rhOOT);
    fInTree->SetBranchAddress("rhID", &rhID, &b_rhID);
  }

  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  for (Int_t ipho = 0; ipho < Config::nTotalPhotons; ipho++) 
  {
    auto & pho = phos[ipho];
    fInTree->SetBranchAddress(Form("phoE_%i",ipho), &pho.E, &pho.b_E);
    fInTree->SetBranchAddress(Form("phopt_%i",ipho), &pho.pt, &pho.b_pt);
    fInTree->SetBranchAddress(Form("phoeta_%i",ipho), &pho.eta, &pho.b_eta);
    fInTree->SetBranchAddress(Form("phophi_%i",ipho), &pho.phi, &pho.b_phi);
    fInTree->SetBranchAddress(Form("phoscE_%i",ipho), &pho.scE, &pho.b_scE);
    fInTree->SetBranchAddress(Form("phosceta_%i",ipho), &pho.sceta, &pho.b_sceta);
    fInTree->SetBranchAddress(Form("phoscphi_%i",ipho), &pho.scphi, &pho.b_scphi);
    fInTree->SetBranchAddress(Form("phoHoE_%i",ipho), &pho.HoE, &pho.b_HoE);
    fInTree->SetBranchAddress(Form("phor9_%i",ipho), &pho.r9, &pho.b_r9);
    fInTree->SetBranchAddress(Form("phoChgHadIso_%i",ipho), &pho.ChgHadIso, &pho.b_ChgHadIso);
    fInTree->SetBranchAddress(Form("phoNeuHadIso_%i",ipho), &pho.NeuHadIso, &pho.b_NeuHadIso);
    fInTree->SetBranchAddress(Form("phoPhoIso_%i",ipho), &pho.PhoIso, &pho.b_PhoIso);
    fInTree->SetBranchAddress(Form("phoEcalPFClIso_%i",ipho), &pho.EcalPFClIso, &pho.b_EcalPFClIso);
    fInTree->SetBranchAddress(Form("phoHcalPFClIso_%i",ipho), &pho.HcalPFClIso, &pho.b_HcalPFClIso);
    fInTree->SetBranchAddress(Form("phoTrkIso_%i",ipho), &pho.TrkIso, &pho.b_TrkIso);
    fInTree->SetBranchAddress(Form("phosieie_%i",ipho), &pho.sieie, &pho.b_sieie);
    fInTree->SetBranchAddress(Form("phosipip_%i",ipho), &pho.sipip, &pho.b_sipip);
    fInTree->SetBranchAddress(Form("phosieip_%i",ipho), &pho.sieip, &pho.b_sieip);
    fInTree->SetBranchAddress(Form("phosmaj_%i",ipho), &pho.smaj, &pho.b_smaj);
    fInTree->SetBranchAddress(Form("phosmin_%i",ipho), &pho.smin, &pho.b_smin);
    fInTree->SetBranchAddress(Form("phoalpha_%i",ipho), &pho.alpha, &pho.b_alpha);
    if (Config::readRecHits)
    {
      fInTree->SetBranchAddress(Form("phoseed_%i",ipho), &pho.seed, &pho.b_seed);
      fInTree->SetBranchAddress(Form("phorecHits_%i",ipho), &pho.recHits, &pho.b_recHits);
    }
    else
    {
      fInTree->SetBranchAddress(Form("phoseedtime_%i",ipho), &pho.seedtime, &pho.b_seedtime);
      fInTree->SetBranchAddress(Form("phoseedE_%i",ipho), &pho.seedE, &pho.b_seedE);
      fInTree->SetBranchAddress(Form("phoseedID_%i",ipho), &pho.seedID, &pho.b_seedID);;
    }
    fInTree->SetBranchAddress(Form("phoisOOT_%i",ipho), &pho.isOOT, &pho.b_isOOT);
    fInTree->SetBranchAddress(Form("phoisEB_%i",ipho), &pho.isEB, &pho.b_isEB);
    fInTree->SetBranchAddress(Form("phoisHLT_%i",ipho), &pho.isHLT, &pho.b_isHLT);
    fInTree->SetBranchAddress(Form("phoisTrk_%i",ipho), &pho.isTrk, &pho.b_isTrk);
    fInTree->SetBranchAddress(Form("phoID_%i",ipho), &pho.ID, &pho.b_ID);
    
    if (fIsMC)
    {
      fInTree->SetBranchAddress(Form("phoisGen_%i",ipho), &pho.isGen, &pho.b_isGen);
      if (fIsGMSB || fIsHVDS)
      {
	fInTree->SetBranchAddress(Form("phoisSignal_%i",ipho), &pho.isSignal, &pho.b_isSignal);
      }
    }
  }
}

