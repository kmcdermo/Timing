#include "PlotPhotons.hh"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

PlotPhotons::PlotPhotons(TString filename, TString outdir) : fOutDir(outdir)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  // initialize tree
  PlotPhotons::InitTree();

  // in routine initialization
  fNEvCheck = 1000;

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
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    PlotPhotons::FillGenInfo();
    PlotPhotons::FillGenParticles();
    PlotPhotons::FillObjectCounts();
    PlotPhotons::FillMET();
    PlotPhotons::FillJets();
    PlotPhotons::FillRecoPhotons();
  }
}

void PlotPhotons::FillGenInfo()
{
  fPlots["genpuobs"]->Fill(genpuobs);
  fPlots["genputrue"]->Fill(genputrue);
}

void PlotPhotons::FillGenParticles()
{
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
}

void PlotPhotons::FillObjectCounts()
{
  fPlots["nvtx"]->Fill(nvtx);
  fPlots["njets"]->Fill(njets);
  fPlots["nphotons"]->Fill(nphotons);  

  int nlooseph = 0;
  int nmediumph = 0;
  int ntightph = 0;
  for (int iph = 0; iph < nphotons; iph++)
  { 
    if ( (*phVID)[iph] >= 1 ) nlooseph++;
    if ( (*phVID)[iph] >= 2 ) nmediumph++;
    if ( (*phVID)[iph] >= 3 ) ntightph++;
  }
  fPlots["nlooseph"]->Fill(nlooseph);
  fPlots["nmediumph"]->Fill(nmediumph);
  fPlots["ntightph"]->Fill(ntightph);
}

void PlotPhotons::FillMET()
{
  fPlots["t1pfmet"]->Fill(t1pfmet);
  fPlots["t1pfmetphi"]->Fill(t1pfmetphi);
  fPlots["t1pfmeteta"]->Fill(t1pfmeteta);
  fPlots["t1pfmetsumEt"]->Fill(t1pfmetsumEt);
}

void PlotPhotons::FillJets()
{
  for (int ijet = 0; ijet < njets; ijet++)
  {
    fPlots["jetE"]->Fill((*jetE)[ijet]);
    fPlots["jetpt"]->Fill((*jetpt)[ijet]);
    fPlots["jetphi"]->Fill((*jetphi)[ijet]);
    fPlots["jeteta"]->Fill((*jeteta)[ijet]);
  }
}

void PlotPhotons::FillRecoPhotons()
{
  for (int iph = 0; iph < nphotons; iph++)
  {
    fPlots["phE"]->Fill((*phE)[iph]);
    fPlots["phpt"]->Fill((*phpt)[iph]);
    fPlots["phphi"]->Fill((*phphi)[iph]);
    fPlots["pheta"]->Fill((*pheta)[iph]);
    fPlots["phscE"]->Fill((*phscE)[iph]);
    fPlots["phnrhs"]->Fill((*phnrhs)[iph]);

    if ( (*phmatch)[iph] > 0 ) // gen matched photons
    {
      fPlots["phE_gen"]->Fill((*phE)[iph]);
      fPlots["phpt_gen"]->Fill((*phpt)[iph]);
      fPlots["phphi_gen"]->Fill((*phphi)[iph]);
      fPlots["pheta_gen"]->Fill((*pheta)[iph]);
      fPlots["phscE_gen"]->Fill((*phscE)[iph]);
      fPlots["phnrhs_gen"]->Fill((*phnrhs)[iph]);
    }

    // loop over rechits (+ seed info)
    for (int irh = 0; irh < (*phnrhs)[iph]; irh++)
    {
      fPlots["phrhEs"]->Fill((*phrhEs)[iph][irh]);
      fPlots["phrhtimes"]->Fill((*phrhtimes)[iph][irh]);
      if ( (*phseedpos)[iph] == irh ) // seed info
      {
	fPlots["phseedE"]->Fill((*phrhEs)[iph][irh]);
	fPlots["phseedtime"]->Fill((*phrhtimes)[iph][irh]);
      }
      
      if ( (*phmatch)[iph] > 0 ) // gen matched photons
      {
	fPlots["phrhEs_gen"]->Fill((*phrhEs)[iph][irh]);
	fPlots["phrhtimes_gen"]->Fill((*phrhtimes)[iph][irh]);
	if ( (*phseedpos)[iph] == irh ) // seed info
        {
	  fPlots["phseedE_gen"]->Fill((*phrhEs)[iph][irh]);
	  fPlots["phseedtime_gen"]->Fill((*phrhtimes)[iph][irh]);
	}
      }
    }
  }
}

void PlotPhotons::SetupTH1Fs()
{
  PlotPhotons::SetupGenInfoTH1Fs();
  PlotPhotons::SetupGenParticlesTH1Fs();
  PlotPhotons::SetupObjectCountsTH1Fs();
  PlotPhotons::SetupMETTH1Fs();
  PlotPhotons::SetupJetsTH1Fs();
  PlotPhotons::SetupRecoPhotonsTH1Fs();
}

void PlotPhotons::SetupGenInfoTH1Fs()
{
  fPlots["genpuobs"] = PlotPhotons::MakeTH1F("genpuobs","Generator N PU Observed",100,0.f,100.f,"nPU (obs)","Events");
  fPlots["genputrue"] = PlotPhotons::MakeTH1F("genputrue","Generator N PU (true)",100,0.f,100.f,"nPU (true)","Events");
}

void PlotPhotons::SetupGenParticlesTH1Fs()
{
  fPlots["genN1mass"] = PlotPhotons::MakeTH1F("genN1mass","Generator Leading Neutralino Mass [GeV]",100,0.f,500.f,"Mass [GeV/c^{2}]","Neutralinos");
  fPlots["genN1E"] = PlotPhotons::MakeTH1F("genN1E","Generator Leading Neutralino E [GeV]",100,0.f,2000.f,"Energy [GeV]","Neutralinos");
  fPlots["genN1pt"] = PlotPhotons::MakeTH1F("genN1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Neutralinos");
  fPlots["genN1phi"] = PlotPhotons::MakeTH1F("genN1phi","Generator Leading Neutralino #phi",100,-3.2,3.2,"#phi","Neutralinos");
  fPlots["genN1eta"] = PlotPhotons::MakeTH1F("genN1eta","Generator Leading Neutralino #eta",100,-5.0,5.0,"#eta","Neutralinos");
  fPlots["genph1E"] = PlotPhotons::MakeTH1F("genph1E","Generator Leading Photon E [GeV]",100,0.f,2000.f,"Energy [GeV]","Photons");
  fPlots["genph1pt"] = PlotPhotons::MakeTH1F("genph1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Photons");
  fPlots["genph1phi"] = PlotPhotons::MakeTH1F("genph1phi","Generator Leading Photon #phi",100,-3.2,3.2,"#phi","Photons");
  fPlots["genph1eta"] = PlotPhotons::MakeTH1F("genph1eta","Generator Leading Photon #eta",100,-5.0,5.0,"#eta","Photons");
  fPlots["gengr1E"] = PlotPhotons::MakeTH1F("gengr1E","Generator Leading Gravitino E [GeV]",100,0.f,2000.f,"Energy [GeV]","Gravitinos");
  fPlots["gengr1pt"] = PlotPhotons::MakeTH1F("gengr1pt","Generator Leading p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Gravitinos");
  fPlots["gengr1phi"] = PlotPhotons::MakeTH1F("gengr1phi","Generator Leading Gravitino #phi",100,-3.2,3.2,"#phi","Gravitinos");
  fPlots["gengr1eta"] = PlotPhotons::MakeTH1F("gengr1eta","Generator Leading Gravitino #eta",100,-5.0,5.0,"#eta","Gravitinos");

  fPlots["genN2mass"] = PlotPhotons::MakeTH1F("genN2mass","Generator Subleading Neutralino Mass [GeV]",100,0.f,500.f,"Mass [GeV/c^{2}]","Neutralinos");
  fPlots["genN2E"] = PlotPhotons::MakeTH1F("genN2E","Generator Subleading Neutralino E [GeV]",100,0.f,2000.f,"Energy [GeV]","Neutralinos");
  fPlots["genN2pt"] = PlotPhotons::MakeTH1F("genN2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Neutralinos");
  fPlots["genN2phi"] = PlotPhotons::MakeTH1F("genN2phi","Generator Subleading Neutralino #phi",100,-3.2,3.2,"#phi","Neutralinos");
  fPlots["genN2eta"] = PlotPhotons::MakeTH1F("genN2eta","Generator Subleading Neutralino #eta",100,-5.0,5.0,"#eta","Neutralinos");
  fPlots["genph2E"] = PlotPhotons::MakeTH1F("genph2E","Generator Subleading Photon E [GeV]",100,0.f,2000.f,"Energy [GeV]","Photons");
  fPlots["genph2pt"] = PlotPhotons::MakeTH1F("genph2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Photons");
  fPlots["genph2phi"] = PlotPhotons::MakeTH1F("genph2phi","Generator Subleading Photon #phi",100,-3.2,3.2,"#phi","Photons");
  fPlots["genph2eta"] = PlotPhotons::MakeTH1F("genph2eta","Generator Subleading Photon #eta",100,-5.0,5.0,"#eta","Photons");
  fPlots["gengr2E"] = PlotPhotons::MakeTH1F("gengr2E","Generator Subleading Gravitino E [GeV]",100,0.f,2000.f,"Energy [GeV]","Gravitinos");
  fPlots["gengr2pt"] = PlotPhotons::MakeTH1F("gengr2pt","Generator Subleading p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Gravitinos");
  fPlots["gengr2phi"] = PlotPhotons::MakeTH1F("gengr2phi","Generator Subleading Gravitino #phi",100,-3.2,3.2,"#phi","Gravitinos");
  fPlots["gengr2eta"] = PlotPhotons::MakeTH1F("gengr2eta","Generator Subleading Gravitino #eta",100,-5.0,5.0,"#eta","Gravitinos");
}

void PlotPhotons::SetupObjectCountsTH1Fs()
{
  fPlots["nvtx"] = PlotPhotons::MakeTH1F("nvtx","nVertices (reco)",100,0.f,100.f,"nPV","Events");
  fPlots["njets"] = PlotPhotons::MakeTH1F("njets","nJets (reco)",40,0.f,40.f,"nJets","Events");
  fPlots["nphotons"] = PlotPhotons::MakeTH1F("nphotons","nPhotons (reco)",20,0.f,20.f,"nPhotons","Events");
  fPlots["nlooseph"] = PlotPhotons::MakeTH1F("nlooseph","nLoosePhotons",20,0.f,20.f,"nLoosePhotons","Events");
  fPlots["nmediumph"] = PlotPhotons::MakeTH1F("nmediumph","nMediumPhotons",20,0.f,20.f,"nMediumPhotons","Events");
  fPlots["ntightph"] = PlotPhotons::MakeTH1F("ntightph","nTightPhotons",20,0.f,20.f,"nTightPhotons","Events");
}

void PlotPhotons::SetupMETTH1Fs()
{
  fPlots["t1pfmet"] = PlotPhotons::MakeTH1F("t1pfmet","Type1 PF MET [GeV]",100,0.f,2000.f,"MET [GeV]","Events");
  fPlots["t1pfmetphi"] = PlotPhotons::MakeTH1F("t1pfmetphi","Type1 PF MET #phi",100,-3.2,3.2,"#phi","Events");
  fPlots["t1pfmeteta"] = PlotPhotons::MakeTH1F("t1pfmeteta","Type1 PF MET #eta",100,-5.0,5.0,"#eta","Events");
  fPlots["t1pfmetsumEt"] = PlotPhotons::MakeTH1F("t1pfmetsumEt","Type1 PF MET #Sigma E_{T} [GeV]",100,0.f,2000.f,"#Sigma E_{T} [GeV]","Events");
}

void PlotPhotons::SetupJetsTH1Fs()
{
  fPlots["jetE"] = PlotPhotons::MakeTH1F("jetE","Jets Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Jets");
  fPlots["jetpt"] = PlotPhotons::MakeTH1F("jetpt","Jets p_{T} [GeV/c] (reco)",100,0.f,2000.f,"p_{T} [GeV/c]","Jets");
  fPlots["jetphi"] = PlotPhotons::MakeTH1F("jetphi","Jets #phi (reco)",100,-3.2,3.2,"#phi","Jets");
  fPlots["jeteta"] = PlotPhotons::MakeTH1F("jeteta","Jets #eta (reco)",100,-5.0,5.0,"#eta","Jets");
}

void PlotPhotons::SetupRecoPhotonsTH1Fs()
{
  // All reco photons
  fPlots["phE"] = PlotPhotons::MakeTH1F("phE","Photons Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Photons");
  fPlots["phpt"] = PlotPhotons::MakeTH1F("phpt","Photons p_{T} [GeV/c] (reco)",100,0.f,2000.f,"p_{T} [GeV/c]","Photons");
  fPlots["phphi"] = PlotPhotons::MakeTH1F("phphi","Photons #phi (reco)",100,-3.2,3.2,"#phi","Photons");
  fPlots["pheta"] = PlotPhotons::MakeTH1F("pheta","Photons #eta (reco)",100,-5.0,5.0,"#eta","Photons");
  fPlots["phscE"] = PlotPhotons::MakeTH1F("phscE","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Photons");
  fPlots["phnrhs"] = PlotPhotons::MakeTH1F("phnrhs","nRecHits from Photons (reco)",100,0.f,100.f,"nRecHits","Photons");
  
  // all rec hits + seed info
  fPlots["phrhEs"] = PlotPhotons::MakeTH1F("phrhEs","Photons RecHits Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","RecHits");
  fPlots["phrhtimes"] = PlotPhotons::MakeTH1F("phrhtimes","Photons RecHits Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","RecHits");
  fPlots["phseedE"] = PlotPhotons::MakeTH1F("phseedE","Photons Seed RecHit Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Seed RecHits");
  fPlots["phseedtime"] = PlotPhotons::MakeTH1F("phseedtime","Photons Seed RecHit Time [ns] (reco)",200,-100.f,100.f,"Time [ns]","Seed RecHits");
  
  // Gen matched reco quantities
  fPlots["phE_gen"] = PlotPhotons::MakeTH1F("phE_gen","Photons Energy [GeV] (reco: gen matched)",100,0.f,2000.f,"Energy [GeV]","Photons");
  fPlots["phpt_gen"] = PlotPhotons::MakeTH1F("phpt_gen","Photons p_{T} [GeV/c] (reco: gen matched)",100,0.f,2000.f,"p_{T} [GeV/c]","Photons");
  fPlots["phphi_gen"] = PlotPhotons::MakeTH1F("phphi_gen","Photons #phi (reco: gen matched)",100,-3.2,3.2,"#phi","Photons");
  fPlots["pheta_gen"] = PlotPhotons::MakeTH1F("pheta_gen","Photons #eta (reco: gen matched)",100,-5.0,5.0,"#eta","Photons");
  fPlots["phscE_gen"] = PlotPhotons::MakeTH1F("phscE_gen","Photons SuperCluster Energy [GeV] (reco)",100,0.f,2000.f,"Energy [GeV]","Photons");
  fPlots["phnrhs_gen"] = PlotPhotons::MakeTH1F("phnrhs_gen","nRecHits from Photons (reco: gen matched)",100,0.f,100.f,"nRecHits","Photons");

  // all rec hits + seed info for gen matched photons
  fPlots["phrhEs_gen"] = PlotPhotons::MakeTH1F("phrhEs_gen","Photons RecHits Energy [GeV] (reco: gen matched)",100,0.f,2000.f,"Energy [GeV]","RecHits");
  fPlots["phrhtimes_gen"] = PlotPhotons::MakeTH1F("phrhtimes_gen","Photons RecHits Time [ns] (reco: gen matched)",200,-100.f,100.f,"Time [ns]","RecHits");
  fPlots["phseedE_gen"] = PlotPhotons::MakeTH1F("phseedE_gen","Photons Seed RecHit Energy [GeV] (reco: gen matched)",100,0.f,2000.f,"Energy [GeV]","Seed RecHits");
  fPlots["phseedtime_gen"] = PlotPhotons::MakeTH1F("phseedtime_gen","Photons Seed RecHit Time [ns] (reco: gen matched)",200,-100.f,100.f,"Time [ns]","Seed RecHits");
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
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    
    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    mapiter->second->Draw("HIST");
    
    // first save as linear, then log
    canv->SetLogy(0);
    canv->SaveAs(Form("%s/lin/%s.png",fOutDir.Data(),mapiter->first.Data()));

    canv->SetLogy(1);
    canv->SaveAs(Form("%s/log/%s.png",fOutDir.Data(),mapiter->first.Data()));

    delete canv;
    delete mapiter->second;
  }
  fPlots.clear();
}

void PlotPhotons::InitTree()
{
  // Set object pointer
  genjetE = 0;
  genjetpt = 0;
  genjetphi = 0;
  genjeteta = 0;
  genjetmatch = 0;
  jetE = 0;
  jetpt = 0;
  jetphi = 0;
  jeteta = 0;
  jetmatch = 0;
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
  fInTree->SetBranchAddress("gengr1E", &gengr1E, &b_gengr1E);
  fInTree->SetBranchAddress("gengr1pt", &gengr1pt, &b_gengr1pt);
  fInTree->SetBranchAddress("gengr1phi", &gengr1phi, &b_gengr1phi);
  fInTree->SetBranchAddress("gengr1eta", &gengr1eta, &b_gengr1eta);
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
  fInTree->SetBranchAddress("gengr2E", &gengr2E, &b_gengr2E);
  fInTree->SetBranchAddress("gengr2pt", &gengr2pt, &b_gengr2pt);
  fInTree->SetBranchAddress("gengr2phi", &gengr2phi, &b_gengr2phi);
  fInTree->SetBranchAddress("gengr2eta", &gengr2eta, &b_gengr2eta);
  fInTree->SetBranchAddress("ngenjets", &ngenjets, &b_ngenjets);
  fInTree->SetBranchAddress("genjetE", &genjetE, &b_genjetE);
  fInTree->SetBranchAddress("genjetpt", &genjetpt, &b_genjetpt);
  fInTree->SetBranchAddress("genjetphi", &genjetphi, &b_genjetphi);
  fInTree->SetBranchAddress("genjeteta", &genjeteta, &b_genjeteta);
  fInTree->SetBranchAddress("genjetmatch", &genjetmatch, &b_genjetmatch);
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
  fInTree->SetBranchAddress("jetmatch", &jetmatch, &b_jetmatch);
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
