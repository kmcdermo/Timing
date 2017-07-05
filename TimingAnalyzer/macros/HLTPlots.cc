#include "HLTPlots.hh"
#include "TCanvas.h"
#include "TH1F.h"
#include "TEfficiency.h"

#include "common/common.h"
#include <iostream>

HLTPlots::HLTPlots()
{
  fInFile = TFile::Open("input/DATA/2017/HLT_CHECK/30_06_17/jets/hltdump.root");
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
  TEfficiency * effptEB = new TEfficiency("pteffEB","HLT Efficiency vs Leading Photon p_{T} [EB];Photon Offline p_{T};Efficiency",100,0,1000);
  TEfficiency * effptEE = new TEfficiency("pteffEE","HLT Efficiency vs Leading Photon p_{T} [EE];Photon Offline p_{T};Efficiency",100,0,1000);

  TEfficiency * effeta = new TEfficiency("effeta","HLT Efficiency vs Leading Photon #eta;Photon Offline #eta;Efficiency",30,-3.f,3.f);
  TEfficiency * effphi = new TEfficiency("effeta","HLT Efficiency vs Leading Photon #phi;Photon Offline #phi;Efficiency",32,-3.2f,3.2f);

  TEfficiency * efftime = new TEfficiency("efftime","HLT Efficiency vs Leading Photon Seed Time [ns];Photon Offline Seed Time [ns];Efficiency",100,-25.,25.);

  TEfficiency * effHT = new TEfficiency("effHT","HLT Efficiency vs PF H_{T};Offline PF H_{T} (Min PFJet p_{T} > 15);Efficiency",100,0,2000.f);

  const Float_t htcut = 400.f;
  const Bool_t isoph = true;
  const Bool_t isidL = false;
  const Bool_t iser  = false;
  
  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {
    if (ientry%10000 == 0 || ientry == 0) std::cout << "Entry " << ientry << " out of " << fInTree->GetEntries() << std::endl;

    fInTree->GetEntry(ientry);

    Int_t goodpho = -1;
    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      if (!(*phIsHLTMatched)[iph][1]) continue; // [0] == hollowtrk cut, [1] == displacedIdcut
      const Float_t pt = (*phpt)[iph];

      if ((*phr9)[iph] < 0.95) continue;
      if ((*phsmaj)[iph] > 1.f) continue;
      if ((*phsmin)[iph] > 0.3) continue;
      if ((*phHollowTkIso)[iph] > (3.f + 0.002*pt)) continue;

      // track veto 
      //if (!(*phEleVeto)[iph]) continue;
      //if ((*phPixSeed)[iph]) continue;

      if (std::abs((*phsceta)[iph]) < ECAL::etaEB)
      {
	if ((*phHoE)[iph] > 0.0396) continue;
	if ((*phsieie)[iph] > 0.01022) continue;
	if ((*phPFClEcalIso)[iph] > (2.5 + 0.01*pt)) continue;
	if ((*phPFClHcalIso)[iph] > (6.f + 0.03*pt + 0.00003*pt*pt)) continue;

	if (pt > 70.f) goodpho = iph;

	const Float_t pfjetHT = HLTPlots::HT(isoph,iph,isidL,iser);
	if (pfjetHT < htcut) continue;

	effptEB->Fill((*triggerBits)[0],pt);
	if (goodpho == iph)
        {
	  const Float_t pfjetHT70 = HLTPlots::HT(isoph,iph,isidL,iser);
	  if (pfjetHT70 < htcut) 
	  {
	    effeta->Fill((*triggerBits)[0],(*phsceta)[iph]);
	    effphi->Fill((*triggerBits)[0],(*phscphi)[iph]);
	    efftime->Fill((*triggerBits)[0],(*phseedtime)[iph]);
	  } // end ht check
	} // end check on good pho
	break;
      } // end check over EB
      else if ((std::abs((*phsceta)[iph]) > ECAL::etaEEmin) && (std::abs((*phsceta)[iph]) < ECAL::etaEEmax))
      {
	if ((*phHoE)[iph] > 0.0219) continue;
	if ((*phsieie)[iph] > 0.03001) continue;
	if ((*phPFClEcalIso)[iph] > (4.f + 0.01*pt)) continue;
	if ((*phPFClHcalIso)[iph] > (3.5 + 0.03*pt + 0.00003*pt*pt)) continue;

	if (pt > 70.f) goodpho = iph;
	
	const Float_t pfjetHT = HLTPlots::HT(isoph,iph,isidL,iser);
	if (pfjetHT < htcut) continue;

	effptEE->Fill((*triggerBits)[0],pt);
	if (goodpho == iph) 
	{
	  const Float_t pfjetHT70 = HLTPlots::HT(isoph,iph,isidL,iser);
	  if (pfjetHT70 < htcut) 
	  {
	    effeta->Fill((*triggerBits)[0],(*phsceta)[iph]);
	    effphi->Fill((*triggerBits)[0],(*phscphi)[iph]);
	    efftime->Fill((*triggerBits)[0],(*phseedtime)[iph]);
	  } // end ht check
	} // end check on good pho
	break;
      } // end check over EE
    } // end loop over photons

    if (goodpho < 0) continue;

    const Float_t pfjetHT70 = HLTPlots::HT(isoph,goodpho,isidL,iser);
    effHT->Fill((*triggerBits)[0],pfjetHT70);
  } // end loop over events

  TCanvas * canv = new TCanvas();
  canv->cd();

  TString toreplace = "HLT Efficiency vs ";
  Ssiz_t length = toreplace.Length();

  effptEB->Draw("AP");
  canv->SaveAs("effptEB.png");
  TH1F * hptEB = (TH1F*)effptEB->GetCopyTotalHisto();
  TString hptEBtitle = hptEB->GetTitle();
  hptEB->SetTitle(hptEBtitle.Remove(hptEBtitle.Index(toreplace),length));
  hptEB->GetYaxis()->SetTitle("nEvents");
  hptEB->Draw("EP");
  canv->SaveAs("hptEB.png");

  effptEE->Draw("AP");
  canv->SaveAs("effptEE.png");
  TH1F * hptEE = (TH1F*)effptEE->GetCopyTotalHisto();
  TString hptEEtitle = hptEE->GetTitle();
  hptEE->SetTitle(hptEEtitle.Remove(hptEEtitle.Index(toreplace),length));
  hptEE->GetYaxis()->SetTitle("nEvents");
  hptEE->Draw("EP");
  canv->SaveAs("hptEE.png");

  effeta->Draw("AP");
  canv->SaveAs("effeta.png");
  TH1F * heta  = (TH1F*)effeta ->GetCopyTotalHisto();
  TString hetatitle = heta->GetTitle();
  heta->SetTitle(hetatitle.Remove(hetatitle.Index(toreplace),length));
  heta->GetYaxis()->SetTitle("nEvents");
  heta->Draw("EP");
  canv->SaveAs("heta.png");

  effphi->Draw("AP");
  canv->SaveAs("effphi.png");
  TH1F * hphi  = (TH1F*)effphi ->GetCopyTotalHisto();
  TString hphititle = hphi->GetTitle();
  hphi->SetTitle(hphititle.Remove(hphititle.Index(toreplace),length));
  hphi->GetYaxis()->SetTitle("nEvents");
  hphi->Draw("EP");
  canv->SaveAs("hphi.png");

  efftime->Draw("AP");
  canv->SaveAs("efftime.png");
  TH1F * htime  = (TH1F*)efftime ->GetCopyTotalHisto();
  TString htimetitle = htime->GetTitle();
  htime->SetTitle(htimetitle.Remove(htimetitle.Index(toreplace),length));
  htime->GetYaxis()->SetTitle("nEvents");
  htime->Draw("EP");
  canv->SaveAs("htime.png");

  effHT->Draw("AP");
  canv->SaveAs("effHT.png");
  TH1F * hHT  = (TH1F*)effHT ->GetCopyTotalHisto();
  TString hHTtitle = hHT->GetTitle();
  hHT->SetTitle(hHTtitle.Remove(hHTtitle.Index(toreplace),length));
  hHT->GetYaxis()->SetTitle("nEvents");
  hHT->Draw("EP");
  canv->SaveAs("hHT.png");

  delete canv;
  delete effptEB;
  delete effptEE;
  delete effeta;
  delete effphi;
  delete efftime;
  delete effHT;
  delete hptEB;
  delete hptEE;
  delete heta;
  delete hphi;
  delete htime;
  delete hHT;
}

Float_t HLTPlots::HT(const Bool_t isoph, const Int_t iph, const Bool_t isidL, const Bool_t iser)
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

void HLTPlots::InitTree()
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
