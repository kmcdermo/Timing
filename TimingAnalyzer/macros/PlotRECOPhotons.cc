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
  fNEvCheck = 10000;
  
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
  Int_t nPhotonsGED = 0;
  Int_t nPhotonsOOT = 0;
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    if ((*phseedpos)[iph] == -9999.f) continue;

    const bool isEB = (std::abs((*phsceta)[iph]) < ECAL::etaEB);
    const bool isEE = (std::abs((*phsceta)[iph]) > ECAL::etaEEmin && std::abs((*phsceta)[iph]) < ECAL::etaEEmax);
    if (!isEB && !isEE) continue;

    if (fApplyrhECut && ((*phrhE)[iph][(*phseedpos)[iph]] < frhECut)) continue;
    if (fApplyPtCut && ((*phpt)[iph] < fPtCut)) continue;
    if (fApplyHoECut && ((*phHoE)[iph] > fHoECut)) continue;
    if (fApplyR9Cut && ((*phr9)[iph] < fR9Cut)) continue;
    if (isEB) 
    {
      if (fApplySieieCut && ((*phsieie)[iph] > fSieieEBCut)) continue;
    }
    else 
    {
      if (fApplySieieCut && ((*phsieie)[iph] > fSieieEECut)) continue;
    }

    const bool isOOT = (*phisOOT)[iph]; 
    isOOT ? nPhotonsGED++ : nPhotonsOOT++;

    TString label = Form("%s_%s",(isOOT ? "oot" : "ged"),(isEB ? "eb" : "ee"));

    fPlots[Form("phE_%s",label.Data())]->Fill((*phE)[iph]);
    fPlots[Form("phpt_%s",label.Data())]->Fill((*phpt)[iph]);
    fPlots[Form("phphi_%s",label.Data())]->Fill((*phphi)[iph]);
    fPlots[Form("pheta_%s",label.Data())]->Fill((*pheta)[iph]);

    fPlots[Form("phHoE_%s",label.Data())]->Fill((*phHoE)[iph]);
    fPlots[Form("phr9_%s",label.Data())]->Fill((*phr9)[iph]);
    fPlots[Form("phsieie_%s",label.Data())]->Fill((*phsieie)[iph]);
    fPlots[Form("phsmaj_%s",label.Data())]->Fill((*phsmaj)[iph]);
    fPlots[Form("phsmin_%s",label.Data())]->Fill((*phsmin)[iph]);

    fPlots[Form("phseedE_%s",label.Data())]->Fill((*phrhE)[iph][(*phseedpos)[iph]]);
    fPlots[Form("phseedtime_%s",label.Data())]->Fill((*phrhtime)[iph][(*phseedpos)[iph]]);
    fPlots[Form("phseedOOT_%s",label.Data())]->Fill((*phrhOOT)[iph][(*phseedpos)[iph]]);
  }
  fPlots["nPhotonsGED"]->Fill(nPhotonsGED);
  fPlots["nPhotonsOOT"]->Fill(nPhotonsOOT);
}

void PlotRECOPhotons::SetupRecoPhotons()
{
  fPlots["nPhotonsGED"] = PlotRECOPhotons::MakeTH1F("nPhotonsGED","nPhotons",20,0.f,20.f,"nPhotons (GED)","Events");
  fPlots["nPhotonsOOT"] = PlotRECOPhotons::MakeTH1F("nPhotonsOOT","nPhotons",20,0.f,20.f,"nPhotons (OOT)","Events");

  // lots-o-plots
  TStrVec collections = {"oot","ged"};
  TStrVec detectors = {"eb","ee"};

  for (auto & collection : collections)
  {
    for (auto & detector : detectors)
    {
      TString label = Form("%s_%s",collection.Data(),detector.Data());

      fPlots[Form("phE_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phE_%s",label.Data()),"Photon Energy [GeV]",100,0.f,3000.f,"Energy [GeV]","Photons");
      fPlots[Form("phpt_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phpt_%s",label.Data()),"Photon p_{T} [GeV/c]",100,0.f,2000.f,"p_{T} [GeV/c]","Photons");
      fPlots[Form("phphi_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phphi_%s",label.Data()),"Photon #phi",100,-3.2f,3.2f,"#phi","Photons");
      fPlots[Form("pheta_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("pheta_%s",label.Data()),"Photon #eta",100,-5.f,5.f,"#eta","Photons");

      fPlots[Form("phHoE_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phHoE_%s",label.Data()),"Photon H/E",100,0.f,2.f,"H/E","Photons");
      fPlots[Form("phr9_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phr9_%s",label.Data()),"Photon R_{9}",100,0.f,2.f,"R_{9}","Photons");
      fPlots[Form("phsieie_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phsieie_%s",label.Data()),"Photon #sigma_{i#eta i#eta}",100,0.f,1.f,"#sigma_{i#eta i#eta}","Photons");
      fPlots[Form("phsmaj_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phsmaj_%s",label.Data()),"Photon S_{Major}",100,0.f,5.f,"S_{Major}","Photons");
      fPlots[Form("phsmin_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phsmin_%s",label.Data()),"Photon S_{Minor}",100,0.f,5.f,"S_{Minor}","Photons");

      fPlots[Form("phseedE_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phseedE_%s",label.Data()),"Photon Seed Energy [GeV]",100,0.f,1000.f,"Seed Energy [GeV]","Photons");
      fPlots[Form("phseedtime_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phseedtime_%s",label.Data()),"Photon Seed Time [ns]",100,-25.f,25.f,"Seed Time [ns]","Photons");
      fPlots[Form("phseedOOT_%s",label.Data())] = PlotRECOPhotons::MakeTH1F(Form("phseedOOT_%s",label.Data()),"Photon Seed OOT Flag",2,0,2,"OOT Flag","Photons");
    }
  }
}

TH1F * PlotRECOPhotons::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  return hist;
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

void PlotRECOPhotons::InitTree()
{
  // need to set vector pointers, otherwise root craps out
  phisOOT = 0;
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
  fInTree->SetBranchAddress("phisOOT", &phisOOT, &b_phisOOT);
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
