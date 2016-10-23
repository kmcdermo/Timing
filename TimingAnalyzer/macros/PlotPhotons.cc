#include "PlotPhotons.hh"
#include "TSystem.h"
#include "TCanvas.h"

#include <iostream>

PlotPhotons::PlotPhotons(TString filename, TString outdir) : fOutDir(outdir)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  // initialize tree
  PlotPhotons::InitTree();

  // in routine initialization
  fNEvCheck = 500;

  // output
  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());
    gSystem->Exec(Form("%s/lin",mkDir.Data()));
    gSystem->Exec(Form("%s/log",mkDir.Data()));
  }

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

PlotPhotons::~PlotPhotons()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
}

void PlotPhotons::DoPlots()
{
  PlotPhotons::SetupTH1Fs();
  PlotPhotons::EventLoop();
  PlotPhotons::OutputTH1Fs();
}

void PlotPhotons::EventLoop()
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry: " << entry << std::endl;

    fPlots["genph1E"]->Fill(genph1E);
  }
}

void PlotPhotons::SetupTH1Fs()
{
  fPlots["genph1E"] = PlotPhotons::MakeTH1F("genph1E","Generator Leading Photon E [GeV]",100,0.f,1000.f,"Energy [GeV]","Events");
}

TH1F * PlotPhotons::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  return hist;
}

void PlotPhotons::OutputTH1Fs()
{
  fOutFile->cd();

  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    // save to output file
    (*mapiter).second->Write(); 

    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    (*mapiter).second->Draw("HIST");
    
    // first save as linear, then log
    canv->SetLogy(0);
    canv->SaveAs(Form("%s/lin/%s.png",fOutDir.Data(),(*mapiter).first.Data()));

    canv->SetLogy(1);
    canv->SaveAs(Form("%s/log/%s.png",fOutDir.Data(),(*mapiter).first.Data()));

    delete canv;
    delete (*mapiter).second;
  }
  fPlots.clear();
}

void PlotPhotons::InitTree()
{
  // Set object pointer
  jetE = 0;
  jetpt = 0;
  jetphi = 0;
  jeteta = 0;
  phmatch = 0;
  phVID = 0;
  phE = 0;
  phpt = 0;
  phphi = 0;
  pheta = 0;
  phscX = 0;
  phscY = 0;
  phscZ = 0;
  phscE = 0;
  phnrhs = 0;
  phseedpos = 0;
  phrhXs = 0;
  phrhYs = 0;
  phrhZs = 0;
  phrhEs = 0;
  phrhtimes = 0;
  phrhIDs = 0;
  phrhOOTs = 0;

  // set branch addresses
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("genwgt", &genwgt, &b_genwgt);
  fInTree->SetBranchAddress("genpuobs", &genpuobs, &b_genpuobs);
  fInTree->SetBranchAddress("genputrue", &genputrue, &b_genputrue);
  fInTree->SetBranchAddress("genN1mass", &genN1mass, &b_genN1mass);
  fInTree->SetBranchAddress("genN1E", &genN1E, &b_genN1E);
  fInTree->SetBranchAddress("genN1pt", &genN1pt, &b_genN1pt);
  fInTree->SetBranchAddress("genN1phi", &genN1phi, &b_genN1phi);
  fInTree->SetBranchAddress("genN1eta", &genN1eta, &b_genN1eta);
  fInTree->SetBranchAddress("genph1E", &genph1E, &b_genph1E);
  fInTree->SetBranchAddress("genph1pt", &genph1pt, &b_genph1pt);
  fInTree->SetBranchAddress("genph1phi", &genph1phi, &b_genph1phi);
  fInTree->SetBranchAddress("genph1eta", &genph1eta, &b_genph1eta);
  fInTree->SetBranchAddress("genph1match", &genph1match, &b_genph1match);
  fInTree->SetBranchAddress("gengl1E", &gengl1E, &b_gengl1E);
  fInTree->SetBranchAddress("gengl1pt", &gengl1pt, &b_gengl1pt);
  fInTree->SetBranchAddress("gengl1phi", &gengl1phi, &b_gengl1phi);
  fInTree->SetBranchAddress("gengl1eta", &gengl1eta, &b_gengl1eta);
  fInTree->SetBranchAddress("genN2mass", &genN2mass, &b_genN2mass);
  fInTree->SetBranchAddress("genN2E", &genN2E, &b_genN2E);
  fInTree->SetBranchAddress("genN2pt", &genN2pt, &b_genN2pt);
  fInTree->SetBranchAddress("genN2phi", &genN2phi, &b_genN2phi);
  fInTree->SetBranchAddress("genN2eta", &genN2eta, &b_genN2eta);
  fInTree->SetBranchAddress("genph2E", &genph2E, &b_genph2E);
  fInTree->SetBranchAddress("genph2pt", &genph2pt, &b_genph2pt);
  fInTree->SetBranchAddress("genph2phi", &genph2phi, &b_genph2phi);
  fInTree->SetBranchAddress("genph2eta", &genph2eta, &b_genph2eta);
  fInTree->SetBranchAddress("genph2match", &genph2match, &b_genph2match);
  fInTree->SetBranchAddress("gengl2E", &gengl2E, &b_gengl2E);
  fInTree->SetBranchAddress("gengl2pt", &gengl2pt, &b_gengl2pt);
  fInTree->SetBranchAddress("gengl2phi", &gengl2phi, &b_gengl2phi);
  fInTree->SetBranchAddress("gengl2eta", &gengl2eta, &b_gengl2eta);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
  fInTree->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
  fInTree->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
  fInTree->SetBranchAddress("t1pfmet", &t1pfmet, &b_t1pfmet);
  fInTree->SetBranchAddress("t1pfmetphi", &t1pfmetphi, &b_t1pfmetphi);
  fInTree->SetBranchAddress("t1pfmeteta", &t1pfmeteta, &b_t1pfmeteta);
  fInTree->SetBranchAddress("t1pfmetsumEt", &t1pfmetsumEt, &b_t1pfmetsumEt);
  fInTree->SetBranchAddress("njets", &njets, &b_njets);
  fInTree->SetBranchAddress("jetE", &jetE, &b_jetE);
  fInTree->SetBranchAddress("jetpt", &jetpt, &b_jetpt);
  fInTree->SetBranchAddress("jetphi", &jetphi, &b_jetphi);
  fInTree->SetBranchAddress("jeteta", &jeteta, &b_jeteta);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nhotons);
  fInTree->SetBranchAddress("phmatch", &phmatch, &b_phmatch);
  fInTree->SetBranchAddress("phVID", &phVID, &b_phVID);
  fInTree->SetBranchAddress("phE", &phE, &b_phE);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
  fInTree->SetBranchAddress("phscX", &phscX, &b_phscX);
  fInTree->SetBranchAddress("phscY", &phscY, &b_phscY);
  fInTree->SetBranchAddress("phscZ", &phscZ, &b_phscZ);
  fInTree->SetBranchAddress("phscE", &phscE, &b_phscE);
  fInTree->SetBranchAddress("phnrhs", &phnrhs, &b_phnrhs);
  fInTree->SetBranchAddress("phseedpos", &phseedpos, &b_phseedpos);
  fInTree->SetBranchAddress("phrhXs", &phrhXs, &b_phrhXs);
  fInTree->SetBranchAddress("phrhYs", &phrhYs, &b_phrhYs);
  fInTree->SetBranchAddress("phrhZs", &phrhZs, &b_phrhZs);
  fInTree->SetBranchAddress("phrhEs", &phrhEs, &b_phrhEs);
  fInTree->SetBranchAddress("phrhtimes", &phrhtimes, &b_phrhtimes);
  fInTree->SetBranchAddress("phrhIDs", &phrhIDs, &b_phrhIDs);
  fInTree->SetBranchAddress("phrhOOTs", &phrhOOTs, &b_phrhOOTs);
}
