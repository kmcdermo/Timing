#include "PlotPhotons.hh"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotPhotons::PlotPhotons(TString filename, Bool_t isGMSB, Bool_t isHVDS, Bool_t isBkg, 
			 Bool_t isHLT2, Bool_t isHLT3,
			 Bool_t applyevcut, TString outdir, Bool_t savehists,
			 Bool_t applyjetptcut, Float_t jetptcut, Bool_t applynjetscut, Int_t njetscut,
			 Bool_t applyph1ptcut, Float_t ph1ptcut, Bool_t applyph1vidcut, TString ph1vid,
			 Bool_t applyphanyptcut, Float_t phanyptcut, Bool_t applyphanyvidcut, TString phanyvid,
			 Bool_t applyrhecut, Float_t rhEcut,
			 Bool_t applyecalacceptcut, Bool_t applyEBonly, Bool_t applyEEonly,
			 Bool_t applyphmcmatchingcut, Bool_t applyexactphmcmatch, Bool_t applyantiphmcmatch) :
  fOutDir(outdir), fIsGMSB(isGMSB), fIsHVDS(isHVDS), fIsBkg(isBkg), 
  fIsHLT2(isHLT2), fIsHLT3(isHLT3),
  fApplyEvCut(applyevcut), fSaveHists(savehists),
  fApplyJetPtCut(applyjetptcut), fJetPtCut(jetptcut), fApplyNJetsCut(applynjetscut), fNJetsCut(njetscut),
  fApplyPh1PtCut(applyph1ptcut), fPh1PtCut(ph1ptcut), fApplyPh1VIDCut(applyph1vidcut), fPh1VID(ph1vid), 
  fApplyPhAnyPtCut(applyphanyptcut), fPhAnyPtCut(phanyptcut), fApplyPhAnyVIDCut(applyphanyvidcut), fPhAnyVID(phanyvid), 
  fApplyrhECut(applyrhecut), frhECut(rhEcut),
  fApplyECALAcceptCut(applyecalacceptcut), fApplyEBOnly(applyEBonly), fApplyEEOnly(applyEEonly),
  fApplyPhMCMatchingCut(applyphmcmatchingcut), fApplyExactPhMCMatch(applyexactphmcmatch), fApplyAntiPhMCMatch(applyantiphmcmatch)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  if   (fIsGMSB || fIsHVDS || fIsBkg) fIsMC = true;
  else                                fIsMC = false;
  
  // initialize tree
  PlotPhotons::InitTree();

  // in routine initialization
  fNEvCheck = 1000;
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
  fEfficiency["Jets"]    = 0;
  fEfficiency["Trigger"] = 0;
  fEfficiency["Trg+Pho"] = 0;
  fEfficiency["Events"]  = 0;

  // output
  // setup outdir name
  if (!fApplyNJetsCut && !fApplyPh1PtCut && !fApplyPh1VIDCut && !fApplyPhAnyPtCut && !fApplyPhAnyVIDCut && !fApplyrhECut && !fApplyPhMCMatchingCut && !fApplyECALAcceptCut)
  { 
    fOutDir += "/Inclusive";
  }
  else 
  {
    fOutDir += "/cuts";
    if (fApplyJetPtCut)    fOutDir += Form("_jetpt%3.1f"  , fJetPtCut);
    if (fApplyNJetsCut)    fOutDir += Form("_njets%i"     , fNJetsCut);
    if (fApplyPh1PtCut)    fOutDir += Form("_ph1pt%3.1f"  , fPh1PtCut);
    if (fApplyPh1VIDCut)   fOutDir += Form("_ph1VID%s"    , fPh1VID.Data());
    if (fApplyPhAnyPtCut)  fOutDir += Form("_phanypt%3.1f", fPhAnyPtCut);
    if (fApplyPhAnyVIDCut) fOutDir += Form("_phanyVID%s"  , fPhAnyVID.Data());
    if (fApplyrhECut)      fOutDir += Form("_rhE%2.1f"    , frhECut);
    if (fApplyPhMCMatchingCut)
    {
      if      (fApplyExactPhMCMatch)  fOutDir += Form("_exactphmc");
      else if (fApplyAntiPhMCMatch)   fOutDir += Form("_antiphmc");
      else    { std::cout << "Need to specify a matching criterion!" << std::endl; exit(0); }
    }
    if (fApplyECALAcceptCut) 
    { 
      if      (fApplyEBOnly) fOutDir += Form("_EBOnly");
      else if (fApplyEEOnly) fOutDir += Form("_EEOnly");
      else                   fOutDir += Form("_ecalaccept");
    }
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
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    // count events passing given selection
    const Bool_t passed = PlotPhotons::CountEvents();
    if (fApplyEvCut && !passed) continue;

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
  const Int_t ph1 = PlotPhotons::GetLeadingPhoton();
  const Bool_t photon_b = (ph1 != -1);
  if (photon_b) fEfficiency["Photons"]++;

  // jet selection efficiency
  const Int_t nJets = PlotPhotons::GetNJetsAbovePt();
  const Bool_t jets_b = (fApplyNJetsCut ? (nJets >= fNJetsCut) : true);
  if (jets_b) fEfficiency["Jets"]++;

  //trigger efficiency
  const Bool_t hlt_b = ((fIsHLT2 || fIsHLT3) ? hltdispho45 : true);
  if (hlt_b) fEfficiency["Trigger"]++;

  //trigger + photon selection efficiency
  if (hlt_b && photon_b)
  {
    fEfficiency["Trg+Pho"]++;
  }

  // total efficiency
  if (photon_b && jets_b && hlt_b)
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
    // basic photon selection
    if (fApplyPh1PtCut && ((*phpt)[iph] < fPh1PtCut)) continue;
    if (fApplyPh1VIDCut && ((*phVID)[iph] < fPhVIDMap[fPh1VID])) continue;
    const Float_t eta = std::abs((*phsceta)[iph]);
    if (fApplyECALAcceptCut && ((eta < 1.4442) || ((eta > 1.566) && (eta < 2.5))))
    {
      if      (fApplyEBOnly && (eta > 1.4442)) continue;
      else if (fApplyEEOnly && ((eta < 1.566) || (eta > 2.5))) continue;
    }
    else continue;
    
    // photon seed crystal selection
    if ((*phseedpos)[iph] == -9999) continue;
    if (fApplyrhECut && ((*phrhE)[iph][(*phseedpos)[iph]] < frhECut)) continue;

    // MC matching selection
    if (fApplyPhMCMatchingCut)
    {
      if      (fApplyExactPhMCMatch)
      {
	if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] <= 0)) continue; // set to <= 0 for exact matching 
	else if (fIsBkg && ((*phisMatched)[iph] == 0)) continue; // set to == 0 for exact matching
      }
      else if (fApplyAntiPhMCMatch)
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

Int_t PlotPhotons::GetGoodPhotons(std::vector<Int_t> & goodphotons)
{
  const Int_t ph1 = PlotPhotons::GetLeadingPhoton();
  if (ph1 != -1)
  {
    for (Int_t iph = 0; iph < nphotons; iph++) // remove pt cut
    {
      // basic photon selection
      if (fApplyPhAnyPtCut && ((*phpt)[iph] < fPhAnyPtCut)) continue;
      if (fApplyPhAnyVIDCut && ((*phVID)[iph] < fPhVIDMap[fPhAnyVID])) continue;
      const Float_t eta = std::abs((*phsceta)[iph]);
      if (fApplyECALAcceptCut && ((eta < 1.4442) || ((eta > 1.566) && (eta < 2.5))))
      {
	if      (fApplyEBOnly && (eta > 1.4442)) continue;
	else if (fApplyEEOnly && ((eta < 1.566) || (eta > 2.5))) continue;
      }
      else continue;
    
      // photon seed crystal selection
      if ((*phseedpos)[iph] == -9999) continue;
      if (fApplyrhECut && ((*phrhE)[iph][(*phseedpos)[iph]] < frhECut)) continue;

      // MC matching selection
      if (fApplyPhMCMatchingCut)
      {
	if      (fApplyExactPhMCMatch)
        {
	  if      ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] <= 0)) continue; // set to <= 0 for exact matching 
	  else if (fIsBkg && ((*phisMatched)[iph] == 0)) continue; // set to == 0 for exact matching
	}
	else if (fApplyAntiPhMCMatch)
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

Int_t PlotPhotons::GetNJetsAbovePt()
{
  Int_t nJets = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++) 
  {
    if (fApplyJetPtCut && ((*jetpt)[ijet] < fJetPtCut)) break; // jets are ordered in pT!
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

    fPlots["genNgamma"]->Fill(gamma(genN1p,genN1mass));

    // ctau is distance / beta*gamma
    fPlots["genNctau"]->Fill(genN1traveld/genN1bg);

    // decay products
    fPlots["genphE"]->Fill(genph1E);
    fPlots["genphpt"]->Fill(genph1pt);
    fPlots["genphphi"]->Fill(genph1phi);
    fPlots["genpheta"]->Fill(genph1eta);
    fPlots["gengrE"]->Fill(gengr1E);
    fPlots["gengrpt"]->Fill(gengr1pt);
    fPlots["gengrphi"]->Fill(gengr1phi);
    fPlots["gengreta"]->Fill(gengr1eta);
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

    fPlots["genNgamma"]->Fill(gamma(genN2p,genN2mass));

    // ctau is distance / beta*gamma
    fPlots["genNctau"]->Fill(genN2traveld/genN2bg);

    // decay products
    fPlots["genphE"]->Fill(genph2E);
    fPlots["genphpt"]->Fill(genph2pt);
    fPlots["genphphi"]->Fill(genph2phi);
    fPlots["genpheta"]->Fill(genph2eta);
    fPlots["gengrE"]->Fill(gengr2E);
    fPlots["gengrpt"]->Fill(gengr2pt);
    fPlots["gengrphi"]->Fill(gengr2phi);
    fPlots["gengreta"]->Fill(gengr2eta);
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

    fPlots["genvPiongamma"]->Fill(gamma(genvPionp,(*genvPionmass)[ipion]));
      
    // ctau is distance / beta*gamma
    fPlots["genvPionctau"]->Fill(genvPiontraveld/genvPionbg);

    // now do gen photon info
    fPlots["genHVph1E"]->Fill((*genHVph1E)[ipion]);
    fPlots["genHVph1pt"]->Fill((*genHVph1pt)[ipion]);
    fPlots["genHVph1phi"]->Fill((*genHVph1phi)[ipion]);
    fPlots["genHVph1eta"]->Fill((*genHVph1eta)[ipion]);

    fPlots["genHVph2E"]->Fill((*genHVph2E)[ipion]);
    fPlots["genHVph2pt"]->Fill((*genHVph2pt)[ipion]);
    fPlots["genHVph2phi"]->Fill((*genHVph2phi)[ipion]);
    fPlots["genHVph2eta"]->Fill((*genHVph2eta)[ipion]);
  }
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
  fPlots["njets"]->Fill(njets); 

  const Int_t nJets = PlotPhotons::GetNJetsAbovePt();
  fPlots["nJets"]->Fill(nJets);  

  Int_t nMatchedJets = 0;
  for (Int_t ijet = 0; ijet < (fApplyNJetsCut ? nJets : njets); ijet++) 
  {
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
  fPlots["nphotons"]->Fill(nphotons);

  std::vector<Int_t> goodphotons;
  PlotPhotons::GetGoodPhotons(goodphotons);

  fPlots["nPhotons"]->Fill(goodphotons.size());

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
  
    // loop over rechits (+ seed info)
    Int_t nRecHits = 0;
    for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
    {
      if (fApplyrhECut && (*phrhE)[iph][irh] < frhECut) continue;
      nRecHits++;

      fPlots["phrhE"]->Fill((*phrhE)[iph][irh]);
      fPlots["phrhtime"]->Fill((*phrhtime)[iph][irh]);
      fPlots["phrhOOT"]->Fill((*phrhOOT)[iph][irh]);
      if ( (*phseedpos)[iph] == irh ) // seed info
      {
	fPlots["phseedE"]->Fill((*phrhE)[iph][irh]);
	fPlots["phseedtime"]->Fill((*phrhtime)[iph][irh]);
	fPlots["phseedOOT"]->Fill((*phrhOOT)[iph][irh]);
      }
    } // end loop over nrechits
    fPlots["phnrh"]->Fill(nRecHits);

    // MC matching selection --> redundant when fApplyMCP
    if ( ((fIsGMSB || fIsHVDS) && ((*phmatch)[iph] > 0)) || (fIsBkg && ((*phisMatched)[iph])) ) nMatchedPhotons++;
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
    fPlots["ph1smin"]->Fill((*phsmin)[ph1]);
    fPlots["ph1smaj"]->Fill((*phsmaj)[ph1]);
    fPlots["ph1seedtime"]->Fill((*phrhtime)[ph1][(*phseedpos)[ph1]]);
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
    fPlots["phdelaysmin"]->Fill((*phsmin)[phdelay]);
    fPlots["phdelaysmaj"]->Fill((*phsmaj)[phdelay]);
    fPlots["phdelayseedtime"]->Fill((*phrhtime)[phdelay][(*phseedpos)[phdelay]]);
  }
}

void PlotPhotons::FillTrigger()
{
  const Int_t phdelay = PlotPhotons::GetMostDelayedPhoton();

  if (phdelay != -1)
  {
    const Float_t phdelayseedtime = (*phrhtime)[phdelay][(*phseedpos)[phdelay]];
    const Float_t phdelayeta = (*pheta)[phdelay];
    const Float_t phdelayphi = (*phphi)[phdelay];
    const Float_t phdelaypt = (*phpt)[phdelay];

    // first is denom
    // second is numer

    // full displaced menu
    if (fIsHLT2)
    {
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltpho120_met40"],hltpho120_met40,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltpho175"],hltpho175,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdoublepho60"],hltdoublepho60,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45"],hltdispho45,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_jet50"],hltdispho45_jet50,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_dijet50"],hltdispho45_dijet50,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_dijet35"],hltdispho45_dijet35,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_trijet35"],hltdispho45_trijet35,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_el100veto"],hltdispho45_el100veto,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_notkveto"],hltdispho45_notkveto,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho50"],hltdispho50,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60"],hltdispho60,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_jet50"],hltdispho60_jet50,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_dijet50"],hltdispho60_dijet50,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_dijet35"],hltdispho60_dijet35,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_trijet35"],hltdispho60_trijet35,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho60_notkveto"],hltdispho60_notkveto,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho80"],hltdispho80,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho100"],hltdispho100,phdelayseedtime);
    } // end block over HLT Displaced Photon menu

    // HLT Dispho45 Breakdown
    if (fIsHLT3)
    {
      // time
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45"],hltdispho45,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_notkveto"],hltdispho45_notkveto,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_notime"],hltdispho45_notime,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_nosmaj"],hltdispho45_nosmaj,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_nosmin"],hltdispho45_nosmin,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_nosieie"],hltdispho45_nosieie,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_nor9"],hltdispho45_nor9,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_nohoe"],hltdispho45_nohoe,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_noet"],hltdispho45_noet,phdelayseedtime);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayseedtime_hltdispho45_nol1match"],hltdispho45_nol1match,phdelayseedtime);

      // eta
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45"],hltdispho45,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_notkveto"],hltdispho45_notkveto,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_notime"],hltdispho45_notime,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_nosmaj"],hltdispho45_nosmaj,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_nosmin"],hltdispho45_nosmin,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_nosieie"],hltdispho45_nosieie,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_nor9"],hltdispho45_nor9,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_nohoe"],hltdispho45_nohoe,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_noet"],hltdispho45_noet,phdelayeta);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayeta_hltdispho45_nol1match"],hltdispho45_nol1match,phdelayeta);

      // phi
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45"],hltdispho45,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_notkveto"],hltdispho45_notkveto,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_notime"],hltdispho45_notime,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_nosmaj"],hltdispho45_nosmaj,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_nosmin"],hltdispho45_nosmin,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_nosieie"],hltdispho45_nosieie,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_nor9"],hltdispho45_nor9,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_nohoe"],hltdispho45_nohoe,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_noet"],hltdispho45_noet,phdelayphi);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelayphi_hltdispho45_nol1match"],hltdispho45_nol1match,phdelayphi);

      // pt
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45"],hltdispho45,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_notkveto"],hltdispho45_notkveto,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_notime"],hltdispho45_notime,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_nosmaj"],hltdispho45_nosmaj,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_nosmin"],hltdispho45_nosmin,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_nosieie"],hltdispho45_nosieie,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_nor9"],hltdispho45_nor9,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_nohoe"],hltdispho45_nohoe,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_noet"],hltdispho45_noet,phdelaypt);
      PlotPhotons::FillTriggerPlot(fTrigPlots["phdelaypt_hltdispho45_nol1match"],hltdispho45_nol1match,phdelaypt);
    } // displaced breakdown menu
  } 
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

  fPlots["genphE"] = PlotPhotons::MakeTH1F("genph1E","Generator Photon E [GeV]",100,0.f,2500.f,"Energy [GeV]","Photons","GMSB");
  fPlots["genphpt"] = PlotPhotons::MakeTH1F("genph1pt","Generator p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","GMSB");
  fPlots["genphphi"] = PlotPhotons::MakeTH1F("genph1phi","Generator Photon #phi",100,-3.2,3.2,"#phi","Photons","GMSB");
  fPlots["genpheta"] = PlotPhotons::MakeTH1F("genph1eta","Generator Photon #eta",100,-6.0,6.0,"#eta","Photons","GMSB");

  fPlots["gengrE"] = PlotPhotons::MakeTH1F("gengr1E","Generator Gravitino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Gravitinos","GMSB");
  fPlots["gengrpt"] = PlotPhotons::MakeTH1F("gengr1pt","Generator p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Gravitinos","GMSB");
  fPlots["gengrphi"] = PlotPhotons::MakeTH1F("gengr1phi","Generator Gravitino #phi",100,-3.2,3.2,"#phi","Gravitinos","GMSB");
  fPlots["gengreta"] = PlotPhotons::MakeTH1F("gengr1eta","Generator Gravitino #eta",100,-6.0,6.0,"#eta","Gravitinos","GMSB");
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
}

void PlotPhotons::SetupVertices()
{
  fPlots["nvtx"] = PlotPhotons::MakeTH1F("nvtx","nVertices (reco)",100,0.f,100.f,"nPV","Events","Vertices");
}

void PlotPhotons::SetupMET()
{
  fPlots["t1pfMETpt"] = PlotPhotons::MakeTH1F("t1pfMETpt","Type1 PF MET [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
  fPlots["t1pfMETphi"] = PlotPhotons::MakeTH1F("t1pfMETphi","Type1 PF MET #phi",100,-3.2,3.2,"#phi","Events","MET");
  fPlots["t1pfMETsumEt"] = PlotPhotons::MakeTH1F("t1pfMETsumEt","Type1 PF MET #Sigma E_{T} [GeV]",100,0.f,8000.f,"#Sigma E_{T} [GeV]","Events","MET");
  fPlots["t1pfMETuncorpt"] = PlotPhotons::MakeTH1F("t1pfMETuncorpt","Type1 PF MET (Raw) [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
  fPlots["t1pfMETuncorphi"] = PlotPhotons::MakeTH1F("t1pfMETuncorphi","Type1 PF MET (Raw) #phi",100,-3.2,3.2,"#phi","Events","MET");
  fPlots["t1pfMETuncorsumEt"] = PlotPhotons::MakeTH1F("t1pfMETuncorsumEt","Type1 PF MET (Raw) #Sigma E_{T} [GeV]",100,0.f,8000.f,"#Sigma E_{T} [GeV]","Events","MET");
  fPlots["t1pfMETcalopt"] = PlotPhotons::MakeTH1F("t1pfMETcalopt","Type1 PF MET (Calo) [GeV]",100,0.f,8000.f,"MET [GeV]","Events","MET");
  fPlots["t1pfMETcalophi"] = PlotPhotons::MakeTH1F("t1pfMETcalophi","Type1 PF MET (Calo) #phi",100,-3.2,3.2,"#phi","Events","MET");
  fPlots["t1pfMETcalosumEt"] = PlotPhotons::MakeTH1F("t1pfMETcalosumEt","Type1 PF MET (Calo) #Sigma E_{T} [GeV]",100,0.f,8000.f,"#Sigma E_{T} [GeV]","Events","MET");
  if (fIsGMSB || fIsHVDS)
  {
    fPlots["t1pfMETgenMETpt"] = PlotPhotons::MakeTH1F("t1pfMETgenMETpt","Type1 PF Gen MET [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
    fPlots["t1pfMETgenMETphi"] = PlotPhotons::MakeTH1F("t1pfMETgenMETphi","Type1 PF Gen MET #phi",100,-3.2,3.2,"#phi","Events","MET");
    fPlots["t1pfMETgenMETsumEt"] = PlotPhotons::MakeTH1F("t1pfMETgenMETsumEt","Type1 PF Gen MET #Sigma E_{T} [GeV]",100,0.f,8000.f,"#Sigma E_{T} [GeV]","Events","MET");
  }
}

void PlotPhotons::SetupJets()
{
  fPlots["njets"] = PlotPhotons::MakeTH1F("njets","nAK4Jets",40,0.f,40.f,"nAK4Jets","Events","AK4Jets");
  fPlots["nJets"] = PlotPhotons::MakeTH1F("nJets",Form("nAK4Jets, p_{T} > %4.1f GeV/c",fJetPtCut),40,0.f,40.f,Form("nAK4Jets [p_{T} > %4.1f GeV/c]",fJetPtCut),"Events","AK4Jets");
  if (fIsGMSB) fPlots["nMatchedJets"] = PlotPhotons::MakeTH1F("nMatchedJets","nMatchedJets (reco to gen)",40,0.f,40.f,"nMatchedJets","Events","AK4Jets");

  fPlots["jetE"] = PlotPhotons::MakeTH1F("jetE","Jets Energy [GeV] (reco)",100,0.f,3000.f,"Energy [GeV]","Jets","AK4Jets");
  fPlots["jetpt"] = PlotPhotons::MakeTH1F("jetpt","Jets p_{T} [GeV/c] (reco)",100,0.f,3000.f,"Jet p_{T} [GeV/c]","Jets","AK4Jets");
  fPlots["jetphi"] = PlotPhotons::MakeTH1F("jetphi","Jets #phi (reco)",100,-3.2,3.2,"#phi","Jets","AK4Jets");
  fPlots["jeteta"] = PlotPhotons::MakeTH1F("jeteta","Jets #eta (reco)",100,-6.0,6.0,"#eta","Jets","AK4Jets");
}

void PlotPhotons::SetupRecoPhotons()
{
  fPlots["nphotons"] = PlotPhotons::MakeTH1F("nphotons","nPhotons (reco)",20,0.f,20.f,"nPhotons","Events","RecoPhotons");
  fPlots["nPhotons"] = PlotPhotons::MakeTH1F("nPhotons","nGoodPhotons (reco)",20,0.f,20.f,"nGoodPhotons","Events","RecoPhotons");
  if (fIsMC) fPlots["nMatchedPhotons"] = PlotPhotons::MakeTH1F("nMatchedPhotons","nMatchedPhotons (reco to gen)",20,0.f,20.f,"nMatchedPhotons","Events","RecoPhotons");

  // All reco photons
  fPlots["phE"] = PlotPhotons::MakeTH1F("phE","Photons Energy [GeV] (reco)",100,0.f,2500.f,"Energy [GeV]","Photons","RecoPhotons");
  fPlots["phpt"] = PlotPhotons::MakeTH1F("phpt","Photons p_{T} [GeV/c] (reco)",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","RecoPhotons");
  fPlots["phphi"] = PlotPhotons::MakeTH1F("phphi","Photons #phi (reco)",100,-3.2,3.2,"#phi","Photons","RecoPhotons");
  fPlots["pheta"] = PlotPhotons::MakeTH1F("pheta","Photons #eta (reco)",100,-5.0,5.0,"#eta","Photons","RecoPhotons");
  fPlots["phHoE"] = PlotPhotons::MakeTH1F("phHoE","Photons HE/EE (reco)",100,0.f,5.0,"HE/EE","Photons","RecoPhotons");
  fPlots["phr9"] = PlotPhotons::MakeTH1F("phr9","Photons R9 (reco)",100,0.f,25.0,"R9","Photons","RecoPhotons");
  fPlots["phChgIso"] = PlotPhotons::MakeTH1F("phChgIso","Photons #rho-corrected Charged Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Charged Hadron Isolation","Photons","RecoPhotons");
  fPlots["phNeuIso"] = PlotPhotons::MakeTH1F("phNeuIso","Photons #rho-corrected Neutral Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Neutral Hadron Isolation","Photons","RecoPhotons");
  fPlots["phIso"] = PlotPhotons::MakeTH1F("phIso","Photons #rho-corrected Photon Isolation (reco)",100,0.f,1000.f,"#rho-corrected Photon Isolation","Photons","RecoPhotons");
  fPlots["phsuisseX"] = PlotPhotons::MakeTH1F("phsuisseX","Photons Swiss Cross (reco)",100,0.f,1.f,"Swiss Cross","Photons","RecoPhotons");
  fPlots["phsieie"] = PlotPhotons::MakeTH1F("phsieie","Photons #sigma_{i#eta i#eta} (reco)",100,0,0.1,"#sigma_{i#eta i#eta}","Photons","RecoPhotons");
  fPlots["phsipip"] = PlotPhotons::MakeTH1F("phsipip","Photons #sigma_{i#phi i#phi} (reco)",100,0,0.1,"#sigma_{i#phi i#phi}","Photons","RecoPhotons");
  fPlots["phsieip"] = PlotPhotons::MakeTH1F("phsieip","Photons #sigma_{i#eta i#phi} (reco)",100,-0.005,0.005,"#sigma_{i#eta i#phi}","Photons","RecoPhotons");
  fPlots["phsmaj"] = PlotPhotons::MakeTH1F("phsmaj","Photons S_{major} (reco)",100,0.f,5.f,"S_{major}","Photons","RecoPhotons");
  fPlots["phsmin"] = PlotPhotons::MakeTH1F("phsmin","Photons S_{minor} (reco)",100,0.f,1.f,"S_{minor}","Photons","RecoPhotons");
  fPlots["phsmin_ov_phsmaj"] = PlotPhotons::MakeTH1F("phsmin_ov_phsmaj","Photons S_{minor} / S_{major} (reco)",100,0,1.f,"S_{minor}/S_{major}","Photons","RecoPhotons");
  fPlots["phalpha"] = PlotPhotons::MakeTH1F("phalpha","Photons Cluster Shape #alpha (reco)",100,-1.7,1.7,"Cluster Shape #alpha","Photons","RecoPhotons");
  fPlots["phscE"] = PlotPhotons::MakeTH1F("phscE","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2500.f,"Energy [GeV]","Photons","RecoPhotons");
  fPlots["phnrh"] = PlotPhotons::MakeTH1F("phnrh","nRecHits from Photons (reco)",100,0.f,100.f,"nRecHits","Photons","RecoPhotons");

  // all rec hits + seed info
  fPlots["phrhE"] = PlotPhotons::MakeTH1F("phrhE","Photons RecHits Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","RecHits","RecoPhotons");
  fPlots["phrhtime"] = PlotPhotons::MakeTH1F("phrhtime","Photons RecHits Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","RecHits","RecoPhotons");
  fPlots["phrhOOT"] = PlotPhotons::MakeTH1F("phrhOOT","Photons RecHits OoT Flag (reco)",2,0.f,2.f,"OoT Flag","RecHits","RecoPhotons");
  fPlots["phseedE"] = PlotPhotons::MakeTH1F("phseedE","Photons Seed RecHit Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Seed RecHits","RecoPhotons");
  fPlots["phseedtime"] = PlotPhotons::MakeTH1F("phseedtime","Photons Seed RecHit Time [ns] (reco)",200,-10.f,10.f,"Time [ns]","Seed RecHits","RecoPhotons");
  fPlots["phseedOOT"] = PlotPhotons::MakeTH1F("phseedOOT","Photons Seed RecHit OoT Flag (reco)",2,0.f,2.f,"OoT Flag","Seed RecHits","RecoPhotons");  
}

void PlotPhotons::SetupLeading()
{
  fPlots["ph1pt"] = PlotPhotons::MakeTH1F("ph1pt","Leading Photon p_{T} [GeV/c] (reco)",100,0.f,2000.f,"p_{T} [GeV/c]","Events","Leading");
  fPlots["ph1HoE"] = PlotPhotons::MakeTH1F("ph1HoE","Leading Photon HE/EE (reco)",100,0.f,5.0,"HE/EE","Events","Leading");
  fPlots["ph1r9"] = PlotPhotons::MakeTH1F("ph1r9","Leading Photon R9 (reco)",100,0.f,5.0,"R9","Events","Leading");
  fPlots["ph1sieie"] = PlotPhotons::MakeTH1F("ph1sieie","Leading Photon #sigma_{i#eta i#eta} (reco)",100,0,0.1,"#sigma_{i#eta i#eta}","Events","Leading");
  fPlots["ph1smaj"] = PlotPhotons::MakeTH1F("ph1smaj","Leading Photon S_{major} (reco)",100,0.f,5.f,"S_{major}","Events","Leading");
  fPlots["ph1smin"] = PlotPhotons::MakeTH1F("ph1smin","Leading Photon S_{minor} (reco)",100,0.f,1.f,"S_{minor}","Events","Leading");
  fPlots["ph1seedtime"] = PlotPhotons::MakeTH1F("ph1seedtime","Leading Photon Seed RecHit Time [ns]",100,-5.f,20.f,"Seed RecHit Time [ns]","Events","Leading");
}

void PlotPhotons::SetupMostDelayed()
{
  fPlots["phdelaypt"] = PlotPhotons::MakeTH1F("phdelaypt","Most Delayed Photon p_{T} [GeV/c] (reco)",100,0.f,2000.f,"p_{T} [GeV/c]","Events","MostDelayed");
  fPlots["phdelayHoE"] = PlotPhotons::MakeTH1F("phdelayHoE","Most Delayed Photon HE/EE (reco)",100,0.f,5.0,"HE/EE","Events","MostDelayed");
  fPlots["phdelayr9"] = PlotPhotons::MakeTH1F("phdelayr9","Most Delayed Photon R9 (reco)",100,0.f,5.0,"R9","Events","MostDelayed");
  fPlots["phdelaysieie"] = PlotPhotons::MakeTH1F("phdelaysieie","Most Delayed Photon #sigma_{i#eta i#eta} (reco)",100,0,0.1,"#sigma_{i#eta i#eta}","Events","MostDelayed");
  fPlots["phdelaysmaj"] = PlotPhotons::MakeTH1F("phdelaysmaj","Most Delayed Photon S_{major} (reco)",100,0.f,5.f,"S_{major}","Events","MostDelayed");
  fPlots["phdelaysmin"] = PlotPhotons::MakeTH1F("phdelaysmin","Most Delayed Photon S_{minor} (reco)",100,0.f,1.f,"S_{minor}","Events","MostDelayed");
  fPlots["phdelayseedtime"] = PlotPhotons::MakeTH1F("phdelayseedtime","Most Delayed Photon Seed RecHit Time [ns]",100,-5.f,20.f,"Seed RecHit Time [ns]","Events","MostDelayed");
}

void PlotPhotons::SetupTrigger()
{
  if (fIsHLT2)
  {
    fTrigPlots["phdelayseedtime_hltpho120_met40"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltpho120_met40","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_Photon120_R9Id90_HE10_Iso40_EBOnly_PFMET40_v","trigger");
    fTrigPlots["phdelayseedtime_hltpho175"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltpho175","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_Photon175_v","trigger");
    fTrigPlots["phdelayseedtime_hltdoublepho60"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdoublepho60","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DoublePhoton60_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_jet50"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_jet50","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_PFJet50_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_dijet50"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_dijet50","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_DiPFJet50_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_dijet35"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_dijet35","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_DiPFJet35_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_trijet35"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_trijet35","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_TriPFJet35_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_el100veto"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_el100veto","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_El100Veto_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_notkveto"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_notkveto","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noTrackVeto_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho50"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho50","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton50_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton60_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_jet50"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_jet50","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton60_PFJet50_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_dijet50"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_dijet50","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton60_DiPFJet50_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_dijet35"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_dijet35","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton60_DiPFJet35_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_trijet35"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_trijet35","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton60_TriPFJet35_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho60_notkveto"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho60_notkveto","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton60_noTrackVeto_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho80"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho80","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton80_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho100"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho100","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton100_v","trigger");
  }
  if (fIsHLT3)
  {
    // seed time
    fTrigPlots["phdelayseedtime_hltdispho45"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_notkveto"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_notkveto","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noTrackVeto_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_notime"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_notime","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noTime_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_nosmaj"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_nosmaj","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noSmaj_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_nosmin"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_nosmin","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noSmin_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_nosieie"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_nosieie","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noSieie_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_nor9"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_nor9","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noR9_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_nohoe"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_nohoe","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noHoE_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_noet"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_noet","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noEt_v","trigger");
    fTrigPlots["phdelayseedtime_hltdispho45_nol1match"] = PlotPhotons::MakeTrigTH1Fs("phdelayseedtime_hltdispho45_nol1match","Most Delayed Photon Seed recHit Time",100,-5.f,20.f,"Most Delayed Photon Seed recHit Time","Events","HLT_DisplacedPhoton45_noL1Match_v","trigger");

    // eta
    fTrigPlots["phdelayeta_hltdispho45"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_notkveto"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_notkveto","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noTrackVeto_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_notime"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_notime","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noTime_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_nosmaj"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_nosmaj","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noSmaj_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_nosmin"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_nosmin","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noSmin_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_nosieie"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_nosieie","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noSieie_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_nor9"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_nor9","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noR9_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_nohoe"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_nohoe","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noHoE_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_noet"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_noet","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noEt_v","trigger");
    fTrigPlots["phdelayeta_hltdispho45_nol1match"] = PlotPhotons::MakeTrigTH1Fs("phdelayeta_hltdispho45_nol1match","Most Delayed Photon #eta",100,-5.f,5.f,"Most Delayed Photon #eta","Events","HLT_DisplacedPhoton45_noL1Match_v","trigger");

    // phi
    fTrigPlots["phdelayphi_hltdispho45"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_notkveto"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_notkveto","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noTrackVeto_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_notime"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_notime","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noTime_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_nosmaj"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_nosmaj","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noSmaj_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_nosmin"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_nosmin","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noSmin_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_nosieie"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_nosieie","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noSieie_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_nor9"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_nor9","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noR9_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_nohoe"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_nohoe","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noHoE_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_noet"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_noet","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noEt_v","trigger");
    fTrigPlots["phdelayphi_hltdispho45_nol1match"] = PlotPhotons::MakeTrigTH1Fs("phdelayphi_hltdispho45_nol1match","Most Delayed Photon #phi",100,-3.2f,3.2f,"Most Delayed Photon #phi","Events","HLT_DisplacedPhoton45_noL1Match_v","trigger");

    // pt
    fTrigPlots["phdelaypt_hltdispho45"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_notkveto"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_notkveto","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noTrackVeto_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_notime"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_notime","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noTime_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_nosmaj"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_nosmaj","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noSmaj_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_nosmin"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_nosmin","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noSmin_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_nosieie"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_nosieie","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noSieie_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_nor9"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_nor9","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noR9_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_nohoe"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_nohoe","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noHoE_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_noet"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_noet","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noEt_v","trigger");
    fTrigPlots["phdelaypt_hltdispho45_nol1match"] = PlotPhotons::MakeTrigTH1Fs("phdelaypt_hltdispho45_nol1match","Most Delayed Photon p_{T}",100,0.f,2000.f,"Most Delayed Photon p_{T}","Events","HLT_DisplacedPhoton45_noL1Match_v","trigger");
  }
}

void PlotPhotons::SetupAnalysis()
{
  fPlots2D["MET_vs_ph1seedtime"] = PlotPhotons::MakeTH2F("MET_vs_ph1seedtime","MET vs Leading Photon Seed RecHit Time",80,-2.f,14.f,"Leading Photon Seed RecHit Time [ns]",100,0.f,2000.f,"MET","Analysis");
  fPlots2D["MET_vs_phdelayseedtime"] = PlotPhotons::MakeTH2F("MET_vs_phdelayseedtime","MET vs Most Delayed Photon Seed RecHit Time",80,-2.f,14.f,"Most Delayed Photon Seed RecHit Time [ns]",100,0.f,2000.f,"MET","Analysis");
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
      //canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
      canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->first.Data()));
      
      canv->SetLogy(1);
      //canv->SaveAs(Form("%s/%s/log/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
      canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),mapiter->first.Data()));
      
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
      canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->second.first->GetName()));
      
      canv->SetLogy(1);
      canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),mapiter->second.first->GetName()));

      // second numer
      mapiter->second.second->Draw("HIST");
      
      // first save as linear, then log
      canv->SetLogy(0);
      canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->second.second->GetName()));
      
      canv->SetLogy(1);
      canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),mapiter->second.second->GetName()));

      // efficiency last
      eff->Draw("EP");
      
      // first save as linear, then log
      canv->SetLogy(0);
      canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),eff->GetName()));
      
      canv->SetLogy(1);
      canv->SaveAs(Form("%s/%s_log.png",fOutDump.Data(),eff->GetName()));
      
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
      //canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
      canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->first.Data()));
    
      delete canv;
    }

    delete mapiter->second;
  }
  fPlots2D.clear();
}

void PlotPhotons::InitTree()
{
  // Set object pointer
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
  phisMatched = 0;
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

  if (fIsHLT2)
  {
    fInTree->SetBranchAddress("hltpho120_met40", &hltpho120_met40, &b_hltpho120_met40);
    fInTree->SetBranchAddress("hltpho175", &hltpho175, &b_hltpho175);
    fInTree->SetBranchAddress("hltdoublepho60", &hltdoublepho60, &b_hltdoublepho60);
    fInTree->SetBranchAddress("hltdispho45", &hltdispho45, &b_hltdispho45);
    fInTree->SetBranchAddress("hltdispho45_jet50", &hltdispho45_jet50, &b_hltdispho45_jet50);
    fInTree->SetBranchAddress("hltdispho45_dijet50", &hltdispho45_dijet50, &b_hltdispho45_dijet50);
    fInTree->SetBranchAddress("hltdispho45_dijet35", &hltdispho45_dijet35, &b_hltdispho45_dijet35);
    fInTree->SetBranchAddress("hltdispho45_trijet35", &hltdispho45_trijet35, &b_hltdispho45_trijet35);
    fInTree->SetBranchAddress("hltdispho45_el100veto", &hltdispho45_el100veto, &b_hltdispho45_el100veto);
    fInTree->SetBranchAddress("hltdispho45_notkveto", &hltdispho45_notkveto, &b_hltdispho45_notkveto);
    fInTree->SetBranchAddress("hltdispho50", &hltdispho50, &b_hltdispho50);
    fInTree->SetBranchAddress("hltdispho60", &hltdispho60, &b_hltdispho60);
    fInTree->SetBranchAddress("hltdispho60_jet50", &hltdispho60_jet50, &b_hltdispho60_jet50);
    fInTree->SetBranchAddress("hltdispho60_dijet50", &hltdispho60_dijet50, &b_hltdispho60_dijet50);
    fInTree->SetBranchAddress("hltdispho60_dijet35", &hltdispho60_dijet35, &b_hltdispho60_dijet35);
    fInTree->SetBranchAddress("hltdispho60_trijet35", &hltdispho60_trijet35, &b_hltdispho60_trijet35);
    fInTree->SetBranchAddress("hltdispho60_notkveto", &hltdispho60_notkveto, &b_hltdispho60_notkveto);
    fInTree->SetBranchAddress("hltdispho80", &hltdispho80, &b_hltdispho80);
    fInTree->SetBranchAddress("hltdispho100", &hltdispho100, &b_hltdispho100);
  }
  else if (fIsHLT3)
  {
    fInTree->SetBranchAddress("hltdispho45", &hltdispho45, &b_hltdispho45);
    fInTree->SetBranchAddress("hltdispho45_notkveto", &hltdispho45_notkveto, &b_hltdispho45_notkveto);
    fInTree->SetBranchAddress("hltdispho45_notime", &hltdispho45_notime, &b_hltdispho45_notime);
    fInTree->SetBranchAddress("hltdispho45_nosmaj", &hltdispho45_nosmaj, &b_hltdispho45_nosmaj);
    fInTree->SetBranchAddress("hltdispho45_nosmin", &hltdispho45_nosmin, &b_hltdispho45_nosmin);
    fInTree->SetBranchAddress("hltdispho45_nosieie", &hltdispho45_nosieie, &b_hltdispho45_nosieie);
    fInTree->SetBranchAddress("hltdispho45_nor9", &hltdispho45_nor9, &b_hltdispho45_nor9);
    fInTree->SetBranchAddress("hltdispho45_nohoe", &hltdispho45_nohoe, &b_hltdispho45_nohoe);
    fInTree->SetBranchAddress("hltdispho45_noet", &hltdispho45_noet, &b_hltdispho45_noet);
    fInTree->SetBranchAddress("hltdispho45_nol1match", &hltdispho45_nol1match, &b_hltdispho45_nol1match);
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
  if (fIsMC) fInTree->SetBranchAddress("phisMatched", &phisMatched, &b_phisMatched);
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
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrheta", &phrheta, &b_phrheta);
  fInTree->SetBranchAddress("phrhphi", &phrhphi, &b_phrhphi);
  fInTree->SetBranchAddress("phrhE", &phrhE, &b_phrhE);
  fInTree->SetBranchAddress("phrhtime", &phrhtime, &b_phrhtime);
  fInTree->SetBranchAddress("phrhID", &phrhID, &b_phrhID);
  fInTree->SetBranchAddress("phrhOOT", &phrhOOT, &b_phrhOOT);
}
