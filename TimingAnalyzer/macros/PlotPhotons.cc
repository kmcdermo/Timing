#include "PlotPhotons.hh"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotPhotons::PlotPhotons(TString filename, Bool_t isGMSB, Bool_t isBkg, Bool_t applyevcut, TString outdir,
			 Bool_t applyjetptcut, Float_t jetptcut, Bool_t applyphptcut, Float_t phptcut,
			 Bool_t applyphvidcut, TString phvid, Bool_t applyrhecut, Float_t rhEcut,
			 Bool_t applyecalacceptcut, Bool_t applyEBonly, Bool_t applyEEonly) :
  fOutDir(outdir), fIsGMSB(isGMSB), fIsBkg(isBkg), fApplyEvCut(applyevcut),
  fApplyJetPtCut(applyjetptcut), fJetPtCut(jetptcut), fApplyPhPtCut(applyphptcut), fPhPtCut(phptcut),
  fApplyPhVIDCut(applyphvidcut), fPhVID(phvid), fApplyrhECut(applyrhecut), frhECut(rhEcut),
  fApplyECALAcceptCut(applyecalacceptcut), fApplyEBOnly(applyEBonly), fApplyEEOnly(applyEEonly)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  if   (fIsGMSB || fIsBkg) fIsMC = true;
  else                     fIsMC = false;
  
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
  if (filename.Contains("ctau100" ,TString::kExact)) fCTau = 3.65;  // 100  mm --> 36.5  mm == 3.65  cm
  if (filename.Contains("ctau2000",TString::kExact)) fCTau = 73.5;  // 2000 mm --> 730.5 mm == 73.5  cm
  if (filename.Contains("ctau6000",TString::kExact)) fCTau = 219.2; // 6000 mm --> 2192  mm == 219.2 cm  

  // initialize efficiency map
  fEfficiency["Photons"] = 0;
  fEfficiency["RecHits"] = 0;
  fEfficiency["Jets"]    = 0;
  fEfficiency["Events"]  = 0;

  // output
  // setup outdir name
  if (!fApplyJetPtCut && !applyphptcut && !applyphvidcut && !applyrhecut)
  { 
    fOutDir += "/Inclusive";
  }
  else 
  {
    fOutDir += "/cuts";
    if (fApplyJetPtCut)      fOutDir += Form("_jetpt%3.1f",fJetPtCut);
    if (fApplyPhPtCut)       fOutDir += Form("_phpt%3.1f" ,fPhPtCut);
    if (fApplyPhVIDCut)      fOutDir += Form("_phVID%s"   ,fPhVID.Data());
    if (fApplyrhECut)        fOutDir += Form("_rhE%2.1f"  ,frhECut);
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

void PlotPhotons::DoPlots(Bool_t generic, Bool_t eff, Bool_t analysis)
{
  PlotPhotons::SetupPlots(generic, eff, analysis);
  PlotPhotons::EventLoop(generic, eff, analysis);
  PlotPhotons::MakeSubDirs();
  PlotPhotons::OutputTEffs();
  PlotPhotons::OutputTH1Fs();
  PlotPhotons::OutputTH2Fs();
  PlotPhotons::DumpEventCounts();
}

void PlotPhotons::SetupPlots(Bool_t generic, Bool_t eff, Bool_t analysis)
{
  if (generic) 
  {
    if (fIsMC)
    {
      //      PlotPhotons::SetupGenInfo();
      //      if (fIsGMSB)
      //      {
      //      PlotPhotons::SetupGenParticles();
      //      PlotPhotons::SetupGenJets();
      //      }
    }
    PlotPhotons::SetupObjectCounts();
    //    PlotPhotons::SetupMET();
    PlotPhotons::SetupJets();
    PlotPhotons::SetupRecoPhotons();
  }

  if (eff)
  {
    if (fIsMC)
    {
      PlotPhotons::SetupEffs();
    }
  }

  if (analysis)
  {
    PlotPhotons::SetupAnalysis();
  }
}

void PlotPhotons::EventLoop(Bool_t generic, Bool_t eff, Bool_t analysis)
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    
    // standard printout
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    // N-1 efficiency couting
    Bool_t event_b = false;
    PlotPhotons::CountEvents(event_b);
    if (fApplyEvCut && !event_b) continue;

    // plots!
    if (generic)
    {
      if (fIsMC)
      {
	//	PlotPhotons::FillGenInfo();
	//	PlotPhotons::FillGMSB();
	//	PlotPhotons::FillGenJets();
      }
      PlotPhotons::FillObjectCounts();
      //      PlotPhotons::FillMET();
      PlotPhotons::FillJets();
      PlotPhotons::FillRecoPhotons();
    }

    if (eff)
    {
      if (fIsMC)
      {
	PlotPhotons::FillEffs();
      }
    }

    if (analysis)
    {
      PlotPhotons::FillAnalysis();
    }
  }
}

void PlotPhotons::CountEvents(Bool_t & event_b)
{
  // photon selection efficiency
  Bool_t phpt_b  = false;
  Bool_t phvid_b = false;
  Bool_t phsceta_b = false;
  Bool_t seedrh_b = false;
  Bool_t seedE_b = false;

  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    if (fApplyPhPtCut && (*phpt)[iph] < fPhPtCut) continue;
    phpt_b = true;
    if (fApplyPhVIDCut && ((*phVID)[iph] < fPhVIDMap[fPhVID])) continue;
    phvid_b = true;
    if (fApplyECALAcceptCut && (std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    phsceta_b = true;
    if ((*phseedpos)[iph] == -9999) continue;
    seedrh_b = true;
    if (fApplyrhECut && (*phrhE)[iph][(*phseedpos)[iph]] < frhECut) continue;
    seedE_b = true;  

    // leading photon passing all the selection as required
    break;
  }
  if (phpt_b && phvid_b && phsceta_b) fEfficiency["Photons"]++;
  if (seedrh_b && seedE_b) fEfficiency["RecHits"]++;

  // jet selection efficiency
  Int_t njets_ptcut = 0;
  Bool_t njets_ptcut_b = false;
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (fApplyJetPtCut && (*jetpt)[ijet] < fJetPtCut) continue;
    njets_ptcut++;
    if (njets_ptcut == 3) 
    {
      njets_ptcut_b = true; 
      break;
    }
  }
  if (njets_ptcut_b) fEfficiency["Jets"]++;

  // total efficiency
  if (njets_ptcut_b && phpt_b && phvid_b && phsceta_b && seedrh_b && seedE_b) 
  {
    fEfficiency["Events"]++;
    event_b = true;
  }
}

void PlotPhotons::FillAnalysis()
{
  Bool_t photon_b = false;
  Bool_t rh_b     = false;
  Bool_t jets_b   = false;

  // check for one good photon with an associated seed
  Int_t ph1 = -1;
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    if (fApplyPhPtCut && (*phpt)[iph] < fPhPtCut) continue;
    if (fApplyPhVIDCut && ((*phVID)[iph]) < fPhVIDMap[fPhVID]) continue;
    if (fApplyECALAcceptCut && (std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    photon_b = true;
    if ((*phseedpos)[iph] == -9999) continue;
    if (fApplyrhECut && (*phrhE)[iph][(*phseedpos)[iph]] < frhECut) continue;
    rh_b = true;    
    
    ph1 = iph; // save the good photon
    break;
  }

  // jet selection efficiency
  Int_t njets_ptcut = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (fApplyJetPtCut && (*jetpt)[ijet] < fJetPtCut) continue;
    njets_ptcut++;
    if (njets_ptcut == 3) 
    {
      jets_b = true;
      break;
    }
  }
  
  if (photon_b && jets_b && rh_b) 
  {
    Float_t seed1time = (*phrhtime)[ph1][(*phseedpos)[ph1]];
    fPlots2D["MET_vs_seed1time"]->Fill(seed1time,t1pfMETpt);
  }
}

void PlotPhotons::FillEffs()
{
  if (nNeutoPhGr >= 1)
  {
    const Bool_t ph1match = (genph1match>0);
    fEffs["effMatchVsGenPhPt"]->Fill(ph1match,genph1pt);
    fEffs["effMatchVsGenPhPhi"]->Fill(ph1match,genph1phi);
    fEffs["effMatchVsGenPhEta"]->Fill(ph1match,genph1eta);
    fEffs["effMatchVsGenPhVtx"]->Fill(ph1match,std::sqrt(rad2(genN1decayvx,genN1decayvy,genN1decayvz)));
    if (nNeutoPhGr >= 2)
    {
      const Bool_t ph2match = (genph2match>0);
      fEffs["effMatchVsGenPhPt"]->Fill(ph2match,genph2pt);
      fEffs["effMatchVsGenPhPhi"]->Fill(ph2match,genph2phi);
      fEffs["effMatchVsGenPhEta"]->Fill(ph2match,genph2eta);
      fEffs["effMatchVsGenPhVtx"]->Fill(ph2match,std::sqrt(rad2(genN2decayvx,genN2decayvy,genN2decayvz)));
    }
  }
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

  fPlots["genN1mass"]->Fill(genN1mass);
  fPlots["genN1E"]->Fill(genN1E);
  fPlots["genN1pt"]->Fill(genN1pt);
  fPlots["genN1phi"]->Fill(genN1phi);
  fPlots["genN1eta"]->Fill(genN1eta);
  fPlots["genph1E"]->Fill(genph1E);
  fPlots["genph1pt"]->Fill(genph1pt);
  fPlots["genph1phi"]->Fill(genph1phi);
  fPlots["genph1eta"]->Fill(genph1eta);
  fPlots["gengr1E"]->Fill(gengr1E);
  fPlots["gengr1pt"]->Fill(gengr1pt);
  fPlots["gengr1phi"]->Fill(gengr1phi);
  fPlots["gengr1eta"]->Fill(gengr1eta);

  fPlots["genN2mass"]->Fill(genN2mass);
  fPlots["genN2E"]->Fill(genN2E);
  fPlots["genN2pt"]->Fill(genN2pt);
  fPlots["genN2phi"]->Fill(genN2phi);
  fPlots["genN2eta"]->Fill(genN2eta);
  fPlots["genph2E"]->Fill(genph2E);
  fPlots["genph2pt"]->Fill(genph2pt);
  fPlots["genph2phi"]->Fill(genph2phi);
  fPlots["genph2eta"]->Fill(genph2eta);
  fPlots["gengr2E"]->Fill(gengr2E);
  fPlots["gengr2pt"]->Fill(gengr2pt);
  fPlots["gengr2phi"]->Fill(gengr2phi);
  fPlots["gengr2eta"]->Fill(gengr2eta);

  // fill proper distance plots
  if (nNeutoPhGr >= 1)
  {
    // distances in lab frame
    fPlots["genN1prodz"]->Fill(std::abs(genN1prodvz));
    fPlots["genN1prodr"]->Fill(std::sqrt(rad2(genN1prodvx,genN1prodvy)));
    fPlots["genN1prodd"]->Fill(std::sqrt(rad2(genN1prodvx,genN1prodvy,genN1prodvz)));
    fPlots2D["genN1prodr_vs_z"]->Fill(std::abs(genN1prodvz),std::sqrt(rad2(genN1prodvx,genN1prodvy)));

    fPlots["genN1decayz"]->Fill(std::abs(genN1decayvz));
    fPlots["genN1decayr"]->Fill(std::sqrt(rad2(genN1decayvx,genN1decayvy)));
    fPlots["genN1decayd"]->Fill(std::sqrt(rad2(genN1decayvx,genN1decayvy,genN1decayvz)));
    fPlots2D["genN1decayr_vs_z"]->Fill(std::abs(genN1decayvz),std::sqrt(rad2(genN1decayvx,genN1decayvy)));

    fPlots["genN1decayz_zoom"]->Fill(std::abs(genN1decayvz));
    fPlots["genN1decayr_zoom"]->Fill(std::sqrt(rad2(genN1decayvx,genN1decayvy)));
    fPlots["genN1decayd_zoom"]->Fill(std::sqrt(rad2(genN1decayvx,genN1decayvy,genN1decayvz)));
    fPlots2D["genN1decayr_vs_z_zoom"]->Fill(std::abs(genN1decayvz),std::sqrt(rad2(genN1decayvx,genN1decayvy)));

    // calculate proper distance for the first neutralino
    const Float_t genN1traveld = std::sqrt(rad2(genN1decayvx-genN1prodvx,genN1decayvy-genN1prodvy,genN1decayvz-genN1prodvz)); 
    fPlots["genN1traveld"]->Fill(genN1traveld);
    fPlots["genN1traveld_zoom"]->Fill(genN1traveld);

    // calculate total momentum for the first neutralino
    TLorentzVector genN1_lorvec; genN1_lorvec.SetPtEtaPhiE(genN1pt,genN1eta,genN1phi,genN1E);
    const Float_t genN1p  = std::sqrt(rad2(genN1_lorvec.Px(),genN1_lorvec.Py(),genN1_lorvec.Pz()));
    const Float_t genN1bg = bg(genN1p,genN1mass);

    // ctau is distance / beta*gamma
    fPlots["genN1ctau"]->Fill(genN1traveld/genN1bg);
  }
  if (nNeutoPhGr >= 2)
  {
    // distances in lab frame
    fPlots["genN2prodz"]->Fill(std::abs(genN2prodvz));
    fPlots["genN2prodr"]->Fill(std::sqrt(rad2(genN2prodvx,genN2prodvy)));
    fPlots["genN2prodd"]->Fill(std::sqrt(rad2(genN2prodvx,genN2prodvy,genN2prodvz)));
    fPlots2D["genN2prodr_vs_z"]->Fill(std::abs(genN2prodvz),std::sqrt(rad2(genN2prodvx,genN2prodvy)));

    fPlots["genN2decayz"]->Fill(std::abs(genN2decayvz));
    fPlots["genN2decayr"]->Fill(std::sqrt(rad2(genN2decayvx,genN2decayvy)));
    fPlots["genN2decayd"]->Fill(std::sqrt(rad2(genN2decayvx,genN2decayvy,genN2decayvz)));
    fPlots2D["genN2decayr_vs_z"]->Fill(std::abs(genN2decayvz),std::sqrt(rad2(genN2decayvx,genN2decayvy)));

    fPlots["genN2decayz_zoom"]->Fill(std::abs(genN2decayvz));
    fPlots["genN2decayr_zoom"]->Fill(std::sqrt(rad2(genN2decayvx,genN2decayvy)));
    fPlots["genN2decayd_zoom"]->Fill(std::sqrt(rad2(genN2decayvx,genN2decayvy,genN2decayvz)));
    fPlots2D["genN2decayr_vs_z_zoom"]->Fill(std::abs(genN2decayvz),std::sqrt(rad2(genN2decayvx,genN2decayvy)));

    // calculate proper distance for the first neutralino
    const Float_t genN2traveld = std::sqrt(rad2(genN2decayvx-genN2prodvx,genN2decayvy-genN2prodvy,genN2decayvz-genN2prodvz)); 
    fPlots["genN2traveld"]->Fill(genN2traveld);
    fPlots["genN2traveld_zoom"]->Fill(genN2traveld);

    // calculate total momentum for the first neutralino
    TLorentzVector genN2_lorvec; genN2_lorvec.SetPtEtaPhiE(genN2pt,genN2eta,genN2phi,genN2E);
    const Float_t genN2p  = std::sqrt(rad2(genN2_lorvec.Px(),genN2_lorvec.Py(),genN2_lorvec.Pz()));
    const Float_t genN2bg = bg(genN2p,genN2mass);

    // ctau is distance / beta*gamma
    fPlots["genN2ctau"]->Fill(genN2traveld/genN2bg);
  }

  // 2D plots
  fPlots2D["genN2EvsN1E"]->Fill(genN1E,genN2E);
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

void PlotPhotons::FillObjectCounts()
{
  fPlots["nvtx"]->Fill(nvtx);
 
  Int_t nMatchedJets = 0;
  Int_t nJets = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (fApplyJetPtCut && (*jetpt)[ijet] < fJetPtCut) continue;
    nJets++;
    if (fIsGMSB)
    {
      if ( (*jetmatch)[ijet] >= 0 ) nMatchedJets++; 
    }
  }
  fPlots["njets"]->Fill(nJets);  
  if (fIsGMSB) fPlots["nmatchedjets"]->Fill(nMatchedJets);  

  Int_t nPhotons = 0;
  Int_t nLoosePh = 0;
  Int_t nMediumPh = 0;
  Int_t nTightPh = 0;
  for (Int_t iph = 0; iph < nphotons; iph++)
  { 
    if (fApplyPhPtCut && (*phpt)[iph] < fPhPtCut) continue;
    if (fApplyECALAcceptCut && (std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    nPhotons++;
    if ( (*phVID)[iph] >= 1 ) nLoosePh++;
    if ( (*phVID)[iph] >= 2 ) nMediumPh++;
    if ( (*phVID)[iph] >= 3 ) nTightPh++;
  }
  fPlots["nphotons"]->Fill(nPhotons);  
  fPlots["nlooseph"]->Fill(nLoosePh);
  fPlots["nmediumph"]->Fill(nMediumPh);
  fPlots["ntightph"]->Fill(nTightPh);
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
  if (fIsGMSB)
  {
    fPlots["t1pfMETgenMETpt"]->Fill(t1pfMETgenMETpt);
    fPlots["t1pfMETgenMETphi"]->Fill(t1pfMETgenMETphi);
    fPlots["t1pfMETgenMETsumEt"]->Fill(t1pfMETgenMETsumEt);
  }
}

void PlotPhotons::FillJets()
{
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (ijet == 0) fPlots["jet1pt"]->Fill((*jetpt)[ijet]);

    if (fApplyJetPtCut && (*jetpt)[ijet] < fJetPtCut) continue;
    fPlots["jetE"]->Fill((*jetE)[ijet]);
    fPlots["jetpt"]->Fill((*jetpt)[ijet]);
    fPlots["jetphi"]->Fill((*jetphi)[ijet]);
    fPlots["jeteta"]->Fill((*jeteta)[ijet]);
    
    if (fIsGMSB)
    {
      if ( (*jetmatch)[ijet] >= 0 ) 
      {
	fPlots["jetE_gen"]->Fill((*jetE)[ijet]);
	fPlots["jetpt_gen"]->Fill((*jetpt)[ijet]);
	fPlots["jetphi_gen"]->Fill((*jetphi)[ijet]);
	fPlots["jeteta_gen"]->Fill((*jeteta)[ijet]);
      }
    }
  }
  if (njets == 0) fPlots["jet1pt"]->Fill(-1.f);
}

void PlotPhotons::FillRecoPhotons()
{
  Bool_t passed_pt  = false; // n-1 for VID plots
  Bool_t passed_vid = false; // n-1 for pT  plots

  Int_t passed = 0;
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    if (fApplyECALAcceptCut && (std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;

    // N-1 like plots
    if ((*phpt)[iph]  >= fPhPtCut          && !passed_pt)  
    { 
      //      fPlots["ph1VID_nm1"]->Fill((*phVID)[iph]); 
      if ( (*phVID)[iph] == 0 ) fPlots["ph1VID_nm1"]->Fill(0); 
      if ( (*phVID)[iph] >= 1 ) fPlots["ph1VID_nm1"]->Fill(1); 
      if ( (*phVID)[iph] >= 2 ) fPlots["ph1VID_nm1"]->Fill(2); 
      if ( (*phVID)[iph] >= 3 ) fPlots["ph1VID_nm1"]->Fill(3); 

      passed_pt  = true; 
    }
    if ((*phVID)[iph] >= fPhVIDMap[fPhVID] && !passed_vid) { fPlots["ph1pt_nm1"] ->Fill((*phpt) [iph]); passed_vid = true; }

    if (fApplyPhPtCut && ((*phpt)[iph] < fPhPtCut)) continue;
    if (fApplyPhVIDCut && ((*phVID)[iph] < fPhVIDMap[fPhVID])) continue;
    //    if ((fApplyEBOnly && (std::abs((*phsceta)[iph]) > 1.4442)) || (fApplyEEOnly && (std::abs((*phsceta)[iph]) < 1.566 || std::abs((*phsceta)[iph]) > 2.5))) continue;
    //    if ( (fIsGMSB && (*phmatch)[iph] <= 0) || (fIsBkg && (*phisMatched)[iph] == 0) ) continue; // set to != 0 for QCD anti-matching, == 0 for GJet exact matching

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
    fPlots["phscE"]->Fill((*phscE)[iph]);
    fPlots["phisMatched"]->Fill((*phisMatched)[iph]);

    if (passed == 0)
    {
      fPlots["ph1pt"]->Fill((*phpt)[iph]);
      fPlots["ph1HoE"]->Fill((*phHoE)[iph]);
      fPlots["ph1r9"]->Fill((*phr9)[iph]);
      fPlots["ph1ChgIso"]->Fill(std::max((*phChgIso)[iph],0.f));    
      fPlots["ph1NeuIso"]->Fill(std::max((*phNeuIso)[iph],0.f));
      fPlots["ph1Iso"]->Fill(std::max((*phIso)[iph],0.f));
      fPlots["ph1suisseX"]->Fill((*phsuisseX)[iph]);
      fPlots["ph1sieie"]->Fill((*phsieie)[iph]);
      fPlots["ph1sipip"]->Fill((*phsipip)[iph]);
      fPlots["ph1sieip"]->Fill((*phsieip)[iph]);
      fPlots["ph1smaj"]->Fill((*phsmaj)[iph]);
      fPlots["ph1smin"]->Fill((*phsmin)[iph]);
      fPlots["ph1smin_ov_ph1smaj"]->Fill((*phsmin)[iph]/(*phsmaj)[iph]);
      fPlots2D["ph1smin_vs_ph1smaj"]->Fill((*phsmaj)[iph],(*phsmin)[iph]);
    }

    if (fIsMC) 
    {
      if ( (fIsGMSB && (*phmatch)[iph] > 0) || (fIsBkg && (*phisMatched)[iph]) ) // gen matched photons
      {
	fPlots["phE_gen"]->Fill((*phE)[iph]);
	fPlots["phpt_gen"]->Fill((*phpt)[iph]);
	fPlots["phphi_gen"]->Fill((*phphi)[iph]);
	fPlots["pheta_gen"]->Fill((*pheta)[iph]);
	fPlots["phscE_gen"]->Fill((*phscE)[iph]);
      }
    }
  
    // loop over rechits (+ seed info)
    Int_t nRecHits = 0;
    Int_t nRecHits_gen = 0;
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
      
	if (passed == 0) 
	{
	  fPlots["ph1seedtime"]->Fill((*phrhtime)[iph][irh]);
	  fPlots2D["ph1seedtime_vs_ph1smaj"]->Fill((*phsmaj)[iph],(*phrhtime)[iph][irh]);
	  fPlots2D["ph1seedtime_vs_ph1smin"]->Fill((*phsmin)[iph],(*phrhtime)[iph][irh]);
	  fPlots2D["ph1seedtime_vs_ph1smin_ov_ph1smaj"]->Fill(((*phsmin)[iph]/(*phsmaj)[iph]),(*phrhtime)[iph][irh]);
	  fPlots2D["ph1seedtime_vs_ph1pt"]->Fill((*phpt)[iph],(*phrhtime)[iph][irh]);
	}
      }

      if (fIsGMSB)
      {
	if ( (*phmatch)[iph] > 0 ) // gen matched photons
        {
	  nRecHits_gen++;
	  fPlots["phrhE_gen"]->Fill((*phrhE)[iph][irh]);
	  fPlots["phrhtime_gen"]->Fill((*phrhtime)[iph][irh]);
	  fPlots["phrhOOT_gen"]->Fill((*phrhOOT)[iph][irh]);
	  if ( (*phseedpos)[iph] == irh ) // seed info
	  {
	    fPlots["phseedE_gen"]->Fill((*phrhE)[iph][irh]);
	    fPlots["phseedtime_gen"]->Fill((*phrhtime)[iph][irh]);
	    fPlots["phseedOOT_gen"]->Fill((*phrhOOT)[iph][irh]);
	  }
	}
      } // end conditional over isMC
    } // end loop over nrechits

    fPlots["phnrh"]->Fill(nRecHits);
    if (fIsGMSB && (*phmatch)[iph] > 0) fPlots["phnrh_gen"]->Fill(nRecHits_gen);
    passed++;
  } // end loop over nphotons

  // fill -1 for total failure
  if (!passed_pt)  { fPlots["ph1VID_nm1"]->Fill(-1.f); }
  if (!passed_vid) { fPlots["ph1pt_nm1"] ->Fill(-1.f); }
}

void PlotPhotons::SetupAnalysis()
{
  fPlots2D["MET_vs_seed1time"] = PlotPhotons::MakeTH2F("MET_vs_seed1time","MET vs Leading Photon Seed RecHit Time",80,-2.f,14.f,"Leading Photon Seed RecHit Time [ns]",100,0.f,2000.f,"MET","Analysis");
}

void PlotPhotons::SetupEffs()
{
  fEffs["effMatchVsGenPhPt"] = PlotPhotons::MakeTEff("effMatchVsGenPhPt","Gen to Reco Photon Matching Efficiency vs. Gen Photon p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","#epsilon","Efficiencies");
  fEffs["effMatchVsGenPhPhi"] = PlotPhotons::MakeTEff("effMatchVsGenPhPhi","Gen to Reco Photon Matching Efficiency vs. Gen Photon #phi",100,-3.2,3.2,"#phi","#epsilon","Efficiencies");
  fEffs["effMatchVsGenPhEta"] = PlotPhotons::MakeTEff("effMatchVsGenPhEta","Gen to Reco Photon Matching Efficiency vs. Gen Photon #eta",100,-6.0,6.0,"#eta","#epsilon","Efficiencies");
  fEffs["effMatchVsGenPhVtx"] = PlotPhotons::MakeTEff("effMatchVsGenPhVtx","Gen to Reco Photon Matching Efficiency vs. Gen Photon Production Vertex Distance [cm]",200,0.f,100.f,"Production Vertex Distance [cm]","#epsilon","Efficiencies");
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

  fPlots["genN1mass"] = PlotPhotons::MakeTH1F("genN1mass","Generator Leading Neutralino Mass [GeV]",100,0.f,500.f,"Mass [GeV/c^{2}]","Neutralinos","GMSB");
  fPlots["genN1E"] = PlotPhotons::MakeTH1F("genN1E","Generator Leading Neutralino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Neutralinos","GMSB");
  fPlots["genN1pt"] = PlotPhotons::MakeTH1F("genN1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Neutralinos","GMSB");
  fPlots["genN1phi"] = PlotPhotons::MakeTH1F("genN1phi","Generator Leading Neutralino #phi",100,-3.2,3.2,"#phi","Neutralinos","GMSB");
  fPlots["genN1eta"] = PlotPhotons::MakeTH1F("genN1eta","Generator Leading Neutralino #eta",100,-6.0,6.0,"#eta","Neutralinos","GMSB");
  fPlots["genN1prodz"] = PlotPhotons::MakeTH1F("genN1prodz","Generator Leading Neutralino Production Vertex z-Position [cm]",100,0.f,20.f,"Production Vertex z-Position [cm]","Neutralinos","GMSB");
  fPlots["genN1prodr"] = PlotPhotons::MakeTH1F("genN1prodr","Generator Leading Neutralino Production Vertex r-Position [cm]",100,0.f,2.f,"Production Vertex r-Position [cm]","Neutralinos","GMSB");
  fPlots["genN1prodd"] = PlotPhotons::MakeTH1F("genN1prodd","Generator Leading Neutralino Production Vertex Distance [cm]",100,0.f,20.f,"Production Vertex Distance [cm]","Neutralinos","GMSB");
  fPlots2D["genN1prodr_vs_z"] = PlotPhotons::MakeTH2F("genN1prodr_vs_z","Generator Leading Neutralino Production Vertex r vs. z Position [cm]",100,0.f,20.f,"Production Vertex z-Position [cm]",100,0.f,2.f,"Production Vertex r-Position [cm]","GMSB");
  fPlots["genN1decayz"] = PlotPhotons::MakeTH1F("genN1decayz","Generator Leading Neutralino Decay Vertex z-Position [cm]",143,0.f,10000.f,"Decay Vertex z-Position [cm]","Neutralinos","GMSB");
  fPlots["genN1decayr"] = PlotPhotons::MakeTH1F("genN1decayr","Generator Leading Neutralino Decay Vertex r-Position [cm]",125,0.f,10000.f,"Decay Vertex r-Position [cm]","Neutralinos","GMSB");
  fPlots["genN1decayd"] = PlotPhotons::MakeTH1F("genN1decayd","Generator Leading Neutralino Decay Vertex Distance [cm]",100,0.f,10000.f,"Decay Vertex Distance [cm]","Neutralinos","GMSB");
  fPlots2D["genN1decayr_vs_z"] = PlotPhotons::MakeTH2F("genN1decayr_vs_z","Generator Leading Neutralino Decay Vertex r vs. z Position [cm]",100,0.f,10000.f,"Decay Vertex z-Position [cm]",100,0.f,10000.f,"Decay Vertex r-Position [cm]","GMSB");
  fPlots["genN1decayz_zoom"] = PlotPhotons::MakeTH1F("genN1decayz_zoom","Generator Leading Neutralino Decay Vertex z-Position [cm]",100,0.f,350.f,"Decay Vertex z-Position [cm]","Neutralinos","GMSB");
  fPlots["genN1decayr_zoom"] = PlotPhotons::MakeTH1F("genN1decayr_zoom","Generator Leading Neutralino Decay Vertex r-Position [cm]",100,0.f,160.f,"Decay Vertex r-Position [cm]","Neutralinos","GMSB");
  fPlots["genN1decayd_zoom"] = PlotPhotons::MakeTH1F("genN1decayd_zoom","Generator Leading Neutralino Decay Vertex Distance [cm]",100,0.f,380.f,"Decay Vertex Distance [cm]","Neutralinos","GMSB");
  fPlots2D["genN1decayr_vs_z_zoom"] = PlotPhotons::MakeTH2F("genN1decayr_vs_z_zoom","Generator Leading Neutralino Decay Vertex r vs. z Position [cm]",100,0.f,350.f,"Decay Vertex z-Position [cm]",100,0.f,160.f,"Decay Vertex r-Position [cm]","GMSB");
  fPlots["genN1traveld"] = PlotPhotons::MakeTH1F("genN1traveld","Generator Leading Neutralino Travel Distance [cm]",100,0.f,10000.f,"Distance [cm]","Neutralinos","GMSB");
  fPlots["genN1traveld_zoom"] = PlotPhotons::MakeTH1F("genN1traveld_zoom","Generator Leading Neutralino Travel Distance [cm]",100,0.f,400.f,"Distance [cm]","Neutralinos","GMSB");
  fPlots["genN1ctau"] = PlotPhotons::MakeTH1F("genN1ctau","Generator Leading Neutralino c#tau [cm]",200,0.f,fCTau*20.f,"c#tau [cm]","Neutralinos","GMSB");
  fPlots["genph1E"] = PlotPhotons::MakeTH1F("genph1E","Generator Leading Photon E [GeV]",100,0.f,2500.f,"Energy [GeV]","Photons","GMSB");
  fPlots["genph1pt"] = PlotPhotons::MakeTH1F("genph1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","GMSB");
  fPlots["genph1phi"] = PlotPhotons::MakeTH1F("genph1phi","Generator Leading Photon #phi",100,-3.2,3.2,"#phi","Photons","GMSB");
  fPlots["genph1eta"] = PlotPhotons::MakeTH1F("genph1eta","Generator Leading Photon #eta",100,-6.0,6.0,"#eta","Photons","GMSB");
  fPlots["gengr1E"] = PlotPhotons::MakeTH1F("gengr1E","Generator Leading Gravitino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Gravitinos","GMSB");
  fPlots["gengr1pt"] = PlotPhotons::MakeTH1F("gengr1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Gravitinos","GMSB");
  fPlots["gengr1phi"] = PlotPhotons::MakeTH1F("gengr1phi","Generator Leading Gravitino #phi",100,-3.2,3.2,"#phi","Gravitinos","GMSB");
  fPlots["gengr1eta"] = PlotPhotons::MakeTH1F("gengr1eta","Generator Leading Gravitino #eta",100,-6.0,6.0,"#eta","Gravitinos","GMSB");

  fPlots["genN2mass"] = PlotPhotons::MakeTH1F("genN2mass","Generator Subleading Neutralino Mass [GeV]",100,0.f,500.f,"Mass [GeV/c^{2}]","Neutralinos","GMSB");
  fPlots["genN2E"] = PlotPhotons::MakeTH1F("genN2E","Generator Subleading Neutralino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Neutralinos","GMSB");
  fPlots["genN2pt"] = PlotPhotons::MakeTH1F("genN2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Neutralinos","GMSB");
  fPlots["genN2phi"] = PlotPhotons::MakeTH1F("genN2phi","Generator Subleading Neutralino #phi",100,-3.2,3.2,"#phi","Neutralinos","GMSB");
  fPlots["genN2eta"] = PlotPhotons::MakeTH1F("genN2eta","Generator Subleading Neutralino #eta",100,-6.0,6.0,"#eta","Neutralinos","GMSB");
  fPlots["genN2prodz"] = PlotPhotons::MakeTH1F("genN2prodz","Generator Subleading Neutralino Production Vertex z-Position [cm]",100,0.f,20.f,"Production Vertex z-Position [cm]","Neutralinos","GMSB");
  fPlots["genN2prodr"] = PlotPhotons::MakeTH1F("genN2prodr","Generator Subleading Neutralino Production Vertex r-Position [cm]",100,0.f,2.f,"Production Vertex r-Position [cm]","Neutralinos","GMSB");
  fPlots["genN2prodd"] = PlotPhotons::MakeTH1F("genN2prodd","Generator Subleading Neutralino Production Vertex Distance [cm]",100,0.f,20.f,"Production Vertex Distance [cm]","Neutralinos","GMSB");
  fPlots2D["genN2prodr_vs_z"] = PlotPhotons::MakeTH2F("genN2prodr_vs_z","Generator Subleading Neutralino Production Vertex r vs. z Position [cm]",100,0.f,20.f,"Production Vertex z-Position [cm]",100,0.f,2.f,"Production Vertex r-Position [cm]","GMSB");
  fPlots["genN2decayz"] = PlotPhotons::MakeTH1F("genN2decayz","Generator Leading Neutralino Decay Vertex z-Position [cm]",143,0.f,10000.f,"Decay Vertex z-Position [cm]","Neutralinos","GMSB");
  fPlots["genN2decayr"] = PlotPhotons::MakeTH1F("genN2decayr","Generator Leading Neutralino Decay Vertex r-Position [cm]",125,0.f,10000.f,"Decay Vertex r-Position [cm]","Neutralinos","GMSB");
  fPlots["genN2decayd"] = PlotPhotons::MakeTH1F("genN2decayd","Generator Leading Neutralino Decay Vertex Distance [cm]",100,0.f,10000.f,"Decay Vertex Distance [cm]","Neutralinos","GMSB");
  fPlots2D["genN2decayr_vs_z"] = PlotPhotons::MakeTH2F("genN2decayr_vs_z","Generator Leading Neutralino Decay Vertex r vs. z Position [cm]",100,0.f,10000.f,"Decay Vertex z-Position [cm]",100,0.f,10000.f,"Decay Vertex r-Position [cm]","GMSB");
  fPlots["genN2decayz_zoom"] = PlotPhotons::MakeTH1F("genN2decayz_zoom","Generator Leading Neutralino Decay Vertex z-Position [cm]",100,0.f,350.f,"Decay Vertex z-Position [cm]","Neutralinos","GMSB");
  fPlots["genN2decayr_zoom"] = PlotPhotons::MakeTH1F("genN2decayr_zoom","Generator Leading Neutralino Decay Vertex r-Position [cm]",100,0.f,160.f,"Decay Vertex r-Position [cm]","Neutralinos","GMSB");
  fPlots["genN2decayd_zoom"] = PlotPhotons::MakeTH1F("genN2decayd_zoom","Generator Leading Neutralino Decay Vertex Distance [cm]",100,0.f,380.f,"Decay Vertex Distance [cm]","Neutralinos","GMSB");
  fPlots2D["genN2decayr_vs_z_zoom"] = PlotPhotons::MakeTH2F("genN2decayr_vs_z_zoom","Generator Leading Neutralino Decay Vertex r vs. z Position [cm]",100,0.f,350.f,"Decay Vertex z-Position [cm]",100,0.f,160.f,"Decay Vertex r-Position [cm]","GMSB");
  fPlots["genN2traveld"] = PlotPhotons::MakeTH1F("genN2traveld","Generator Leading Neutralino Travel Distance [cm]",100,0.f,10000.f,"Distance [cm]","Neutralinos","GMSB");
  fPlots["genN2traveld_zoom"] = PlotPhotons::MakeTH1F("genN2traveld_zoom","Generator Leading Neutralino Travel Distance [cm]",100,0.f,400.f,"Distance [cm]","Neutralinos","GMSB");
  fPlots["genN2ctau"] = PlotPhotons::MakeTH1F("genN2ctau","Generator Subleading Neutralino c#tau [cm]",200,0.f,fCTau*20.f,"c#tau [cm]","Neutralinos","GMSB");
  fPlots["genph2E"] = PlotPhotons::MakeTH1F("genph2E","Generator Subleading Photon E [GeV]",100,0.f,2500.f,"Energy [GeV]","Photons","GMSB");
  fPlots["genph2pt"] = PlotPhotons::MakeTH1F("genph2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","GMSB");
  fPlots["genph2phi"] = PlotPhotons::MakeTH1F("genph2phi","Generator Subleading Photon #phi",100,-3.2,3.2,"#phi","Photons","GMSB");
  fPlots["genph2eta"] = PlotPhotons::MakeTH1F("genph2eta","Generator Subleading Photon #eta",100,-6.0,6.0,"#eta","Photons","GMSB");
  fPlots["gengr2E"] = PlotPhotons::MakeTH1F("gengr2E","Generator Subleading Gravitino E [GeV]",100,0.f,2500.f,"Energy [GeV]","Gravitinos","GMSB");
  fPlots["gengr2pt"] = PlotPhotons::MakeTH1F("gengr2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2500.f,"p_{T} [GeV/c]","Gravitinos","GMSB");
  fPlots["gengr2phi"] = PlotPhotons::MakeTH1F("gengr2phi","Generator Subleading Gravitino #phi",100,-3.2,3.2,"#phi","Gravitinos","GMSB");
  fPlots["gengr2eta"] = PlotPhotons::MakeTH1F("gengr2eta","Generator Subleading Gravitino #eta",100,-6.0,6.0,"#eta","Gravitinos","GMSB");

  // 2D Histogram
  fPlots2D["genN2EvsN1E"] = PlotPhotons::MakeTH2F("genN2EvsN1E","Generator Neutralino2 E vs Neutralino1 E [GeV]",100,0.f,2500.f,"Neutralino1 Energy [GeV]",100,0.f,2500.f,"Neutralino2 Energy [GeV]","GMSB");
}

void PlotPhotons::SetupGenJets()
{
  fPlots["ngenjets"] = PlotPhotons::MakeTH1F("ngenjets","nGenJets",40,0.f,40.f,"nGenJets","Events","GenJets");
  fPlots["genjetE"] = PlotPhotons::MakeTH1F("genjetE","Generator Jets Energy [GeV]",100,0.f,3000.f,"Energy [GeV]","Generator Jets","GenJets");
  fPlots["genjetpt"] = PlotPhotons::MakeTH1F("genjetpt","Generator Jets p_{T} [GeV/c]",100,0.f,3000.f,"Generator jet p_{T} [GeV/c]","Generator Jets","GenJets");
  fPlots["genjetphi"] = PlotPhotons::MakeTH1F("genjetphi","Generator Jets #phi",100,-3.2,3.2,"#phi","Generator Jets","GenJets");
  fPlots["genjeteta"] = PlotPhotons::MakeTH1F("genjeteta","Generator Jets #eta",100,-6.0,6.0,"#eta","Generator Jets","GenJets");
}

void PlotPhotons::SetupObjectCounts()
{
  fPlots["nvtx"] = PlotPhotons::MakeTH1F("nvtx","nVertices (reco)",100,0.f,100.f,"nPV","Events","nReco");
  fPlots["njets"] = PlotPhotons::MakeTH1F("njets",Form("nJets, p_{T} > %4.1f GeV/c",fJetPtCut),40,0.f,40.f,Form("nJets [p_{T} > %4.1f GeV/c]",fJetPtCut),"Events","nReco");
  if (fIsGMSB) fPlots["nmatchedjets"] = PlotPhotons::MakeTH1F("nmatchedjets","nMatchedJets (reco to gen)",40,0.f,40.f,"nMatchedJets","Events","nReco");
  fPlots["nphotons"] = PlotPhotons::MakeTH1F("nphotons","nPhotons (reco)",20,0.f,20.f,"nPhotons","Events","nReco");
  fPlots["nlooseph"] = PlotPhotons::MakeTH1F("nlooseph","nLoosePhotons",20,0.f,20.f,"nLoosePhotons","Events","nReco");
  fPlots["nmediumph"] = PlotPhotons::MakeTH1F("nmediumph","nMediumPhotons",20,0.f,20.f,"nMediumPhotons","Events","nReco");
  fPlots["ntightph"] = PlotPhotons::MakeTH1F("ntightph","nTightPhotons",20,0.f,20.f,"nTightPhotons","Events","nReco");
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
  if (fIsGMSB)
  {
    fPlots["t1pfMETgenMETpt"] = PlotPhotons::MakeTH1F("t1pfMETgenMETpt","Type1 PF Gen MET [GeV]",100,0.f,2000.f,"MET [GeV]","Events","MET");
    fPlots["t1pfMETgenMETphi"] = PlotPhotons::MakeTH1F("t1pfMETgenMETphi","Type1 PF Gen MET #phi",100,-3.2,3.2,"#phi","Events","MET");
    fPlots["t1pfMETgenMETsumEt"] = PlotPhotons::MakeTH1F("t1pfMETgenMETsumEt","Type1 PF Gen MET #Sigma E_{T} [GeV]",100,0.f,8000.f,"#Sigma E_{T} [GeV]","Events","MET");
  }
}

void PlotPhotons::SetupJets()
{
  fPlots["jet1pt"] = PlotPhotons::MakeTH1F("jet1pt","Jets p_{T} [GeV/c] (reco)",101,-30.f,3000.f,"Jet p_{T} [GeV/c]","Jets","AK4Jets");

  fPlots["jetE"] = PlotPhotons::MakeTH1F("jetE","Jets Energy [GeV] (reco)",100,0.f,3000.f,"Energy [GeV]","Jets","AK4Jets");
  fPlots["jetpt"] = PlotPhotons::MakeTH1F("jetpt","Jets p_{T} [GeV/c] (reco)",100,0.f,3000.f,"Jet p_{T} [GeV/c]","Jets","AK4Jets");
  fPlots["jetphi"] = PlotPhotons::MakeTH1F("jetphi","Jets #phi (reco)",100,-3.2,3.2,"#phi","Jets","AK4Jets");
  fPlots["jeteta"] = PlotPhotons::MakeTH1F("jeteta","Jets #eta (reco)",100,-6.0,6.0,"#eta","Jets","AK4Jets");

  if (fIsGMSB) 
  {
    fPlots["jetE_gen"]   = PlotPhotons::MakeTH1F("jetE_gen","Jets Energy [GeV] (reco: gen matched)",100,0.f,3000.f,"Energy [GeV]","Jets","AK4Jets");
    fPlots["jetpt_gen"]  = PlotPhotons::MakeTH1F("jetpt_gen","Jets p_{T} [GeV/c] (reco: gen matched)",100,0.f,3000.f,"p_{T} [GeV/c]","Jets","AK4Jets");
    fPlots["jetphi_gen"] = PlotPhotons::MakeTH1F("jetphi_gen","Jets #phi (reco: gen matched)",100,-3.2,3.2,"#phi","Jets","AK4Jets");
    fPlots["jeteta_gen"] = PlotPhotons::MakeTH1F("jeteta_gen","Jets #eta (reco: gen matched)",100,-6.0,6.0,"#eta","Jets","AK4Jets");
  }
}

void PlotPhotons::SetupRecoPhotons()
{
  // N-1 LIKE PLOTS
  fPlots["ph1pt_nm1"] = PlotPhotons::MakeTH1F("ph1pt_nm1","Photons p_{T} [GeV/c] (reco)",101,-15.f,1500.f,"Leading Loose ID Photon p_{T} [GeV/c]","Events","RecoPhotons");
  fPlots["ph1VID_nm1"] = PlotPhotons::MakeTH1F("ph1VID_nm1","Photons VID (reco)",5,-1,4,Form("Leading Photon VID [p_{T} > %4.1f GeV/c]",fPhPtCut),"Events","RecoPhotons");
  fPlots["ph1VID_nm1"]->GetXaxis()->SetBinLabel(1,"No Photon");
  fPlots["ph1VID_nm1"]->GetXaxis()->SetBinLabel(2,"Fail");
  fPlots["ph1VID_nm1"]->GetXaxis()->SetBinLabel(3,"Loose");
  fPlots["ph1VID_nm1"]->GetXaxis()->SetBinLabel(4,"Medium");
  fPlots["ph1VID_nm1"]->GetXaxis()->SetBinLabel(5,"Tight");

  // All reco photons
  fPlots["phE"] = PlotPhotons::MakeTH1F("phE","Photons Energy [GeV] (reco)",100,0.f,2500.f,"Energy [GeV]","Photons","RecoPhotons");
  fPlots["phpt"] = PlotPhotons::MakeTH1F("phpt","Photons p_{T} [GeV/c] (reco)",100,0.f,2500.f,"p_{T} [GeV/c]","Photons","RecoPhotons");
  fPlots["phphi"] = PlotPhotons::MakeTH1F("phphi","Photons #phi (reco)",100,-3.2,3.2,"#phi","Photons","RecoPhotons");
  fPlots["pheta"] = PlotPhotons::MakeTH1F("pheta","Photons #eta (reco)",100,-5.0,5.0,"#eta","Photons","RecoPhotons");
  fPlots["phHoE"] = PlotPhotons::MakeTH1F("phHoE","Photons HE/EE (reco)",100,0.f,5.0,"HE/EE","Photons","RecoPhotons");
  fPlots["phr9"] = PlotPhotons::MakeTH1F("phr9","Photons r9 (reco)",100,0.f,25.0,"HE/EE","Photons","RecoPhotons");
  fPlots["phChgIso"] = PlotPhotons::MakeTH1F("phChgIso","Photons #rho-corrected Charged Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Charged Hadron Isolation","Photons","RecoPhotons");
  fPlots["phNeuIso"] = PlotPhotons::MakeTH1F("phNeuIso","Photons #rho-corrected Neutral Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Neutral Hadron Isolation","Photons","RecoPhotons");
  fPlots["phIso"] = PlotPhotons::MakeTH1F("phIso","Photons #rho-corrected Photon Isolation (reco)",100,0.f,1000.f,"#rho-corrected Photon Isolation","Photons","RecoPhotons");
  fPlots["phsuisseX"] = PlotPhotons::MakeTH1F("phsuisse","Photons Swiss Cross (reco)",100,0.f,1.f,"Swiss Cross","Photons","RecoPhotons");
  fPlots["phsieie"] = PlotPhotons::MakeTH1F("phsieie","Photons #sigma_{i#eta{}i#eta} (reco)",100,0,0.1,"#sigma_{i#eta{}i#eta}","Photons","RecoPhotons");
  fPlots["phsipip"] = PlotPhotons::MakeTH1F("phsipip","Photons #sigma_{i#phi{}i#phi} (reco)",100,0,0.1,"#sigma_{i#phi{}i#phi}","Photons","RecoPhotons");
  fPlots["phsieip"] = PlotPhotons::MakeTH1F("phsieip","Photons #sigma_{i#eta{}i#phi} (reco)",100,-0.005,0.005,"#sigma_{i#eta{}i#phi}","Photons","RecoPhotons");
  fPlots["phsmaj"] = PlotPhotons::MakeTH1F("phsmaj","Photons S_{major} (reco)",100,0.f,0.05,"S_{major}","Photons","RecoPhotons");
  fPlots["phsmin"] = PlotPhotons::MakeTH1F("phsmin","Photons S_{minor} (reco)",100,0.f,0.05,"S_{minor}","Photons","RecoPhotons");
  fPlots["phsmin_ov_phsmaj"] = PlotPhotons::MakeTH1F("phsmin_ov_phsmaj","Photons S_{minor} / S_{major} (reco)",100,0,1.f,"S_{minor}/S_{major}","Photons","RecoPhotons");
  fPlots["phscE"] = PlotPhotons::MakeTH1F("phscE","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2500.f,"Energy [GeV]","Photons","RecoPhotons");
  fPlots["phnrh"] = PlotPhotons::MakeTH1F("phnrh","nRecHits from Photons (reco)",100,0.f,100.f,"nRecHits","Photons","RecoPhotons");
  fPlots["phisMatched"] = PlotPhotons::MakeTH1F("phisMatched","Photons isMatched (reco)",2,0,2,"isMatched","Photons","RecoPhotons");

  // all rec hits + seed info
  fPlots["phrhE"] = PlotPhotons::MakeTH1F("phrhE","Photons RecHits Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","RecHits","RecoPhotons");
  fPlots["phrhtime"] = PlotPhotons::MakeTH1F("phrhtime","Photons RecHits Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","RecHits","RecoPhotons");
  fPlots["phrhOOT"] = PlotPhotons::MakeTH1F("phrhOOT","Photons RecHits OoT Flag (reco)",2,0.f,2.f,"OoT Flag","RecHits","RecoPhotons");
  fPlots["phseedE"] = PlotPhotons::MakeTH1F("phseedE","Photons Seed RecHit Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Seed RecHits","RecoPhotons");
  fPlots["phseedtime"] = PlotPhotons::MakeTH1F("phseedtime","Photons Seed RecHit Time [ns] (reco)",200,-10.f,10.f,"Time [ns]","Seed RecHits","RecoPhotons");
  fPlots["phseedOOT"] = PlotPhotons::MakeTH1F("phseedOOT","Photons Seed RecHit OoT Flag (reco)",2,0.f,2.f,"OoT Flag","Seed RecHits","RecoPhotons");  

  // Leading photon info
  fPlots["ph1pt"] = PlotPhotons::MakeTH1F("ph1pt","Leading Photon p_{T} [GeV/c]",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]","Events","RecoPhotons");
  fPlots["ph1HoE"] = PlotPhotons::MakeTH1F("ph1HoE","Photons HE/EE (reco)",100,0.f,5.0,"HE/EE","Photons","RecoPhotons");
  fPlots["ph1r9"] = PlotPhotons::MakeTH1F("ph1r9","Photons r9 (reco)",100,0.f,25.0,"HE/EE","Photons","RecoPhotons");
  fPlots["ph1ChgIso"] = PlotPhotons::MakeTH1F("ph1ChgIso","Photons #rho-corrected Charged Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Charged Hadron Isolation","Photons","RecoPhotons");
  fPlots["ph1NeuIso"] = PlotPhotons::MakeTH1F("ph1NeuIso","Photons #rho-corrected Neutral Hadron Isolation (reco)",100,0.f,1000.f,"#rho-corrected Neutral Hadron Isolation","Photons","RecoPhotons");
  fPlots["ph1Iso"] = PlotPhotons::MakeTH1F("ph1Iso","Photons #rho-corrected Photon Isolation (reco)",100,0.f,1000.f,"#rho-corrected Photon Isolation","Photons","RecoPhotons");
  fPlots["ph1suisseX"] = PlotPhotons::MakeTH1F("ph1suisse","Photons Swiss Cross (reco)",100,0.f,1.f,"Swiss Cross","Photons","RecoPhotons");
  fPlots["ph1sieie"] = PlotPhotons::MakeTH1F("ph1sieie","Photons #sigma_{i#eta{}i#eta} (reco)",100,0,0.1,"#sigma_{i#eta{}i#eta}","Photons","RecoPhotons");
  fPlots["ph1sipip"] = PlotPhotons::MakeTH1F("ph1sipip","Photons #sigma_{i#phi{}i#phi} (reco)",100,0,0.1,"#sigma_{i#phi{}i#phi}","Photons","RecoPhotons");
  fPlots["ph1sieip"] = PlotPhotons::MakeTH1F("ph1sieip","Photons #sigma_{i#eta{}i#phi} (reco)",100,-0.005,0.005,"#sigma_{i#eta{}i#phi}","Photons","RecoPhotons");
  fPlots["ph1smaj"] = PlotPhotons::MakeTH1F("ph1smaj","Leading Photon S_{major}",100,0.f,0.05,"Leading Photon S_{major}","Events","RecoPhotons");
  fPlots["ph1smin"] = PlotPhotons::MakeTH1F("ph1smin","Leading Photon S_{minor}",100,0.f,0.05,"Leading Photon S_{minor}","Events","RecoPhotons");
  fPlots["ph1smin_ov_ph1smaj"] = PlotPhotons::MakeTH1F("ph1smin_ov_ph1smaj","Leading Photon S_{minor} / S_{major}",100,0.f,1.f,"Leading Photon S_{minor}/S_{major}","Events","RecoPhotons");
  fPlots["ph1seedtime"] = PlotPhotons::MakeTH1F("ph1seedtime","Leading Photon Seed RecHit Time [ns]",80,-5.f,15.f,"Leading Photon Seed RecHit Time [ns]","Events","RecoPhotons");

  fPlots2D["ph1smin_vs_ph1smaj"] = PlotPhotons::MakeTH2F("ph1smin_vs_ph1smaj","Leading Photon S_{minor} vs S_{major}",100,0.f,0.1,"Leading Photon S_{major}",100,0.f,0.1,"Leading Photon S_{minor}","RecoPhotons");
  fPlots2D["ph1seedtime_vs_ph1smaj"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1smaj","Leading Photon Seed RecHit Time vs S_{major}",100,0.f,0.1,"Leading Photon S_{major}",100,-5.f,15.f,"Leading Seed RecHit Time","RecoPhotons");
  fPlots2D["ph1seedtime_vs_ph1smin"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1smin","Leading Photon Seed RecHit Time vs S_{minor}",100,0.f,0.1,"Leading Photon S_{minor}",100,-5.f,15.f,"Leading Seed RecHit Time","RecoPhotons");
  fPlots2D["ph1seedtime_vs_ph1smin_ov_ph1smaj"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1smin_ov_ph1smaj","Leading Photon Seed RecHit Time [ns] vs S_{minor}/S_{major}",100,0.f,1.f,"Leading Photon S_{minor}/S_{major}",100,-5.f,15.f,"Leading Seed RecHit Time [ns]","RecoPhotons");
  fPlots2D["ph1seedtime_vs_ph1pt"] = PlotPhotons::MakeTH2F("ph1seedtime_vs_ph1pt","Leading Photon Seed RecHit Time [ns] vs p_{T} [GeV/c]",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]",100,-5.f,15.f,"Leading Seed RecHit Time [ns]","RecoPhotons");
  
  if (fIsMC)
  {
    // Gen matched reco quantities
    fPlots["phE_gen"] = PlotPhotons::MakeTH1F("phE_gen","Photons Energy [GeV] (reco: gen matched)",100,0.f,2000.f,"Energy [GeV]","Photons","RecoPhotons");
    fPlots["phpt_gen"] = PlotPhotons::MakeTH1F("phpt_gen","Photons p_{T} [GeV/c] (reco: gen matched)",100,0.f,2000.f,"p_{T} [GeV/c]","Photons","RecoPhotons");
    fPlots["phphi_gen"] = PlotPhotons::MakeTH1F("phphi_gen","Photons #phi (reco: gen matched)",100,-3.2,3.2,"#phi","Photons","RecoPhotons");
    fPlots["pheta_gen"] = PlotPhotons::MakeTH1F("pheta_gen","Photons #eta (reco: gen matched)",100,-5.0,5.0,"#eta","Photons","RecoPhotons");
    fPlots["phscE_gen"] = PlotPhotons::MakeTH1F("phscE_gen","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Photons","RecoPhotons");
    fPlots["phnrh_gen"] = PlotPhotons::MakeTH1F("phnrh_gen","nRecHits from Photons (reco: gen matched)",100,0.f,100.f,"nRecHits","Photons","RecoPhotons");
    
    // all rec hits + seed info for gen matched photons
    fPlots["phrhE_gen"] = PlotPhotons::MakeTH1F("phrhE_gen","Photons RecHits Energy [GeV] (reco: gen matched)",100,0.f,2000.f,"Energy [GeV]","RecHits","RecoPhotons");
    fPlots["phrhtime_gen"] = PlotPhotons::MakeTH1F("phrhtime_gen","Photons RecHits Time [ns] (reco: gen matched)",200,-100.f,100.f,"Time [ns]","RecHits","RecoPhotons");
    fPlots["phrhOOT_gen"] = PlotPhotons::MakeTH1F("phrhOOT_gen","Photons RecHits OoT Flag (reco: gen matched)",2,0.f,2.f,"OoT Flag","RecHits","RecoPhotons");
    fPlots["phseedE_gen"] = PlotPhotons::MakeTH1F("phseedE_gen","Photons Seed RecHit Energy [GeV] (reco: gen matched)",100,0.f,2000.f,"Energy [GeV]","Seed RecHits","RecoPhotons");
    fPlots["phseedtime_gen"] = PlotPhotons::MakeTH1F("phseedtime_gen","Photons Seed RecHit Time [ns] (reco: gen matched)",200,-10.f,10.f,"Time [ns]","Seed RecHits","RecoPhotons");
    fPlots["phseedOOT_gen"] = PlotPhotons::MakeTH1F("phseedOOT_gen","Photons Seed RecHit OoT Flag (reco: gen matched)",2,0.f,2.f,"OoT Flag","Seed RecHits","RecoPhotons");  
  }
}

TEfficiency * PlotPhotons::MakeTEff(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir)
{
  TEfficiency * eff = new TEfficiency(hname.Data(),Form("%s;%s;%s",htitle.Data(),xtitle.Data(),ytitle.Data()),nbinsx,xlow,xhigh);
  eff->SetLineColor(kBlack);

  fSubDirs[hname] = subdir;

  return eff;
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
  std::cout << std::endl << "-----------------" << std::endl << std::endl;
  Int_t total = fInTree->GetEntries();
  std::cout << "Total Events: " << total << std::endl;

  for (TStrIntMapIter mapiter = fEfficiency.begin(); mapiter != fEfficiency.end(); ++mapiter)
  {
    std::cout << "nEvents Passing " << (*mapiter).first.Data() << " Selection: " << (*mapiter).second <<std::endl;
  }

  // Compute event level efficiency + binomial error
  Int_t   passed      = fEfficiency["Events"];
  Float_t efficiency  = float(passed)/float(total);
  Float_t uncertainty = std::sqrt((efficiency*(1.f-efficiency))/float(total));
  std::cout << "Event level efficiency: " << efficiency << " +/- " << uncertainty << std::endl;

  std::cout << std::endl << "-----------------" << std::endl << std::endl;
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

void PlotPhotons::OutputTEffs()
{
  fOutFile->cd();

  for (TEffMapIter mapiter = fEffs.begin(); mapiter != fEffs.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    
    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    mapiter->second->Draw("AP");
    
    // first save as linear, then log
    canv->SetLogy(0);
    //canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
    canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->first.Data()));

    delete canv;
    delete mapiter->second;
  }
  fEffs.clear();
}

void PlotPhotons::OutputTH1Fs()
{
  fOutFile->cd();

  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    
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
    delete mapiter->second;
  }
  fPlots.clear();
}

void PlotPhotons::OutputTH2Fs()
{
  fOutFile->cd();

  for (TH2MapIter mapiter = fPlots2D.begin(); mapiter != fPlots2D.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    
    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    mapiter->second->Draw("colz");
    
    // first save as linear, then log
    canv->SetLogy(0);
    //canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
    canv->SaveAs(Form("%s/%s_lin.png",fOutDump.Data(),mapiter->first.Data()));

    delete canv;
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

  // set branch addresses
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
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
  if (fIsGMSB)
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
  if (fIsGMSB) fInTree->SetBranchAddress("phmatch", &phmatch, &b_phmatch);
  if (fIsMC)   fInTree->SetBranchAddress("phisMatched", &phisMatched, &b_phisMatched);
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
