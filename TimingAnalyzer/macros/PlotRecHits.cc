#include "PlotRecHits.hh"
#include "TStyle.h"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotRecHits::PlotRecHits(TString filename, TString outdir, 
			 Bool_t applyrhecut, Float_t rhEcut, Bool_t applyecalacceptcut) :
  fOutDir(outdir),
  fApplyrhECut(applyrhecut), frhECut(rhEcut),
  fApplyECALAcceptCut(applyecalacceptcut)
{
  // overall setup
  gStyle->SetOptStat("emrou");

  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/rhtree");

  // initialize tree
  PlotRecHits::InitTree();

  // in routine initialization
  fNEvCheck = 1000;
  
  // output
  // setup outdir name
  if (!fApplyrhECut && ! fApplyECALAcceptCut)
  { 
    fOutDir += "/Inclusive";
  }
  else 
  {
    fOutDir += "/cuts";
    if (fApplyrhECut)        fOutDir += Form("_rhE%2.1f",frhECut);
    if (fApplyECALAcceptCut) fOutDir += Form("_ecalaccept");
  }

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
  PlotRecHits::MakeSubDirs();
  PlotRecHits::OutputTH1Fs();
  PlotRecHits::OutputTH2Fs();
  PlotRecHits::ClearTH1Map();
  PlotRecHits::ClearTH2Map();
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
  for (int irh = 0; irh < nrhEB+nrhEE; irh++)
  {
    if (fApplyECALAcceptCut && (std::abs((*rhetas)[irh]) > 2.5 || (std::abs((*rhetas)[irh]) > 1.4442 && std::abs((*rhetas)[irh]) < 1.566))) continue;
    if (fApplyrhECut && (*rhEs)[irh] < frhECut) continue;
    nRecHits++;

    fPlots["rhEs"]->Fill((*rhEs)[irh]);
    fPlots["rhEs_zoom"]->Fill((*rhEs)[irh]);
    fPlots["rhtimes"]->Fill((*rhtimes)[irh]);
    fPlots["rhOOTs"]->Fill((*rhOOTs)[irh]);
    if ( std::abs((*rhtimes)[irh]) > 5.0 ) 
    {
      fPlots["rhOOTs_absTgt5"]->Fill((*rhOOTs)[irh]);
    }

    if ( (*rhOOTs)[irh] ) // make plots for only OOT True
    {
      fPlots["rhEs_OOTT"]->Fill((*rhEs)[irh]);
      fPlots["rhEs_OOTT_zoom"]->Fill((*rhEs)[irh]);
      fPlots["rhEs_OOTT_zoomer"]->Fill((*rhEs)[irh]);
      fPlots["rhtimes_OOTT"]->Fill((*rhtimes)[irh]);
      fPlots["rhtimes_OOTT_zoom"]->Fill((*rhtimes)[irh]);
    }
    else // make plots for only OOT False
    {
      fPlots["rhEs_OOTF"]->Fill((*rhEs)[irh]);
      fPlots["rhEs_OOTF_zoom"]->Fill((*rhEs)[irh]);
      fPlots["rhEs_OOTF_zoomer"]->Fill((*rhEs)[irh]);
      if ( std::abs((*rhtimes)[irh]) > 5.0 ) 
      {  
	fPlots["rhEs_OOTF_absTgt5"]->Fill((*rhEs)[irh]);
	fPlots["rhEs_OOTF_absTgt5_zoom"]->Fill((*rhEs)[irh]);
	fPlots["rhEs_OOTF_absTgt5_zoomer"]->Fill((*rhEs)[irh]);
      }
      fPlots["rhtimes_OOTF"]->Fill((*rhtimes)[irh]);
    }// end block over OOT only plots
  }// end loop over all rechits in event
  fPlots["nrhs"]->Fill(nRecHits);
}

void PlotRecHits::SetupRecHits()
{
  fPlots["nrhs"] = PlotRecHits::MakeTH1F("nrhs","nRecHits in Event (reco)",600,0.f,6000.f,"nRecHits","Events","FullRecHits");
  fPlots["rhEs"] = PlotRecHits::MakeTH1F("rhEs"," RecHits Energy [GeV] (reco)",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_zoom"] = PlotRecHits::MakeTH1F("rhEs_zoom"," RecHits Energy [GeV] (reco)",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhtimes"] = PlotRecHits::MakeTH1F("rhtimes"," RecHits Time [ns] (reco)",300,-150.f,150.f,"Time [ns]","RecHits","FullRecHits");
  fPlots["rhOOTs"] = PlotRecHits::MakeTH1F("rhOOTs"," RecHits OoT Flag (reco)",2,0.f,2.f,"OoT Flag","RecHits","FullRecHits");
  fPlots["rhOOTs_absTgt5"] = PlotRecHits::MakeTH1F("rhOOTs_absTgt5"," RecHits OoT Flag (reco) - |T|>5ns",2,0.f,2.f,"OoT Flag","RecHits","FullRecHits");
  // OOT T+F plots
  fPlots["rhEs_OOTT"] = PlotRecHits::MakeTH1F("rhEs_OOTT"," RecHits Energy [GeV] (reco) - OoT:T",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTT_zoom"] = PlotRecHits::MakeTH1F("rhEs_OOTT_zoom"," RecHits Energy [GeV] (reco) - OoT:T",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTT_zoomer"] = PlotRecHits::MakeTH1F("rhEs_OOTT_zoomer"," RecHits Energy [GeV] (reco) - OoT:T",100,0.f,5.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhtimes_OOTT"] = PlotRecHits::MakeTH1F("rhtimes_OOTT"," RecHits Time [ns] (reco) - OoT:T",300,-150.f,150.f,"Time [ns]","RecHits","FullRecHits");
  fPlots["rhtimes_OOTT_zoom"] = PlotRecHits::MakeTH1F("rhtimes_OOTT_zoom"," RecHits Time [ns] (reco) - OoT:T",200,-10.f,10.f,"Time [ns]","RecHits","FullRecHits");
  fPlots["rhEs_OOTF"] = PlotRecHits::MakeTH1F("rhEs_OOTF"," RecHits Energy [GeV] (reco) - OoT:F",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTF_zoom"] = PlotRecHits::MakeTH1F("rhEs_OOTF_zoom"," RecHits Energy [GeV] (reco) - OoT:F",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTF_zoomer"] = PlotRecHits::MakeTH1F("rhEs_OOTF_zoomer"," RecHits Energy [GeV] (reco) - OoT:F",100,0.f,5.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTF_absTgt5"] = PlotRecHits::MakeTH1F("rhEs_OOTF_absTgt5"," RecHits Energy [GeV] (reco) - OoT:F and |T|>5ns",100,0.f,1000.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTF_absTgt5_zoom"] = PlotRecHits::MakeTH1F("rhEs_OOTF_absTgt5_zoom"," RecHits Energy [GeV] (reco) - OoT:F and |T|>5ns",100,0.f,100.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhEs_OOTF_absTgt5_zoomer"] = PlotRecHits::MakeTH1F("rhEs_OOTF_absTgt5_zoomer"," RecHits Energy [GeV] (reco) - OoT:F and |T|>5ns",100,0.f,5.f,"Energy [GeV]","RecHits","FullRecHits");
  fPlots["rhtimes_OOTF"] = PlotRecHits::MakeTH1F("rhtimes_OOTF"," RecHits Time [ns] (reco) - OoT:F",300,-150.f,150.f,"Time [ns]","RecHits","FullRecHits");
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

void PlotRecHits::ClearTH1Map()
{
  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
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

void PlotRecHits::ClearTH2Map()
{
  for (TH2MapIter mapiter = fPlots2D.begin(); mapiter != fPlots2D.end(); ++mapiter) 
  { 
    delete mapiter->second;
  }
  fPlots2D.clear();
}

void PlotRecHits::InitTree()
{
  // need to set vector pointers, otherwise root craps out
  rhEs    = 0;
  rhphis  = 0;
  rhetas  = 0;
  rhtimes = 0;
  rhIDs   = 0;
  rhOOTs  = 0;

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("nrhEB", &nrhEB, &b_nrhEB);
  fInTree->SetBranchAddress("nrhEE", &nrhEE, &b_nrhEE);
  fInTree->SetBranchAddress("rhEs", &rhEs, &b_rhEs);
  fInTree->SetBranchAddress("rhphis", &rhphis, &b_rhphis);
  fInTree->SetBranchAddress("rhetas", &rhetas, &b_rhetas);
  fInTree->SetBranchAddress("rhtimes", &rhtimes, &b_rhtimes);
  fInTree->SetBranchAddress("rhIDs", &rhIDs, &b_rhIDs);
  fInTree->SetBranchAddress("rhOOTs", &rhOOTs, &b_rhOOTs);
}
