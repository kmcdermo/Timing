#include "HLTPlots.hh"
#include "TCanvas.h"
#include "TEfficiency.h"

#include "common/common.h"
#include <iostream>

HLTPlots::HLTPlots()
{
  fInFile = TFile::Open("input/DATA/HLT/hltdump.root");
  fInTree = (TTree*)fInFile->Get("tree/tree");

  HLTPlots::InitTree();
}

HLTPlots::~HLTPlots()
{
  delete fInTree;
  delete fInFile;
}

void HLTPlots::DoPlots()
{
  TEfficiency * pteffEB = new TEfficiency("pteffEB","Leading Photon Efficiency (EB);Photon p_{T};Efficiency",100,0,1000);
  TEfficiency * pteffEE = new TEfficiency("pteffEE","Leading Photon Efficiency (EE);Photon p_{T};Efficiency",100,0,1000);

  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {
    fInTree->GetEntry(ientry);

    const Int_t iph = 0;
    const Float_t pt = (*phpt)[iph];

    if (pfjetHT < 400.f) continue;
    if (!(*phIsHLTMatched)[iph][0]) continue;
    //    if (pt < 70.f) continue;
    if ((*phr9)[iph] < 0.95) continue;
    if ((*phsmaj)[iph] > 1.f) continue;
    if ((*phsmin)[iph] > 0.3) continue;
    if ((*phHollowTkIso)[iph] > (3.f + 0.002*pt)) continue;

    if ((*phsceta)[iph] < ECAL::etaEB)
    {
      if ((*phHoE)[iph] > 0.0396) continue;
      if ((*phsieie)[iph] > 0.01022) continue;
      if ((*phPFClEcalIso)[iph] > (2.5 + 0.01*pt)) continue;
      if ((*phPFClHcalIso)[iph] > (6.f + 0.03*pt + 0.00003*pt*pt)) continue;
      
      pteffEB->Fill((*triggerBits)[0],pt);
    }
    else if (((*phsceta)[iph] > ECAL::etaEEmin) && ((*phsceta)[iph] < ECAL::etaEEmax))
    {
      if ((*phHoE)[iph] > 0.0219) continue;
      if ((*phsieie)[iph] > 0.03001) continue;
      if ((*phPFClEcalIso)[iph] > (4.f + 0.01*pt)) continue;
      if ((*phPFClHcalIso)[iph] > (3.5 + 0.03*pt + 0.00003*pt*pt)) continue;

      pteffEE->Fill((*triggerBits)[0],pt);
    }
  }

  TCanvas * canv = new TCanvas();
  canv->cd();

  pteffEB->Draw("AP");
  canv->SaveAs("pteffEB.png");

  pteffEE->Draw("AP");
  canv->SaveAs("pteffEE.png");

  delete canv;
  delete pteffEE;
  delete pteffEB;
}

void HLTPlots::InitTree()
{
  triggerBits = 0;
  trigobjE = 0;
  trigobjeta = 0;
  trigobjphi = 0;
  trigobjpt = 0;
  phE = 0;
  phpt = 0;
  phphi = 0;
  pheta = 0;
  phscE = 0;
  phsceta = 0;
  phscphi = 0;
  phHoE = 0;
  phr9 = 0;
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
  fInTree->SetBranchAddress("pfjetHT", &pfjetHT, &b_pfjetHT);
  fInTree->SetBranchAddress("trigobjE", &trigobjE, &b_trigobjE);
  fInTree->SetBranchAddress("trigobjeta", &trigobjeta, &b_trigobjeta);
  fInTree->SetBranchAddress("trigobjphi", &trigobjphi, &b_trigobjphi);
  fInTree->SetBranchAddress("trigobjpt", &trigobjpt, &b_trigobjpt);
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
