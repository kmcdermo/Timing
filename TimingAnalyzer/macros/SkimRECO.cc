#include "SkimRECO.hh"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

SkimRECO::SkimRECO(TString filename, TString outdir, Bool_t savehistnames,
		   Float_t jetptcut, Int_t njetcut, 
		   Float_t phptcut, Float_t phsieieEBcut, Float_t phsieieEEcut,
		   Float_t phsmajEBcut, Float_t phsmajEEcut, Float_t phsminEBcut, Float_t phsminEEcut, 
		   Bool_t applyEBonly, Bool_t applyEEonly) :
  fOutDir(outdir), fSaveHistNames(savehistnames),
  fJetPtCut(jetptcut), fNJetCut(njetcut), 
  fPhPtCut(phptcut), fPhSieieEBCut(phsieieEBcut), fPhSieieEECut(phsieieEEcut),
  fPhSmajEBCut(phsmajEBcut), fPhSmajEECut(phsmajEEcut), fPhSminEBCut(phsminEBcut), fPhSminEECut(phsminEEcut),
  fApplyEBOnly(applyEBonly), fApplyEEOnly(applyEEonly)
{
  // input
  fInFile = TFile::Open(filename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");
  
  // initialize tree
  SkimRECO::InitTree();

  // in routine initialization
  fNEvCheck = 1000;

  // overall efficiency
  fNPassed = 0;

  FileStat_t dummyFileStat; 
  if (gSystem->GetPathInfo(fOutDir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s",fOutDir.Data());
    gSystem->Exec(mkDir.Data());

    TString mkLin = Form("mkdir -p %s/lin",fOutDir.Data());
    gSystem->Exec(mkLin.Data());

    TString mkLog = Form("mkdir -p %s/log",fOutDir.Data());
    gSystem->Exec(mkLog.Data());
  }

  // dump cuts used in the selection
  std::ofstream cutdump;
  cutdump.open(Form("%s/cuts.txt",fOutDir.Data()),std::ios_base::trunc);
  cutdump << "jet cuts: " << std::endl
	  << "  pT: " << fJetPtCut 
	  << "  nJets over pT cut: " << fNJetCut << std::endl << std::endl;
  cutdump << "photon cuts: " << std::endl
	  << "  pT: " << fPhPtCut << std::endl
	  << "  Sieie EB: " << fPhSieieEBCut << " EE: " << fPhSieieEECut << std::endl
	  << "  Smaj EB: " << fPhSmajEBCut << " EE: " << fPhSmajEECut << std::endl
	  << "  Smin EB: " << fPhSminEBCut << " EE: " << fPhSminEECut << std::endl;
  cutdump.close();

  // dump hists for stacking purposes
  if (fSaveHistNames)
  {
    fHistDump.open("output/skim/histdump.txt",std::ios_base::trunc);
  }  

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

SkimRECO::~SkimRECO()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
  if (fSaveHistNames) fHistDump.close();
}

void SkimRECO::DoSkim()
{
  SkimRECO::SetupPlots();
  SkimRECO::EventLoop();
  SkimRECO::OutputTH1Fs();
  SkimRECO::OutputTH2Fs();
  SkimRECO::OutputTEffs();
  SkimRECO::DumpEfficiency();
}

void SkimRECO::SetupPlots()
{
  SkimRECO::SetupNminus1();
  SkimRECO::SetupTEffs();
  SkimRECO::SetupAnalysis();
}

void SkimRECO::EventLoop()
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);
    
    // standard printout
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    // Efficiency related plots
    Bool_t passed = false;
    SkimRECO::DoEfficiency(passed);

    if (passed) fNPassed++;
  }
}

void SkimRECO::DoEfficiency(Bool_t & passed)
{
  /////////////////////////////////////
  // First assess individual objects //
  // passing the full selection      //
  /////////////////////////////////////

  // jets
  Int_t njets_ptcut = SkimRECO::GetNJetsPt();

  // photons
  std::vector<Int_t> phs;
  SkimRECO::GetPhs(phs);

  ///////////////
  // N-1 Plots //
  ///////////////
  SkimRECO::FillNminus1(njets_ptcut,phs);

  ///////////////////////////
  // Full Efficiency Plots //
  ///////////////////////////

  // bool if event passed selection
  passed = (phs.size() > 0 && njets_ptcut > fNJetCut);

  SkimRECO::FillTEffs(passed,phs);
  if (passed) SkimRECO::FillAnalysis(phs);
}

void SkimRECO::FillNminus1(const Int_t njets_ptcut, const std::vector<Int_t> & phs)
{
  // Still need a few more n-1 vectors
  std::vector<Int_t> phspt, phssieie, phssmaj, phssmin;
  SkimRECO::GetPhsPt(phspt);
  SkimRECO::GetPhsSieie(phssieie);
  SkimRECO::GetPhsSmaj(phssmaj);
  SkimRECO::GetPhsSmin(phssmin);

  // njets plots
  if (phs.size() > 0)
  {    
    fPlots["njets_nm1"]->Fill(njets_ptcut);
    if (njets_ptcut > 0) fPlots["jet0pt_nm1"]->Fill((*jetpt)[0]);
    if (njets_ptcut > 1) fPlots["jet1pt_nm1"]->Fill((*jetpt)[1]);
    if (njets_ptcut > 2) fPlots["jet2pt_nm1"]->Fill((*jetpt)[2]);
  }

  // photon plots
  if (njets_ptcut > fNJetCut)
  { 
    // photon pT plots
    if (phspt.size() > 0) fPlots["ph0pt_nm1"]->Fill((*phpt)[phspt[0]]);
    if (phspt.size() > 1) fPlots["ph1pt_nm1"]->Fill((*phpt)[phspt[1]]);
    if (phspt.size() > 2) fPlots["ph2pt_nm1"]->Fill((*phpt)[phspt[2]]);

    // SigmaIetaIeta plots
    if (phssieie.size() > 0)
    {
      if   (std::abs((*phsceta)[phssieie[0]]) < 1.4442) fPlots["ph0sieieEB_nm1"]->Fill((*phsieie)[phssieie[0]]);    
      else                                           	fPlots["ph0sieieEE_nm1"]->Fill((*phsieie)[phssieie[0]]);    
    }
    if (phssieie.size() > 1)
    {
      if   (std::abs((*phsceta)[phssieie[1]]) < 1.4442) fPlots["ph1sieieEB_nm1"]->Fill((*phsieie)[phssieie[1]]);    
      else                                           	fPlots["ph1sieieEE_nm1"]->Fill((*phsieie)[phssieie[1]]);    
    }
    if (phssieie.size() > 2)
    {
      if   (std::abs((*phsceta)[phssieie[2]]) < 1.4442) fPlots["ph2sieieEB_nm1"]->Fill((*phsieie)[phssieie[2]]);    
      else                                           	fPlots["ph2sieieEE_nm1"]->Fill((*phsieie)[phssieie[2]]);    
    }

    // Smajor plots
    if (phssmaj.size() > 0)
    {
      if   (std::abs((*phsceta)[phssmaj[0]]) < 1.4442) fPlots["ph0smajEB_nm1"]->Fill((*phsmaj)[phssmaj[0]]);    
      else                                             fPlots["ph0smajEE_nm1"]->Fill((*phsmaj)[phssmaj[0]]);    
    }
    if (phssmaj.size() > 1)
    {
      if   (std::abs((*phsceta)[phssmaj[1]]) < 1.4442) fPlots["ph1smajEB_nm1"]->Fill((*phsmaj)[phssmaj[1]]);    
      else                                             fPlots["ph1smajEE_nm1"]->Fill((*phsmaj)[phssmaj[1]]);    
    }
    if (phssmaj.size() > 2)
    {
      if   (std::abs((*phsceta)[phssmaj[2]]) < 1.4442) fPlots["ph2smajEB_nm1"]->Fill((*phsmaj)[phssmaj[2]]);    
      else                                             fPlots["ph2smajEE_nm1"]->Fill((*phsmaj)[phssmaj[2]]);    
    }

    // Sminor plots
    if (phssmin.size() > 0)
    {
      if   (std::abs((*phsceta)[phssmin[0]]) < 1.4442) fPlots["ph0sminEB_nm1"]->Fill((*phsmin)[phssmin[0]]);    
      else                                             fPlots["ph0sminEE_nm1"]->Fill((*phsmin)[phssmin[0]]);    
    }
    if (phssmin.size() > 1)
    {
      if   (std::abs((*phsceta)[phssmin[1]]) < 1.4442) fPlots["ph1sminEB_nm1"]->Fill((*phsmin)[phssmin[1]]);    
      else                                             fPlots["ph1sminEE_nm1"]->Fill((*phsmin)[phssmin[1]]);    
    }
    if (phssmin.size() > 2)
    {
      if   (std::abs((*phsceta)[phssmin[2]]) < 1.4442) fPlots["ph2sminEB_nm1"]->Fill((*phsmin)[phssmin[2]]);    
      else                                             fPlots["ph2sminEE_nm1"]->Fill((*phsmin)[phssmin[2]]);    
    }
  }
}

void SkimRECO::FillTEffs(const Bool_t passed, const std::vector<Int_t> & phs) {}

void SkimRECO::FillAnalysis(const std::vector<Int_t> & phs) 
{
  if (phs.size() > 0) fPlots["ph0seedtime"]->Fill((*phseedtime)[phs[0]]);
  if (phs.size() > 1) fPlots["ph1seedtime"]->Fill((*phseedtime)[phs[1]]);
  if (phs.size() > 2) fPlots["ph2seedtime"]->Fill((*phseedtime)[phs[2]]);

  fPlots["MET"]->Fill(t1pfMETpt);
}


Int_t SkimRECO::GetNJetsPt()
{
  Int_t njets_ptcut = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if ((*jetpt)[ijet] < fJetPtCut) break;
    njets_ptcut++;
  }
  return njets_ptcut;
}

void SkimRECO::GetPhs(std::vector<Int_t> & phs)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsieie)[iph] > fPhSieieEBCut) || (!isEB && (*phsieie)[iph] > fPhSieieEECut)) continue;
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phs.push_back(iph);
  }
}

void SkimRECO::GetPhsPt(std::vector<Int_t> & phspt)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all other cuts
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsieie)[iph] > fPhSieieEBCut) || (!isEB && (*phsieie)[iph] > fPhSieieEECut)) continue;
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    // if passed all that, save the photon!
    phspt.push_back(iph);
  }
}

void SkimRECO::GetPhsSieie(std::vector<Int_t> & phssieie)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all other cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;
    
    // if passed all that, save the photon!
    phssieie.push_back(iph);
  }
}

void SkimRECO::GetPhsSmaj(std::vector<Int_t> & phssmaj)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsieie)[iph] > fPhSieieEBCut) || (!isEB && (*phsieie)[iph] > fPhSieieEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phssmaj.push_back(iph);
  }
}

void SkimRECO::GetPhsSmin(std::vector<Int_t> & phssmin)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsieie)[iph] > fPhSieieEBCut) || (!isEB && (*phsieie)[iph] > fPhSieieEECut)) continue;
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;

    phssmin.push_back(iph);
  }
}

void SkimRECO::SetupNminus1()
{
  // njets
  fPlots["njets_nm1"]  = SkimRECO::MakeTH1F("njets_nm1","nJets (N-1)",10,0.f,10.f,Form("nJets (p_{T} > %4.1f [GeV/c])",fJetPtCut),"Events");
  fPlots["jet0pt_nm1"] = SkimRECO::MakeTH1F("jet0pt_nm1","Jet p_{T} [GeV/c] (N-1)",100,0.f,1000.f,"Leading Jet p_{T} [GeV/c]","Events");
  fPlots["jet1pt_nm1"] = SkimRECO::MakeTH1F("jet1pt_nm1","Jet p_{T} [GeV/c] (N-1)",100,0.f,1000.f,"Subleading Jet p_{T} [GeV/c]","Events");
  fPlots["jet2pt_nm1"] = SkimRECO::MakeTH1F("jet2pt_nm1","Jet p_{T} [GeV/c] (N-1)",100,0.f,1000.f,"Subsubleading Jet p_{T} [GeV/c]","Events");

  // Leading photons
  fPlots["ph0pt_nm1"] = SkimRECO::MakeTH1F("ph0pt_nm1","Photon p_{T} [GeV/c] (N-1)",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]","Events");
  fPlots["ph1pt_nm1"] = SkimRECO::MakeTH1F("ph1pt_nm1","Photon p_{T} [GeV/c] (N-1)",100,0.f,2500.f,"Subleading Photon p_{T} [GeV/c]","Events");
  fPlots["ph2pt_nm1"] = SkimRECO::MakeTH1F("ph2pt_nm1","Photon p_{T} [GeV/c] (N-1)",100,0.f,2500.f,"Subsubleading Photon p_{T} [GeV/c]","Events");

  fPlots["ph0sieieEB_nm1"] = SkimRECO::MakeTH1F("ph0sieieEB_nm1","Photon #sigma_{i#eta i#eta} EB (N-1)",100,0,0.1,"Leading Photon #sigma_{i#eta i#eta}","Events");
  fPlots["ph0sieieEE_nm1"] = SkimRECO::MakeTH1F("ph0sieieEE_nm1","Photon #sigma_{i#eta i#eta} EE (N-1)",100,0,0.1,"Leading Photon #sigma_{i#eta i#eta}","Events");
  fPlots["ph1sieieEB_nm1"] = SkimRECO::MakeTH1F("ph1sieieEB_nm1","Photon #sigma_{i#eta i#eta} EB (N-1)",100,0,0.1,"Subleading Photon #sigma_{i#eta i#eta}","Events");
  fPlots["ph1sieieEE_nm1"] = SkimRECO::MakeTH1F("ph1sieieEE_nm1","Photon #sigma_{i#eta i#eta} EE (N-1)",100,0,0.1,"Subleading Photon #sigma_{i#eta i#eta}","Events");
  fPlots["ph2sieieEB_nm1"] = SkimRECO::MakeTH1F("ph2sieieEB_nm1","Photon #sigma_{i#eta i#eta} EB (N-1)",100,0,0.1,"Subsubleading Photon #sigma_{i#eta i#eta}","Events");
  fPlots["ph2sieieEE_nm1"] = SkimRECO::MakeTH1F("ph2sieieEE_nm1","Photon #sigma_{i#eta i#eta} EE (N-1)",100,0,0.1,"Subsubleading Photon #sigma_{i#eta i#eta}","Events");

  fPlots["ph0smajEB_nm1"] = SkimRECO::MakeTH1F("ph0smajEB_nm1","Photon S_{major} EB (N-1)",100,0,0.1,"Leading Photon S_{major}","Events");
  fPlots["ph0smajEE_nm1"] = SkimRECO::MakeTH1F("ph0smajEE_nm1","Photon S_{major} EE (N-1)",100,0,0.1,"Leading Photon S_{major}","Events");
  fPlots["ph1smajEB_nm1"] = SkimRECO::MakeTH1F("ph1smajEB_nm1","Photon S_{major} EB (N-1)",100,0,0.1,"Subleading Photon S_{major}","Events");
  fPlots["ph1smajEE_nm1"] = SkimRECO::MakeTH1F("ph1smajEE_nm1","Photon S_{major} EE (N-1)",100,0,0.1,"Subleading Photon S_{major}","Events");
  fPlots["ph2smajEB_nm1"] = SkimRECO::MakeTH1F("ph2smajEB_nm1","Photon S_{major} EB (N-1)",100,0,0.1,"Subsubleading Photon S_{major}","Events");
  fPlots["ph2smajEE_nm1"] = SkimRECO::MakeTH1F("ph2smajEE_nm1","Photon S_{major} EE (N-1)",100,0,0.1,"Subsubleading Photon S_{major}","Events");

  fPlots["ph0sminEB_nm1"] = SkimRECO::MakeTH1F("ph0sminEB_nm1","Photon S_{minor} EB (N-1)",100,0,0.1,"Leading Photon S_{minor}","Events");
  fPlots["ph0sminEE_nm1"] = SkimRECO::MakeTH1F("ph0sminEE_nm1","Photon S_{minor} EE (N-1)",100,0,0.1,"Leading Photon S_{minor}","Events");
  fPlots["ph1sminEB_nm1"] = SkimRECO::MakeTH1F("ph1sminEB_nm1","Photon S_{minor} EB (N-1)",100,0,0.1,"Subleading Photon S_{minor}","Events");
  fPlots["ph1sminEE_nm1"] = SkimRECO::MakeTH1F("ph1sminEE_nm1","Photon S_{minor} EE (N-1)",100,0,0.1,"Subleading Photon S_{minor}","Events");
  fPlots["ph2sminEB_nm1"] = SkimRECO::MakeTH1F("ph2sminEB_nm1","Photon S_{minor} EB (N-1)",100,0,0.1,"Subsubleading Photon S_{minor}","Events");
  fPlots["ph2sminEE_nm1"] = SkimRECO::MakeTH1F("ph2sminEE_nm1","Photon S_{minor} EE (N-1)",100,0,0.1,"Subsubleading Photon S_{minor}","Events");
}

void SkimRECO::SetupTEffs() {}

void SkimRECO::SetupAnalysis() 
{
  fPlots["ph0seedtime"] = SkimRECO::MakeTH1F("ph0seedtime","Photon Seed Time [ns]",80,-5.f,15.f,"Leading Photon Seed Time [ns]","Events");
  fPlots["ph1seedtime"] = SkimRECO::MakeTH1F("ph1seedtime","Photon Seed Time [ns]",80,-5.f,15.f,"Subleading Photon Seed Time [ns]","Events");
  fPlots["ph2seedtime"] = SkimRECO::MakeTH1F("ph2seedtime","Photon Seed Time [ns]",80,-5.f,15.f,"Subsubleading Photon Seed Time [ns]","Events");

  fPlots["MET"] = SkimRECO::MakeTH1F("MET","MET",100,0.f,1000.f,"Missing E_{T} [GeV])","Events");
}

TH1F * SkimRECO::MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle)
{
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh);
  hist->SetLineColor(kBlack);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  return hist;
}

TEfficiency * SkimRECO::MakeTEff(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle)
{
  TEfficiency * eff = new TEfficiency(hname.Data(),Form("%s;%s;%s",htitle.Data(),xtitle.Data(),ytitle.Data()),nbinsx,xlow,xhigh);
  eff->SetLineColor(kBlack);

  return eff;
}

TH2F * SkimRECO::MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle)
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  return hist;
}

void SkimRECO::OutputTH1Fs()
{
  fOutFile->cd();

  for (TH1MapIter mapiter = fPlots.begin(); mapiter != fPlots.end(); ++mapiter) 
  { 
    // scale to 1.0
    mapiter->second->Scale(1.f/mapiter->second->Integral());

    // save to output file
    mapiter->second->Write(mapiter->second->GetName(),TObject::kWriteDelete); 
    
    // save hist to a text file
    if (fSaveHistNames) fHistDump << mapiter->second->GetName() << std::endl;

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

void SkimRECO::OutputTH2Fs()
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
    canv->SaveAs(Form("%s/lin/%s.png",fOutDir.Data(),mapiter->first.Data()));

    delete canv;
    delete mapiter->second;
  }
  fPlots2D.clear();
}

void SkimRECO::OutputTEffs()
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
    canv->SaveAs(Form("%s/lin/%s.png",fOutDir.Data(),mapiter->first.Data()));

    // first save as linear, then log
    canv->SetLogy(1);
    canv->SaveAs(Form("%s/log/%s.png",fOutDir.Data(),mapiter->first.Data()));

    delete canv;
    delete mapiter->second;
  }
  fEffs.clear();
}

void SkimRECO::DumpEfficiency()
{
  const Int_t nEntries = fInTree->GetEntries();
  const Float_t eff = Float_t(fNPassed)/Float_t(nEntries);

  std::cout << "nEntries: " << nEntries << " nPassed: " << fNPassed << " efficiency: " << eff << std::endl;
}

void SkimRECO::InitTree()
{
  // Set object pointer
  jetE = 0;
  jetpt = 0;
  jetphi = 0;
  jeteta = 0;
  phE = 0;
  phpt = 0;
  phphi = 0;
  pheta = 0;
  phscE = 0;
  phscphi = 0;
  phsceta = 0;
  phsieie = 0;
  phsipip = 0;
  phsieip = 0;
  phsmaj = 0;
  phsmin = 0;
  phHoE = 0;
  phChgIso = 0;
  phNeuIso = 0;
  phPhIso = 0;
  phr9 = 0;
  phsuisseX = 0;
  phHoE_b = 0;
  phsieie_b = 0;
  phChgIso_b = 0;
  phNeuIso_b = 0;
  phPhIso_b = 0;
  phnrh = 0;
  phnrhEcut = 0;
  phnrhOOT = 0;
  phseedE = 0;
  phseedphi = 0;
  phseedeta = 0;
  phseedtime = 0;
  phseedOOT = 0;

  // set branch addresses
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
  fInTree->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
  fInTree->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
  fInTree->SetBranchAddress("t1pfMETpt", &t1pfMETpt, &b_t1pfMETpt);
  fInTree->SetBranchAddress("t1pfMETphi", &t1pfMETphi, &b_t1pfMETphi);
  fInTree->SetBranchAddress("t1pfMETsumEt", &t1pfMETsumEt, &b_t1pfMETsumEt);
  fInTree->SetBranchAddress("njets", &njets, &b_njets);
  fInTree->SetBranchAddress("jetE", &jetE, &b_jetE);
  fInTree->SetBranchAddress("jetpt", &jetpt, &b_jetpt);
  fInTree->SetBranchAddress("jetphi", &jetphi, &b_jetphi);
  fInTree->SetBranchAddress("jeteta", &jeteta, &b_jeteta);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fInTree->SetBranchAddress("phE", &phE, &b_phE);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
  fInTree->SetBranchAddress("phscE", &phscE, &b_phscE);
  fInTree->SetBranchAddress("phscphi", &phscphi, &b_phscphi);
  fInTree->SetBranchAddress("phsceta", &phsceta, &b_phsceta);
  fInTree->SetBranchAddress("phsieie", &phsieie, &b_phsieie);
  fInTree->SetBranchAddress("phsipip", &phsipip, &b_phsipip);
  fInTree->SetBranchAddress("phsieip", &phsieip, &b_phsieip);
  fInTree->SetBranchAddress("phsmaj", &phsmaj, &b_phsmaj);
  fInTree->SetBranchAddress("phsmin", &phsmin, &b_phsmin);
  fInTree->SetBranchAddress("phHoE", &phHoE, &b_phHoE);
  fInTree->SetBranchAddress("phChgIso", &phChgIso, &b_phChgIso);
  fInTree->SetBranchAddress("phNeuIso", &phNeuIso, &b_phNeuIso);
  fInTree->SetBranchAddress("phPhIso", &phPhIso, &b_phPhIso);
  fInTree->SetBranchAddress("phr9", &phr9, &b_phr9);
  fInTree->SetBranchAddress("phsuisseX", &phsuisseX, &b_phsuisseX);
  fInTree->SetBranchAddress("phHoE_b", &phHoE_b, &b_phHoE_b);
  fInTree->SetBranchAddress("phsieie_b", &phsieie_b, &b_phsieie_b);
  fInTree->SetBranchAddress("phChgIso_b", &phChgIso_b, &b_phChgIso_b);
  fInTree->SetBranchAddress("phNeuIso_b", &phNeuIso_b, &b_phNeuIso_b);
  fInTree->SetBranchAddress("phPhIso_b", &phPhIso_b, &b_phPhIso_b);
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phnrhEcut", &phnrhEcut, &b_phnrhEcut);
  fInTree->SetBranchAddress("phnrhOOT", &phnrhOOT, &b_phnrhOOT);
  fInTree->SetBranchAddress("phseedE", &phseedE, &b_phseedE);
  fInTree->SetBranchAddress("phseedphi", &phseedphi, &b_phseedphi);
  fInTree->SetBranchAddress("phseedeta", &phseedeta, &b_phseedeta);
  fInTree->SetBranchAddress("phseedtime", &phseedtime, &b_phseedtime);
  fInTree->SetBranchAddress("phseedOOT", &phseedOOT, &b_phseedOOT);
}
