#include "PlotRECOPhotons.hh"
#include "TStyle.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotRECOPhotons::PlotRECOPhotons(TString filename, TString outdir, Bool_t applyrhEcut, Float_t rhEcut,
				 Bool_t applyptcut, Float_t ptcut, Bool_t applyhoecut, Float_t hoecut, Bool_t applyr9cut, Float_t r9cut,
				 Bool_t applysieiecut, Float_t sieieEBcut, Float_t sieieEEcut) :
  fOutDir(outdir), fApplyrhECut(applyrhEcut), frhECut(rhEcut),
  fApplyPtCut(applyptcut), fPtCut(ptcut), fApplyHoECut(applyhoecut), fHoECut(hoecut), fApplyR9Cut(applyr9cut), fR9Cut(r9cut),
  fApplySieieCut(applysieiecut), fSieieEBCut(sieieEBcut), fSieieEECut(sieieEEcut)
{
  // overall setup
  gStyle->SetOptStat("emrou");

  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  // initialize tree
  PlotRECOPhotons::InitTree();

  // in routine initialization
  fNEvCheck = 1000;
  
  // extra appendages
  if (fApplyrhECut) fOutDir += "_rhE";

  // setup outdir name
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());
  }

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

PlotRECOPhotons::~PlotRECOPhotons()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
}

void PlotRECOPhotons::DoPlots()
{
  PlotRECOPhotons::SetupPlots();
  PlotRECOPhotons::EventLoop();
  //  PlotRECOPhotons::MakeSubDirs();
  PlotRECOPhotons::OutputTH1Fs();
}

void PlotRECOPhotons::SetupPlots()
{
  PlotRECOPhotons::SetupRecoPhotons();
}

void PlotRECOPhotons::EventLoop()
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    PlotRECOPhotons::FillRecoPhotons();
  }
}

void PlotRECOPhotons::FillRecoPhotons()
{
  Int_t nPhotons = 0;
  Int_t nPhotonsOOT = 0;
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    if (fApplyrhECut && (*phrhE)[iph][(*phseedpos)[iph]] < frhECut) continue;

    if ((*phrhOOT)[iph][(*phseedpos)[iph]])
    {
      if ((fApplyPtCut && (*phpt)[iph] < fPtCut)) continue;
      if ((fApplyHoECut && (*phHoE)[iph] > fHoECut)) continue;
      if ((fApplyR9Cut && (*phr9)[iph] < fR9Cut)) continue;
      if (fApplySieieCut) 
      {
	if (std::abs((*phsceta)[iph]) < 1.5 && (*phsieie)[iph] > fSieieEBCut) continue;
	if (std::abs((*phsceta)[iph]) > 1.5 && (*phsieie)[iph] > fSieieEECut) continue;
      }

      fPlots["phE_OOT"]->Fill((*phE)[iph]);
      fPlots["phpt_OOT"]->Fill((*phpt)[iph]);
      fPlots["phphi_OOT"]->Fill((*phphi)[iph]);
      fPlots["pheta_OOT"]->Fill((*pheta)[iph]);
      fPlots["phseedtime_OOT"]->Fill((*phrhtime)[iph][(*phseedpos)[iph]]);
      fPlots["phseedE_OOT"]->Fill((*phrhE)[iph][(*phseedpos)[iph]]);

      fPlots["phHoE_OOT"]->Fill((*phHoE)[iph]);
      fPlots["phr9_OOT"]->Fill((*phr9)[iph]);
      if      (std::abs((*phsceta)[iph]) < ECAL::etaEB) fPlots["phsieieEB_OOT"]->Fill((*phsieie)[iph]);
      else if (std::abs((*phsceta)[iph]) > ECAL::etaEEmin && std::abs((*phsceta)[iph]) < ECAL::etaEEmax) fPlots["phsieieEE_OOT"]->Fill((*phsieie)[iph]);
      nPhotonsOOT++;
    }
    nPhotons++;

    fPlots["phE"]->Fill((*phE)[iph]);
    fPlots["phpt"]->Fill((*phpt)[iph]);
    fPlots["phphi"]->Fill((*phphi)[iph]);
    fPlots["pheta"]->Fill((*pheta)[iph]);
    
    if      (std::abs((*phsceta)[iph]) < ECAL::etaEB) 
    {
      fPlots["phsieieEB"]->Fill((*phsieie)[iph]);
      fPlots["phsieipEB"]->Fill((*phsieip)[iph]);
      fPlots["phsipipEB"]->Fill((*phsipip)[iph]);

      fPlots["phsmajEB"]->Fill((*phsmaj)[iph]);
      fPlots["phsminEB"]->Fill((*phsmin)[iph]);

      fPlots["phEcalIsoEB"]->Fill((*phEcalIso)[iph]);
      fPlots["phHcalIsoEB"]->Fill((*phHcalIso)[iph]);
    }
    else if (std::abs((*phsceta)[iph]) > ECAL::etaEEmin && std::abs((*phsceta)[iph]) < ECAL::etaEEmax) 
    {
      fPlots["phsieieEE"]->Fill((*phsieie)[iph]);
      fPlots["phsieipEE"]->Fill((*phsieip)[iph]);
      fPlots["phsipipEE"]->Fill((*phsipip)[iph]);

      fPlots["phsmajEE"]->Fill((*phsmaj)[iph]);
      fPlots["phsminEE"]->Fill((*phsmin)[iph]);

      fPlots["phEcalIsoEE"]->Fill((*phEcalIso)[iph]);
      fPlots["phHcalIsoEE"]->Fill((*phHcalIso)[iph]);
    }
      
    fPlots["phseedtime"]->Fill((*phrhtime)[iph][(*phseedpos)[iph]]);
    fPlots["phseedOOT"]->Fill((*phrhOOT)[iph][(*phseedpos)[iph]]);
    fPlots["phseedE"]->Fill((*phrhE)[iph][(*phseedpos)[iph]]);

    Int_t nRH = 0;
    Int_t nRHOOT = 0;
    for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
    {
      if (fApplyrhECut && (*phrhE)[iph][irh] < frhECut) continue;
      
      nRH++;
      if ((*phrhOOT)[iph][irh]) nRHOOT++;
      fPlots["phrhtime"]->Fill((*phrhtime)[iph][irh]);
      fPlots["phrhE"]->Fill((*phrhE)[iph][irh]);
    }
    fPlots["phnrh"]->Fill(nRH);
    fPlots["phnrhOOT"]->Fill(nRHOOT);
  }

  fPlots["nphotons"]->Fill(nPhotons);
  fPlots["nphotonsOOT"]->Fill(nPhotonsOOT);
}

void PlotRECOPhotons::SetupRecoPhotons()
{
  fPlots["nphotons"] = PlotRECOPhotons::MakeTH1F("nphotons","nPhotons per Event",20,0.f,20.f,"nPhotons","Events","gedPhotons/GeneralProps");
  fPlots["nphotonsOOT"] = PlotRECOPhotons::MakeTH1F("nphotonsOOT","nPhotons OoT per Event",20,0.f,20.f,"nPhotons OoT","Events","gedPhotons/GeneralProps");
  fPlots["phnrh"] = PlotRECOPhotons::MakeTH1F("phnrh","nRecHits per Photon",100,0.f,250.f,"nRecHits","Photons","gedPhotons/GeneralProps");
  fPlots["phnrhOOT"] = PlotRECOPhotons::MakeTH1F("phnrhOOT","nRecHits OoT per Photon",20,0.f,20.f,"nRecHits OoT","Photons","gedPhotons/GeneralProps");

  fPlots["phE"] = PlotRECOPhotons::MakeTH1F("phE","Photon Energy [GeV]",100,0.f,2000.f,"Energy [GeV]","Photons","gedPhotons/GeneralProps");
  fPlots["phpt"] = PlotRECOPhotons::MakeTH1F("phpt","Photon p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Photons","gedPhotons/GeneralProps");
  fPlots["phphi"] = PlotRECOPhotons::MakeTH1F("phphi","Photon #phi",32,-3.2,3.2,"#phi","Photons","gedPhotons/GeneralProps");
  fPlots["pheta"] = PlotRECOPhotons::MakeTH1F("pheta","Photon #eta",100,-5.0,5.0,"#eta","Photons","gedPhotons/GeneralProps");

  // OOT == True Only
  fPlots["phE_OOT"] = PlotRECOPhotons::MakeTH1F("phE_OOT","Photon OoT Energy [GeV]",100,0.f,200.f,"Energy [GeV]","Photons","gedPhotons/GeneralProps");
  fPlots["phpt_OOT"] = PlotRECOPhotons::MakeTH1F("phpt_OOT","Photon OoT p_{T} [GeV/c]",100,0.f,200.f,"p_{T} [GeV/c]","Photons","gedPhotons/GeneralProps");
  fPlots["phphi_OOT"] = PlotRECOPhotons::MakeTH1F("phphi_OOT","Photon OoT #phi",32,-3.2,3.2,"#phi","Photons","gedPhotons/GeneralProps");
  fPlots["pheta_OOT"] = PlotRECOPhotons::MakeTH1F("pheta_OOT","Photon OoT #eta",100,-5.0,5.0,"#eta","Photons","gedPhotons/GeneralProps");

  fPlots["phHoE_OOT"] = PlotRECOPhotons::MakeTH1F("phHoE_OOT","Photon OoT HoverE",100,0.f,2.f,"HoverE","Photons","gedPhotons/GeneralProps");
  fPlots["phr9_OOT"] = PlotRECOPhotons::MakeTH1F("phr9_OOT","Photon OoT R9",100,0.f,2.f,"R9","Photons","gedPhotons/GeneralProps");
  fPlots["phsieieEB_OOT"] = PlotRECOPhotons::MakeTH1F("phsieieEB_OOT","Photon OoT #sigma_{i#eta i#eta} EB",100,0.f,0.1f,"#sigma_{i#eta i#eta} EB","Photons","gedPhotons/GeneralProps");
  fPlots["phsieieEE_OOT"] = PlotRECOPhotons::MakeTH1F("phsieieEE_OOT","Photon OoT #sigma_{i#eta i#eta} EE",100,0.f,0.1f,"#sigma_{i#eta i#eta} EE","Photons","gedPhotons/GeneralProps");

  // EB only
  fPlots["phsieieEB"] = PlotRECOPhotons::MakeTH1F("phsieieEB","Photon #sigma_{i#eta i#eta} EB",100,0.f,0.1f,"#sigma_{i#eta i#eta} EB","Photons","gedPhotons/GeneralProps");
  fPlots["phsieipEB"] = PlotRECOPhotons::MakeTH1F("phsieipEB","Photon #sigma_{i#eta i#phi} EB",100,-0.005f,0.005f,"#sigma_{i#eta i#phi} EB","Photons","gedPhotons/GeneralProps");
  fPlots["phsipipEB"] = PlotRECOPhotons::MakeTH1F("phsipipEB","Photon #sigma_{i#phi i#phi} EB",100,0.f,0.1f,"#sigma_{i#phi i#phi} EB","Photons","gedPhotons/GeneralProps");

  fPlots["phsmajEB"] = PlotRECOPhotons::MakeTH1F("phsmajEB","Photon S_{Major} EB",100,0.f,3.f,"S_{Major} EB","Photons","gedPhotons/GeneralProps");
  fPlots["phsminEB"] = PlotRECOPhotons::MakeTH1F("phsminEB","Photon S_{Minor} EB",100,0.f,3.f,"S_{Minor} EB","Photons","gedPhotons/GeneralProps");

  fPlots["phEcalIsoEB"] = PlotRECOPhotons::MakeTH1F("phEcalIsoEB","Photon Ecal PFCluster Iso EB",100,0.f,1000.f,"Ecal PFCluster Iso EB","Photons","gedPhotons/GeneralProps");
  fPlots["phHcalIsoEB"] = PlotRECOPhotons::MakeTH1F("phHcalIsoEB","Photon Hcal PFCluster Iso EB",100,0.f,1000.f,"Hcal PFCluster Iso EB","Photons","gedPhotons/GeneralProps");

  // EE only
  fPlots["phsieieEE"] = PlotRECOPhotons::MakeTH1F("phsieieEE","Photon #sigma_{i#eta i#eta} EE",100,0.f,0.1f,"#sigma_{i#eta i#eta} EE","Photons","gedPhotons/GeneralProps");
  fPlots["phsieipEE"] = PlotRECOPhotons::MakeTH1F("phsieipEE","Photon #sigma_{i#eta i#phi} EE",100,-0.005f,0.005f,"#sigma_{i#eta i#phi} EE","Photons","gedPhotons/GeneralProps");
  fPlots["phsipipEE"] = PlotRECOPhotons::MakeTH1F("phsipipEE","Photon #sigma_{i#phi i#phi} EE",100,0.f,0.1f,"#sigma_{i#phi i#phi} EE","Photons","gedPhotons/GeneralProps");

  fPlots["phsmajEE"] = PlotRECOPhotons::MakeTH1F("phsmajEE","Photon S_{Major} EE",100,0.f,3.f,"S_{Major} EE","Photons","gedPhotons/GeneralProps");
  fPlots["phsminEE"] = PlotRECOPhotons::MakeTH1F("phsminEE","Photon S_{Minor} EE",100,0.f,3.f,"S_{Minor} EE","Photons","gedPhotons/GeneralProps");

  fPlots["phEcalIsoEE"] = PlotRECOPhotons::MakeTH1F("phEcalIsoEE","Photon Ecal PFCluster Iso EE",100,0.f,1000.f,"Ecal PFCluster Iso EE","Photons","gedPhotons/GeneralProps");
  fPlots["phHcalIsoEE"] = PlotRECOPhotons::MakeTH1F("phHcalIsoEE","Photon Hcal PFCluster Iso EE",100,0.f,1000.f,"Hcal PFCluster Iso EE","Photons","gedPhotons/GeneralProps");

  // seed stuff
  fPlots["phseedtime"] = PlotRECOPhotons::MakeTH1F("phseedtime","Photon Seed Time [ns]",100,-25.f,25.f,"Seed Time [ns]","Photons","gedPhotons/GeneralProps");
  fPlots["phseedOOT"] = PlotRECOPhotons::MakeTH1F("phseedOOT","Photon Seed OoT Flag",2,0.f,2.f,"Seed Out-of-Time Flag","Photons","gedPhotons/GeneralProps");
  fPlots["phseedE"] = PlotRECOPhotons::MakeTH1F("phseedE","Photon Seed Energy [GeV]",100.f,0.f,1000.f,"Energy [GeV]","Photons","gedPhotons/GeneralProps");

  fPlots["phrhtime"] = PlotRECOPhotons::MakeTH1F("phrhtime","Photon RecHit Time [ns]",100,-200.f,200.f,"RecHit Time [ns]","RecHits","gedPhotons/GeneralProps");
  fPlots["phrhE"] = PlotRECOPhotons::MakeTH1F("phrhE","Photon RecHit Energy [GeV]",100.f,0.f,1000.f,"Energy [GeV]","RecHits","gedPhotons/GeneralProps");

  // OOT == True only for rechits
  fPlots["phseedtime_OOT"] = PlotRECOPhotons::MakeTH1F("phseedtime_OOT","Photon OoT Seed Time [ns]",25,-25.f,25.f,"Seed Time [ns]","Photons","gedPhotons/GeneralProps");
  fPlots["phseedE_OOT"] = PlotRECOPhotons::MakeTH1F("phseedE_OOT","Photon OoT Seed Energy [GeV]",100.f,0.f,100.f,"Energy [GeV]","Photons","gedPhotons/GeneralProps");
}

TH1F * PlotRECOPhotons::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

TH2F * PlotRECOPhotons::MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir)
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

void PlotRECOPhotons::MakeSubDirs()
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

void PlotRECOPhotons::OutputTH1Fs()
{
  fOutFile->cd();

  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->first.Data(),TObject::kWriteDelete); 
    delete mapiter->second;
  }
 
  fPlots.clear();
}

void PlotRECOPhotons::OutputTH2Fs()
{
  fOutFile->cd();

  for (TH2MapIter mapiter = fPlots2D.begin(); mapiter != fPlots2D.end(); ++mapiter) 
  { 
    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    delete mapiter->second;
  }

  fPlots2D.clear();
}

void PlotRECOPhotons::InitTree()
{
  // need to set vector pointers, otherwise root craps out
  phE = 0;
  phpt = 0;
  phphi = 0;
  pheta = 0;
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
  phEcalIso = 0;
  phHcalIso = 0;
  phnrh = 0;
  phseedpos = 0;
  phrheta = 0;
  phrhphi = 0;
  phrhE = 0;
  phrhtime = 0;
  phrhID = 0;
  phrhOOT = 0;

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fInTree->SetBranchAddress("phE", &phE, &b_phE);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
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
  fInTree->SetBranchAddress("phEcalIso", &phEcalIso, &b_phEcalIso);
  fInTree->SetBranchAddress("phHcalIso", &phHcalIso, &b_phHcalIso);
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrheta", &phrheta, &b_phrheta);
  fInTree->SetBranchAddress("phrhphi", &phrhphi, &b_phrhphi);
  fInTree->SetBranchAddress("phrhE", &phrhE, &b_phrhE);
  fInTree->SetBranchAddress("phrhtime", &phrhtime, &b_phrhtime);
  fInTree->SetBranchAddress("phrhID", &phrhID, &b_phrhID);
  fInTree->SetBranchAddress("phrhOOT", &phrhOOT, &b_phrhOOT);
}
