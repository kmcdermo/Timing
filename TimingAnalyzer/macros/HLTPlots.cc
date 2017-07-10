#include "HLTPlots.hh"
#include "TCanvas.h"

#include "common/common.h"
#include <iostream>

HLTPlots::HLTPlots(const TString infile, const TString outdir, const Bool_t isoph, const Bool_t isidL, const Bool_t iser, 
		   const Bool_t applyht, const Float_t htcut, const Bool_t applyphdenom, const Bool_t applyphpt) :
  fIsoPh(isoph), fIsIdL(isidL), fIsER(iser), fApplyHT(applyht), fHTCut(htcut), fApplyPhDenom(applyphdenom), fApplyPhPt(applyphpt)
{
  fInFile = TFile::Open(infile.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");

  TString outstring = "cuts";
  if (fApplyHT) outstring += Form("_htcut_%i",Int_t(fHTCut));
  if (fIsoPh) outstring += "_nopho";
  if (fIsIdL) outstring += "_jetIdL";
  if (fIsER)  outstring += "_jetER";
  if (fApplyPhDenom) outstring += "_phden";
  if (fApplyPhPt) outstring += "_phpt";
  
  fOutDir = outdir+"/"+outstring;
  makeOutDir(fOutDir);

  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");

  HLTPlots::InitTree();
}

HLTPlots::~HLTPlots()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
}

void HLTPlots::DoPlots()
{
  const Int_t nbinsx = 22;
  Double_t xbins[nbinsx+1] = {0,10,20,30,40,45,50,52,54,56,58,60,62,64,66,68,70,75,80,100,200,500,1000};
  
  TEfficiency * effptEB = new TEfficiency("effptEB","HLT Efficiency vs Leading Photon p_{T} [EB];Photon Offline p_{T};Efficiency",nbinsx,xbins);
  TEfficiency * effptEE = new TEfficiency("effptEE","HLT Efficiency vs Leading Photon p_{T} [EE];Photon Offline p_{T};Efficiency",nbinsx,xbins);
  TEfficiency * effeta = new TEfficiency("effeta","HLT Efficiency vs Leading Photon #eta;Photon Offline #eta;Efficiency",30,-3.f,3.f);
  TEfficiency * effphi = new TEfficiency("effphi","HLT Efficiency vs Leading Photon #phi;Photon Offline #phi;Efficiency",32,-3.2f,3.2f);
  TEfficiency * efftime = new TEfficiency("efftime","HLT Efficiency vs Leading Photon Seed Time [ns];Photon Offline Seed Time [ns];Efficiency",100,-25.,25.);
  TEfficiency * effHT = new TEfficiency("effHT","HLT Efficiency vs PF H_{T};Offline PF H_{T} (Min PFJet p_{T} > 15);Efficiency",100,0,2000.f);

  const Int_t idenom = 1;
  const Int_t inumer = 2;
  const Int_t ilast  = 9;
  const Float_t ph1pt = 65.f;

  for (UInt_t ientry = 0; ientry < fInTree->GetEntries(); ientry++)
  {
    if (ientry%100000 == 0 || ientry == 0) std::cout << "Entry " << ientry << " out of " << fInTree->GetEntries() << std::endl;

    fInTree->GetEntry(ientry);

    Int_t goodpho = -1;
    for (Int_t iph = 0; iph < nphotons; iph++)
    {
      if (fApplyPhDenom && !(*phIsHLTMatched)[iph][idenom]) continue; 

      const Float_t pt = (*phpt)[iph];
      if (fApplyPhPt && pt > ph1pt) continue;

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

	if (fApplyHT)
	{
	  JetInfo jetinfo;
	  HLTPlots::HT(iph,jetinfo);
	  if (jetinfo.pfjetHT < fHTCut) continue;
	}

	goodpho = iph;
	break;
      } // end check over EB
      else if ((std::abs((*phsceta)[iph]) > ECAL::etaEEmin) && (std::abs((*phsceta)[iph]) < ECAL::etaEEmax))
      {
	if ((*phHoE)[iph] > 0.0219) continue;
	if ((*phsieie)[iph] > 0.03001) continue;
	if ((*phPFClEcalIso)[iph] > (4.f + 0.01*pt)) continue;
	if ((*phPFClHcalIso)[iph] > (3.5 + 0.03*pt + 0.00003*pt*pt)) continue;
	
	if (fApplyHT)
	{
	  JetInfo jetinfo;
	  HLTPlots::HT(iph,jetinfo);
	  if (jetinfo.pfjetHT < fHTCut) continue;
	}

	goodpho = iph;
	break;
      } // end check over EE
    } // end loop over photons

    if (goodpho < 0) continue;
    
    const Bool_t passed = (fApplyPhDenom ? (*phIsHLTMatched)[goodpho][inumer] : (*triggerBits)[0]);

    if (std::abs((*phsceta)[goodpho]) < ECAL::etaEB)
    {
      effptEB->Fill(passed,(*phpt)[goodpho]);
    }
    else if ((std::abs((*phsceta)[goodpho]) > ECAL::etaEEmin) && (std::abs((*phsceta)[goodpho]) < ECAL::etaEEmax))
    {
      effptEE->Fill(passed,(*phpt)[goodpho]);
    }

    effeta->Fill(passed,(*phsceta)[goodpho]);
    effphi->Fill(passed,(*phscphi)[goodpho]);
    efftime->Fill(passed,(*phseedtime)[goodpho]);

    JetInfo jetinfo;
    HLTPlots::HT(goodpho,jetinfo);
    effHT->Fill(passed,jetinfo.pfjetHT);
  } // end loop over events

  HLTPlots::OutputEfficiency(effptEB,"hptEB");
  HLTPlots::OutputEfficiency(effptEE,"hptEE");
  HLTPlots::OutputEfficiency(effeta,"heta");
  HLTPlots::OutputEfficiency(effphi,"hphi");
  HLTPlots::OutputEfficiency(efftime,"htime");
  HLTPlots::OutputEfficiency(effHT,"hHT");

  delete effptEB;
  delete effptEE;
  delete effeta;
  delete effphi;
  delete efftime;
  delete effHT;
}

void HLTPlots::HT(const Int_t iph, JetInfo & jetinfo)
{
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if (fIsER && (*jeteta)[ijet] > 3.f) continue;
    if (fIsoPh && deltaR((*phphi)[iph],(*pheta)[iph],(*jetphi)[iph],(*jeteta)[iph]) < 0.4) continue;
    if (fIsIdL && !(*jetidL)[ijet]) continue;
    jetinfo.pfjetHT += (*jetpt)[ijet];
    jetinfo.nJets++;
  }
}

void HLTPlots::OutputEfficiency(TEfficiency *& teff, const TString outname)
{
  fOutFile->cd();
  teff->Write(teff->GetName(),TObject::kWriteDelete);

  TCanvas * canv = new TCanvas();
  canv->cd();

  TString toreplace = "HLT Efficiency vs ";
  Ssiz_t length = toreplace.Length();

  teff->Draw("AP");
  canv->SaveAs(Form("%s/%s.png",fOutDir.Data(),teff->GetName()));
  TH1F * hist = (TH1F*)teff->GetCopyTotalHisto();
  TString histtitle = hist->GetTitle();
  hist->SetTitle(histtitle.Remove(histtitle.Index(toreplace),length));
  hist->GetYaxis()->SetTitle("nEvents");
  hist->Draw("EP");
  canv->SaveAs(Form("%s/%s.png",fOutDir.Data(),outname.Data()));

  delete canv;
  delete hist;
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
