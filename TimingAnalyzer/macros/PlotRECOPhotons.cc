#include "PlotRECOPhotons.hh"
#include "TStyle.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotRECOPhotons::PlotRECOPhotons(TString filename, TString outdir, 
				 Bool_t applyptcut, Float_t ptcut) :
  fOutDir(outdir), fApplyPtCut(applyptcut), fPtCut(ptcut)
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
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    if ((fApplyPtCut && (*phpt)[iph] < fPtCut) && ((*phrhOOT)[iph][(*phseedpos)[iph]])) continue;
    nPhotons++;

    fPlots["phE"]->Fill((*phE)[iph]);
    fPlots["phpt"]->Fill((*phpt)[iph]);
    fPlots["phphi"]->Fill((*phphi)[iph]);
    fPlots["pheta"]->Fill((*pheta)[iph]);
    
    fPlots["phseedtime"]->Fill((*phrhtime)[iph][(*phseedpos)[iph]]);
    fPlots["phseedOOT"]->Fill((*phrhOOT)[iph][(*phseedpos)[iph]]);

    Int_t nRH = 0;
    Int_t nRHOOT = 0;
    for (Int_t irh = 0; irh < (*phnrh)[iph]; irh++)
    {
      nRH++;
      if ((*phrhOOT)[iph][irh]) nRHOOT++;
    }
    fPlots["phnrh"]->Fill(nRH);
    fPlots["phnrhOOT"]->Fill(nRHOOT);
  }

  fPlots["nphotons"]->Fill(nPhotons);
}

void PlotRECOPhotons::SetupRecoPhotons()
{
  fPlots["nphotons"] = PlotRECOPhotons::MakeTH1F("nphotons","nPhotons per Event",20,0.f,20.f,"nPhotons","Events","gedPhotons/GeneralProps");

  fPlots["phE"] = PlotRECOPhotons::MakeTH1F("phE","Photon Energy [GeV]",100,0.f,2000.f,"Energy [GeV]","Photons","gedPhotons/GeneralProps");
  fPlots["phpt"] = PlotRECOPhotons::MakeTH1F("phpt","Photon p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Photons","gedPhotons/GeneralProps");
  fPlots["phphi"] = PlotRECOPhotons::MakeTH1F("phphi","Photon #phi",100,-3.2,3.2,"#phi","Photons","gedPhotons/GeneralProps");
  fPlots["pheta"] = PlotRECOPhotons::MakeTH1F("pheta","Photon #eta",100,-3.0,3.0,"#eta","Photons","gedPhotons/GeneralProps");

  fPlots["phnrh"] = PlotRECOPhotons::MakeTH1F("phnrh","nRecHits per Photon",150,0.f,150.f,"nRecHits","Photons","gedPhotons/GeneralProps");
  fPlots["phnrhOOT"] = PlotRECOPhotons::MakeTH1F("phnrhOOT","nRecHits OOT per Photon",20,0.f,20.f,"nRecHits OOT","Photons","gedPhotons/GeneralProps");
  fPlots["phseedtime"] = PlotRECOPhotons::MakeTH1F("phseedtime","Photon Seed Time [ns]",100,-20.f,20.f,"Seed Time [ns]","Photons","gedPhotons/GeneralProps");
  fPlots["phseedOOT"] = PlotRECOPhotons::MakeTH1F("phseedOOT","Photon Seed OOT Flag",2,0.f,2.f,"Seed Out-of-Time Flag","Photons","gedPhotons/GeneralProps");
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
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrheta", &phrheta, &b_phrheta);
  fInTree->SetBranchAddress("phrhphi", &phrhphi, &b_phrhphi);
  fInTree->SetBranchAddress("phrhE", &phrhE, &b_phrhE);
  fInTree->SetBranchAddress("phrhtime", &phrhtime, &b_phrhtime);
  fInTree->SetBranchAddress("phrhID", &phrhID, &b_phrhID);
  fInTree->SetBranchAddress("phrhOOT", &phrhOOT, &b_phrhOOT);
}
