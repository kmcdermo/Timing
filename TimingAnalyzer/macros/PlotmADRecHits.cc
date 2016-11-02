#include "PlotmADRecHits.hh"
#include "TStyle.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotmADRecHits::PlotmADRecHits(TString filename, TString outdir, 
			       Bool_t applyhltcut, Bool_t applyphptcut, Float_t phptcut,
			       Bool_t applyrhecut, Float_t rhEcut, Bool_t applyecalacceptcut,
			       Bool_t applyvidcut, TString VIDcut) :
  fOutDir(outdir), fApplyHLTCut(applyhltcut),
  fApplyPhPtCut(applyphptcut), fPhPtCut(phptcut),
  fApplyrhECut(applyrhecut), frhECut(rhEcut),
  fApplyECALAcceptCut(applyecalacceptcut),
  fApplyVIDCut(applyvidcut), fPhVID(VIDcut)
{
  // overall setup
  gStyle->SetOptStat("emrou");

  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/phrhtree");

  // initialize tree
  PlotmADRecHits::InitTree();

  // in routine initialization
  fNPhCheck = 1000;
  // make the vid maps!
  fPhVIDMap["loose"]  = 1;
  fPhVIDMap["medium"] = 2;
  fPhVIDMap["tight"]  = 3;

  // output
  // setup outdir name
  if (!fApplyHLTCut && !fApplyPhPtCut && !fApplyrhECut && !fApplyECALAcceptCut && !fApplyVIDCut)
  { 
    fOutDir += "/Inclusive";
  }
  else 
  {
    fOutDir += "/cuts";
    if (fApplyHLTCut)        fOutDir += Form("_hlt");
    if (fApplyPhPtCut)       fOutDir += Form("_phpt%3.1f" ,fPhPtCut);
    if (fApplyrhECut)        fOutDir += Form("_rhE%2.1f"  ,frhECut);
    if (fApplyECALAcceptCut) fOutDir += Form("_ecalaccept");
    if (fApplyVIDCut)        fOutDir += Form("_phVID%s",fPhVID.Data()); 
  }

  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());
  }

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

PlotmADRecHits::~PlotmADRecHits()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
}

void PlotmADRecHits::DoPlots()
{
  PlotmADRecHits::SetupPlots();
  PlotmADRecHits::PhotonLoop();
  PlotmADRecHits::MakeSubDirs();
  PlotmADRecHits::OutputTH1Fs();
  PlotmADRecHits::OutputTH2Fs();
  PlotmADRecHits::OutputTotalTH1Fs();
  PlotmADRecHits::ClearTH1Map();
  PlotmADRecHits::ClearTH2Map();
}

void PlotmADRecHits::SetupPlots()
{
  PlotmADRecHits::SetupRecoPhotons();
}

void PlotmADRecHits::PhotonLoop()
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    if (entry%fNPhCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    if (fApplyHLTCut && !hltdoubleph60) continue;
    if (fApplyPhPtCut && phE < fPhPtCut) continue;
    if (fApplyECALAcceptCut && (std::abs(pheta) > 2.5 || (std::abs(pheta) > 1.4442 && std::abs(pheta) < 1.566))) continue;
    if (fApplyVIDCut && fPhVIDMap[fPhVID] < phVID) continue;

    PlotmADRecHits::FillPatPhotons();
  }
}

void PlotmADRecHits::FillPatPhotons()
{
  fPlots["phE"]->Fill(phE);
  fPlots["phpt"]->Fill(phpt);
  fPlots["phE_zoom"]->Fill(phE);
  fPlots["phpt_zoom"]->Fill(phpt);
  fPlots["phphi"]->Fill(phphi);
  fPlots["pheta"]->Fill(pheta);

  fPlots["phscE"]->Fill(phscE);
  fPlots["phscE_zoom"]->Fill(phscE);
  fPlots["phscphi"]->Fill(phscphi);
  fPlots["phsceta"]->Fill(phsceta);
  
  //////////////////////
  //                  //
  // Reduced Rec Hits //
  //                  //
  //////////////////////

  // --> Core Rec Hits <-- //
  int nRecHits = 0;
  for (int irh = 0; irh < phnrhs; irh++)
  {
    if (fApplyrhECut && (*phrhEs)[irh] < frhECut) continue;
    nRecHits++;

    fPlots["phrhEs"]->Fill((*phrhEs)[irh]);
    fPlots["phrhEs_zoom"]->Fill((*phrhEs)[irh]);
    fPlots["phrhdelRs"]->Fill((*phrhdelRs)[irh]);
    fPlots["phrhtimes"]->Fill((*phrhtimes)[irh]);
    fPlots["phrhOOTs"]->Fill((*phrhOOTs)[irh]);
    if ( std::abs((*phrhtimes)[irh]) > 0.3 ) 
    {
      fPlots["phrhOOTs_absTgt3"]->Fill((*phrhOOTs)[irh]);
    }

    if ( (*phrhOOTs)[irh] ) // make plots for only OOT True
    {
      fPlots["phrhEs_OOTT"]->Fill((*phrhEs)[irh]);
      fPlots["phrhEs_OOTT_zoom"]->Fill((*phrhEs)[irh]);
      fPlots["phrhEs_OOTT_zoomer"]->Fill((*phrhEs)[irh]);
      fPlots["phrhdelRs_OOTT"]->Fill((*phrhdelRs)[irh]);
      fPlots["phrhtimes_OOTT"]->Fill((*phrhtimes)[irh]);
      fPlots["phrhtimes_OOTT_zoom"]->Fill((*phrhtimes)[irh]);
    }
    else // make plots for only OOT False
    {
      fPlots["phrhEs_OOTF"]->Fill((*phrhEs)[irh]);
      fPlots["phrhEs_OOTF_zoom"]->Fill((*phrhEs)[irh]);
      if ( std::abs((*phrhtimes)[irh]) > 0.3 ) 
      {  
	fPlots["phrhEs_OOTF_absTgt3"]->Fill((*phrhEs)[irh]);
	fPlots["phrhEs_OOTF_absTgt3_zoom"]->Fill((*phrhEs)[irh]);
      }
      fPlots["phrhdelRs_OOTF"]->Fill((*phrhdelRs)[irh]);
      fPlots["phrhtimes_OOTF"]->Fill((*phrhtimes)[irh]);
    }// end block over OOT only plots

    // --> Seed Rec Hits <-- //
    if ( phseedpos == irh ) // seed info
    {
      fPlots["phseedE"]->Fill((*phrhEs)[irh]);
      fPlots["phseedE_zoom"]->Fill((*phrhEs)[irh]);
      fPlots["phseeddelR"]->Fill((*phrhdelRs)[irh]);
      fPlots["phseedtime"]->Fill((*phrhtimes)[irh]);
      fPlots["phseedOOT"]->Fill((*phrhOOTs)[irh]);

      if ( (*phrhOOTs)[irh] ) // make plots for only OOT True
      {
	fPlots["phseedE_OOTT"]->Fill((*phrhEs)[irh]);
	fPlots["phseedE_OOTT_zoom"]->Fill((*phrhEs)[irh]);
	fPlots["phseeddelR_OOTT"]->Fill((*phrhdelRs)[irh]);
	fPlots["phseedtime_OOTT"]->Fill((*phrhtimes)[irh]);
	fPlots["phseedtime_OOTT_zoom"]->Fill((*phrhtimes)[irh]);
      }
      else // make plots for only OOT False
      {
	fPlots["phseedE_OOTF"]->Fill((*phrhEs)[irh]);
	fPlots["phseedE_OOTF_zoom"]->Fill((*phrhEs)[irh]);
	fPlots["phseeddelR_OOTF"]->Fill((*phrhdelRs)[irh]);
	fPlots["phseedtime_OOTF"]->Fill((*phrhtimes)[irh]);
      }// end block over OOT only plots
    } // end block over seeds
  } // end loop over "core" rechits
  fPlots["phnrhs"]->Fill(nRecHits);

  // --> Add-On Rec Hits <-- //
  int nRecHits_add = 0;
  for (int irh = phnrhs; irh < phnrhs_add; irh++)
  {
    if (fApplyrhECut && (*phrhEs)[irh] < frhECut) continue;
    nRecHits_add++;
    
    fPlots["phrhEs_add"]->Fill((*phrhEs)[irh]);
    fPlots["phrhEs_zoom_add"]->Fill((*phrhEs)[irh]);
    fPlots["phrhdelRs_add"]->Fill((*phrhdelRs)[irh]);
    fPlots["phrhtimes_add"]->Fill((*phrhtimes)[irh]);
    fPlots["phrhOOTs_add"]->Fill((*phrhOOTs)[irh]);

    if ( (*phrhOOTs)[irh] ) // make plots for only OOT True
    {
      fPlots["phrhEs_OOTT_add"]->Fill((*phrhEs)[irh]);
      fPlots["phrhEs_OOTT_zoom_add"]->Fill((*phrhEs)[irh]);
      fPlots["phrhdelRs_OOTT_add"]->Fill((*phrhdelRs)[irh]);
      fPlots["phrhtimes_OOTT_add"]->Fill((*phrhtimes)[irh]);
      fPlots["phrhtimes_OOTT_zoom_add"]->Fill((*phrhtimes)[irh]);
    }
    else // make plots for only OOT False
    {
      fPlots["phrhEs_OOTF_add"]->Fill((*phrhEs)[irh]);
      fPlots["phrhEs_OOTF_zoom_add"]->Fill((*phrhEs)[irh]);
      fPlots["phrhdelRs_OOTF_add"]->Fill((*phrhdelRs)[irh]);
      fPlots["phrhtimes_OOTF_add"]->Fill((*phrhtimes)[irh]);
    }// end block over OOT only plots
  } // end loop over "core" rechits
  fPlots["phnrhs_add"]->Fill(nRecHits_add);

  // --> Total Rec Hits <-- //
  fPlots["phnrhs_total"]->Fill(nRecHits+nRecHits_add);
}

void PlotmADRecHits::SetupRecoPhotons()
{
  // All reco photons + associated supercluster
  fPlots["phE"] = PlotmADRecHits::MakeTH1F("phE","Photons Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Photons","calibPhotons/GeneralProps");
  fPlots["phpt"] = PlotmADRecHits::MakeTH1F("phpt","Photons p_{T} [GeV/c] (reco)",100,0.f,2000.f,"p_{T} [GeV/c]","Photons","calibPhotons/GeneralProps");
  fPlots["phE_zoom"] = PlotmADRecHits::MakeTH1F("phE_zoom","Photons Energy [GeV] (reco)",100,0.f,100.f,"Energy [GeV]","Photons","calibPhotons/GeneralProps");
  fPlots["phpt_zoom"] = PlotmADRecHits::MakeTH1F("phpt_zoom","Photons p_{T} [GeV/c] (reco)",100,0.f,100.f,"p_{T} [GeV/c]","Photons","calibPhotons/GeneralProps");
  fPlots["phphi"] = PlotmADRecHits::MakeTH1F("phphi","Photons #phi (reco)",100,-3.2,3.2,"#phi","Photons","calibPhotons/GeneralProps");
  fPlots["pheta"] = PlotmADRecHits::MakeTH1F("pheta","Photons #eta (reco)",100,-3.0,3.0,"#eta","Photons","calibPhotons/GeneralProps");

  fPlots["phscE"] = PlotmADRecHits::MakeTH1F("phscE","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Photons","calibPhotons/GeneralProps");
  fPlots["phscE_zoom"] = PlotmADRecHits::MakeTH1F("phscE_zoom","Photons SuperCluster Energy [GeV] (reco)",100,0.f,100.f,"Energy [GeV]","Photons","calibPhotons/GeneralProps");
  fPlots["phscphi"] = PlotmADRecHits::MakeTH1F("phscphi","Photons SuperCluster #phi (reco)",100,-3.2,3.2,"#phi","Photons","calibPhotons/GeneralProps");
  fPlots["phsceta"] = PlotmADRecHits::MakeTH1F("phsceta","Photons SuperCluster #eta (reco)",100,-3.0,3.0,"#eta","Photons","calibPhotons/GeneralProps");
  
  //////////////////////
  //                  //
  // Reduced Rec Hits //
  //                  //
  //////////////////////
  // --> "core rechits" <--
  fPlots["phnrhs"] = PlotmADRecHits::MakeTH1F("phnrhs","nRecHits from Photons (reco)",100,0.f,100.f,"nRecHits","Photons","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs"] = PlotmADRecHits::MakeTH1F("phrhEs","Photons RecHits Energy [GeV] (reco)",100,0.f,1000.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_zoom"] = PlotmADRecHits::MakeTH1F("phrhEs_zoom","Photons RecHits Energy [GeV] (reco)",100,0.f,100.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhdelRs"] = PlotmADRecHits::MakeTH1F("phrhdelRs","#DeltaR of RecHits to Photon (reco)",100,0.f,1.0f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhtimes"] = PlotmADRecHits::MakeTH1F("phrhtimes","Photons RecHits Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhOOTs"] = PlotmADRecHits::MakeTH1F("phrhOOTs","Photons RecHits OoT Flag (reco)",2,0.f,2.f,"OoT Flag","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhOOTs_absTgt3"] = PlotmADRecHits::MakeTH1F("phrhOOTs_absTgt3","Photons RecHits OoT Flag (reco) - |T|>3ns",2,0.f,2.f,"OoT Flag","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  // OOT T+F plots
  fPlots["phrhEs_OOTT"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTT","Photons RecHits Energy [GeV] (reco) - OoT:T",100,0.f,1000.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_OOTT_zoom"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTT_zoom","Photons RecHits Energy [GeV] (reco) - OoT:T",100,0.f,100.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_OOTT_zoomer"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTT_zoomer","Photons RecHits Energy [GeV] (reco) - OoT:T",100,0.f,5.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhdelRs_OOTT"] = PlotmADRecHits::MakeTH1F("phrhdelRs_OOTT","#DeltaR of RecHits to Photon (reco) - OoT:T",100,0.f,1.0f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhtimes_OOTT"] = PlotmADRecHits::MakeTH1F("phrhtimes_OOTT","Photons RecHits Time [ns] (reco) - OoT:T",200,-100.f,100.f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhtimes_OOTT_zoom"] = PlotmADRecHits::MakeTH1F("phrhtimes_OOTT_zoom","Photons RecHits Time [ns] (reco) - OoT:T",200,-10.f,10.f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_OOTF"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTF","Photons RecHits Energy [GeV] (reco) - OoT:F",100,0.f,1000.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_OOTF_zoom"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTF_zoom","Photons RecHits Energy [GeV] (reco) - OoT:F",100,0.f,100.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_OOTF_absTgt3"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTF_absTgt3","Photons RecHits Energy [GeV] (reco) - OoT:F and |T|>3ns",100,0.f,1000.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhEs_OOTF_absTgt3_zoom"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTF_absTgt3_zoom","Photons RecHits Energy [GeV] (reco) - OoT:F and |T|>3ns",100,0.f,10.f,"Energy [GeV]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhdelRs_OOTF"] = PlotmADRecHits::MakeTH1F("phrhdelRs_OOTF","#DeltaR of RecHits to Photon (reco) - OoT:F",100,0.f,1.0f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");
  fPlots["phrhtimes_OOTF"] = PlotmADRecHits::MakeTH1F("phrhtimes_OOTF","Photons RecHits Time [ns] (reco) - OoT:F",200,-100.f,100.f,"Time [ns]","RecHits","calibPhotons/ReducedRHs/CoreRHs");

  // --> seed plots <--
  fPlots["phseedE"] = PlotmADRecHits::MakeTH1F("phseedE","Photons Seed RecHit Energy [GeV] (reco)",100,0.f,1000.f,"Energy [GeV]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedE_zoom"] = PlotmADRecHits::MakeTH1F("phseedE_zoom","Photons Seed RecHit Energy [GeV] (reco)",100,0.f,100.f,"Energy [GeV]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseeddelR"] = PlotmADRecHits::MakeTH1F("phseeddelR","#DeltaR of Seed RecHit to Photon (reco)",100,0.f,1.0f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedtime"] = PlotmADRecHits::MakeTH1F("phseedtime","Photons Seed RecHit Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedOOT"] = PlotmADRecHits::MakeTH1F("phseedOOT","Photons Seed RecHit OoT Flag (reco)",2,0.f,2.f,"OoT Flag","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");  
  // OOT T+F plots
  fPlots["phseedE_OOTT"] = PlotmADRecHits::MakeTH1F("phseedE_OOTT","Photons Seed RecHit Energy [GeV] (reco) - OoT:T",100,0.f,1000.f,"Energy [GeV]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedE_OOTT_zoom"] = PlotmADRecHits::MakeTH1F("phseedE_OOTT_zoom","Photons Seed RecHit Energy [GeV] (reco) - OoT:T",100,0.f,100.f,"Energy [GeV]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseeddelR_OOTT"] = PlotmADRecHits::MakeTH1F("phseeddelR_OOTT","#DeltaR of Seed RecHit to Photon (reco) - OoT:T",100,0.f,1.0f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedtime_OOTT"] = PlotmADRecHits::MakeTH1F("phseedtime_OOTT","Photons Seed RecHit Time [ns] (reco) - OoT:T",200,-100.f,100.f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedtime_OOTT_zoom"] = PlotmADRecHits::MakeTH1F("phseedtime_OOTT_zoom","Photons Seed RecHit Time [ns] (reco) - OoT:T",200,-10.f,10.f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedE_OOTF"] = PlotmADRecHits::MakeTH1F("phseedE_OOTF","Photons Seed RecHit Energy [GeV] (reco) - OoT:F",100,0.f,1000.f,"Energy [GeV]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedE_OOTF_zoom"] = PlotmADRecHits::MakeTH1F("phseedE_OOTF_zoom","Photons Seed RecHit Energy [GeV] (reco) - OoT:F",100,0.f,100.f,"Energy [GeV]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseeddelR_OOTF"] = PlotmADRecHits::MakeTH1F("phseeddelR_OOTF","#DeltaR of Seed RecHit to Photon (reco) - OoT:F",100,0.f,1.0f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");
  fPlots["phseedtime_OOTF"] = PlotmADRecHits::MakeTH1F("phseedtime_OOTF","Photons Seed RecHit Time [ns] (reco) - OoT:F",200,-100.f,100.f,"Time [ns]","Seed RecHits","calibPhotons/ReducedRHs/SeedRHs");

  // --> "add-on recHits" <--
  fPlots["phnrhs_add"] = PlotmADRecHits::MakeTH1F("phnrhs_add","nRecHits from Photons (reco) [add-ons]",100,0.f,100.f,"nRecHits_add","Photons","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhEs_add"] = PlotmADRecHits::MakeTH1F("phrhEs_add","Photons RecHits Energy [GeV] (reco) [add-ons]",100,0.f,1000.f,"Energy [GeV]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhEs_zoom_add"] = PlotmADRecHits::MakeTH1F("phrhEs_zoom_add","Photons RecHits Energy [GeV] (reco) [add-ons]",100,0.f,100.f,"Energy [GeV]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhdelRs_add"] = PlotmADRecHits::MakeTH1F("phrhdelRs_add","#DeltaR of RecHits to Photon (reco) [add-ons]",100,0.f,1.0f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhtimes_add"] = PlotmADRecHits::MakeTH1F("phrhtimes_add","Photons RecHits Time [ns] (reco) [add-ons]",200,-100.f,100.f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhOOTs_add"] = PlotmADRecHits::MakeTH1F("phrhOOTs_add","Photons RecHits OoT Flag (reco) [add-ons]",2,0.f,2.f,"OoT Flag","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  // OOT T+F plots
  fPlots["phrhEs_OOTT_add"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTT_add","Photons RecHits Energy [GeV] (reco) [add-ons] - OoT:T",100,0.f,1000.f,"Energy [GeV]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhEs_OOTT_zoom_add"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTT_zoom_add","Photons RecHits Energy [GeV] (reco) [add-ons] - OoT:T",100,0.f,100.f,"Energy [GeV]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhdelRs_OOTT_add"] = PlotmADRecHits::MakeTH1F("phrhdelRs_OOTT_add","#DeltaR of RecHits to Photon (reco) [add-ons] - OoT:T",100,0.f,1.0f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhtimes_OOTT_add"] = PlotmADRecHits::MakeTH1F("phrhtimes_OOTT_add","Photons RecHits Time [ns] (reco) [add-ons] - OoT:T",200,-100.f,100.f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhtimes_OOTT_zoom_add"] = PlotmADRecHits::MakeTH1F("phrhtimes_OOTT_zoom_add","Photons RecHits Time [ns] (reco) [add-ons] - OoT:T",200,-10.f,10.f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhEs_OOTF_add"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTF_add","Photons RecHits Energy [GeV] (reco) [add-ons] - OoT:F",100,0.f,1000.f,"Energy [GeV]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhEs_OOTF_zoom_add"] = PlotmADRecHits::MakeTH1F("phrhEs_OOTF_zoom_add","Photons RecHits Energy [GeV] (reco) [add-ons] - OoT:F",100,0.f,100.f,"Energy [GeV]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhdelRs_OOTF_add"] = PlotmADRecHits::MakeTH1F("phrhdelRs_OOTF_add","#DeltaR of RecHits to Photon (reco) [add-ons] - OoT:F",100,0.f,1.0f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");
  fPlots["phrhtimes_OOTF_add"] = PlotmADRecHits::MakeTH1F("phrhtimes_OOTF_add","Photons RecHits Time [ns] (reco) [add-ons] - OoT:F",200,-100.f,100.f,"Time [ns]","RecHits_add","calibPhotons/ReducedRHs/AddonRHs");

  // --> "total recHits" <--
  fPlots["phnrhs_total"] = PlotmADRecHits::MakeTH1F("phnrhs_total","nRecHits from Photons (reco) [core+add-ons]",100,0.f,100.f,"nRecHits_total","Photons","calibPhotons/ReducedRHs/TotalRHs");
}

TH1F * PlotmADRecHits::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

TH2F * PlotmADRecHits::MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir)
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

void PlotmADRecHits::MakeSubDirs()
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

void PlotmADRecHits::OutputTH1Fs()
{
  fOutFile->cd();

  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    // save relevant plots for making "totals"
    if (mapiter->first.Contains("_add",TString::kExact) && !mapiter->first.Contains("phnrhs",TString::kExact))
    {
      TString hname      = mapiter->first;
      TString replacestr = "_add";
      Ssiz_t  length     = replacestr.Length();
      Ssiz_t  hnamepos   = hname.Index(replacestr.Data());
      TString toreplace  = "";
      hname.Replace(hnamepos,length,toreplace);
     
      fTotalNames.push_back(hname);
    }

    // save to output file
    mapiter->second->Write(mapiter->first.Data(),TObject::kWriteDelete); 
    
    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    mapiter->second->Draw("HIST");
    
    // first save as linear, then log
    canv->SetLogy(0);
    canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
    canv->SaveAs(Form("%s/%s_lin.png",fOutDir.Data(),mapiter->first.Data()));

    canv->SetLogy(1);
    canv->SaveAs(Form("%s/%s/log/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
    canv->SaveAs(Form("%s/%s_log.png",fOutDir.Data(),mapiter->first.Data()));

    delete canv;
  }
}

void PlotmADRecHits::OutputTotalTH1Fs()
{
  for (UInt_t iplot = 0; iplot < fTotalNames.size(); iplot++)
  {
    TString hname = fTotalNames[iplot];

    // make a copy of previous plot
    TH1F * htotal = (TH1F*)fPlots[hname]->Clone(Form("%s_total",hname.Data()));
    htotal->SetTitle(Form("%s [totals]",fPlots[hname]->GetTitle()));
    htotal->GetYaxis()->SetTitle(Form("%s [totals]",fPlots[hname]->GetYaxis()->GetTitle()));

    // add on additional rechit info
    htotal->Add(fPlots[Form("%s_add",hname.Data())]);
    
    //now do standard output routines
    // save to output file
    htotal->Write(htotal->GetName(),TObject::kWriteDelete); 
    
    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    htotal->Draw("HIST");
    
    // first save as linear, then log
    TString subdir = "calibPhotons/ReducedRHs/TotalRHs";

    canv->SetLogy(0);
    canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),subdir.Data(),htotal->GetName()));
    canv->SaveAs(Form("%s/%s_lin.png",fOutDir.Data(),htotal->GetName()));
    
    canv->SetLogy(1);
    canv->SaveAs(Form("%s/%s/log/%s.png",fOutDir.Data(),subdir.Data(),htotal->GetName()));
    canv->SaveAs(Form("%s/%s_log.png",fOutDir.Data(),htotal->GetName()));
    
    delete canv;
    delete htotal;
  }
}

void PlotmADRecHits::ClearTH1Map()
{
  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    delete mapiter->second;
  }
  fPlots.clear();
}

void PlotmADRecHits::OutputTH2Fs()
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
    canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDirs[mapiter->first].Data(),mapiter->first.Data()));
    canv->SaveAs(Form("%s/%s_lin.png",fOutDir.Data(),mapiter->first.Data()));

    delete canv;
  }
}

void PlotmADRecHits::ClearTH2Map()
{
  for (TH2MapIter mapiter = fPlots2D.begin(); mapiter != fPlots2D.end(); ++mapiter) 
  { 
    delete mapiter->second;
  }
  fPlots2D.clear();
}

void PlotmADRecHits::InitTree()
{
  // need to set vector pointers, otherwise root craps out
  phrhEs    = 0;
  phrhphis  = 0;
  phrhetas  = 0;
  phrhdelRs = 0;
  phrhtimes = 0;
  phrhIDs   = 0;
  phrhOOTs  = 0;

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("hltdoubleph60", &hltdoubleph60, &b_hltdoubleph60);
  fInTree->SetBranchAddress("phE", &phE, &b_phE);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
  fInTree->SetBranchAddress("phVID", &phVID, &b_phVID);
  fInTree->SetBranchAddress("phscE", &phscE, &b_phscE);
  fInTree->SetBranchAddress("phscphi", &phscphi, &b_phscphi);
  fInTree->SetBranchAddress("phsceta", &phsceta, &b_phsceta);
  fInTree->SetBranchAddress("phnrhs", &phnrhs, &b_phnrhs);
  fInTree->SetBranchAddress("phnrhs_add", &phnrhs_add, &b_phnrhs_add);
  fInTree->SetBranchAddress("phrhEs", &phrhEs, &b_phrhEs);
  fInTree->SetBranchAddress("phrhphis", &phrhphis, &b_phrhphis);
  fInTree->SetBranchAddress("phrhetas", &phrhetas, &b_phrhetas);
  fInTree->SetBranchAddress("phrhdelRs", &phrhdelRs, &b_phrhdelRs);
  fInTree->SetBranchAddress("phrhtimes", &phrhtimes, &b_phrhtimes);
  fInTree->SetBranchAddress("phrhIDs", &phrhIDs, &b_phrhIDs);
  fInTree->SetBranchAddress("phrhOOTs", &phrhOOTs, &b_phrhOOTs);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
}
