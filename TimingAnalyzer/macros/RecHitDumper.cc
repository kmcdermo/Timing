#include "PlotConfig.hh"
#include "RecHitDumper.hh"
#include "TSystem.h"

#include <iostream>

RecHitDumper::RecHitDumper(TString filename, TString outdir,
			   TString ph1config, TString phanyconfig, TString photonconfig) :
  fOutDir(outdir)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  // initialize tree
  RecHitDumper::InitTree();

  // in routine initialization from config files
  RecHitDumper::InitPh1Config(ph1config);
  RecHitDumper::InitPhAnyConfig(phanyconfig);
  RecHitDumper::InitPhotonConfig(photonconfig);

  // make the vid maps!
  fPhVIDMap["none"]   = 0;
  fPhVIDMap["loose"]  = 1;
  fPhVIDMap["medium"] = 2;
  fPhVIDMap["tight"]  = 3;

  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());
  }
}

RecHitDumper::~RecHitDumper(){}

void RecHitDumper::DoDump(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed)
{
  RecHitDumper::SetupFiles(allph,leading,mostdelayed);
  RecHitDumper::EventLoop(allph,leading,mostdelayed);
  RecHitDumper::SaveFiles(allph,leading,mostdelayed);
}

void RecHitDumper::SetupFiles(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed)
{
  if (allph) 
  {
    fSeedDumpAll.open(Form("%s/seeddump-allphotons.txt",fOutDir.Data()),std::ios_base::trunc);
    fRHDumpAll  .open(Form("%s/rhdump-allphotons.txt"  ,fOutDir.Data()),std::ios_base::trunc);
  }
  if (leading) 
  {
    fSeedDumpLeading.open(Form("%s/seeddump-leadingphoton.txt",fOutDir.Data()),std::ios_base::trunc);
    fRHDumpLeading  .open(Form("%s/rhdump-leadingphoton.txt"  ,fOutDir.Data()),std::ios_base::trunc);
  }
  if (mostdelayed) 
  {
    fSeedDumpMostDelayed.open(Form("%s/seeddump-mostdelayedphoton.txt",fOutDir.Data()),std::ios_base::trunc);
    fRHDumpMostDelayed  .open(Form("%s/rhdump-mostdelayedphoton.txt"  ,fOutDir.Data()),std::ios_base::trunc);
  }
}

void RecHitDumper::EventLoop(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed)
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    
    // standard printout
    if (entry%1000 == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    if (allph)       RecHitDumper::FillAllPhotons();
    if (leading)     RecHitDumper::FillLeadingPhoton();
    if (mostdelayed) RecHitDumper::FillMostDelayedPhoton();

  } // end loop over events
}

void RecHitDumper::FillAllPhotons()
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // eta specific selection
    const Float_t eta  = std::abs((*phsceta)[iph]);
    if (Config::ApplyECALAcceptCut && (((eta > 1.4442) && (eta < 1.566)) || (eta > 2.5))) continue;

    // no cuts on energy of recHits or on seed existing -- so be careful! 
    for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
    {
      fRHDumpAll << event << " " << iph << " " << irh << " " << (*phrhID)[iph][irh] << " " << (*phrhOOT)[iph][irh] << " " << (*phrhE)[iph][irh] << " " << (*phrhtime)[iph][irh] << std::endl;
      if (irh == (*phseedpos)[iph])
      {
	fSeedDumpAll << event << " " << iph << " " << irh << " " << (*phrhID)[iph][irh] << " " << (*phrhOOT)[iph][irh] << " " << (*phrhE)[iph][irh] << " " << (*phrhtime)[iph][irh] << std::endl;
      }
    }
  }
}

void RecHitDumper::FillLeadingPhoton()
{
  Int_t ph1 = RecHitDumper::GetLeadingPhoton();

  if (ph1 != -1)
  {
    for (Int_t irh = 0; irh < (*phnrh)[ph1]; irh++)
    {
      if (Config::ApplyrhECut && (*phrhE)[ph1][irh] < Config::rhECut) continue;
      fRHDumpLeading << event << " " << ph1 << " " << irh << " " << (*phrhID)[ph1][irh] << " " << (*phrhOOT)[ph1][irh] << " " << (*phrhE)[ph1][irh] << " " << (*phrhtime)[ph1][irh] << std::endl;
      if (irh == (*phseedpos)[ph1])
      {
	fSeedDumpLeading << event << " " << ph1 << " " << irh << " " << (*phrhID)[ph1][irh] << " " << (*phrhOOT)[ph1][irh] << " " << (*phrhE)[ph1][irh] << " " << (*phrhtime)[ph1][irh] << std::endl;
      }
    }
  }
}

void RecHitDumper::FillMostDelayedPhoton()
{
  Int_t phdelay = RecHitDumper::GetMostDelayedPhoton();

  if (phdelay != -1)
  {
    for (Int_t irh = 0; irh < (*phnrh)[phdelay]; irh++)
    {
      if (Config::ApplyrhECut && (*phrhE)[phdelay][irh] < Config::rhECut) continue;
      fRHDumpMostDelayed << event << " " << phdelay << " " << irh << " " << (*phrhID)[phdelay][irh] << " " << (*phrhOOT)[phdelay][irh] << " " << (*phrhE)[phdelay][irh] << " " << (*phrhtime)[phdelay][irh] << std::endl;
      if (irh == (*phseedpos)[phdelay])
      {
	fSeedDumpMostDelayed << event << " " << phdelay << " " << irh << " " << (*phrhID)[phdelay][irh] << " " << (*phrhOOT)[phdelay][irh] << " " << (*phrhE)[phdelay][irh] << " " << (*phrhtime)[phdelay][irh] << std::endl;
      }
    }
  }
}

void RecHitDumper::SaveFiles(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed)
{
  if (allph) 
  {
    fSeedDumpAll.close();
    fRHDumpAll  .close();
  }
  if (leading) 
  {
    fSeedDumpLeading.close();
    fRHDumpLeading  .close();
  }
  if (mostdelayed) 
  {
    fSeedDumpMostDelayed.close();
    fRHDumpMostDelayed  .close();
  }
}

Int_t RecHitDumper::GetLeadingPhoton()
{
  // first ensure that at least one photon passes the analysis selection
  Int_t ph1 = -1;
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // basic photon selection
    if      (Config::ApplyPh1PtCut)
    {  
      if ((*phpt)[iph] < Config::Ph1PtCut) continue;
    }
    else if (Config::ApplyPhAnyPtCut) 
    {
      if ((*phpt)[iph] < Config::PhAnyPtCut) continue;
    }
    if (Config::ApplyPh1VIDCut && ((*phVID)[iph] < fPhVIDMap[Config::Ph1VID])) continue;
    if (Config::ApplyPh1R9Cut && ((*phr9)[iph] < Config::Ph1R9Cut)) continue;

    // eta specific selection
    const Float_t eta  = std::abs((*phsceta)[iph]);
    const Float_t smaj = (*phsmaj)[iph];
    const Float_t smin = (*phsmin)[iph];
    if (Config::ApplyECALAcceptCut && ((eta < 1.4442) || ((eta > 1.566) && (eta < 2.5))))
    {
      // EB/EE only selection
      if      (Config::ApplyEBOnly && (eta > 1.4442)) continue;
      else if (Config::ApplyEEOnly && ((eta < 1.566) || (eta > 2.5))) continue;

      // eta specific selection
      if      (eta < 1.4442) 
      {
	if ((Config::ApplyPh1SmajEBMin && (smaj < Config::Ph1SmajEBMin)) || (Config::ApplyPh1SmajEBMax && (smaj > Config::Ph1SmajEBMax))) continue;
	if ((Config::ApplyPh1SminEBMin && (smin < Config::Ph1SminEBMin)) || (Config::ApplyPh1SminEBMax && (smin > Config::Ph1SminEBMax))) continue;
      }
      else if ((eta > 1.566) && (eta < 2.5))
      {
	if ((Config::ApplyPh1SmajEEMin && (smaj < Config::Ph1SmajEEMin)) || (Config::ApplyPh1SmajEEMax && (smaj > Config::Ph1SmajEEMax))) continue;
	if ((Config::ApplyPh1SminEEMin && (smin < Config::Ph1SminEEMin)) || (Config::ApplyPh1SminEEMax && (smin > Config::Ph1SminEEMax))) continue;
      }
    }
    else continue; // photon SC totally outside acceptance
    
    // photon seed crystal selection
    if ((*phseedpos)[iph] == -9999) continue;
    if (Config::ApplyrhECut && ((*phrhE)[iph][(*phseedpos)[iph]] < Config::rhECut)) continue;

    // MC matching selection
    if (Config::ApplyPhMCMatchingCut)
    {
      if      (Config::ApplyExactPhMCMatch)
      {
	if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] <= 0)) continue; // set to <= 0 for exact matching 
	else if (fIsBkg && ((*phisMatched)[iph] == 0)) continue; // set to == 0 for exact matching
      }
      else if (Config::ApplyAntiPhMCMatch)
      {
	if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] > 0)) continue; // set to <=0 for exact matching
	else if (fIsBkg && ((*phisMatched)[iph] != 0)) continue; // set to != 0 for anti-matching
      }
    }

    // Photons ordered in pT, so save index of first photon passing
    ph1 = iph; 
    break;
  } // end loop over nphotons
  return ph1;
}

Int_t RecHitDumper::GetGoodPhotons(std::vector<Int_t> & goodphotons)
{
  const Int_t ph1 = RecHitDumper::GetLeadingPhoton();
  if (ph1 != -1)
  {
    for (Int_t iph = 0; iph < nphotons; iph++) // remove pt cut
    {
      // basic photon selection
      if (Config::ApplyPhAnyPtCut && ((*phpt)[iph] < Config::PhAnyPtCut)) continue;
      if (Config::ApplyPhAnyVIDCut && ((*phVID)[iph] < fPhVIDMap[Config::PhAnyVID])) continue;
      if (Config::ApplyPhAnyR9Cut && ((*phr9)[iph] < Config::PhAnyR9Cut)) continue;

      // eta specific selection
      const Float_t eta  = std::abs((*phsceta)[iph]);
      const Float_t smaj = (*phsmaj)[iph];
      const Float_t smin = (*phsmin)[iph];
      if (Config::ApplyECALAcceptCut && ((eta < 1.4442) || ((eta > 1.566) && (eta < 2.5))))
      {
	// EB/EE only selection
	if      (Config::ApplyEBOnly && (eta > 1.4442)) continue;
	else if (Config::ApplyEEOnly && ((eta < 1.566) || (eta > 2.5))) continue;

	// eta specific selection
	if      (eta < 1.4442) 
        {
	  if ((Config::ApplyPhAnySmajEBMin && (smaj < Config::PhAnySmajEBMin)) || (Config::ApplyPhAnySmajEBMax && (smaj > Config::PhAnySmajEBMax))) continue;
	  if ((Config::ApplyPhAnySminEBMin && (smin < Config::PhAnySminEBMin)) || (Config::ApplyPhAnySminEBMax && (smin > Config::PhAnySminEBMax))) continue;
	}
	else if ((eta > 1.566) && (eta < 2.5))
        {
	  if ((Config::ApplyPhAnySmajEEMin && (smaj < Config::PhAnySmajEEMin)) || (Config::ApplyPhAnySmajEEMax && (smaj > Config::PhAnySmajEEMax))) continue;
	  if ((Config::ApplyPhAnySminEEMin && (smin < Config::PhAnySminEEMin)) || (Config::ApplyPhAnySminEEMax && (smin > Config::PhAnySminEEMax))) continue;
	}
      }
      else continue; // photon SC totally outside acceptance
    
      // photon seed crystal selection
      if ((*phseedpos)[iph] == -9999) continue;
      if (Config::ApplyrhECut && ((*phrhE)[iph][(*phseedpos)[iph]] < Config::rhECut)) continue;

      // MC matching selection
      if (Config::ApplyPhMCMatchingCut)
      {
	if      (Config::ApplyExactPhMCMatch)
        {
	  if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] <= 0)) continue; // set to <= 0 for exact matching 
	  else if (fIsBkg && ((*phisMatched)[iph] == 0)) continue; // set to == 0 for exact matching
	}
	else if (Config::ApplyAntiPhMCMatch)
        {
	  if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] > 0)) continue; // set to <=0 for exact matching
	  else if (fIsBkg && ((*phisMatched)[iph] != 0)) continue; // set to != 0 for anti-matching
	}
      }

      // Save photon index
      goodphotons.push_back(iph);
    } // end loop over nphotons
  } // end check over leading photon
  return ph1;
}

Int_t RecHitDumper::GetMostDelayedPhoton()
{
  std::vector<Int_t> goodphotons;
  Int_t phdelay = RecHitDumper::GetGoodPhotons(goodphotons);
  for (UInt_t gph = 0; gph < goodphotons.size(); gph++)
  {
    const Int_t iph = goodphotons[gph];
    if ((*phrhtime)[iph][(*phseedpos)[iph]] > (*phrhtime)[phdelay][(*phseedpos)[phdelay]]) phdelay = iph;
  }
  return phdelay;
}

void RecHitDumper::InitTree()
{
  // Set object pointer
  phpt = 0;
  phsceta = 0;
  phr9 = 0;
  phsmaj = 0;
  phsmin = 0;
  phVID = 0;
  phnrh = 0;
  phseedpos = 0;
  phrhE = 0;
  phrhtime = 0;
  phrhID = 0;
  phrhOOT = 0;
 
  // set branch addresses
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("phsceta", &phsceta, &b_phsceta);
  fInTree->SetBranchAddress("phr9", &phr9, &b_phr9);
  fInTree->SetBranchAddress("phsmaj", &phsmaj, &b_phsmaj);
  fInTree->SetBranchAddress("phsmin", &phsmin, &b_phsmin);
  fInTree->SetBranchAddress("phVID", &phVID, &b_phVID);
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrhE", &phrhE, &b_phrhE);
  fInTree->SetBranchAddress("phrhtime", &phrhtime, &b_phrhtime);
  fInTree->SetBranchAddress("phrhID", &phrhID, &b_phrhID);
  fInTree->SetBranchAddress("phrhOOT", &phrhOOT, &b_phrhOOT);
}

void RecHitDumper::InitPh1Config(TString config)
{
  std::ifstream input;
  input.open(config.Data(),std::ios::in);
  
  TString var = "", val = "";
  while (input >> var >> val)
  {
    if (var.EqualTo("ApplyPh1PtCut")) Config::ApplyPh1PtCut = val.Atoi();
    if (var.EqualTo("Ph1PtCut")) Config::Ph1PtCut = val.Atof();
    if (var.EqualTo("ApplyPh1VIDCut")) Config::ApplyPh1VIDCut = val.Atoi();
    if (var.EqualTo("Ph1VID")) Config::Ph1VID = val;
    if (var.EqualTo("ApplyPh1R9Cut")) Config::ApplyPh1R9Cut = val.Atoi();
    if (var.EqualTo("Ph1R9Cut")) Config::Ph1R9Cut = val.Atof();
    if (var.EqualTo("ApplyPh1SmajEBMin")) Config::ApplyPh1SmajEBMin = val.Atoi();
    if (var.EqualTo("Ph1SmajEBMin")) Config::Ph1SmajEBMin = val.Atof();
    if (var.EqualTo("ApplyPh1SmajEBMax")) Config::ApplyPh1SmajEBMax = val.Atoi();
    if (var.EqualTo("Ph1SmajEBMax")) Config::Ph1SmajEBMax = val.Atof();
    if (var.EqualTo("ApplyPh1SmajEEMin")) Config::ApplyPh1SmajEEMin = val.Atoi();
    if (var.EqualTo("Ph1SmajEEMin")) Config::Ph1SmajEEMin = val.Atof();
    if (var.EqualTo("ApplyPh1SmajEEMax")) Config::ApplyPh1SmajEEMax = val.Atoi();
    if (var.EqualTo("Ph1SmajEEMax")) Config::Ph1SmajEEMax = val.Atof();
    if (var.EqualTo("ApplyPh1SminEBMin")) Config::ApplyPh1SminEBMin = val.Atoi();
    if (var.EqualTo("Ph1SminEBMin")) Config::Ph1SminEBMin = val.Atof();
    if (var.EqualTo("ApplyPh1SminEBMax")) Config::ApplyPh1SminEBMax = val.Atoi();
    if (var.EqualTo("Ph1SminEBMax")) Config::Ph1SminEBMax = val.Atof();
    if (var.EqualTo("ApplyPh1SminEEMin")) Config::ApplyPh1SminEEMin = val.Atoi();
    if (var.EqualTo("Ph1SminEEMin")) Config::Ph1SminEEMin = val.Atof();
    if (var.EqualTo("ApplyPh1SminEEMax")) Config::ApplyPh1SminEEMax = val.Atoi();
    if (var.EqualTo("Ph1SminEEMax")) Config::Ph1SminEEMax = val.Atof();
  }
}

void RecHitDumper::InitPhAnyConfig(TString config)
{
  std::ifstream input;
  input.open(config.Data(),std::ios::in);
  
  TString var = "", val = "";
  while (input >> var >> val)
  {
    if (var.EqualTo("ApplyPhAnyPtCut")) Config::ApplyPhAnyPtCut = val.Atoi();
    if (var.EqualTo("PhAnyPtCut")) Config::PhAnyPtCut = val.Atof();
    if (var.EqualTo("ApplyPhAnyVIDCut")) Config::ApplyPhAnyVIDCut = val.Atoi();
    if (var.EqualTo("PhAnyVID")) Config::PhAnyVID = val;
    if (var.EqualTo("ApplyPhAnyR9Cut")) Config::ApplyPhAnyR9Cut = val.Atoi();
    if (var.EqualTo("PhAnyR9Cut")) Config::PhAnyR9Cut = val.Atof();
    if (var.EqualTo("ApplyPhAnySmajEBMin")) Config::ApplyPhAnySmajEBMin = val.Atoi();
    if (var.EqualTo("PhAnySmajEBMin")) Config::PhAnySmajEBMin = val.Atof();
    if (var.EqualTo("ApplyPhAnySmajEBMax")) Config::ApplyPhAnySmajEBMax = val.Atoi();
    if (var.EqualTo("PhAnySmajEBMax")) Config::PhAnySmajEBMax = val.Atof();
    if (var.EqualTo("ApplyPhAnySmajEEMin")) Config::ApplyPhAnySmajEEMin = val.Atoi();
    if (var.EqualTo("PhAnySmajEEMin")) Config::PhAnySmajEEMin = val.Atof();
    if (var.EqualTo("ApplyPhAnySmajEEMax")) Config::ApplyPhAnySmajEEMax = val.Atoi();
    if (var.EqualTo("PhAnySmajEEMax")) Config::PhAnySmajEEMax = val.Atof();
    if (var.EqualTo("ApplyPhAnySminEBMin")) Config::ApplyPhAnySminEBMin = val.Atoi();
    if (var.EqualTo("PhAnySminEBMin")) Config::PhAnySminEBMin = val.Atof();
    if (var.EqualTo("ApplyPhAnySminEBMax")) Config::ApplyPhAnySminEBMax = val.Atoi();
    if (var.EqualTo("PhAnySminEBMax")) Config::PhAnySminEBMax = val.Atof();
    if (var.EqualTo("ApplyPhAnySminEEMin")) Config::ApplyPhAnySminEEMin = val.Atoi();
    if (var.EqualTo("PhAnySminEEMin")) Config::PhAnySminEEMin = val.Atof();
    if (var.EqualTo("ApplyPhAnySminEEMax")) Config::ApplyPhAnySminEEMax = val.Atoi();
    if (var.EqualTo("PhAnySminEEMax")) Config::PhAnySminEEMax = val.Atof();
  }
}

void RecHitDumper::InitPhotonConfig(TString config)
{
  std::ifstream input;
  input.open(config.Data(),std::ios::in);
  
  TString var = "", val = "";
  while (input >> var >> val)
  {
    if (var.EqualTo("ApplyrhECut")) Config::ApplyrhECut = val.Atoi();
    if (var.EqualTo("rhECut")) Config::rhECut = val.Atof();
    if (var.EqualTo("ApplyECALAcceptCut")) Config::ApplyECALAcceptCut = val.Atoi();
    if (var.EqualTo("ApplyEBOnly")) Config::ApplyEBOnly = val.Atoi();
    if (var.EqualTo("ApplyEEOnly")) Config::ApplyEEOnly = val.Atoi();
    if (var.EqualTo("ApplyPhMCMatchingCut")) Config::ApplyPhMCMatchingCut = val.Atoi();
    if (var.EqualTo("ApplyExactPhMCMatch")) Config::ApplyExactPhMCMatch = val.Atoi();
    if (var.EqualTo("ApplyAntiPhMCMatch")) Config::ApplyAntiPhMCMatch = val.Atoi();
  }
}
