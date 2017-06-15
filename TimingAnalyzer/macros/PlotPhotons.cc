#include "PlotConfig.hh"
#include "PlotPhotons.hh"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include "common/common.h"

#include <iostream>

PlotPhotons::PlotPhotons(TString filename, Bool_t isGMSB, Bool_t isHVDS, Bool_t isBkg, 
			 TString outdir, Bool_t savehists, Bool_t savesub,
			 TString genericconfig, TString hltconfig, TString jetconfig, 
			 TString ph1config, TString phanyconfig, TString photonconfig) :
  fOutDir(outdir), fIsGMSB(isGMSB), fIsHVDS(isHVDS), fIsBkg(isBkg), 
  fSaveHists(savehists), fSaveSub(savesub)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  if   (fIsGMSB || fIsHVDS || fIsBkg) fIsMC = true;
  else                                fIsMC = false;
  
  // in routine initialization from config files
  PlotPhotons::InitGenericConfig(genericconfig);
  PlotPhotons::InitHLTConfig(hltconfig);
  PlotPhotons::InitJetConfig(jetconfig);
  PlotPhotons::InitPh1Config(ph1config);
  PlotPhotons::InitPhAnyConfig(phanyconfig);
  PlotPhotons::InitPhotonConfig(photonconfig);

  // initialize tree
  PlotPhotons::InitTree();

  // make the vid maps!
  fPhVIDMap["none"]   = 0;
  fPhVIDMap["loose"]  = 1;
  fPhVIDMap["medium"] = 2;
  fPhVIDMap["tight"]  = 3;

  //ctau for plots
  fCTau = 1000;
  if (filename.Contains("gmsb",TString::kExact))
  {
    if      (filename.Contains("ctau100" ,TString::kExact)) fCTau = 3.65;  // 100  mm --> 36.5  mm == 3.65  cm
    else if (filename.Contains("ctau2000",TString::kExact)) fCTau = 73.05; // 2000 mm --> 730.5 mm == 73.5  cm
    else if (filename.Contains("ctau6000",TString::kExact)) fCTau = 219.2; // 6000 mm --> 2192  mm == 219.2 cm
    else    {std::cout << "GMSB: Need a valid ctau!" << std::endl; exit(0);}
  }
  else if (filename.Contains("hvds",TString::kExact))
  {
    if      (filename.Contains("ctau100" ,TString::kExact)) fCTau = 10.0;  // 100  mm
    else if (filename.Contains("ctau1000",TString::kExact)) fCTau = 100.0; // 1000 mm
    else    {std::cout << "HVDS: Need a valid ctau!" << std::endl; exit(0);}
  }

  // initialize efficiency map
  fEfficiency["Photons"] = 0;
  fEfficiency["HT"]      = 0;
  fEfficiency["Jets"]    = 0;
  fEfficiency["Jets+HT"] = 0;
  fEfficiency["Reco"]    = 0;
  fEfficiency["Trigger"] = 0;
  fEfficiency["Events"]  = 0;
  
  // output
  // setup outdir name
  fOutDir += "/cuts";
  if (Config::ApplyJetHtCut)    fOutDir += Form("_jetht%5.1f"   , Config::JetHtCut);
  if (Config::ApplyMinJetPtCut) fOutDir += Form("_minjetpt%4.1f", Config::MinJetPtCut);
  if (Config::ApplyJetPtCut)    fOutDir += Form("_jetpt%4.1f"   , Config::JetPtCut);
  if (Config::ApplyNJetsCut)    fOutDir += Form("_njets%i"      , Config::NJetsCut);
  if (Config::ApplyPh1PtCut)    fOutDir += Form("_ph1pt%4.1f"   , Config::Ph1PtCut);
  if (Config::ApplyPh1VIDCut)   fOutDir += Form("_ph1VID%s"     , Config::Ph1VID.Data());
  if (Config::ApplyPh1R9Cut)    fOutDir += Form("_ph1r9%3.1f"   , Config::Ph1R9Cut);
  if (Config::ApplyPhAnyPtCut)  fOutDir += Form("_phanypt%4.1f" , Config::PhAnyPtCut);
  if (Config::ApplyPhAnyVIDCut) fOutDir += Form("_phanyVID%s"   , Config::PhAnyVID.Data());
  if (Config::ApplyPhAnyR9Cut)  fOutDir += Form("_phanyr9%3.1f" , Config::PhAnyR9Cut);
  if (Config::ApplyrhECut)      fOutDir += Form("_rhE%3.1f"     , Config::rhECut);
  if (Config::ApplyPhMCMatchingCut)
  {
    if      (Config::ApplyExactPhMCMatch) fOutDir += Form("_exactphmc");
    else if (Config::ApplyAntiPhMCMatch)  fOutDir += Form("_antiphmc");
    else    { std::cout << "Need to specify a matching criterion!" << std::endl; exit(0); }
  }
  if (Config::ApplyHLTMatching)
  {
    fOutDir += Form("_hltmatch");
  }  
  if (Config::ApplyECALAcceptCut) 
  { 
    if      (Config::ApplyEBOnly) fOutDir += Form("_EBOnly");
    else if (Config::ApplyEEOnly) fOutDir += Form("_EEOnly");
    else                          fOutDir += Form("_ecalaccept");
  }
  fOutDump = fOutDir; // --> simple directory for dump of plots

  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());
  }

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

PlotPhotons::~PlotPhotons()
{
  delete fInTree;
  delete fInFile;
  delete fOutFile;
}

void PlotPhotons::DoPlots(Bool_t geninfo, Bool_t vtxs, Bool_t met, Bool_t jets, Bool_t photons, Bool_t ph1, Bool_t phdelay, Bool_t trigger, Bool_t analysis)
{
  PlotPhotons::SetupPlots(geninfo, vtxs, met, jets, photons, ph1, phdelay, trigger, analysis);
  PlotPhotons::EventLoop(geninfo, vtxs, met, jets, photons, ph1, phdelay, trigger, analysis);
  PlotPhotons::MakeSubDirs();
  PlotPhotons::OutputTH1Fs();
  PlotPhotons::OutputTrigTH1Fs();
  PlotPhotons::OutputTH2Fs();
  PlotPhotons::OutputTrigTH2Fs();
  PlotPhotons::DumpEventCounts();
}

void PlotPhotons::SetupPlots(Bool_t geninfo, Bool_t vtxs, Bool_t met, Bool_t jets, Bool_t photons, Bool_t ph1, Bool_t phdelay, Bool_t trigger, Bool_t analysis)
{
  if (geninfo && fIsMC)
  {
    PlotPhotons::SetupGenInfo();
    if (fIsGMSB)
    {
      PlotPhotons::SetupGMSB();
      PlotPhotons::SetupGenJets();
    }
    if (fIsHVDS)
    {
      PlotPhotons::SetupHVDS();
    }
  }
  
  if (vtxs)     PlotPhotons::SetupVertices();
  if (met)      PlotPhotons::SetupMET();
  if (jets)     PlotPhotons::SetupJets();
  if (photons)  PlotPhotons::SetupRecoPhotons();
  if (ph1)      PlotPhotons::SetupLeading();
  if (phdelay)  PlotPhotons::SetupMostDelayed();
  if (trigger)  PlotPhotons::SetupTrigger();
  if (analysis) PlotPhotons::SetupAnalysis();
}

void PlotPhotons::EventLoop(Bool_t geninfo, Bool_t vtxs, Bool_t met, Bool_t jets, Bool_t photons, Bool_t ph1, Bool_t phdelay, Bool_t trigger, Bool_t analysis)
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    
    // standard printout
    if (entry%Config::NEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    // count events passing given selection
    const Bool_t passed = PlotPhotons::CountEvents();
    if (Config::ApplyEvCut && !passed) continue;

    // plots!
    if (geninfo && fIsMC)
    {
      PlotPhotons::FillGenInfo();
      if (fIsGMSB)
      {	
	PlotPhotons::FillGMSB();
	PlotPhotons::FillGenJets();
      }
      if (fIsHVDS)
      {
	PlotPhotons::FillHVDS();
      }
    }

    if (vtxs)     PlotPhotons::FillVertices();
    if (met)      PlotPhotons::FillMET();
    if (jets)     PlotPhotons::FillJets();
    if (photons)  PlotPhotons::FillRecoPhotons();
    if (ph1)      PlotPhotons::FillLeading();
    if (phdelay)  PlotPhotons::FillMostDelayed();
    if (trigger)  PlotPhotons::FillTrigger();
    if (analysis) PlotPhotons::FillAnalysis(passed);
  } // end loop over events
}

Bool_t PlotPhotons::CountEvents()
{
  // photon selection efficiency
  std::vector<Int_t> goodphotons;
  const Int_t ph1 = PlotPhotons::GetGoodPhotons(goodphotons);
  const Bool_t photon_b = (ph1 != -1);
  if (photon_b) fEfficiency["Photons"]++;

  // HT selection efficiency
  const Float_t jetHT = PlotPhotons::GetJetHTAbovePt();
  const Bool_t jetht_b = (Config::ApplyJetHtCut ? (jetHT > Config::JetHtCut) : true);
  if (jetht_b) fEfficiency["HT"]++;

  // multi-jet selection efficiency
  const Int_t nJets = PlotPhotons::GetNJetsAbovePt();
  const Bool_t njets_b = (Config::ApplyNJetsCut ? (nJets >= Config::NJetsCut) : true);
  if (njets_b) fEfficiency["Jets"]++;

  // is event compatible with both jets and HT?
  if (jetht_b && njets_b) fEfficiency["Jets+HT"]++;
  
  // event level (no HLT) efficiency
  const Bool_t event_b = (photon_b && jetht_b && njets_b);
  if (event_b) fEfficiency["Reco"]++;

  //trigger efficiency
  const Bool_t hlt_b = (*triggerBits)[0];   //((Config::isHLT4) ? (*triggerBits)[9] : true); // dispho60_ht350
  if (hlt_b) fEfficiency["Trigger"]++;

  if (event_b && hlt_b)
  {
    fEfficiency["Events"]++;
    return true;
  }
  else return false;
}

Int_t PlotPhotons::GetLeadingPhoton()
{
  // first ensure that at least one photon passes the analysis selection
  Int_t ph1 = -1;
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // HLT matching??
    if (Config::ApplyHLTMatching)
    {
      if (Config::filterIdx != -1 && !(*phIsHLTMatched)[iph][Config::filterIdx]) continue;
    }

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
    if (Config::ApplyECALAcceptCut && ((eta < ECAL::etaEB) || ((eta > ECAL::etaEEmin) && (eta < ECAL::etaEEmax))))
    {
      // EB/EE only selection
      if      (Config::ApplyEBOnly && (eta > ECAL::etaEB)) continue;
      else if (Config::ApplyEEOnly && ((eta < ECAL::etaEEmin) || (eta > ECAL::etaEEmax))) continue;

      // eta specific selection
      if      (eta < ECAL::etaEB) 
      {
	if ((Config::ApplyPh1SmajEBMin && (smaj < Config::Ph1SmajEBMin)) || (Config::ApplyPh1SmajEBMax && (smaj > Config::Ph1SmajEBMax))) continue;
	if ((Config::ApplyPh1SminEBMin && (smin < Config::Ph1SminEBMin)) || (Config::ApplyPh1SminEBMax && (smin > Config::Ph1SminEBMax))) continue;
      }
      else if ((eta > ECAL::etaEEmin) && (eta < ECAL::etaEEmax))
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
	else if (fIsBkg && ((*phIsGenMatched)[iph] == 0)) continue; // set to == 0 for exact matching
      }
      else if (Config::ApplyAntiPhMCMatch)
      {
	if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] > 0)) continue; // set to <=0 for exact matching
	else if (fIsBkg && ((*phIsGenMatched)[iph] != 0)) continue; // set to != 0 for anti-matching
      }
    }

    // Photons ordered in pT, so save index of first photon passing
    ph1 = iph; 
    break;
  } // end loop over nphotons
  return ph1;
}

Int_t PlotPhotons::GetGoodPhotons(std::vector<Int_t> & goodphotons)
{
  const Int_t ph1 = PlotPhotons::GetLeadingPhoton();
  if (ph1 != -1)
  {
    for (Int_t iph = 0; iph < nphotons; iph++) // remove pt cut
    {
      // HLT matching??
      if (Config::ApplyHLTMatching)
      {
	if (Config::filterIdx != -1 && !(*phIsHLTMatched)[iph][Config::filterIdx]) continue;
      }

      // basic photon selection
      if (Config::ApplyPhAnyPtCut && ((*phpt)[iph] < Config::PhAnyPtCut)) continue;
      if (Config::ApplyPhAnyVIDCut && ((*phVID)[iph] < fPhVIDMap[Config::PhAnyVID])) continue;
      if (Config::ApplyPhAnyR9Cut && ((*phr9)[iph] < Config::PhAnyR9Cut)) continue;

      // eta specific selection
      const Float_t eta  = std::abs((*phsceta)[iph]);
      const Float_t smaj = (*phsmaj)[iph];
      const Float_t smin = (*phsmin)[iph];
      if (Config::ApplyECALAcceptCut && ((eta < ECAL::etaEB) || ((eta > ECAL::etaEEmin) && (eta < ECAL::etaEEmax))))
      {
	// EB/EE only selection
	if      (Config::ApplyEBOnly && (eta > ECAL::etaEB)) continue;
	else if (Config::ApplyEEOnly && ((eta < ECAL::etaEEmin) || (eta > ECAL::etaEEmax))) continue;

	// eta specific selection
	if      (eta < ECAL::etaEB) 
        {
	  if ((Config::ApplyPhAnySmajEBMin && (smaj < Config::PhAnySmajEBMin)) || (Config::ApplyPhAnySmajEBMax && (smaj > Config::PhAnySmajEBMax))) continue;
	  if ((Config::ApplyPhAnySminEBMin && (smin < Config::PhAnySminEBMin)) || (Config::ApplyPhAnySminEBMax && (smin > Config::PhAnySminEBMax))) continue;
	}
	else if ((eta > ECAL::etaEEmin) && (eta < ECAL::etaEEmax))
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
	  else if (fIsBkg && ((*phIsGenMatched)[iph] == 0)) continue; // set to == 0 for exact matching
	}
	else if (Config::ApplyAntiPhMCMatch)
        {
	  if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] > 0)) continue; // set to <=0 for exact matching
	  else if (fIsBkg && ((*phIsGenMatched)[iph] != 0)) continue; // set to != 0 for anti-matching
	}
      }

      // Save photon index
      goodphotons.push_back(iph);
    } // end loop over nphotons
  } // end check over leading photon
  return ph1;
}

Int_t PlotPhotons::GetMostDelayedPhoton()
{
  std::vector<Int_t> goodphotons;
  Int_t phdelay = PlotPhotons::GetGoodPhotons(goodphotons);
  for (UInt_t gph = 0; gph < goodphotons.size(); gph++)
  {
    const Int_t iph = goodphotons[gph];
    if ((*phrhtime)[iph][(*phseedpos)[iph]] > (*phrhtime)[phdelay][(*phseedpos)[phdelay]]) phdelay = iph;
  }
  return phdelay;
}

Float_t PlotPhotons::GetJetHTAbovePt()
{
  Float_t jetHT = 0.f;
  for (Int_t ijet = 0; ijet < njets; ijet++) 
  {
    if (Config::ApplyMinJetPtCut && ((*jetpt)[ijet] < Config::MinJetPtCut)) break; // jets are ordered in pT!
    jetHT += (*jetpt)[ijet];
  }
  return jetHT;
}

Int_t PlotPhotons::GetNJetsAbovePt()
{
  Int_t nJets = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++) 
  {
    if (Config::ApplyJetPtCut && ((*jetpt)[ijet] < Config::JetPtCut)) break; // jets are ordered in pT!
    nJets++;
  }
  return nJets;
}

void PlotPhotons::FillGenInfo()
{
  fPlots["genpuobs"]->Fill(genpuobs);
  fPlots["genputrue"]->Fill(genputrue);
}

void PlotPhotons::FillGMSB()
{
  fPlots["nNeutralino"]->Fill(nNeutralino);
  fPlots["nNeutoPhGr"]->Fill(nNeutoPhGr);

  if (nNeutoPhGr >= 1)
  {
    // first neutralino info
    fPlots["genNE"]->Fill(genN1E);
    fPlots["genNpt"]->Fill(genN1pt);
    fPlots["genNphi"]->Fill(genN1phi);
    fPlots["genNeta"]->Fill(genN1eta);

    // calculate proper distance for the first neutralino
    const Float_t genN1traveld = std::sqrt(rad2(genN1decayvx-genN1prodvx,genN1decayvy-genN1prodvy,genN1decayvz-genN1prodvz));
    fPlots["genNtraveld"]->Fill(genN1traveld);
    fPlots["genNtraveld_zoom"]->Fill(genN1traveld);

    // calculate total momentum for the first neutralino
    TLorentzVector genN1_lorvec; genN1_lorvec.SetPtEtaPhiE(genN1pt,genN1eta,genN1phi,genN1E);
    const Float_t genN1p  = std::sqrt(rad2(genN1_lorvec.Px(),genN1_lorvec.Py(),genN1_lorvec.Pz()));
    const Float_t genN1bg = bg(genN1p,genN1mass);

    const Float_t genN1gamma = gamma(genN1p,genN1mass);
    fPlots["genNgamma"]->Fill(genN1gamma);

    // ctau is distance / beta*gamma
    const Float_t genN1ctau = genN1traveld/genN1bg; 
    fPlots["genNctau"]->Fill(genN1ctau);

    // decay products
    fPlots["genphE"]->Fill(genph1E);
    fPlots["genphpt"]->Fill(genph1pt);
    fPlots["genphphi"]->Fill(genph1phi);
    fPlots["genpheta"]->Fill(genph1eta);
    fPlots["gengrE"]->Fill(gengr1E);
    fPlots["gengrpt"]->Fill(gengr1pt);
    fPlots["gengrphi"]->Fill(gengr1phi);
    fPlots["gengreta"]->Fill(gengr1eta);

    // photon arrival time
    const Float_t genN1decayvr = std::sqrt(rad2(genN1decayvx,genN1decayvy));
    if (genN1decayvr < ECAL::rEB && genN1decayvz < ECAL::zEE) 
    {
      Float_t genphtime = PlotPhotons::GetGenPhotonArrivalTime(genN1decayvr,genN1decayvz,ineta(genph1eta));
      genphtime = ((genphtime != -1.f) ? genphtime + genN1ctau * genN1gamma / sol : -1.f);
      fPlots["genphtime"]->Fill(genphtime);
      fPlots2D["genphtime_vs_genphpt"]->Fill(genph1pt,genphtime);
    }
    else 
    {
      fPlots["genphtime"]->Fill(-2.f);
      fPlots2D["genphtime_vs_genphpt"]->Fill(genph1pt,-2.f);
    }
  }
  if (nNeutoPhGr >= 2)
  {
    // second neutralino info
    fPlots["genNE"]->Fill(genN1E);
    fPlots["genNpt"]->Fill(genN1pt);
    fPlots["genNphi"]->Fill(genN1phi);
    fPlots["genNeta"]->Fill(genN1eta);

    // calculate proper distance for the second neutralino
    const Float_t genN2traveld = std::sqrt(rad2(genN2decayvx-genN2prodvx,genN2decayvy-genN2prodvy,genN2decayvz-genN2prodvz)); 
    fPlots["genNtraveld"]->Fill(genN2traveld);
    fPlots["genNtraveld_zoom"]->Fill(genN2traveld);

    // calculate total momentum for the second neutralino
    TLorentzVector genN2_lorvec; genN2_lorvec.SetPtEtaPhiE(genN2pt,genN2eta,genN2phi,genN2E);
    const Float_t genN2p  = std::sqrt(rad2(genN2_lorvec.Px(),genN2_lorvec.Py(),genN2_lorvec.Pz()));
    const Float_t genN2bg = bg(genN2p,genN2mass);

    const Float_t genN2gamma = gamma(genN2p,genN2mass);
    fPlots["genNgamma"]->Fill(genN2gamma);

    // ctau is distance / beta*gamma
    const Float_t genN2ctau = genN2traveld/genN2bg;
    fPlots["genNctau"]->Fill(genN2ctau);

    // decay products
    fPlots["genphE"]->Fill(genph2E);
    fPlots["genphpt"]->Fill(genph2pt);
    fPlots["genphphi"]->Fill(genph2phi);
    fPlots["genpheta"]->Fill(genph2eta);
    fPlots["gengrE"]->Fill(gengr2E);
    fPlots["gengrpt"]->Fill(gengr2pt);
    fPlots["gengrphi"]->Fill(gengr2phi);
    fPlots["gengreta"]->Fill(gengr2eta);
    
    // photon arrival time
    const Float_t genN2decayvr = std::sqrt(rad2(genN2decayvx,genN2decayvy));
    if (genN2decayvr < ECAL::rEB && genN2decayvz < ECAL::zEE) 
    {
      Float_t genphtime = PlotPhotons::GetGenPhotonArrivalTime(genN2decayvr,genN2decayvz,ineta(genph2eta));
      genphtime = ((genphtime != -1.f) ? genphtime + genN2ctau * genN2gamma / sol : -1.f);
      fPlots["genphtime"]->Fill(genphtime);
      fPlots2D["genphtime_vs_genphpt"]->Fill(genph2pt,genphtime);
    }
    else 
    {
      fPlots["genphtime"]->Fill(-2.f);
      fPlots2D["genphtime_vs_genphpt"]->Fill(genph2pt,-2.f);
    }
  }
}

void PlotPhotons::FillGenJets()
{
  fPlots["ngenjets"]->Fill(ngenjets);
  
  for (Int_t igjet = 0; igjet < ngenjets; igjet++)
  {
    fPlots["genjetE"]->Fill((*genjetE)[igjet]);
    fPlots["genjetpt"]->Fill((*genjetpt)[igjet]);
    fPlots["genjetphi"]->Fill((*genjetphi)[igjet]);
    fPlots["genjeteta"]->Fill((*genjeteta)[igjet]);
  }
}

void PlotPhotons::FillHVDS()
{
  for (Int_t ipion = 0; ipion < nvPions; ipion++)
  {
    // first gen pions
    fPlots["genvPionE"]->Fill((*genvPionE)[ipion]);
    fPlots["genvPionpt"]->Fill((*genvPionpt)[ipion]);
    fPlots["genvPionphi"]->Fill((*genvPionphi)[ipion]);
    fPlots["genvPioneta"]->Fill((*genvPioneta)[ipion]);
    
    const Float_t genvPiontraveld = std::sqrt(rad2((*genvPiondecayvx)[ipion]-(*genvPionprodvx)[ipion],
						   (*genvPiondecayvy)[ipion]-(*genvPionprodvy)[ipion],
						   (*genvPiondecayvz)[ipion]-(*genvPionprodvz)[ipion])); 
    fPlots["genvPiontraveld"]->Fill(genvPiontraveld);
    fPlots["genvPiontraveld_zoom"]->Fill(genvPiontraveld);
    
    // calculate total momentum for the second neutralino
    TLorentzVector genvPion_lorvec; genvPion_lorvec.SetPtEtaPhiE((*genvPionpt)[ipion],(*genvPioneta)[ipion],(*genvPionphi)[ipion],(*genvPionE)[ipion]);
    const Float_t genvPionp  = std::sqrt(rad2(genvPion_lorvec.Px(),genvPion_lorvec.Py(),genvPion_lorvec.Pz()));
    const Float_t genvPionbg = bg(genvPionp,(*genvPionmass)[ipion]);

    const Float_t genvPiongamma = gamma(genvPionp,(*genvPionmass)[ipion]);
    fPlots["genvPiongamma"]->Fill(genvPiongamma);
      
    // ctau is distance / beta*gamma
    const Float_t genvPionctau = genvPiontraveld/genvPionbg;
    fPlots["genvPionctau"]->Fill(genvPionctau);

    // now do gen photon info
    fPlots["genHVph1E"]->Fill((*genHVph1E)[ipion]);
    fPlots["genHVph1pt"]->Fill((*genHVph1pt)[ipion]);
    fPlots["genHVph1phi"]->Fill((*genHVph1phi)[ipion]);
    fPlots["genHVph1eta"]->Fill((*genHVph1eta)[ipion]);

    fPlots["genHVph2E"]->Fill((*genHVph2E)[ipion]);
    fPlots["genHVph2pt"]->Fill((*genHVph2pt)[ipion]);
    fPlots["genHVph2phi"]->Fill((*genHVph2phi)[ipion]);
    fPlots["genHVph2eta"]->Fill((*genHVph2eta)[ipion]);

    // photon arrival time
    const Float_t genvPiondecayvr = std::sqrt(rad2((*genvPiondecayvx)[ipion],(*genvPiondecayvy)[ipion]));
    if (genvPiondecayvr < ECAL::rEB && std::abs((*genvPiondecayvz)[ipion]) < ECAL::zEE) 
    {
      std::cout << std::endl;
      std::cout << "decayr: "<< genvPiondecayvr << " decayz: " << (*genvPiondecayvz)[ipion] << " ineta: " << ineta((*genHVph1eta)[ipion]) << std::endl;

      Float_t genph1time = PlotPhotons::GetGenPhotonArrivalTime(genvPiondecayvr,(*genvPiondecayvz)[ipion],ineta((*genHVph1eta)[ipion]));
      genph1time = ((genph1time != -1.f) ? genph1time + genvPionctau * genvPiongamma / sol : -1.f);
      
      std::cout << " w/ vpiontime: " << genph1time << std::endl; 

      fPlots["genHVph1time"]->Fill(genph1time);
      fPlots2D["genHVph1time_vs_genHVph1pt"]->Fill((*genHVph1pt)[ipion],genph1time);

//       Float_t genph2time = PlotPhotons::GetGenPhotonArrivalTime(genvPiondecayvr,(*genvPiondecayvz)[ipion],ineta((*genHVph2eta)[ipion]));
//       genph2time = ((genph2time != -1.f) ? genph2time + genvPionctau * genvPiongamma / sol : -1.f);
//       fPlots["genHVph2time"]->Fill(genph2time);
//       fPlots2D["genHVph2time_vs_genHVph2pt"]->Fill((*genHVph2pt)[ipion],genph2time);
    }
    else 
    {
      fPlots["genHVph1time"]->Fill(-2.f);
      fPlots2D["genHVph1time_vs_genHVph1pt"]->Fill((*genHVph1pt)[ipion],-2.f);
      fPlots["genHVph2time"]->Fill(-2.f);
      fPlots2D["genHVph2time_vs_genHVph2pt"]->Fill((*genHVph2pt)[ipion],-2.f);
    }
  }
}

Float_t PlotPhotons::GetGenPhotonArrivalTime(const Float_t r0, const Float_t z0, const Float_t slope)
{
  Float_t time = -1.f;
  
  const Float_t z = z0 + ( (ECAL::rEB - r0) / slope );
  if (std::abs(z) < ECAL::zEB) 
  {
    time  = std::sqrt(rad2(ECAL::rEB-r0,z-z0)) / sol;
    
    std::cout << "EB: " << time << " (z" << z << ")" ;

    time -= std::sqrt(rad2(ECAL::rEB,z)) / sol; // TOF correction;

    std::cout << " w TOF: " << time;
  }
  else 
  {
    const Float_t r = r0 + ( slope * (ECAL::zEE - z0) );
    if (std::abs(r) > ECAL::rEEmin && std::abs(r) < ECAL::rEEmax)
    {
      time  = std::sqrt(rad2(r-r0,ECAL::zEE-z0)) / sol;

      std::cout << "EE: " << time << " (z" << r << ")" ;

      time -= std::sqrt(rad2(r0,ECAL::zEE)) / sol; // TOF correction;

      std::cout << " w TOF: " << time;
    }
  }

  return time;
}

void PlotPhotons::FillVertices()
{
  fPlots["nvtx"]->Fill(nvtx);
} 

void PlotPhotons::FillMET()
{
  fPlots["t1pfMETpt"]->Fill(t1pfMETpt);
  fPlots["t1pfMETphi"]->Fill(t1pfMETphi);
  fPlots["t1pfMETsumEt"]->Fill(t1pfMETsumEt);
  fPlots["t1pfMETuncorpt"]->Fill(t1pfMETuncorpt);
  fPlots["t1pfMETuncorphi"]->Fill(t1pfMETuncorphi);
  fPlots["t1pfMETuncorsumEt"]->Fill(t1pfMETuncorsumEt);
  fPlots["t1pfMETcalopt"]->Fill(t1pfMETcalopt);
  fPlots["t1pfMETcalophi"]->Fill(t1pfMETcalophi);
  fPlots["t1pfMETcalosumEt"]->Fill(t1pfMETcalosumEt);
  if (fIsGMSB || fIsHVDS)
  {
    fPlots["t1pfMETgenMETpt"]->Fill(t1pfMETgenMETpt);
    fPlots["t1pfMETgenMETphi"]->Fill(t1pfMETgenMETphi);
    fPlots["t1pfMETgenMETsumEt"]->Fill(t1pfMETgenMETsumEt);
  }
}

void PlotPhotons::FillJets()
{
  // jetHT plots
  Float_t jetHT = PlotPhotons::GetJetHTAbovePt();
  fPlots["jetHT"]->Fill(jetHT);

  // nJets plots
  fPlots["njetsinc"]->Fill(njets); 
  const Int_t nJets = PlotPhotons::GetNJetsAbovePt();
  fPlots["njets"]->Fill(nJets);  

  Int_t nMatchedJets = 0;
  for (Int_t ijet = 0; ijet < (Config::ApplyNJetsCut ? nJets : njets); ijet++) // jets are ordered by pT, so up to nJets if applying njets cut!
  {
    const Float_t jetPt = (*jetpt)[ijet];
    fPlots["jetE"]->Fill((*jetE)[ijet]);
    fPlots["jetpt"]->Fill((*jetpt)[ijet]);
    fPlots["jetphi"]->Fill((*jetphi)[ijet]);
    fPlots["jeteta"]->Fill((*jeteta)[ijet]);
    
    if (fIsGMSB && ((*jetmatch)[ijet] >= 0)) nMatchedJets++;
  }

  if (fIsGMSB) fPlots["nMatchedJets"]->Fill(nMatchedJets);  
}

void PlotPhotons::FillRecoPhotons()
{
  fPlots["nphotonsinc"]->Fill(nphotons);

  std::vector<Int_t> goodphotons;
  PlotPhotons::GetGoodPhotons(goodphotons);

  fPlots["nphotons"]->Fill(goodphotons.size());

  Int_t nMatchedPhotons = 0;
  for (UInt_t gph = 0; gph < goodphotons.size(); gph++)
  {
    // get photon index of good photons
    const Int_t iph = goodphotons[gph];

    fPlots["phE"]->Fill((*phE)[iph]);
    fPlots["phpt"]->Fill((*phpt)[iph]);
    fPlots["phphi"]->Fill((*phphi)[iph]);
    fPlots["pheta"]->Fill((*pheta)[iph]);
    fPlots["phHoE"]->Fill((*phHoE)[iph]);
    fPlots["phr9"]->Fill((*phr9)[iph]);
    fPlots["phChgIso"]->Fill(std::max((*phChgIso)[iph],0.f));    
    fPlots["phNeuIso"]->Fill(std::max((*phNeuIso)[iph],0.f));
    fPlots["phIso"]->Fill(std::max((*phIso)[iph],0.f));
    fPlots["phsuisseX"]->Fill((*phsuisseX)[iph]);
    fPlots["phsieie"]->Fill((*phsieie)[iph]);
    fPlots["phsipip"]->Fill((*phsipip)[iph]);
    fPlots["phsieip"]->Fill((*phsieip)[iph]);
    fPlots["phsmaj"]->Fill((*phsmaj)[iph]);
    fPlots["phsmin"]->Fill((*phsmin)[iph]);
    fPlots["phsmin_ov_phsmaj"]->Fill((*phsmin)[iph]/(*phsmaj)[iph]);
    fPlots["phalpha"]->Fill((*phalpha)[iph]);
    fPlots["phscE"]->Fill((*phscE)[iph]);
    fPlots2D["phsmin_vs_phsmaj"]->Fill((*phsmaj)[iph],(*phsmin)[iph]);
  
    // loop over rechits (+ seed info)
    Int_t nRecHits = 0;
    for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
    {
      if (Config::ApplyrhECut && (*phrhE)[iph][irh] < Config::rhECut) continue;
      nRecHits++;

      fPlots["phrhE"]->Fill((*phrhE)[iph][irh]);
      fPlots["phrhtime"]->Fill((*phrhtime)[iph][irh]);
      fPlots["phrhOOT"]->Fill((*phrhOOT)[iph][irh]);
      if ( (*phseedpos)[iph] == irh ) // seed info
      {
	fPlots["phseedE"]->Fill((*phrhE)[iph][irh]);
	fPlots["phseedtime"]->Fill((*phrhtime)[iph][irh]);
	fPlots["phseedOOT"]->Fill((*phrhOOT)[iph][irh]);
    
	// 2D plots of interest
	fPlots2D["phseedtime_vs_phpt"]->Fill((*phpt)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phHoE"]->Fill((*phHoE)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phr9"]->Fill((*phr9)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phsieie"]->Fill((*phsieie)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phsmaj"]->Fill((*phsmaj)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phsmin"]->Fill((*phsmin)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phalpha"]->Fill((*phalpha)[iph],(*phrhtime)[iph][irh]);
	fPlots2D["phseedtime_vs_phsmin_ov_phsmaj"]->Fill(((*phsmin)[iph]/(*phsmaj)[iph]),(*phrhtime)[iph][irh]);
      }
    } // end loop over nrechits
    fPlots["phnrh"]->Fill(nRecHits);

    // MC matching selection --> redundant with ph mc matching
    if ( ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] > 0)) || (fIsBkg && ((*phIsGenMatched)[iph])) ) nMatchedPhotons++;
  } // end loop over nphotons
  if (fIsMC) fPlots["nMatchedPhotons"]->Fill(nMatchedPhotons);
}

void PlotPhotons::FillLeading()
{
  const Int_t ph1 = PlotPhotons::GetLeadingPhoton();
  
  if (ph1 != -1)
  {
    fPlots["ph1pt"]->Fill((*phpt)[ph1]);
    fPlots["ph1HoE"]->Fill((*phHoE)[ph1]);
    fPlots["ph1r9"]->Fill((*phr9)[ph1]);
    fPlots["ph1sieie"]->Fill((*phsieie)[ph1]);
    fPlots["ph1smaj"]->Fill((*phsmaj)[ph1]);
    fPlots["ph1smin"]->Fill((*phsmin)[ph1]);
    fPlots["ph1alpha"]->Fill((*phalpha)[ph1]);
    fPlots["ph1smin_ov_ph1smaj"]->Fill((*phsmin)[ph1]/(*phsmaj)[ph1]);
    fPlots["ph1seedtime"]->Fill((*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1smin_vs_ph1smaj"]->Fill((*phsmaj)[ph1],(*phsmin)[ph1]);

    fPlots2D["ph1seedtime_vs_ph1pt"]->Fill((*phpt)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1HoE"]->Fill((*phHoE)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1r9"]->Fill((*phr9)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1sieie"]->Fill((*phsieie)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1smaj"]->Fill((*phsmaj)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1smin"]->Fill((*phsmin)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1alpha"]->Fill((*phalpha)[ph1],(*phrhtime)[ph1][(*phseedpos)[ph1]]);
    fPlots2D["ph1seedtime_vs_ph1smin_ov_ph1smaj"]->Fill(((*phsmin)[ph1]/(*phsmaj)[ph1]),(*phrhtime)[ph1][(*phseedpos)[ph1]]);
  }
}

void PlotPhotons::FillMostDelayed()
{
  const Int_t phdelay = PlotPhotons::GetMostDelayedPhoton();
  
  if (phdelay != -1)
  {
    fPlots["phdelaypt"]->Fill((*phpt)[phdelay]);
    fPlots["phdelayHoE"]->Fill((*phHoE)[phdelay]);
    fPlots["phdelayr9"]->Fill((*phr9)[phdelay]);
    fPlots["phdelaysieie"]->Fill((*phsieie)[phdelay]);
    fPlots["phdelaysmaj"]->Fill((*phsmaj)[phdelay]);
    fPlots["phdelaysmin"]->Fill((*phsmin)[phdelay]);
    fPlots["phdelayalpha"]->Fill((*phalpha)[phdelay]);
    fPlots["phdelaysmin_ov_phdelaysmaj"]->Fill((*phsmin)[phdelay]/(*phsmaj)[phdelay]);
    fPlots["phdelayseedtime"]->Fill((*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelaysmin_vs_phdelaysmaj"]->Fill((*phsmaj)[phdelay],(*phsmin)[phdelay]);

    fPlots2D["phdelayseedtime_vs_phdelaypt"]->Fill((*phpt)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelayHoE"]->Fill((*phHoE)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelayr9"]->Fill((*phr9)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelaysieie"]->Fill((*phsieie)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelaysmaj"]->Fill((*phsmaj)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelaysmin"]->Fill((*phsmin)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelayalpha"]->Fill((*phalpha)[phdelay],(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
    fPlots2D["phdelayseedtime_vs_phdelaysmin_ov_phdelaysmaj"]->Fill(((*phsmin)[phdelay]/(*phsmaj)[phdelay]),(*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
  }
}

void PlotPhotons::FillTrigger()
{
  std::vector<Int_t> goodphotons;
  const Int_t ph1 = PlotPhotons::GetGoodPhotons(goodphotons);
  const Int_t phdelay = PlotPhotons::GetMostDelayedPhoton();

  if (ph1 != -1)
  {
    const Float_t ph1seedtime = (*phrhtime)[ph1][(*phseedpos)[ph1]];
    const Float_t ph1pt = (*phpt)[ph1];

    if (Config::isHLT4) // consult HLT4.txt for position in vectors -->could make this automatic but eh who cares
    {
      PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["ph1seedtime_vs_ph1pt_hltdispho50_ht350"],((*phIsHLTMatched)[ph1][0]&&(*triggerBits)[6]),ph1pt,ph1seedtime);
      PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["ph1seedtime_vs_ph1pt_hltdispho60_ht350"],((*phIsHLTMatched)[ph1][1]&&(*triggerBits)[9]),ph1pt,ph1seedtime);
      PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["ph1seedtime_vs_ph1pt_hltdispho70_ht350"],((*phIsHLTMatched)[ph1][2]&&(*triggerBits)[12]),ph1pt,ph1seedtime);
    }
  } // end block over leading photon

  if (phdelay != -1)
  {
    const Float_t phdelayseedtime = (*phrhtime)[phdelay][(*phseedpos)[phdelay]];
    const Float_t phdelaypt = (*phpt)[phdelay];

    if (Config::isHLT4) // consult HLT4.txt for position in vectors -->could make this automatic but eh who cares
    {
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltpho120_met40"],(*triggerBits)[0],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltpho175"],(*triggerBits)[1],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdoublepho42_25_m15"],(*triggerBits)[2],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdoublepho60"],(*triggerBits)[3],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltpho90_ht600"],(*triggerBits)[4],phdelayseedtime);

      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho50_ht300"],(*triggerBits)[5],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho50_ht350"],(*triggerBits)[6],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho50_ht400"],(*triggerBits)[7],phdelayseedtime);

      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_ht300"],(*triggerBits)[8],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_ht350"],(*triggerBits)[9],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_ht400"],(*triggerBits)[10],phdelayseedtime);

      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho70_ht300"],(*triggerBits)[11],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho70_ht350"],(*triggerBits)[12],phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho70_ht400"],(*triggerBits)[13],phdelayseedtime);

      PlotPhotons::FillTriggerPlot(fTrigPlots["pu_hltdispho60_ht350"],(*triggerBits)[9],nvtx);

      // 2D plots 
      PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["phdelayseedtime_vs_phdelaypt_hltdispho50_ht350"],((*phIsHLTMatched)[phdelay][0]&&(*triggerBits)[6]),phdelaypt,phdelayseedtime);
      PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["phdelayseedtime_vs_phdelaypt_hltdispho60_ht350"],((*phIsHLTMatched)[phdelay][1]&&(*triggerBits)[9]),phdelaypt,phdelayseedtime);
      PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["phdelayseedtime_vs_phdelaypt_hltdispho70_ht350"],((*phIsHLTMatched)[phdelay][2]&&(*triggerBits)[12]),phdelaypt,phdelayseedtime);
    } // end check over isHLT4
  } // end block over most delayed photon

  if (goodphotons.size() > 0)
  {
    if (Config::isHLT4)
    {
      // all good photons that are HLT amtched --> taxing!
      for (UInt_t gph = 0; gph < goodphotons.size(); gph++)
      {
	// get photon index of good photons
	const Int_t iph = goodphotons[gph];
	const Int_t phgoodseedtime = (*phrhtime)[iph][(*phseedpos)[iph]];
	const Int_t phgoodpt = (*phpt)[iph];
	PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["phgoodseedtime_vs_phgoodpt_hltdispho50_ht350"],((*phIsHLTMatched)[iph][0]&&(*triggerBits)[6]),phgoodpt,phgoodseedtime);
	PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["phgoodseedtime_vs_phgoodpt_hltdispho60_ht350"],((*phIsHLTMatched)[iph][1]&&(*triggerBits)[9]),phgoodpt,phgoodseedtime);
	PlotPhotons::FillTriggerPlot2D(fTrigPlots2D["phgoodseedtime_vs_phgoodpt_hltdispho70_ht350"],((*phIsHLTMatched)[iph][2]&&(*triggerBits)[12]),phgoodpt,phgoodseedtime);
      } // end loop over all good photons
    } // end check over isHLT4
  } // end block over all good photons
}

void PlotPhotons::FillAnalysis(const Bool_t passed)
{
  if (passed)
  {
    Int_t ph1 = PlotPhotons::GetLeadingPhoton();
    fPlots2D["MET_vs_ph1seedtime"]->Fill((*phrhtime)[ph1][(*phseedpos)[ph1]],t1pfMETpt);
    Int_t phdelay = PlotPhotons::GetMostDelayedPhoton();
    fPlots2D["MET_vs_phdelayseedtime"]->Fill((*phrhtime)[phdelay][(*phseedpos)[phdelay]],t1pfMETpt);
  }
}

void PlotPhotons::SetupGenInfo()
{
  fPlots["genpuobs"] = PlotPhotons::MakeTH1F("genpuobs","Generator N PU Observed",100,0.f,100.f,"nPU (obs)","Events","generator");
  fPlots["genputrue"] = PlotPhotons::MakeTH1F("genputrue","Generator N PU (true)",100,0.f,100.f,"nPU (true)","Events","generator");
}

void PlotPhotons::SetupGMSB()
{
  fPlots["nNeutralino"] = PlotPhotons::MakeTH1F("nNeutralino","Generator nNeutralino [Max of 2]",3,0.f,3.f,"nNeutralinos","Events","GMSB");
  fPlots["nNeutoPhGr"]  = PlotPhotons::MakeTH1F("nNeutoPhGr","Generator n(Neutralino -> Photon + Gravitino) [Max of 2]",3,0.f,3.f,"nNeutoPhGr","Events","GMSB");

  fPlots["genNE"] = PlotPhotons::MakeTH1F("genNE","Generator Neutralino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Neutralinos","GMSB");
  fPlots["genNpt"] = PlotPhotons::MakeTH1F("genNpt","Generator p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Neutralinos","GMSB");
  fPlots["genNphi"] = PlotPhotons::MakeTH1F("genNphi","Generator Neutralino #phi",100,-3.2,3.2,"#phi","Neutralinos","GMSB");
  fPlots["genNeta"] = PlotPhotons::MakeTH1F("genNeta","Generator Neutralino #eta",100,-6.0,6.0,"#eta","Neutralinos","GMSB");

  fPlots["genNtraveld"] = PlotPhotons::MakeTH1F("genNtraveld","Generator Neutralino Travel Distance [cm]",100,0.f,10000.f,"Distance [cm]","Neutralinos","GMSB");
  fPlots["genNtraveld_zoom"] = PlotPhotons::MakeTH1F("genNtraveld_zoom","Generator Neutralino Travel Distance [cm]",100,0.f,400.f,"Distance [cm]","Neutralinos","GMSB");
  fPlots["genNgamma"] = PlotPhotons::MakeTH1F("genNgamma","Generator Neutralino #gamma factor",100,0.f,100.f,"#gamma factor","Neutralinos","GMSB");
  fPlots["genNctau"] = PlotPhotons::MakeTH1F("genNctau","Generator Neutralino c#tau [cm]",200,0.f,fCTau*20.f,"c#tau [cm]","Neutralinos","GMSB");

  fPlots["genphE"] = PlotPhotons::MakeTH1F("genphE","Generator Photon E [GeV]",100,0.f,2500.f,"Energy [GeV]","Photons","GMSB");
  fPlots["genphpt"] = PlotPhotons::MakeTH1F("genphpt","Generator p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","GMSB");
  fPlots["genphphi"] = PlotPhotons::MakeTH1F("genphphi","Generator Photon #phi",100,-3.2,3.2,"#phi","Photons","GMSB");
  fPlots["genpheta"] = PlotPhotons::MakeTH1F("genpheta","Generator Photon #eta",100,-6.0,6.0,"#eta","Photons","GMSB");

  fPlots["gengrE"] = PlotPhotons::MakeTH1F("gengrE","Generator Gravitino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Gravitinos","GMSB");
  fPlots["gengrpt"] = PlotPhotons::MakeTH1F("gengrpt","Generator p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Gravitinos","GMSB");
  fPlots["gengrphi"] = PlotPhotons::MakeTH1F("gengrphi","Generator Gravitino #phi",100,-3.2,3.2,"#phi","Gravitinos","GMSB");
  fPlots["gengreta"] = PlotPhotons::MakeTH1F("gengreta","Generator Gravitino #eta",100,-6.0,6.0,"#eta","Gravitinos","GMSB");

  fPlots["genphtime"] = PlotPhotons::MakeTH1F("genphtime","Generator Photon Time [ns]",120,-2.0,28.0,"Arrival Time [ns]","Photons","GMSB");
  fPlots2D["genphtime_vs_genphpt"] = PlotPhotons::MakeTH2F("genphtime_vs_genphpt","Generator Photon Time vs. Photon p_{T}",100,0.f,2500.f,"Photon p_{T} [GeV/c]",120,-2.0,28.0,"Photon Arrival Time [ns]","GMSB");
}

void PlotPhotons::SetupGenJets()
{
  fPlots["ngenjets"] = PlotPhotons::MakeTH1F("ngenjets","nGenJets",40,0.f,40.f,"nGenJets","Events","GenJets");
  fPlots["genjetE"] = PlotPhotons::MakeTH1F("genjetE","Generator Jets Energy [GeV]",100,0.f,3000.f,"Energy [GeV]","Generator Jets","GenJets");
  fPlots["genjetpt"] = PlotPhotons::MakeTH1F("genjetpt","Generator Jets p_{T} [GeV/c]",100,0.f,3000.f,"Generator jet p_{T} [GeV/c]","Generator Jets","GenJets");
  fPlots["genjetphi"] = PlotPhotons::MakeTH1F("genjetphi","Generator Jets #phi",100,-3.2,3.2,"#phi","Generator Jets","GenJets");
  fPlots["genjeteta"] = PlotPhotons::MakeTH1F("genjeteta","Generator Jets #eta",100,-6.0,6.0,"#eta","Generator Jets","GenJets");
}

void PlotPhotons::SetupHVDS()
{
  fPlots["genvPionE"] = PlotPhotons::MakeTH1F("genvPionE","Generator Dark Pions E [GeV]",100,0.f,2500.f,"Energy [GeV]","Dark Pions","HVDS");
  fPlots["genvPionpt"] = PlotPhotons::MakeTH1F("genvPionpt","Generator Dark Pions p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Dark Pions","HVDS");
  fPlots["genvPionphi"] = PlotPhotons::MakeTH1F("genvPionphi","Generator Dark Pions #phi",100,-3.2,3.2,"#phi","Dark Pions","HVDS");
  fPlots["genvPioneta"] = PlotPhotons::MakeTH1F("genvPioneta","Generator Dark Pions #eta",100,-6.0,6.0,"#eta","Dark Pions","HVDS");

  fPlots["genvPiongamma"] = PlotPhotons::MakeTH1F("genvPiongamma","Generator Dark Pions #gamma factor",100,0.f,100.f,"#gamma factor","Dark Pions","HVDS");
  fPlots["genvPiontraveld"] = PlotPhotons::MakeTH1F("genvPiontraveld","Generator Dark Pions Travel Distance [cm]",100,0.f,10000.f,"Distance [cm]","Dark Pions","GMSB");
  fPlots["genvPiontraveld_zoom"] = PlotPhotons::MakeTH1F("genvPiontraveld_zoom","Generator Dark Pions Travel Distance [cm]",100,0.f,400.f,"Distance [cm]","Dark Pions","GMSB");
  fPlots["genvPionctau"] = PlotPhotons::MakeTH1F("genvPionctau","Generator Dark Pions c#tau [cm]",200,0.f,fCTau*20.f,"c#tau [cm]","Dark Pions","GMSB");

  fPlots["genHVph1E"] = PlotPhotons::MakeTH1F("genHVph1E","Generator Leading Photon E [GeV]",100,0.f,2500.f,"Energy [GeV]","Photons","HVDS");
  fPlots["genHVph1pt"] = PlotPhotons::MakeTH1F("genHVph1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","HVDS");
  fPlots["genHVph1phi"] = PlotPhotons::MakeTH1F("genHVph1phi","Generator Leading Photon #phi",100,-3.2,3.2,"#phi","Photons","HVDS");
  fPlots["genHVph1eta"] = PlotPhotons::MakeTH1F("genHVph1eta","Generator Leading Photon #eta",100,-6.0,6.0,"#eta","Photons","HVDS");

  fPlots["genHVph2E"] = PlotPhotons::MakeTH1F("genHVph2E","Generator Subleading Photon E [GeV]",100,0.f,2500.f,"Energy [GeV]","Photons","HVDS");
  fPlots["genHVph2pt"] = PlotPhotons::MakeTH1F("genHVph2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","HVDS");
  fPlots["genHVph2phi"] = PlotPhotons::MakeTH1F("genHVph2phi","Generator Subleading Photon #phi",100,-3.2,3.2,"#phi","Photons","HVDS");
  fPlots["genHVph2eta"] = PlotPhotons::MakeTH1F("genHVph2eta","Generator Subleading Photon #eta",100,-6.0,6.0,"#eta","Photons","HVDS");

  fPlots["genHVph1time"] = PlotPhotons::MakeTH1F("genHVph1time","Generator Leading Photon Time [ns]",120,-2.0,28.0,"Arrival Time [ns]","Photons","HVDS");
  fPlots["genHVph2time"] = PlotPhotons::MakeTH1F("genHVph2time","Generator Subleading Photon Time [ns]",120,-2.0,28.0,"Arrival Time [ns]","Photons","HVDS");

  fPlots2D["genHVph1time_vs_genHVph1pt"] = PlotPhotons::MakeTH2F("genHVph1time_vs_genHVph1pt","Generator Leading Photon Time vs. Photon p_{T}",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]",120,-2.0,28.0,"Photon Arrival Time [ns]","HVDS");
  fPlots2D["genHVph2time_vs_genHVph2pt"] = PlotPhotons::MakeTH2F("genHVph2time_vs_genHVph2pt","Generator Subleading Photon Time vs. Photon p_{T}",100,0.f,2500.f,"Subleading Photon p_{T} [GeV/c]",120,-2.0,28.0,"Photon Arrival Time [ns]","HVDS");
}

void PlotPhotons::SetupVertices()
{
  fPlots["nvtx"] = PlotPhotons::MakeTH1F("nvtx","nVertices (reco)",100,0.f,100.f,"nPV","Events","Vertices");
}

void PlotPhotons::SetupMET()
{
  fPlots["t1pfMETpt"] = PlotPhotons::MakeTH1F("t1pfMETpt","Type1 PF MET [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
  fPlots["t1pfMETphi"] = PlotPhotons::MakeTH1F("t1pfMETphi","Type1 PF MET #phi",100,-3.2,3.2,"MET #phi","Events","MET");
  fPlots["t1pfMETsumEt"] = PlotPhotons::MakeTH1F("t1pfMETsumEt","Type1 PF MET #Sigma E_{T} [GeV]",100,0.f,8000.f,"MET #Sigma E_{T} [GeV]","Events","MET");
  fPlots["t1pfMETuncorpt"] = PlotPhotons::MakeTH1F("t1pfMETuncorpt","Type1 PF MET (Raw) [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
  fPlots["t1pfMETuncorphi"] = PlotPhotons::MakeTH1F("t1pfMETuncorphi","Type1 PF MET (Raw) #phi",100,-3.2,3.2,"MET #phi","Events","MET");
  fPlots["t1pfMETuncorsumEt"] = PlotPhotons::MakeTH1F("t1pfMETuncorsumEt","Type1 PF MET (Raw) #Sigma E_{T} [GeV]",100,0.f,8000.f,"MET #Sigma E_{T} [GeV]","Events","MET");
  fPlots["t1pfMETcalopt"] = PlotPhotons::MakeTH1F("t1pfMETcalopt","Type1 PF MET (Calo) [GeV]",100,0.f,8000.f,"MET [GeV]","Events","MET");
  fPlots["t1pfMETcalophi"] = PlotPhotons::MakeTH1F("t1pfMETcalophi","Type1 PF MET (Calo) #phi",100,-3.2,3.2,"MET #phi","Events","MET");
  fPlots["t1pfMETcalosumEt"] = PlotPhotons::MakeTH1F("t1pfMETcalosumEt","Type1 PF MET (Calo) #Sigma E_{T} [GeV]",100,0.f,8000.f,"MET #Sigma E_{T} [GeV]","Events","MET");
  if (fIsGMSB || fIsHVDS)
  {
    fPlots["t1pfMETgenMETpt"] = PlotPhotons::MakeTH1F("t1pfMETgenMETpt","Type1 PF Gen MET [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
    fPlots["t1pfMETgenMETphi"] = PlotPhotons::MakeTH1F("t1pfMETgenMETphi","Type1 PF Gen MET #phi",100,-3.2,3.2,"MET #phi","Events","MET");
    fPlots["t1pfMETgenMETsumEt"] = PlotPhotons::MakeTH1F("t1pfMETgenMETsumEt","Type1 PF Gen MET #Sigma E_{T} [GeV]",100,0.f,8000.f,"MET #Sigma E_{T} [GeV]","Events","MET");
  }
}

void PlotPhotons::SetupJets()
{
  fPlots["jetHT"] = PlotPhotons::MakeTH1F("jetHT","Jet H_{T} [GeV/c] (reco)",100,0.f,5000.f,Form("Jet H_{T} [GeV/c] (p_{T} > %4.1f GeV/c)",Config::MinJetPtCut),"Events","AK4Jets");

  fPlots["njetsinc"] = PlotPhotons::MakeTH1F("njetsinc","nAK4Jets",40,0.f,40.f,"nAK4Jets","Events","AK4Jets");
  fPlots["njets"] = PlotPhotons::MakeTH1F("njets",Form("nAK4Jets, p_{T} > %4.1f GeV/c",Config::JetPtCut),40,0.f,40.f,Form("nAK4Jets (p_{T} > %4.1f GeV/c)",Config::JetPtCut),"Events","AK4Jets");
  if (fIsGMSB) fPlots["nMatchedJets"] = PlotPhotons::MakeTH1F("nMatchedJets","nMatchedJets (reco to gen)",40,0.f,40.f,"nMatchedJets","Events","AK4Jets");

  fPlots["jetE"] = PlotPhotons::MakeTH1F("jetE","Jets Energy [GeV] (reco)",100,0.f,3000.f,"Energy [GeV]","Jets","AK4Jets");
  fPlots["jetpt"] = PlotPhotons::MakeTH1F("jetpt","Jets p_{T} [GeV/c] (reco)",100,0.f,3000.f,"Jet p_{T} [GeV/c]","Jets","AK4Jets");
  fPlots["jetphi"] = PlotPhotons::MakeTH1F("jetphi","Jets #phi (reco)",100,-3.2,3.2,"#phi","Jets","AK4Jets");
  fPlots["jeteta"] = PlotPhotons::MakeTH1F("jeteta","Jets #eta (reco)",100,-6.0,6.0,"#eta","Jets","AK4Jets");
}

void PlotPhotons::SetupRecoPhotons()
{
  fPlots["nphotonsinc"] = PlotPhotons::MakeTH1F("nphotonsinc","nPhotons (reco)",20,0.f,20.f,"nPhotons","Events","RecoPhotons");
  fPlots["nphotons"] = PlotPhotons::MakeTH1F("nphotons","nGoodPhotons (reco)",20,0.f,20.f,"nGoodPhotons","Events","RecoPhotons");
  if (fIsMC) fPlots["nMatchedPhotons"] = PlotPhotons::MakeTH1F("nMatchedPhotons","nMatchedPhotons (reco to gen)",20,0.f,20.f,"nMatchedPhotons","Events","RecoPhotons");

  // All reco photons
  fPlots["phE"] = PlotPhotons::MakeTH1F("phE","Photons Energy [GeV] (reco)",100,0.f,2500.f,"Energy [GeV]","Photons","RecoPhotons");
  fPlots["phpt"] = PlotPhotons::MakeTH1F("phpt","Photons p_{T} [GeV/c] (reco)",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","RecoPhotons");
  fPlots["phphi"] = PlotPhotons::MakeTH1F("phphi","Photons #phi (reco)",100,-3.2,3.2,"#phi","Photons","RecoPhotons");
  fPlots["pheta"] = PlotPhotons::MakeTH1F("pheta","Photons #eta (reco)",100,-5.0,5.0,"#eta","Photons","RecoPhotons");
  fPlots["phHoE"] = PlotPhotons::MakeTH1F("phHoE","Photons HE/EE (reco)",100,0.f,5.0,"HE/EE","Photons","RecoPhotons");
  fPlots["phr9"] = PlotPhotons::MakeTH1F("phr9","Photons R9 (reco)",100,0.f,5.0,"R9","Photons","RecoPhotons");
  fPlots["phChgIso"] = PlotPhotons::MakeTH1F("phChgIso","Photons #rho-corrected Charged Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Charged Hadron Isolation","Photons","RecoPhotons");
  fPlots["phNeuIso"] = PlotPhotons::MakeTH1F("phNeuIso","Photons #rho-corrected Neutral Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Neutral Hadron Isolation","Photons","RecoPhotons");
  fPlots["phIso"] = PlotPhotons::MakeTH1F("phIso","Photons #rho-corrected Photon Isolation (reco)",100,0.f,1000.f,"#rho-corrected Photon Isolation","Photons","RecoPhotons");
  fPlots["phsuisseX"] = PlotPhotons::MakeTH1F("phsuisseX","Photons Swiss Cross (reco)",100,0.f,1.f,"Swiss Cross","Photons","RecoPhotons");
  fPlots["phsieie"] = PlotPhotons::MakeTH1F("phsieie","Photons #sigma_{i#eta i#eta} (reco)",100,0,0.1,"#sigma_{i#eta i#eta}","Photons","RecoPhotons");
  fPlots["phsipip"] = PlotPhotons::MakeTH1F("phsipip","Photons #sigma_{i#phi i#phi} (reco)",100,0,0.1,"#sigma_{i#phi i#phi}","Photons","RecoPhotons");
  fPlots["phsieip"] = PlotPhotons::MakeTH1F("phsieip","Photons #sigma_{i#eta i#phi} (reco)",100,-0.005,0.005,"#sigma_{i#eta i#phi}","Photons","RecoPhotons");
  fPlots["phsmaj"] = PlotPhotons::MakeTH1F("phsmaj","Photons S_{major} (reco)",100,0.f,5.f,"S_{major}","Photons","RecoPhotons");
  fPlots["phsmin"] = PlotPhotons::MakeTH1F("phsmin","Photons S_{minor} (reco)",100,0.f,1.f,"S_{minor}","Photons","RecoPhotons");
  fPlots["phalpha"] = PlotPhotons::MakeTH1F("phalpha","Photons Cluster Shape #alpha (reco)",100,-2.f,2.f,"Cluster Shape #alpha","Photons","RecoPhotons");
  fPlots["phsmin_ov_phsmaj"] = PlotPhotons::MakeTH1F("phsmin_ov_phsmaj","Photons S_{minor} / S_{major} (reco)",100,0,1.f,"S_{minor}/S_{major}","Photons","RecoPhotons");
  fPlots["phscE"] = PlotPhotons::MakeTH1F("phscE","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2500.f,"Energy [GeV]","Photons","RecoPhotons");
  fPlots["phnrh"] = PlotPhotons::MakeTH1F("phnrh","nRecHits from Photons (reco)",100,0.f,100.f,"nRecHits","Photons","RecoPhotons");

  // all rec hits + seed info
  fPlots["phrhE"] = PlotPhotons::MakeTH1F("phrhE","Photons RecHits Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","RecHits","RecoPhotons");
  fPlots["phrhtime"] = PlotPhotons::MakeTH1F("phrhtime","Photons RecHits Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","RecHits","RecoPhotons");
  fPlots["phrhOOT"] = PlotPhotons::MakeTH1F("phrhOOT","Photons RecHits OoT Flag (reco)",2,0.f,2.f,"OoT Flag","RecHits","RecoPhotons");
  fPlots["phseedE"] = PlotPhotons::MakeTH1F("phseedE","Photons Seed RecHit Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Seed RecHits","RecoPhotons");
  fPlots["phseedtime"] = PlotPhotons::MakeTH1F("phseedtime","Photons Seed RecHit Time [ns] (reco)",100,-20.f,20.f,"Time [ns]","Seed RecHits","RecoPhotons");
  fPlots["phseedOOT"] = PlotPhotons::MakeTH1F("phseedOOT","Photons Seed RecHit OoT Flag (reco)",2,0.f,2.f,"OoT Flag","Seed RecHits","RecoPhotons");  

  // some 2D plots of interest
  fPlots2D["phsmin_vs_phsmaj"] = PlotPhotons::MakeTH2F("phsmin_vs_phsmaj","Photons S_{minor} vs Photons S_{major} (reco)",100,0.f,5.f,"Photon S_{major}",100,0.f,1.f,"Photon S_{minor}","RecoPhotons");
  fPlots2D["phseedtime_vs_phpt"] = PlotPhotons::MakeTH2F("phseedtime_vs_phpt","Photons Seed RecHit Time vs Photons p_{T}",100,0.f,2000.f,"Photon p_{T} [GeV/c]",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phHoE"] = PlotPhotons::MakeTH2F("phseedtime_vs_phHoE","Photons Seed RecHit Time vs Photons HE/EE",100,0.f,5.0,"Photon HE/EE",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phr9"] = PlotPhotons::MakeTH2F("phseedtime_vs_phr9","Photons Seed RecHit Time vs Photons R9",100,0.f,5.0,"Photon R9",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phsieie"] = PlotPhotons::MakeTH2F("phseedtime_vs_phsieie","Photons Seed RecHit Time vs Photons #sigma_{i#eta i#eta}",100,0,0.1,"Photon #sigma_{i#eta i#eta}",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phsmaj"] = PlotPhotons::MakeTH2F("phseedtime_vs_phsmaj","Photons Seed RecHit Time vs Photons S_{major}",100,0.f,5.f,"Photon S_{major}",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phsmin"] = PlotPhotons::MakeTH2F("phseedtime_vs_phsmin","Photons Seed RecHit Time vs Photons S_{minor}",100,0.f,1.f,"Photon S_{minor}",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phalpha"] = PlotPhotons::MakeTH2F("phseedtime_vs_phalpha","Photons Seed RecHit Time vs Photons Cluster #alpha",100,-2.f,2.f,"Photon Cluster Shape #alpha",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["phseedtime_vs_phsmin_ov_phsmaj"] = PlotPhotons::MakeTH2F("phseedtime_vs_phsmin_ov_phsmaj","Photon Seed RecHit Time vs Photon S_{minor}/S_{major}",100,0.f,1.f,"Photon S_{minor}/S_{major}",100,-20.f,20.f,"Photon Seed RecHit Time [ns]","RecoPhotons");
}

void PlotPhotons::SetupLeading()
{
  fPlots["ph1pt"] = PlotPhotons::MakeTH1F("ph1pt","Leading Photon p_{T} [GeV/c] (reco)",100,0.f,2000.f,"Leading Photon p_{T} [GeV/c]","Events","RecoPhotons/Leading");
  fPlots["ph1HoE"] = PlotPhotons::MakeTH1F("ph1HoE","Leading Photon HE/EE (reco)",100,0.f,5.0,"Leading Photon HE/EE","Events","RecoPhotons/Leading");
  fPlots["ph1r9"] = PlotPhotons::MakeTH1F("ph1r9","Leading Photon R9 (reco)",100,0.f,5.0,"Leading Photon R9","Events","RecoPhotons/Leading");
  fPlots["ph1sieie"] = PlotPhotons::MakeTH1F("ph1sieie","Leading Photon #sigma_{i#eta i#eta} (reco)",100,0,0.1,"Leading Photon #sigma_{i#eta i#eta}","Events","RecoPhotons/Leading");
  fPlots["ph1smaj"] = PlotPhotons::MakeTH1F("ph1smaj","Leading Photon S_{major} (reco)",100,0.f,5.f,"Leading Photon S_{major}","Events","RecoPhotons/Leading");
  fPlots["ph1smin"] = PlotPhotons::MakeTH1F("ph1smin","Leading Photon S_{minor} (reco)",100,0.f,1.f,"Leading Photon S_{minor}","Events","RecoPhotons/Leading");
  fPlots["ph1alpha"] = PlotPhotons::MakeTH1F("ph1alpha","Leading Photon Cluster Shape #alpha (reco)",100,-2.f,2.f,"Leading Photon Cluster Shape #alpha","Events","RecoPhotons/Leading");
  fPlots["ph1smin_ov_ph1smaj"] = PlotPhotons::MakeTH1F("ph1smin_ov_ph1smaj","Leading Photon S_{minor} / S_{major} (reco)",100,0,1.f,"Leading Photon S_{minor}/S_{major}","Events","RecoPhotons/Leading");
  fPlots["ph1seedtime"] = PlotPhotons::MakeTH1F("ph1seedtime","Leading Photon Seed RecHit Time [ns]",100,-5.f,20.f,"Leading Photon Seed RecHit Time [ns]","Events","RecoPhotons/Leading");
  fPlots2D["ph1smin_vs_ph1smaj"] = PlotPhotons::MakeTH2F("ph1smin_vs_ph1smaj","Leading Photon S_{minor} vs Leading Photon S_{major} (reco)",100,0.f,5.f,"Leading Photon S_{major}",100,0.f,1.f,"Leading Photon S_{minor}","RecoPhotons/Leading");

  fPlots2D["ph1seedtime_vs_ph1pt"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1pt","Leading Photon Seed RecHit Time vs Leading Photon p_{T}",100,0.f,2000.f,"Leading Photon p_{T} [GeV/c]",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1HoE"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1HoE","Leading Photon Seed RecHit Time vs Leading Photon HE/EE",100,0.f,5.0,"Leading Photon HE/EE",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1r9"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1r9","Leading Photon Seed RecHit Time vs Leading Photon R9",100,0.f,5.0,"Leading Photon R9",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1sieie"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1sieie","Leading Photon Seed RecHit Time vs Leading Photon #sigma_{i#eta i#eta}",100,0,0.1,"Leading Photon #sigma_{i#eta i#eta}",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1smaj"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1smaj","Leading Photon Seed RecHit Time vs Leading Photon S_{major}",100,0.f,5.f,"Leading Photon S_{major}",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1smin"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1smin","Leading Photon Seed RecHit Time vs Leading Photon S_{minor}",100,0.f,1.f,"Leading Photon S_{minor}",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1alpha"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1alpha","Leading Photon Seed RecHit Time vs Leading Photon #alpha",100,-2.f,2.f,"Leading Photon #alpha",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
  fPlots2D["ph1seedtime_vs_ph1smin_ov_ph1smaj"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1smin_ov_ph1smaj","Leading Photon Seed RecHit Time vs Leading Photon S_{minor}/S_{major}",100,0.f,1.f,"Leading Photon S_{minor}/S_{major}",100,-20.f,20.f,"Leading Photon Seed RecHit Time [ns]","RecoPhotons/Leading");
}

void PlotPhotons::SetupMostDelayed()
{
  fPlots["phdelaypt"] = PlotPhotons::MakeTH1F("phdelaypt","Most Delayed Photon p_{T} [GeV/c] (reco)",100,0.f,2000.f,"Most Delayed Photon p_{T} [GeV/c]","Events","RecoPhotons/MostDelayed");
  fPlots["phdelayHoE"] = PlotPhotons::MakeTH1F("phdelayHoE","Most Delayed Photon HE/EE (reco)",100,0.f,5.0,"Most Delayed Photon HE/EE","Events","RecoPhotons/MostDelayed");
  fPlots["phdelayr9"] = PlotPhotons::MakeTH1F("phdelayr9","Most Delayed Photon R9 (reco)",100,0.f,5.0,"Most Delayed Photon R9","Events","RecoPhotons/MostDelayed");
  fPlots["phdelaysieie"] = PlotPhotons::MakeTH1F("phdelaysieie","Most Delayed Photon #sigma_{i#eta i#eta} (reco)",100,0,0.1,"Most Delayed Photon #sigma_{i#eta i#eta}","Events","RecoPhotons/MostDelayed");
  fPlots["phdelaysmaj"] = PlotPhotons::MakeTH1F("phdelaysmaj","Most Delayed Photon S_{major} (reco)",100,0.f,5.f,"Most Delayed Photon S_{major}","Events","RecoPhotons/MostDelayed");
  fPlots["phdelaysmin"] = PlotPhotons::MakeTH1F("phdelaysmin","Most Delayed Photon S_{minor} (reco)",100,0.f,1.f,"Most Delayed Photon S_{minor}","Events","RecoPhotons/MostDelayed");
  fPlots["phdelayalpha"] = PlotPhotons::MakeTH1F("phdelayalpha","Most Delayed Photon Cluster Shape #alpha (reco)",100,-2.f,2.f,"Most Delayed Photon Cluster Shape #alpha","Events","RecoPhotons/MostDelayed");
  fPlots["phdelaysmin_ov_phdelaysmaj"] = PlotPhotons::MakeTH1F("phdelaysmin_ov_phdelaysmaj","Most Delayed Photon S_{minor} / S_{major} (reco)",100,0,1.f,"Most Delayed Photon S_{minor}/S_{major}","Events","RecoPhotons/MostDelayed");
  fPlots["phdelayseedtime"] = PlotPhotons::MakeTH1F("phdelayseedtime","Most Delayed Photon Seed RecHit Time [ns]",100,-5.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","Events","RecoPhotons/MostDelayed");
  fPlots2D["phdelaysmin_vs_phdelaysmaj"] = PlotPhotons::MakeTH2F("phdelaysmin_vs_phdelaysmaj","Most Delayed Photon S_{minor} vs Most Delayed Photon S_{major} (reco)",100,0.f,5.f,"Most Delayed Photon S_{major}",100,0.f,1.f,"Most Delayed Photon S_{minor}","RecoPhotons/MostDelayed");

  fPlots2D["phdelayseedtime_vs_phdelaypt"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelaypt","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T} [GeV/c]",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelayHoE"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelayHoE","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon HE/EE",100,0.f,5.0,"Most Delayed Photon HE/EE",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelayr9"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelayr9","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon R9",100,0.f,5.0,"Most Delayed Photon R9",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelaysieie"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelaysieie","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon #sigma_{i#eta i#eta}",100,0,0.1,"Most Delayed Photon #sigma_{i#eta i#eta}",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelaysmaj"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelaysmaj","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon S_{major}",100,0.f,5.f,"Most Delayed Photon S_{major}",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelaysmin"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelaysmin","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon S_{minor}",100,0.f,1.f,"Most Delayed Photon S_{minor}",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelayalpha"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelayalpha","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon #alpha",100,-2.f,2.f,"Most Delayed Photon Cluster Shape #alpha",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
  fPlots2D["phdelayseedtime_vs_phdelaysmin_ov_phdelaysmaj"] = PlotPhotons::MakeTH2F("phdelayseedtime_vs_phdelaysmin_ov_phdelaysmaj","Most Delayed Photon Seed RecHit Time vs Most Delayed Photon S_{minor}/S_{major}",100,0.f,1.f,"Most Delayed Photon S_{minor}/S_{major}",100,-20.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]","RecoPhotons/MostDelayed");
}

void PlotPhotons::SetupTrigger()
{
  if (Config::isHLT4)
  {
    fTrigPlots["phdelayseedtime_hltpho120_met40"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltpho120_met40","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_Photon120_R9Id90_HE10_Iso40_EBOnly_PFMET40_v","trigger");
    fTrigPlots["phdelayseedtime_hltpho175"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltpho175","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_Photon175_v","trigger");
    fTrigPlots["phdelayseedtime_hltdoublepho60"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdoublepho60","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DoublePhoton60_v","trigger");
    fTrigPlots["phdelayseedtime_hltdoublepho42_25_m15"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdoublepho42_25_m15","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_Photon42_R9Id85_OR_CaloId24b40e_Iso50T80L_Photon25_AND_HE10_R9Id65_Eta2_Mass15_v","trigger");
    fTrigPlots["phdelayseedtime_hltpho90_ht600"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltpho90_ht600","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_Photon90_CaloIdL_PFHT600_v","trigger");

    fTrigPlots["phdelayseedtime_hltdispho50_ht300"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho50_ht300","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton50_R9Id90_CaloIdL_IsoL_PFHT300_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho50_ht350"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho50_ht350","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton50_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho50_ht400"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho50_ht400","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton50_R9Id90_CaloIdL_IsoL_PFHT400_v","trigger");

    fTrigPlots["phdelayseedtime_hltdispho60_ht300"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_ht300","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT300_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_ht350"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_ht350","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_ht400"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_ht400","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT400_v","trigger");

    fTrigPlots["phdelayseedtime_hltdispho70_ht300"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho70_ht300","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton70_R9Id90_CaloIdL_IsoL_PFHT300_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho70_ht350"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho70_ht350","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton70_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho70_ht400"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho70_ht400","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","Events","HLT_DisplacedPhoton70_R9Id90_CaloIdL_IsoL_PFHT400_v","trigger");

    fTrigPlots["pu_hltdispho60_ht350"] = PlotPhotons::MakeTrigTH1Fs("pu_hltdispho60_ht350","n Primary Vertices",80,0.f,80.f,"n Primary Vertices","Events","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");

    fTrigPlots2D["ph1seedtime_vs_ph1pt_hltdispho50_ht350"] = PlotPhotons::MakeTrigTH2Fs("ph1seedtime_vs_ph1pt_hltdispho50_ht350","Leading Photon Seed recHit Time vs Leading Photon p_{T}",25,0,1000,"Leading Photon p_{T}",25,-5.f,20.f,"Leading Photon Seed recHit Time [ns]","HLT_DisplacedPhoton50_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots2D["ph1seedtime_vs_ph1pt_hltdispho60_ht350"] = PlotPhotons::MakeTrigTH2Fs("ph1seedtime_vs_ph1pt_hltdispho60_ht350","Leading Photon Seed recHit Time vs Leading Photon p_{T}",25,0,1000,"Leading Photon p_{T}",25,-5.f,20.f,"Leading Photon Seed recHit Time [ns]","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots2D["ph1seedtime_vs_ph1pt_hltdispho70_ht350"] = PlotPhotons::MakeTrigTH2Fs("ph1seedtime_vs_ph1pt_hltdispho70_ht350","Leading Photon Seed recHit Time vs Leading Photon p_{T}",25,0,1000,"Leading Photon p_{T}",25,-5.f,20.f,"Leading Photon Seed recHit Time [ns]","HLT_DisplacedPhoton70_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");

    fTrigPlots2D["phdelayseedtime_vs_phdelaypt_hltdispho50_ht350"] = PlotPhotons::MakeTrigTH2Fs("phdelayseedtime_vs_phdelaypt_hltdispho50_ht350","Most Delayed Photon Seed recHit Time vs Most Delayed Photon p_{T}",25,0,1000,"Most Delayed Photon p_{T}",25,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","HLT_DisplacedPhoton50_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots2D["phdelayseedtime_vs_phdelaypt_hltdispho60_ht350"] = PlotPhotons::MakeTrigTH2Fs("phdelayseedtime_vs_phdelaypt_hltdispho60_ht350","Most Delayed Photon Seed recHit Time vs Most Delayed Photon p_{T}",25,0,1000,"Most Delayed Photon p_{T}",25,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots2D["phdelayseedtime_vs_phdelaypt_hltdispho70_ht350"] = PlotPhotons::MakeTrigTH2Fs("phdelayseedtime_vs_phdelaypt_hltdispho70_ht350","Most Delayed Photon Seed recHit Time vs Most Delayed Photon p_{T}",25,0,1000,"Most Delayed Photon p_{T}",25,-5.f,20.f,"Most Delayed Photon Seed recHit Time [ns]","HLT_DisplacedPhoton70_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");

    fTrigPlots2D["phgoodseedtime_vs_phgoodpt_hltdispho50_ht350"] = PlotPhotons::MakeTrigTH2Fs("phgoodseedtime_vs_phgoodpt_hltdispho50_ht350","Photons Seed recHit Time vs Photons p_{T}",25,0,1000,"Photons p_{T}",25,-5.f,20.f,"Photons Seed recHit Time [ns]","HLT_DisplacedPhoton50_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots2D["phgoodseedtime_vs_phgoodpt_hltdispho60_ht350"] = PlotPhotons::MakeTrigTH2Fs("phgoodseedtime_vs_phgoodpt_hltdispho60_ht350","Photons Seed recHit Time vs Photons p_{T}",25,0,1000,"Photons p_{T}",25,-5.f,20.f,"Photons Seed recHit Time [ns]","HLT_DisplacedPhoton60_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
    fTrigPlots2D["phgoodseedtime_vs_phgoodpt_hltdispho70_ht350"] = PlotPhotons::MakeTrigTH2Fs("phgoodseedtime_vs_phgoodpt_hltdispho70_ht350","Photons Seed recHit Time vs Photons p_{T}",25,0,1000,"Photons p_{T}",25,-5.f,20.f,"Photons Seed recHit Time [ns]","HLT_DisplacedPhoton70_R9Id90_CaloIdL_IsoL_PFHT350_v","trigger");
  } // isHLT4
}

void PlotPhotons::SetupAnalysis()
{
  fPlots2D["MET_vs_ph1seedtime"] = PlotPhotons::MakeTH2F("MET_vs_ph1seedtime","MET vs Leading Photon Seed RecHit Time",100,-5.f,20.f,"Leading Photon Seed RecHit Time [ns]",100,0.f,2000.f,"MET","Analysis");
  fPlots2D["MET_vs_phdelayseedtime"] = PlotPhotons::MakeTH2F("MET_vs_phdelayseedtime","MET vs Most Delayed Photon Seed RecHit Time",100,-5.f,20.f,"Most Delayed Photon Seed RecHit Time [ns]",100,0.f,2000.f,"MET","Analysis");
}

TH1F * PlotPhotons::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

std::pair<TH1F*,TH1F*> PlotPhotons::MakeTrigTH1Fs(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString path, TString subdir)
{
  TH1F * denom = new TH1F(Form("%s_denom",hname.Data()),Form("%s %s [Denom]",htitle.Data(),path.Data()),nbinsx,xlow,xhigh);
  denom->SetLineColor(kBlack);
  denom->GetXaxis()->SetTitle(Form("%s %s [Denom]",xtitle.Data(),path.Data()));
  denom->GetYaxis()->SetTitle(ytitle.Data());
  denom->Sumw2();

  fSubDirs[denom->GetName()] = subdir;

  TH1F * numer = new TH1F(Form("%s_numer",hname.Data()),Form("%s %s [Numer]",htitle.Data(),path.Data()),nbinsx,xlow,xhigh);
  numer->SetLineColor(kBlack);
  numer->GetXaxis()->SetTitle(Form("%s %s [Numer]",xtitle.Data(),path.Data()));
  numer->GetYaxis()->SetTitle(ytitle.Data());
  numer->Sumw2();

  fSubDirs[numer->GetName()] = subdir;
  
  return std::make_pair(denom,numer);
}

void PlotPhotons::FillTriggerPlot(TH1Pair & th1pair, const Bool_t passed, const Float_t value)
{
  th1pair.first->Fill(value);
  if (passed) th1pair.second->Fill(value);
}

void PlotPhotons::MakeEffPlot(TH1F *& eff, TString hname, TH1F *& denom, TH1F *& numer)
{
  TString title = denom->GetTitle();
  Ssiz_t  start = title.Index("HLT_"); 
  Ssiz_t  end   = title.Index(" [Denom]"); 
  TString var   (title(0,start-1));
  TString path  (title(start,end-start));
  
  TString xtitle = denom->GetXaxis()->GetTitle();
  Ssiz_t  xhlt   = xtitle.Index("HLT_"); 
  TString xvar   (xtitle(0,xhlt-1));
  
  eff = new TH1F(Form("%s_eff",hname.Data()),Form("%s %s Efficiency",var.Data(),path.Data()),denom->GetNbinsX(),denom->GetXaxis()->GetBinLowEdge(1),denom->GetXaxis()->GetBinUpEdge(denom->GetNbinsX()));
  eff->SetLineColor(kBlack);
  eff->GetXaxis()->SetTitle(Form("%s %s [Efficiency]",xvar.Data(),path.Data()));
  eff->GetYaxis()->SetTitle(denom->GetYaxis()->GetTitle());
  eff->Sumw2();
  
  fSubDirs[eff->GetName()] = fSubDirs[denom->GetName()];

  Double_t value = 0;
  Double_t err   = 0;
  for (Int_t ibin = 1; ibin <= eff->GetNbinsX(); ibin++)
  {
    if (denom->GetBinContent(ibin)!=0)
    {
      value = numer->GetBinContent(ibin) / denom->GetBinContent(ibin); 
      // Binonimal errors 
      err = sqrt( value*(1.0-value)/denom->GetBinContent(ibin) );
      //Fill plots with correct values
      eff->SetBinContent(ibin,value);
      eff->SetBinError  (ibin,err);
    }
  }

  // set consistent min/max for comparisons...
  // eff->SetMininum(0.f);
  // eff->SetMaxinum(1.1f);
}

TH2F * PlotPhotons::MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir)
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

void PlotPhotons::FillTriggerPlot2D(TH2Pair & th2pair, const Bool_t passed, const Float_t xvalue, const Float_t yvalue)
{
  th2pair.first->Fill(xvalue,yvalue);
  if (passed) th2pair.second->Fill(xvalue,yvalue);
}

std::pair<TH2F*,TH2F*> PlotPhotons::MakeTrigTH2Fs(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString path, TString subdir)
{
  TH2F * denom = new TH2F(Form("%s_denom",hname.Data()),Form("%s %s [Denom]",htitle.Data(),path.Data()),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  denom->SetLineColor(kBlack);
  denom->GetXaxis()->SetTitle(Form("%s %s [Denom]",xtitle.Data(),path.Data()));
  denom->GetYaxis()->SetTitle(Form("%s %s [Denom]",ytitle.Data(),path.Data()));
  denom->Sumw2();

  fSubDirs[denom->GetName()] = subdir;

  TH2F * numer = new TH2F(Form("%s_numer",hname.Data()),Form("%s %s [Numer]",htitle.Data(),path.Data()),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  numer->SetLineColor(kBlack);
  numer->GetXaxis()->SetTitle(Form("%s %s [Numer]",xtitle.Data(),path.Data()));
  numer->GetYaxis()->SetTitle(Form("%s %s [Numer]",ytitle.Data(),path.Data()));
  numer->Sumw2();

  fSubDirs[numer->GetName()] = subdir;
  
  return std::make_pair(denom,numer);
}

void PlotPhotons::MakeEffPlot2D(TH2F *& eff, TString hname, TH2F *& denom, TH2F *& numer)
{
  TString title = denom->GetTitle();
  Ssiz_t  start = title.Index("HLT_"); 
  Ssiz_t  end   = title.Index(" [Denom]"); 
  TString var   (title(0,start-1));
  TString path  (title(start,end-start));
  
  TString xtitle = denom->GetXaxis()->GetTitle();
  Ssiz_t  xhlt   = xtitle.Index("HLT_"); 
  TString xvar   (xtitle(0,xhlt-1));

  TString ytitle = denom->GetYaxis()->GetTitle();
  Ssiz_t  yhlt   = ytitle.Index("HLT_"); 
  TString yvar   (ytitle(0,xhlt-1));
  
  eff = new TH2F(Form("%s_eff",hname.Data()),Form("%s %s Efficiency",var.Data(),path.Data()),denom->GetNbinsX(),denom->GetXaxis()->GetBinLowEdge(1),denom->GetXaxis()->GetBinUpEdge(denom->GetNbinsX()),denom->GetNbinsY(),denom->GetYaxis()->GetBinLowEdge(1),denom->GetYaxis()->GetBinUpEdge(denom->GetNbinsY()));
  eff->SetLineColor(kBlack);
  eff->GetXaxis()->SetTitle(Form("%s [Efficiency]",xvar.Data()));
  eff->GetYaxis()->SetTitle(Form("%s [Efficiency]",yvar.Data()));
  eff->Sumw2();
  
  eff->SetStats(0);
  fSubDirs[eff->GetName()] = fSubDirs[denom->GetName()];

  Double_t value = 0;
  Double_t err   = 0;
  for (Int_t ibinx = 1; ibinx <= eff->GetNbinsX(); ibinx++)
  {
    for (Int_t ibiny = 1; ibiny <= eff->GetNbinsY(); ibiny++)
    {
      if (denom->GetBinContent(ibinx,ibiny)!=0.f)
      {
	value = numer->GetBinContent(ibinx,ibiny) / denom->GetBinContent(ibinx,ibiny); 
	// Binonimal errors 
	err = sqrt( value*(1.0-value)/denom->GetBinContent(ibinx,ibiny) );
	//Fill plots with correct values
	eff->SetBinContent(ibinx,ibiny,value);
	eff->SetBinError  (ibinx,ibiny,err);
      }
    }
  }

  // set min/max to 0.0/1.0 to make consistent comparisons
  eff->SetMinimum(0.f);
  eff->SetMaximum(1.f);
}

void PlotPhotons::DumpEventCounts()
{
  std::ofstream output;
  output.open(Form("%s/efficiency.txt",fOutDir.Data()),std::ios_base::trunc);

  std::cout << std::endl << "-----------------" << std::endl << std::endl;
  std::cout << fOutDir.Data() << std::endl << std::endl;

  Int_t total = fInTree->GetEntries();

  std::cout << "Total Events: " << total << std::endl;
  output << "Total " << total << std::endl;

  for (TStrIntMapIter mapiter = fEfficiency.begin(); mapiter != fEfficiency.end(); ++mapiter)
  {
    const Int_t   passed      = mapiter->second;
    const Float_t efficiency  = float(passed)/float(total);
    const Float_t uncertainty = std::sqrt((efficiency*(1.f-efficiency))/float(total));

    std::cout << "nEvents Passing " << mapiter->first.Data() << " Selection: " << passed << std::endl;
    output << mapiter->first.Data() << " " << passed << " " << efficiency << " " << uncertainty << std::endl;
  }

  // Compute event level efficiency + binomial error
  const Int_t   passed      = fEfficiency["Events"];
  const Float_t efficiency  = float(passed)/float(total);
  const Float_t uncertainty = std::sqrt((efficiency*(1.f-efficiency))/float(total));

  std::cout << "Event level efficiency: " << efficiency << " +/- " << uncertainty << std::endl;
  std::cout << std::endl << "-----------------" << std::endl << std::endl;
  output.close();
}

void PlotPhotons::MakeSubDirs()
{
  for (TStrMapIter mapiter = fSubDirs.begin(); mapiter != fSubDirs.end(); ++mapiter)
  {
    TString subdir = Form("%s/%s",fOutDir.Data(),mapiter->second.Data());

    FileStat_t dummyFileStat; 
    if (gSystem->GetPathInfo(subdir.Data(), dummyFileStat) == 1)
    {
      TString mkDir = Form("mkdir -p %s",subdir.Data());
      gSystem->Exec(mkDir.Data());
      gSystem->Exec(Form("%s/lin",mkDir.Data()));
      gSystem->Exec(Form("%s/log",mkDir.Data()));
    }
  }
}

void PlotPhotons::OutputTH1Fs()
{
  fOutFile->cd();

  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 

    if (fSaveHists)
    {
      // now draw onto canvas to save as png
      TCanvas * canv = new TCanvas("canv","canv");
      canv->cd();
      mapiter->second->Draw("HIST");
      
      // first save as linear, then log
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->first.Data()));
      
      canv->SetLogy(1);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/log/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
      else            canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),mapiter->first.Data()));
      
      delete canv;
    }

    delete mapiter->second;
  }
  fPlots.clear();
}

void PlotPhotons::OutputTrigTH1Fs()
{
  fOutFile->cd();

  for (TH1PairMapIter mapiter = fTrigPlots.begin(); mapiter != fTrigPlots.end(); ++mapiter) 
  { 
    // make efficiency plot
    TH1F * eff;
    PlotPhotons::MakeEffPlot(eff,mapiter->first,mapiter->second.first,mapiter->second.second);

    // save to output file
    mapiter->second.first ->Write(mapiter->second.first ->GetName(),TObject::kWriteDelete); // denom
    mapiter->second.second->Write(mapiter->second.second->GetName(),TObject::kWriteDelete); // numer
    eff->Write(eff->GetName(),TObject::kWriteDelete);

    if (fSaveHists)
    {
      // now draw onto canvas to save as png
      TCanvas * canv = new TCanvas("canv","canv");
      canv->cd();

      // first denom
      mapiter->second.first->Draw("HIST");
      
      // first save as linear, then log
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDump.Data(),fSubDirs[mapiter->second.first->GetName()].Data(),mapiter->second.first->GetName()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->second.first->GetName()));
      
      canv->SetLogy(1);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/log/%s.png",fOutDump.Data(),fSubDirs[mapiter->second.first->GetName()].Data(),mapiter->second.first->GetName()));
      else            canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),mapiter->second.first->GetName()));

      // second numer
      mapiter->second.second->Draw("HIST");
      
      // first save as linear, then log
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDump.Data(),fSubDirs[mapiter->second.second->GetName()].Data(),mapiter->second.second->GetName()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->second.second->GetName()));
      
      canv->SetLogy(1);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/log/%s.png",fOutDump.Data(),fSubDirs[mapiter->second.second->GetName()].Data(),mapiter->second.second->GetName()));
      else            canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),mapiter->second.second->GetName()));

      // efficiency last
      eff->Draw("EP");
      
      // first save as linear, then log
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDump.Data(),fSubDirs[eff->GetName()].Data(),eff->GetName()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),eff->GetName()));
      
      canv->SetLogy(1);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/log/%s.png",fOutDump.Data(),fSubDirs[eff->GetName()].Data(),eff->GetName()));
      else            canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),eff->GetName()));
      
      delete canv;
    }

    delete eff;
    delete mapiter->second.first;
    delete mapiter->second.second;
  }
  fTrigPlots.clear();
}

void PlotPhotons::OutputTH2Fs()
{
  fOutFile->cd();

  for (TH2MapIter mapiter = fPlots2D.begin(); mapiter != fPlots2D.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    
    if (fSaveHists)
    {
      // now draw onto canvas to save as png
      TCanvas * canv = new TCanvas("canv","canv");
      canv->cd();
      mapiter->second->Draw("colz");
      
      // first save as linear, then log
      canv->SetLogy(0);
      if (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
      else          canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->first.Data()));
    
      delete canv;
    }

    delete mapiter->second;
  }
  fPlots2D.clear();
}

void PlotPhotons::OutputTrigTH2Fs()
{
  fOutFile->cd();

  for (TH2PairMapIter mapiter = fTrigPlots2D.begin(); mapiter != fTrigPlots2D.end(); ++mapiter) 
  { 
    // make efficiency plot
    TH2F * eff;
    PlotPhotons::MakeEffPlot2D(eff,mapiter->first,mapiter->second.first,mapiter->second.second);

    // save to output file
    mapiter->second.first ->Write(mapiter->second.first ->GetName(),TObject::kWriteDelete); // denom
    mapiter->second.second->Write(mapiter->second.second->GetName(),TObject::kWriteDelete); // numer
    eff->Write(eff->GetName(),TObject::kWriteDelete);

    if (fSaveHists)
    {
      // now draw onto canvas to save as png
      TCanvas * canv = new TCanvas("canv","canv");
      canv->cd();

      // first denom
      mapiter->second.first->Draw("COLZ");
      
      // only lin
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDump.Data(),fSubDirs[mapiter->second.first->GetName()].Data(),mapiter->second.first->GetName()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->second.first->GetName()));

      // second numer
      mapiter->second.second->Draw("COLZ");
      
      // only lin
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDump.Data(),fSubDirs[mapiter->second.second->GetName()].Data(),mapiter->second.second->GetName()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->second.second->GetName()));

      // efficiency last
      eff->Draw("COLZ TEXT");
      
      // only lin
      canv->SetLogy(0);
      if   (fSaveSub) canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDump.Data(),fSubDirs[eff->GetName()].Data(),eff->GetName()));
      else            canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),eff->GetName()));
      
      delete canv;
    }

    delete eff;
    delete mapiter->second.first;
    delete mapiter->second.second;
  }
  fTrigPlots2D.clear();
}

void PlotPhotons::InitTree()
{
  // Set object pointer
  triggerBits = 0;
  genjetmatch = 0;
  genjetE = 0;
  genjetpt = 0;
  genjetphi = 0;
  genjeteta = 0;
  jetmatch = 0;
  jetE = 0;
  jetpt = 0;
  jetphi = 0;
  jeteta = 0;
  phE = 0;
  phpt = 0;
  phphi = 0;
  pheta = 0;
  phmatch = 0;
  phIsGenMatched = 0;
  phscE = 0;
  phsceta = 0;
  phscphi = 0;
  phHoE = 0;
  phr9 = 0;
  phChgIso = 0;
  phNeuIso = 0;
  phIso = 0;
  phsuisseX = 0;
  phsieie = 0;
  phsipip = 0;
  phsieip = 0;
  phsmaj = 0;
  phsmin = 0;
  phalpha = 0;
  phVID = 0;
  phHoE_b = 0;
  phsieie_b = 0;
  phChgIso_b = 0;
  phNeuIso_b = 0;
  phIso_b = 0;
  phIsHLTMatched = 0;
  phnrh = 0;
  phseedpos = 0;
  phrheta = 0;
  phrhphi = 0;
  phrhE = 0;
  phrhtime = 0;
  phrhID = 0;
  phrhOOT = 0;
  if (fIsHVDS)
  {
    genvPionprodvx = 0;
    genvPionprodvy = 0;
    genvPionprodvz = 0;
    genvPiondecayvx = 0;
    genvPiondecayvy = 0;
    genvPiondecayvz = 0;
    genvPionmass = 0;
    genvPionE = 0;
    genvPionpt = 0;
    genvPionphi = 0;
    genvPioneta = 0;
    genHVph1E = 0;
    genHVph1pt = 0;
    genHVph1phi = 0;
    genHVph1eta = 0;
    genHVph2E = 0;
    genHVph2pt = 0;
    genHVph2phi = 0;
    genHVph2eta = 0;
    genHVph1match = 0;
    genHVph2match = 0;
  }

  // set branch addresses
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);

  if (Config::isHLT2 || Config::isHLT3 || Config::isHLT4 || Config::isuserHLT)
  {
    fInTree->SetBranchAddress("triggerBits", &triggerBits, &b_triggerBits);
  }

  if (fIsMC)
  {
    fInTree->SetBranchAddress("genwgt", &genwgt, &b_genwgt);
    fInTree->SetBranchAddress("genpuobs", &genpuobs, &b_genpuobs);
    fInTree->SetBranchAddress("genputrue", &genputrue, &b_genputrue);
  }
  if (fIsGMSB)
  {
    fInTree->SetBranchAddress("nNeutralino", &nNeutralino, &b_nNeutralino);
    fInTree->SetBranchAddress("nNeutoPhGr", &nNeutoPhGr, &b_nNeutoPhGr);
    fInTree->SetBranchAddress("genN1mass", &genN1mass, &b_genN1mass);
    fInTree->SetBranchAddress("genN1E", &genN1E, &b_genN1E);
    fInTree->SetBranchAddress("genN1pt", &genN1pt, &b_genN1pt);
    fInTree->SetBranchAddress("genN1phi", &genN1phi, &b_genN1phi);
    fInTree->SetBranchAddress("genN1eta", &genN1eta, &b_genN1eta);
    fInTree->SetBranchAddress("genN1prodvx", &genN1prodvx, &b_genN1prodvx);
    fInTree->SetBranchAddress("genN1prodvy", &genN1prodvy, &b_genN1prodvy);
    fInTree->SetBranchAddress("genN1prodvz", &genN1prodvz, &b_genN1prodvz);
    fInTree->SetBranchAddress("genN1decayvx", &genN1decayvx, &b_genN1decayvx);
    fInTree->SetBranchAddress("genN1decayvy", &genN1decayvy, &b_genN1decayvy);
    fInTree->SetBranchAddress("genN1decayvz", &genN1decayvz, &b_genN1decayvz);
    fInTree->SetBranchAddress("genph1E", &genph1E, &b_genph1E);
    fInTree->SetBranchAddress("genph1pt", &genph1pt, &b_genph1pt);
    fInTree->SetBranchAddress("genph1phi", &genph1phi, &b_genph1phi);
    fInTree->SetBranchAddress("genph1eta", &genph1eta, &b_genph1eta);
    fInTree->SetBranchAddress("genph1match", &genph1match, &b_genph1match);
    fInTree->SetBranchAddress("gengr1mass", &gengr1mass, &b_gengr1mass);
    fInTree->SetBranchAddress("gengr1E", &gengr1E, &b_gengr1E);
    fInTree->SetBranchAddress("gengr1pt", &gengr1pt, &b_gengr1pt);
    fInTree->SetBranchAddress("gengr1phi", &gengr1phi, &b_gengr1phi);
    fInTree->SetBranchAddress("gengr1eta", &gengr1eta, &b_gengr1eta);
    fInTree->SetBranchAddress("genN2mass", &genN2mass, &b_genN2mass);
    fInTree->SetBranchAddress("genN2E", &genN2E, &b_genN2E);
    fInTree->SetBranchAddress("genN2pt", &genN2pt, &b_genN2pt);
    fInTree->SetBranchAddress("genN2phi", &genN2phi, &b_genN2phi);
    fInTree->SetBranchAddress("genN2eta", &genN2eta, &b_genN2eta);
    fInTree->SetBranchAddress("genN2prodvx", &genN2prodvx, &b_genN2prodvx);
    fInTree->SetBranchAddress("genN2prodvy", &genN2prodvy, &b_genN2prodvy);
    fInTree->SetBranchAddress("genN2prodvz", &genN2prodvz, &b_genN2prodvz);
    fInTree->SetBranchAddress("genN2decayvx", &genN2decayvx, &b_genN2decayvx);
    fInTree->SetBranchAddress("genN2decayvy", &genN2decayvy, &b_genN2decayvy);
    fInTree->SetBranchAddress("genN2decayvz", &genN2decayvz, &b_genN2decayvz);
    fInTree->SetBranchAddress("genph2E", &genph2E, &b_genph2E);
    fInTree->SetBranchAddress("genph2pt", &genph2pt, &b_genph2pt);
    fInTree->SetBranchAddress("genph2phi", &genph2phi, &b_genph2phi);
    fInTree->SetBranchAddress("genph2eta", &genph2eta, &b_genph2eta);
    fInTree->SetBranchAddress("genph2match", &genph2match, &b_genph2match);
    fInTree->SetBranchAddress("gengr2mass", &gengr2mass, &b_gengr2mass);
    fInTree->SetBranchAddress("gengr2E", &gengr2E, &b_gengr2E);
    fInTree->SetBranchAddress("gengr2pt", &gengr2pt, &b_gengr2pt);
    fInTree->SetBranchAddress("gengr2phi", &gengr2phi, &b_gengr2phi);
    fInTree->SetBranchAddress("gengr2eta", &gengr2eta, &b_gengr2eta);
    fInTree->SetBranchAddress("ngenjets", &ngenjets, &b_ngenjets);
    fInTree->SetBranchAddress("genjetmatch", &genjetmatch, &b_genjetmatch);
    fInTree->SetBranchAddress("genjetE", &genjetE, &b_genjetE);
    fInTree->SetBranchAddress("genjetpt", &genjetpt, &b_genjetpt);
    fInTree->SetBranchAddress("genjetphi", &genjetphi, &b_genjetphi);
    fInTree->SetBranchAddress("genjeteta", &genjeteta, &b_genjeteta);
  }
  if (fIsHVDS)
  {
    fInTree->SetBranchAddress("nvPions", &nvPions, &b_nvPions);
    fInTree->SetBranchAddress("genvPionprodvx", &genvPionprodvx, &b_genvPionprodvx);
    fInTree->SetBranchAddress("genvPionprodvy", &genvPionprodvy, &b_genvPionprodvy);
    fInTree->SetBranchAddress("genvPionprodvz", &genvPionprodvz, &b_genvPionprodvz);
    fInTree->SetBranchAddress("genvPiondecayvx", &genvPiondecayvx, &b_genvPiondecayvx);
    fInTree->SetBranchAddress("genvPiondecayvy", &genvPiondecayvy, &b_genvPiondecayvy);
    fInTree->SetBranchAddress("genvPiondecayvz", &genvPiondecayvz, &b_genvPiondecayvz);
    fInTree->SetBranchAddress("genvPionmass", &genvPionmass, &b_genvPionmass);
    fInTree->SetBranchAddress("genvPionE", &genvPionE, &b_genvPionE);
    fInTree->SetBranchAddress("genvPionpt", &genvPionpt, &b_genvPionpt);
    fInTree->SetBranchAddress("genvPionphi", &genvPionphi, &b_genvPionphi);
    fInTree->SetBranchAddress("genvPioneta", &genvPioneta, &b_genvPioneta);
    fInTree->SetBranchAddress("genHVph1E", &genHVph1E, &b_genHVph1E);
    fInTree->SetBranchAddress("genHVph1pt", &genHVph1pt, &b_genHVph1pt);
    fInTree->SetBranchAddress("genHVph1phi", &genHVph1phi, &b_genHVph1phi);
    fInTree->SetBranchAddress("genHVph1eta", &genHVph1eta, &b_genHVph1eta);
    fInTree->SetBranchAddress("genHVph2E", &genHVph2E, &b_genHVph2E);
    fInTree->SetBranchAddress("genHVph2pt", &genHVph2pt, &b_genHVph2pt);
    fInTree->SetBranchAddress("genHVph2phi", &genHVph2phi, &b_genHVph2phi);
    fInTree->SetBranchAddress("genHVph2eta", &genHVph2eta, &b_genHVph2eta);
    fInTree->SetBranchAddress("genHVph1match", &genHVph1match, &b_genHVph1match);
    fInTree->SetBranchAddress("genHVph2match", &genHVph2match, &b_genHVph2match);
  }
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
  fInTree->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
  fInTree->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
  fInTree->SetBranchAddress("t1pfMETpt", &t1pfMETpt, &b_t1pfMETpt);
  fInTree->SetBranchAddress("t1pfMETphi", &t1pfMETphi, &b_t1pfMETphi);
  fInTree->SetBranchAddress("t1pfMETsumEt", &t1pfMETsumEt, &b_t1pfMETsumEt);
  fInTree->SetBranchAddress("t1pfMETuncorpt", &t1pfMETuncorpt, &b_t1pfMETuncorpt);
  fInTree->SetBranchAddress("t1pfMETuncorphi", &t1pfMETuncorphi, &b_t1pfMETuncorphi);
  fInTree->SetBranchAddress("t1pfMETuncorsumEt", &t1pfMETuncorsumEt, &b_t1pfMETuncorsumEt);
  fInTree->SetBranchAddress("t1pfMETcalopt", &t1pfMETcalopt, &b_t1pfMETcalopt);
  fInTree->SetBranchAddress("t1pfMETcalophi", &t1pfMETcalophi, &b_t1pfMETcalophi);
  fInTree->SetBranchAddress("t1pfMETcalosumEt", &t1pfMETcalosumEt, &b_t1pfMETcalosumEt);
  if (fIsGMSB || fIsHVDS)
  {
    fInTree->SetBranchAddress("t1pfMETgenMETpt", &t1pfMETgenMETpt, &b_t1pfMETgenMETpt);
    fInTree->SetBranchAddress("t1pfMETgenMETphi", &t1pfMETgenMETphi, &b_t1pfMETgenMETphi);
    fInTree->SetBranchAddress("t1pfMETgenMETsumEt", &t1pfMETgenMETsumEt, &b_t1pfMETgenMETsumEt);
  }
  fInTree->SetBranchAddress("njets", &njets, &b_njets);
  if (fIsGMSB) fInTree->SetBranchAddress("jetmatch", &jetmatch, &b_jetmatch);
  fInTree->SetBranchAddress("jetE", &jetE, &b_jetE);
  fInTree->SetBranchAddress("jetpt", &jetpt, &b_jetpt);
  fInTree->SetBranchAddress("jetphi", &jetphi, &b_jetphi);
  fInTree->SetBranchAddress("jeteta", &jeteta, &b_jeteta);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fInTree->SetBranchAddress("phE", &phE, &b_phE);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
  if (fIsGMSB || fIsHVDS) fInTree->SetBranchAddress("phmatch", &phmatch, &b_phmatch);
  if (fIsMC) fInTree->SetBranchAddress("phIsGenMatched", &phIsGenMatched, &b_phIsGenMatched);
  fInTree->SetBranchAddress("phscE", &phscE, &b_phscE);
  fInTree->SetBranchAddress("phsceta", &phsceta, &b_phsceta);
  fInTree->SetBranchAddress("phscphi", &phscphi, &b_phscphi);
  fInTree->SetBranchAddress("phHoE", &phHoE, &b_phHoE);
  fInTree->SetBranchAddress("phr9", &phr9, &b_phr9);
  fInTree->SetBranchAddress("phChgIso", &phChgIso, &b_phChgIso);
  fInTree->SetBranchAddress("phNeuIso", &phNeuIso, &b_phNeuIso);
  fInTree->SetBranchAddress("phIso", &phIso, &b_phIso);
  fInTree->SetBranchAddress("phsuisseX", &phsuisseX, &b_phsuisseX);
  fInTree->SetBranchAddress("phsieie", &phsieie, &b_phsieie);
  fInTree->SetBranchAddress("phsipip", &phsipip, &b_phsipip);
  fInTree->SetBranchAddress("phsieip", &phsieip, &b_phsieip);
  fInTree->SetBranchAddress("phsmaj", &phsmaj, &b_phsmaj);
  fInTree->SetBranchAddress("phsmin", &phsmin, &b_phsmin);
  fInTree->SetBranchAddress("phalpha", &phalpha, &b_phalpha);
  fInTree->SetBranchAddress("phVID", &phVID, &b_phVID);
  fInTree->SetBranchAddress("phHoE_b", &phHoE_b, &b_phHoE_b);
  fInTree->SetBranchAddress("phsieie_b", &phsieie_b, &b_phsieie_b);
  fInTree->SetBranchAddress("phChgIso_b", &phChgIso_b, &b_phChgIso_b);
  fInTree->SetBranchAddress("phNeuIso_b", &phNeuIso_b, &b_phNeuIso_b);
  fInTree->SetBranchAddress("phIso_b", &phIso_b, &b_phIso_b);
  fInTree->SetBranchAddress("phIsHLTMatched", &phIsHLTMatched, &b_phIsHLTMatched);
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrheta", &phrheta, &b_phrheta);
  fInTree->SetBranchAddress("phrhphi", &phrhphi, &b_phrhphi);
  fInTree->SetBranchAddress("phrhE", &phrhE, &b_phrhE);
  fInTree->SetBranchAddress("phrhtime", &phrhtime, &b_phrhtime);
  fInTree->SetBranchAddress("phrhID", &phrhID, &b_phrhID);
  fInTree->SetBranchAddress("phrhOOT", &phrhOOT, &b_phrhOOT);
}

void PlotPhotons::InitGenericConfig(TString config)
{
  std::ifstream input;
  input.open(config.Data(),std::ios::in);
  
  TString var = "", val = "";
  while (input >> var >> val)
  {
    if (var.EqualTo("NEvCheck"))   Config::NEvCheck = val.Atoi();
    if (var.EqualTo("ApplyEvCut")) Config::ApplyEvCut = val.Atoi();
  }
}

void PlotPhotons::InitHLTConfig(TString config)
{
  std::ifstream input;
  input.open(config.Data(),std::ios::in);
  
  TString var = "", val = "";
  while (input >> var >> val)
  {
    if (var.EqualTo("isHLT2")) Config::isHLT2 = val.Atoi();
    if (var.EqualTo("isHLT3")) Config::isHLT3 = val.Atoi();
    if (var.EqualTo("isHLT4")) Config::isHLT4 = val.Atoi();
    if (var.EqualTo("isuserHLT")) Config::isuserHLT = val.Atoi();
    if (var.EqualTo("ApplyHLTMatching")) Config::ApplyHLTMatching = val.Atoi();
    if (var.EqualTo("filterIdx")) Config::filterIdx = val.Atoi();
  }
}

void PlotPhotons::InitJetConfig(TString config)
{
  std::ifstream input;
  input.open(config.Data(),std::ios::in);
  
  TString var = "", val = "";
  while (input >> var >> val)
  {
    if (var.EqualTo("ApplyJetHtCut")) Config::ApplyJetHtCut = val.Atoi();
    if (var.EqualTo("JetHtCut")) Config::JetHtCut = val.Atof();
    if (var.EqualTo("ApplyMinJetPtCut")) Config::ApplyMinJetPtCut = val.Atoi();
    if (var.EqualTo("MinJetPtCut")) Config::MinJetPtCut = val.Atof();
    if (var.EqualTo("ApplyJetPtCut")) Config::ApplyJetPtCut = val.Atoi();
    if (var.EqualTo("JetPtCut")) Config::JetPtCut = val.Atof();
    if (var.EqualTo("ApplyNJetsCut")) Config::ApplyNJetsCut = val.Atoi();
    if (var.EqualTo("NJetsCut")) Config::NJetsCut = val.Atoi();
  }
}

void PlotPhotons::InitPh1Config(TString config)
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

void PlotPhotons::InitPhAnyConfig(TString config)
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

void PlotPhotons::InitPhotonConfig(TString config)
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
