#include "HLTDump.hh"
#include "TString.h"
#include "TColor.h"

#include <vector>
#include <iostream>

HLTDump::HLTDump(const TString infile, const TString outdir, const Bool_t isoph, const Bool_t isidL, const Bool_t iser, const Int_t psfactor) :
  fIsoPh(isoph), fIsIdL(isidL), fIsER(iser), fPSFactor(psfactor)
{
  fInFile = TFile::Open(infile.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  makeOutDir(outdir);

  TString outstring = "";
  if (fIsoPh) outstring += "_nopho";
  if (fIsIdL) outstring += "_jetIdL";
  if (fIsER)  outstring += "_jetER";

  fOutFile = new TFile(Form("%s/plots%s.root",outdir.Data(),outstring.Data()),"UPDATE");

  HLTDump::InitTree();
}

HLTDump::~HLTDump()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
}

void HLTDump::DoPlots()
{
  const Int_t ncuts = 10;
  
  TH1FVec phopt(ncuts), phoeta(ncuts), ht(ncuts), nJets(ncuts), deltaRs(ncuts);
  TH2FVec phoptvht(ncuts);

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

    nJets[i] = new TH1F(Form("nJets_%s",cut[i].Data()),Form("nJets w/ %s cut",cut[i].Data()),20,0,20);
    nJets[i] ->Sumw2();
    nJets[i] ->SetLineColor(colors[i]);
    nJets[i] ->SetMarkerColor(colors[i]);

    deltaRs[i] = new TH1F(Form("deltaRs_%s",cut[i].Data()),Form("#DeltaR(#gamma,jets) w/ %s cut",cut[i].Data()),100,0.f,6.3f);
    deltaRs[i] ->Sumw2();
    deltaRs[i] ->SetLineColor(colors[i]);
    deltaRs[i] ->SetMarkerColor(colors[i]);

    phoptvht[i] = new TH2F(Form("phoptvht_%s",cut[i].Data()),Form("Leading Photon p_{T} vs PFJet H_{T} w/ %s cut",cut[i].Data()),25,0.f,1000.f,50,0,2000.f);
  }

  BoolVec passed(ncuts);
  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {
    if (ientry % fPSFactor != 0) continue;

    fInTree->GetEntry(ientry);
    HLTDump::InitPassed(passed);
  
    if (ientry%100000 == 0 || ientry == 0) std::cout << "Entry " << ientry << " out of " << fInTree->GetEntries() << std::endl;
  
    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      const Float_t pt = (*phpt)[iph];
      if (!passed[0]) {HLTDump::FillPlots(0,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

      if ((*phr9)[iph] < 0.95) continue;
      if (!passed[1]) {HLTDump::FillPlots(1,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

      if ((*phsmaj)[iph] > 1.f) continue;
      if (!passed[2]) {HLTDump::FillPlots(2,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

      if ((*phsmin)[iph] > 0.3) continue;
      if (!passed[3]) {HLTDump::FillPlots(3,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

      if ((*phHollowTkIso)[iph] > (3.f + 0.002*pt)) continue;
      if (!passed[4]) {HLTDump::FillPlots(4,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

      if (std::abs((*phsceta)[iph]) < ECAL::etaEB)
      {
	if ((*phHoE)[iph] > 0.0396) continue;
	if (!passed[5]) {HLTDump::FillPlots(5,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if ((*phsieie)[iph] > 0.01022) continue;
	if (!passed[6]) {HLTDump::FillPlots(6,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if ((*phPFClEcalIso)[iph] > (2.5 + 0.01*pt)) continue;
	if (!passed[7]) {HLTDump::FillPlots(7,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if ((*phPFClHcalIso)[iph] > (6.f + 0.03*pt + 0.00003*pt*pt)) continue;
	if (!passed[8]) {HLTDump::FillPlots(8,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if (pt < 70.f) continue;
	if (!passed[9]) {HLTDump::FillPlots(9,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if (passed[9]) break;
      } // end check over EB
      else if ((std::abs((*phsceta)[iph]) > ECAL::etaEEmin) && (std::abs((*phsceta)[iph]) < ECAL::etaEEmax))
      {
	if ((*phHoE)[iph] > 0.0219) continue;
	if (!passed[5]) {HLTDump::FillPlots(5,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if ((*phsieie)[iph] > 0.03001) continue;
	if (!passed[6]) {HLTDump::FillPlots(6,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if ((*phPFClEcalIso)[iph] > (4.f + 0.01*pt)) continue;
	if (!passed[7]) {HLTDump::FillPlots(7,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if ((*phPFClHcalIso)[iph] > (3.5 + 0.03*pt + 0.00003*pt*pt)) continue;
	if (!passed[8]) {HLTDump::FillPlots(8,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if (pt < 70.f) continue;
	if (!passed[9]) {HLTDump::FillPlots(9,iph,passed,phopt,phoeta,ht,nJets,deltaRs,phoptvht);}

	if (passed[9]) break;
      } // end check over EE
    } // end loop over photons
  } // end loop over events

  fOutFile->cd();
  for (Int_t i = 0; i < ncuts; i++)
  {
    phopt[i]->Write(phopt[i]->GetName(),TObject::kWriteDelete);
    phoeta[i]->Write(phoeta[i]->GetName(),TObject::kWriteDelete);
    ht[i]->Write(ht[i]->GetName(),TObject::kWriteDelete);
    nJets[i]->Write(nJets[i]->GetName(),TObject::kWriteDelete);
    phoptvht[i]->Write(phoptvht[i]->GetName(),TObject::kWriteDelete);
  }
}

void HLTDump::InitPassed(std::vector<Bool_t>& passed)
{
  for (auto&& pass : passed) {pass = false;}
}

void HLTDump::HT(const Int_t iph, JetInfo & jetinfo)
{
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (fIsER && (*jeteta)[ijet] > 3.f) continue;
    if (fIsIdL && !(*jetidL)[ijet]) continue;
    const Float_t dR = deltaR((*phphi)[iph],(*pheta)[iph],(*jetphi)[iph],(*jeteta)[iph]);
    if (fIsoPh && dR < 0.4) continue;
    jetinfo.pfjetHT += (*jetpt)[ijet];
    jetinfo.nJets++;
    jetinfo.dRs.push_back(dR);
  }
}

void HLTDump::FillPlots(const Int_t i, const Int_t iph, BoolVec& passed, TH1FVec& phopt, TH1FVec& phoeta, TH1FVec& ht, TH1FVec& nJets, TH1FVec& deltaRs, TH2FVec& phoptvht)
{
  JetInfo jetinfo;
  HLTDump::HT(iph,jetinfo);
  passed  [i] = true;
  phopt   [i]->Fill((*phpt)[iph]);
  phoeta  [i]->Fill((*pheta)[iph]);
  ht      [i]->Fill(jetinfo.pfjetHT);
  nJets   [i]->Fill(jetinfo.nJets);
  for (auto& dR : jetinfo.dRs) deltaRs[i]->Fill(dR); 
  phoptvht[i]->Fill((*phpt)[iph],jetinfo.pfjetHT);
}

void HLTDump::InitTree()
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
