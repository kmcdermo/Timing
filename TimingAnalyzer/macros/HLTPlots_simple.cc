#include "HLTPlots_simple.hh"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TColor.h"

#include <vector>
#include "common/common.h"
#include <iostream>

HLTPlots_simple::HLTPlots_simple()
{
  fInFile = TFile::Open("input/DATA/2017/HLT_CHECK/30_06_17/jets/hltdump.root");
  fInTree = (TTree*)fInFile->Get("tree/tree");

  HLTPlots_simple::InitTree();
}

HLTPlots_simple::~HLTPlots_simple()
{
  delete fInTree;
  delete fInFile;
}

void HLTPlots_simple::DoPlots()
{
  TFile * outfile = new TFile("hltcheck/nophoinjet.root","UPDATE");
  const Bool_t isoph = true;
  const Bool_t isidL = false;
  const Bool_t iser  = false;

  const Int_t ncuts = 10;
  
  std::vector<TH1F*> phopt(ncuts), phoeta(ncuts), ht(ncuts);
  std::vector<TH2F*> phoptvht(ncuts);

  std::vector<TString> cut(ncuts);
  cut[0] = "none";
  cut[1] = "R9";
  cut[2] = "Smaj";
  cut[3] = "Smin";
  cut[4] = "HollTkIso";
  cut[5] = "HoE";
  cut[6] = "sieie";
  cut[7] = "EcalIso";
  cut[8] = "HcalIso";
  cut[9] = "phpt";
  
  std::vector<Color_t> colors(ncuts); //= {kRed+1,kGreen+1,kBlue+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3,kBlack};

  for (Int_t i = 0; i < ncuts; i++)
  {
    colors[i] = kBlack;

    phopt[i] = new TH1F(Form("phopt_%s",cut[i].Data()),Form("Leading Photon p_{T} w/ %s cut",cut[i].Data()),100,0.f,1000.f);
    phopt[i] ->Sumw2();
    phopt[i] ->SetLineColor(colors[i]);
    phopt[i] ->SetMarkerColor(colors[i]);

    phoeta[i] = new TH1F(Form("phoeta_%s",cut[i].Data()),Form("Leading Photon #eta w/ %s cut",cut[i].Data()),30,-3.f,3.f);
    phoeta[i] ->Sumw2();
    phoeta[i] ->SetLineColor(colors[i]);
    phoeta[i] ->SetMarkerColor(colors[i]);

    ht[i] = new TH1F(Form("ht_%s",cut[i].Data()),Form("PFJet H_{T} w/ %s cut",cut[i].Data()),100,0,2000.f);
    ht[i] ->Sumw2();
    ht[i] ->SetLineColor(colors[i]);
    ht[i] ->SetMarkerColor(colors[i]);

    phoptvht[i] = new TH2F(Form("phoptvht_%s",cut[i].Data()),Form("Leading Photon p_{T} vs PFJet H_{T} w/ %s cut",cut[i].Data()),25,0.f,1000.f,50,0,2000.f);
  }

  std::vector<Bool_t> passed(ncuts);
  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {
    fInTree->GetEntry(ientry);
    HLTPlots_simple::InitPassed(passed);
  
    if (ientry%10000 == 0 || ientry == 0) std::cout << "Entry " << ientry << " out of " << fInTree->GetEntries() << std::endl;
  
    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      const Float_t pt = (*phpt)[iph];
      if(!passed[0]) 
      {
	const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	passed  [0] = true; 
	phopt   [0]->Fill((*phpt)[iph]); 
	phoeta  [0]->Fill((*pheta)[iph]); 
	ht      [0]->Fill(pfjetHT); 
	phoptvht[0]->Fill((*phpt)[iph],pfjetHT);
      }

      if((*phr9)[iph] < 0.95) continue;
      if(!passed[1]) 
      {
	const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	passed  [1] = true; 
	phopt   [1]->Fill((*phpt)[iph]); 
	phoeta  [1]->Fill((*pheta)[iph]); 
	ht      [1]->Fill(pfjetHT); 
	phoptvht[1]->Fill((*phpt)[iph],pfjetHT);
      }

      if((*phsmaj)[iph] > 1.f) continue;
      if(!passed[2]) 
      {
	const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	passed  [2] = true; 
	phopt   [2]->Fill((*phpt)[iph]); 
	phoeta  [2]->Fill((*pheta)[iph]); 
	ht      [2]->Fill(pfjetHT); 
	phoptvht[2]->Fill((*phpt)[iph],pfjetHT);
      }

      if((*phsmin)[iph] > 0.3) continue;
      if(!passed[3]) 
      {
	const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	passed  [3] = true; 
	phopt   [3]->Fill((*phpt)[iph]); 
	phoeta  [3]->Fill((*pheta)[iph]); 
	ht      [3]->Fill(pfjetHT); 
	phoptvht[3]->Fill((*phpt)[iph],pfjetHT);
      }

      if((*phHollowTkIso)[iph] > (3.f + 0.002*pt)) continue;
      if(!passed[4]) 
      {
	const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	passed  [4] = true; 
	phopt   [4]->Fill((*phpt)[iph]); 
	phoeta  [4]->Fill((*pheta)[iph]); 
	ht      [4]->Fill(pfjetHT); 
	phoptvht[4]->Fill((*phpt)[iph],pfjetHT);
      }

      if(std::abs((*phsceta)[iph]) < ECAL::etaEB)
      {
	if((*phHoE)[iph] > 0.0396) continue;
	if(!passed[5]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [5] = true; 
	  phopt   [5]->Fill((*phpt)[iph]); 
	  phoeta  [5]->Fill((*pheta)[iph]); 
	  ht      [5]->Fill(pfjetHT); 
	  phoptvht[5]->Fill((*phpt)[iph],pfjetHT);
	}

	if((*phsieie)[iph] > 0.01022) continue;
	if(!passed[6]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [6] = true; 
	  phopt   [6]->Fill((*phpt)[iph]); 
	  phoeta  [6]->Fill((*pheta)[iph]); 
	  ht      [6]->Fill(pfjetHT); 
	  phoptvht[6]->Fill((*phpt)[iph],pfjetHT);
	}

	if((*phPFClEcalIso)[iph] > (2.5 + 0.01*pt)) continue;
	if(!passed[7]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [7] = true; 
	  phopt   [7]->Fill((*phpt)[iph]); 
	  phoeta  [7]->Fill((*pheta)[iph]); 
	  ht      [7]->Fill(pfjetHT); 
	  phoptvht[7]->Fill((*phpt)[iph],pfjetHT);
	}

	if((*phPFClHcalIso)[iph] > (6.f + 0.03*pt + 0.00003*pt*pt)) continue;
	if(!passed[8]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [8] = true; 
	  phopt   [8]->Fill((*phpt)[iph]); 
	  phoeta  [8]->Fill((*pheta)[iph]); 
	  ht      [8]->Fill(pfjetHT); 
	  phoptvht[8]->Fill((*phpt)[iph],pfjetHT);
	}

	if(pt < 70.f) continue;
	if(!passed[9]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [9] = true; 
	  phopt   [9]->Fill((*phpt)[iph]); 
	  phoeta  [9]->Fill((*pheta)[iph]); 
	  ht      [9]->Fill(pfjetHT); 
	  phoptvht[9]->Fill((*phpt)[iph],pfjetHT);
	}

	if(passed[9]) break;
      } // end check over EB
      else if((std::abs((*phsceta)[iph]) > ECAL::etaEEmin) && (std::abs((*phsceta)[iph]) < ECAL::etaEEmax))
      {
	if((*phHoE)[iph] > 0.0219) continue;
	if(!passed[5]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [5] = true; 
	  phopt   [5]->Fill((*phpt)[iph]); 
	  phoeta  [5]->Fill((*pheta)[iph]); 
	  ht      [5]->Fill(pfjetHT); 
	  phoptvht[5]->Fill((*phpt)[iph],pfjetHT);
	}

	if((*phsieie)[iph] > 0.03001) continue;
	if(!passed[6]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [6] = true; 
	  phopt   [6]->Fill((*phpt)[iph]); 
	  phoeta  [6]->Fill((*pheta)[iph]); 
	  ht      [6]->Fill(pfjetHT); 
	  phoptvht[6]->Fill((*phpt)[iph],pfjetHT);
	}

	if((*phPFClEcalIso)[iph] > (4.f + 0.01*pt)) continue;
	if(!passed[7]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [7] = true; 
	  phopt   [7]->Fill((*phpt)[iph]); 
	  phoeta  [7]->Fill((*pheta)[iph]); 
	  ht      [7]->Fill(pfjetHT); 
	  phoptvht[7]->Fill((*phpt)[iph],pfjetHT);
	}

	if((*phPFClHcalIso)[iph] > (3.5 + 0.03*pt + 0.00003*pt*pt)) continue;
	if(!passed[8]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [8] = true; 
	  phopt   [8]->Fill((*phpt)[iph]); 
	  phoeta  [8]->Fill((*pheta)[iph]); 
	  ht      [8]->Fill(pfjetHT); 
	  phoptvht[8]->Fill((*phpt)[iph],pfjetHT);
	}

	if(pt < 70.f) continue;
	if(!passed[9]) 
        {
	  const Float_t pfjetHT = HLTPlots_simple::HT(isoph,iph,isidL,iser);
	  passed  [9] = true; 
	  phopt   [9]->Fill((*phpt)[iph]); 
	  phoeta  [9]->Fill((*pheta)[iph]); 
	  ht      [9]->Fill(pfjetHT); 
	  phoptvht[9]->Fill((*phpt)[iph],pfjetHT);
	}

	if(passed[9]) break;
      } // end check over EE
    } // end loop over photons
  } // end loop over events

  outfile->cd();
  for (Int_t i = 0; i < ncuts; i++)
  {
    phopt[i]->Write(phopt[i]->GetName(),TObject::kWriteDelete);
    phoeta[i]->Write(phoeta[i]->GetName(),TObject::kWriteDelete);
    ht[i]->Write(ht[i]->GetName(),TObject::kWriteDelete);
    phoptvht[i]->Write(phoptvht[i]->GetName(),TObject::kWriteDelete);

  }
  delete outfile;
}

void HLTPlots_simple::InitPassed(std::vector<Bool_t>& passed)
{
  for (auto&& pass : passed) {pass = false;}
}

Float_t HLTPlots_simple::HT(const Bool_t isoph, const Int_t iph, const Bool_t isidL, const Bool_t iser)
{
  Float_t ht = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (iser && (*jeteta)[ijet] > 3.f) continue;
    if (isoph && deltaR((*phphi)[iph],(*pheta)[iph],(*jetphi)[iph],(*jeteta)[iph]) < 0.4) continue;
    if (isidL && !(*jetidL)[ijet]) continue;
    ht += (*jetpt)[ijet];
  }
  return ht;
}

void HLTPlots_simple::InitTree()
{
  triggerBits = 0;
  jetE = 0;
  jetpt = 0;
  jetphi = 0;
  jeteta = 0;
  jetidL = 0;
  phE = 0;
  phpt = 0;
  phphi = 0;
  pheta = 0;
  phscE = 0;
  phsceta = 0;
  phscphi = 0;
  phHoE = 0;
  phr9 = 0;
  phEleVeto = 0;  
  phPixSeed = 0;
  phPFClEcalIso = 0;
  phPFClHcalIso = 0;
  phHollowTkIso = 0;
  phsieie = 0;
  phsipip = 0;
  phsieip = 0;
  phsmaj = 0;
  phsmin = 0;
  phalpha = 0;
  phIsHLTMatched = 0;
  phnrh = 0;
  phseedeta = 0;
  phseedphi = 0;
  phseedE = 0;
  phseedtime = 0;
  phseedID = 0;
  phseedOOT = 0;

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("triggerBits", &triggerBits, &b_triggerBits);
  //  fInTree->SetBranchAddress("pfjetHT", &pfjetHT, &b_pfjetHT);
  fInTree->SetBranchAddress("njets", &njets, &b_njets);
  fInTree->SetBranchAddress("jetE", &jetE, &b_jetE);
  fInTree->SetBranchAddress("jetpt", &jetpt, &b_jetpt);
  fInTree->SetBranchAddress("jetphi", &jetphi, &b_jetphi);
  fInTree->SetBranchAddress("jeteta", &jeteta, &b_jeteta);
  fInTree->SetBranchAddress("jetidL", &jetidL, &b_jetidL);
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
  fInTree->SetBranchAddress("phEleVeto", &phEleVeto, &b_phEleVeto);
  fInTree->SetBranchAddress("phPixSeed", &phPixSeed, &b_phPixSeed);
  fInTree->SetBranchAddress("phPFClEcalIso", &phPFClEcalIso, &b_phPFClEcalIso);
  fInTree->SetBranchAddress("phPFClHcalIso", &phPFClHcalIso, &b_phPFClHcalIso);
  fInTree->SetBranchAddress("phHollowTkIso", &phHollowTkIso, &b_phHollowTkIso);
  fInTree->SetBranchAddress("phsieie", &phsieie, &b_phsieie);
  fInTree->SetBranchAddress("phsipip", &phsipip, &b_phsipip);
  fInTree->SetBranchAddress("phsieip", &phsieip, &b_phsieip);
  fInTree->SetBranchAddress("phsmaj", &phsmaj, &b_phsmaj);
  fInTree->SetBranchAddress("phsmin", &phsmin, &b_phsmin);
  fInTree->SetBranchAddress("phalpha", &phalpha, &b_phalpha);
  fInTree->SetBranchAddress("phIsHLTMatched", &phIsHLTMatched, &b_phIsHLTMatched);
  fInTree->SetBranchAddress("phnrh", &phnrh, &b_phnrh);
  fInTree->SetBranchAddress("phseedeta", &phseedeta, &b_phseedeta);
  fInTree->SetBranchAddress("phseedphi", &phseedphi, &b_phseedphi);
  fInTree->SetBranchAddress("phseedE", &phseedE, &b_phseedE);
  fInTree->SetBranchAddress("phseedtime", &phseedtime, &b_phseedtime);
  fInTree->SetBranchAddress("phseedID", &phseedID, &b_phseedID);
  fInTree->SetBranchAddress("phseedOOT", &phseedOOT, &b_phseedOOT);
}
