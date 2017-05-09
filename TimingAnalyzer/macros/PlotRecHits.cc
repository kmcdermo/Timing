#include "PlotRecHits.hh"
#include "TStyle.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <fstream>
#include <iostream>

PlotRecHits::PlotRecHits(TString filename, TString outdir, 
			 Bool_t applyrhecut, Float_t rhEcut) :
  fOutDir(outdir),
  fApplyrhECut(applyrhecut), frhECut(rhEcut)
{
  // overall setup
  gStyle->SetOptStat("emrou");

  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  // initialize tree
  PlotRecHits::InitTree();

  // in routine initialization
  fNEvCheck = 1000;

  // extra appendages
  if (fApplyrhECut) fOutDir += "_rhE";
  
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());
  }

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

PlotRecHits::~PlotRecHits()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
}

void PlotRecHits::DoPlots()
{
  PlotRecHits::SetupPlots();
  PlotRecHits::RecHitsLoop();
  //  PlotRecHits::MakeSubDirs();
  PlotRecHits::OutputTH1Fs();
  //  PlotRecHits::OutputTH2Fs();
}

void PlotRecHits::SetupPlots()
{
  PlotRecHits::SetupRecHits();
}

void PlotRecHits::RecHitsLoop()
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;
    
    PlotRecHits::FillRecHits();
  }
}

void PlotRecHits::FillRecHits()
{
  int nRecHits = 0;
  int nRecHitsEB = 0;
  int nRecHitsEE = 0;
  int nRecHitsOOT = 0;
  for (int irh = 0; irh < nrhEB+nrhEE; irh++)
  {
    if (fApplyrhECut && (*rhE)[irh] < frhECut) continue;
    nRecHits++;

    if ((*rhOOT)[irh]) nRecHitsOOT++;

    if ((*rhisEB)[irh]) nRecHitsEB++;
    else                nRecHitsEE++;

    fPlots["rhE"]->Fill((*rhE)[irh]);
    fPlots["rhE_zoom"]->Fill((*rhE)[irh]);
    fPlots["rhtime"]->Fill((*rhtime)[irh]);
    fPlots["rhOOT"]->Fill((*rhOOT)[irh]);

    if ( (*rhOOT)[irh] ) // make plots for only OOT True
    {
      fPlots["rhE_OOTT"]->Fill((*rhE)[irh]);
      fPlots["rhE_OOTT_zoom"]->Fill((*rhE)[irh]);
      fPlots["rhtime_OOTT"]->Fill((*rhtime)[irh]);
      fPlots["rhtime_OOTT_zoom"]->Fill((*rhtime)[irh]);
    }
    else // make plots for only OOT False
    {
      fPlots["rhE_OOTF"]->Fill((*rhE)[irh]);
      fPlots["rhE_OOTF_zoom"]->Fill((*rhE)[irh]);
      fPlots["rhtime_OOTF"]->Fill((*rhtime)[irh]);
      fPlots["rhtime_OOTF_zoom"]->Fill((*rhtime)[irh]);
    }// end block over OOT only plots
  }// end loop over all rechits in event

  fPlots["nrh"]->Fill(nRecHits);
  fPlots["nrhOOT"]->Fill(nRecHitsOOT);
  fPlots["nrhEB"]->Fill(nRecHitsEB);
  fPlots["nrhEE"]->Fill(nRecHitsEE);
}

void PlotRecHits::SetupRecHits()
{
  fPlots["nrh"] = PlotRecHits::MakeTH1F("nrh","nRecHits in Event (reco)",600,0.f,6000.f,"nRecHits","Events","FullRecHits");
  fPlots["nrhOOT"] = PlotRecHits::MakeTH1F("nrhOOT","nRecHits flagged OoT in Event (reco)",500.f,0,500.f,"nRecHits","Events","FullRecHits");
  fPlots["nrhEB"] = PlotRecHits::MakeTH1F("nrhEB","nRecHits EB in Event (reco)",400.f,0,4000.f,"nRecHits","Events","FullRecHits");
  fPlots["nrhEE"] = PlotRecHits::MakeTH1F("nrhEE","nRecHits EE in Event (reco)",400.f,0,4000.f,"nRecHits","Events","FullRecHits");

  fPlots["rhE"] = PlotRecHits::MakeTH1F("rhE"," RecHit Energy [GeV] (reco)",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhE_zoom"] = PlotRecHits::MakeTH1F("rhE_zoom"," RecHit Energy [GeV] (reco)",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhtime"] = PlotRecHits::MakeTH1F("rhtime"," RecHit Time [ns] (reco)",300,-150.f,150.f,"Time [ns]","RecHits","FullRecHits");
  fPlots["rhOOT"] = PlotRecHits::MakeTH1F("rhOOT"," RecHit OoT Flag (reco)",2,0.f,2.f,"OoT Flag","RecHits","FullRecHits");

  // OOT T+F plots
  fPlots["rhE_OOTT"] = PlotRecHits::MakeTH1F("rhE_OOTT"," RecHit Energy [GeV] (reco) - OoT:T",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhE_OOTT_zoom"] = PlotRecHits::MakeTH1F("rhE_OOTT_zoom"," RecHit Energy [GeV] (reco) - OoT:T",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhtime_OOTT"] = PlotRecHits::MakeTH1F("rhtime_OOTT"," RecHit Time [ns] (reco) - OoT:T",300,-150.f,150.f,"Time [ns]","RecHits","FullRecHits");
  fPlots["rhtime_OOTT_zoom"] = PlotRecHits::MakeTH1F("rhtime_OOTT_zoom"," RecHit Time [ns] (reco) - OoT:T",200,-10.f,10.f,"Time [ns]","RecHits","FullRecHits");

  fPlots["rhE_OOTF"] = PlotRecHits::MakeTH1F("rhE_OOTF"," RecHit Energy [GeV] (reco) - OoT:F",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhE_OOTF_zoom"] = PlotRecHits::MakeTH1F("rhE_OOTF_zoom"," RecHit Energy [GeV] (reco) - OoT:F",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhtime_OOTF"] = PlotRecHits::MakeTH1F("rhtime_OOTF"," RecHit Time [ns] (reco) - OoT:F",300,-150.f,150.f,"Time [ns]","RecHits","FullRecHits");
  fPlots["rhtime_OOTF_zoom"] = PlotRecHits::MakeTH1F("rhtime_OOTF_zoom"," RecHit Time [ns] (reco) - OoT:F",200,-10.f,10.f,"Time [ns]","RecHits","FullRecHits");
}

TH1F * PlotRecHits::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

TH2F * PlotRecHits::MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir)
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  fSubDirs[hname] = subdir;

  return hist;
}

void PlotRecHits::MakeSubDirs()
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

void PlotRecHits::OutputTH1Fs()
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

void PlotRecHits::OutputTH2Fs()
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

void PlotRecHits::InitTree()
{
  // need to set vector pointers, otherwise root craps out
  rhE    = 0;
  rhphi  = 0;
  rheta  = 0;
  rhtime = 0;
  rhID   = 0;
  rhOOT  = 0;
  rhisEB = 0;

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("nrhEB", &nrhEB, &b_nrhEB);
  fInTree->SetBranchAddress("nrhEE", &nrhEE, &b_nrhEE);
  fInTree->SetBranchAddress("rhE", &rhE, &b_rhE);
  fInTree->SetBranchAddress("rhphi", &rhphi, &b_rhphi);
  fInTree->SetBranchAddress("rheta", &rheta, &b_rheta);
  fInTree->SetBranchAddress("rhtime", &rhtime, &b_rhtime);
  fInTree->SetBranchAddress("rhID", &rhID, &b_rhID);
  fInTree->SetBranchAddress("rhOOT", &rhOOT, &b_rhOOT);
  fInTree->SetBranchAddress("rhisEB", &rhisEB, &b_rhisEB);
}
