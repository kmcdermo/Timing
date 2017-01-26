#include"SkimRECO.hh"
#include "TSystem.h"
#include "TCanvas.h"
#include "TObject.h" 

#include <iostream>

SkimRECO::SkimRECO(TString filename, TString outdir, TString subdir, Bool_t dump, Bool_t batch,
		   Float_t jetptcut, Int_t njetscut, 
		   Float_t phptcut, 
		   Int_t phhoecut, Int_t phsieiecut, Int_t phchgisocut, Int_t phneuisocut, Int_t phisocut,
		   Float_t phsmajEBcut, Float_t phsmajEEcut, Float_t phsminEBcut, Float_t phsminEEcut, 
		   Int_t nphscut,
		   Bool_t applyEBonly, Bool_t applyEEonly) :
  fOutDir(outdir), fSubDir(subdir), fDump(dump), fBatch(batch),
  fJetPtCut(jetptcut), fNJetsCut(njetscut), 
  fPhPtCut(phptcut),
  fPhHoECut(phhoecut), fPhSieieCut(phsieiecut), fPhChgIsoCut(phchgisocut), fPhNeuIsoCut(phneuisocut), fPhIsoCut(phisocut),
  fPhSmajEBCut(phsmajEBcut), fPhSmajEECut(phsmajEEcut), fPhSminEBCut(phsminEBcut), fPhSminEECut(phsminEEcut),
  fNPhsCut(nphscut),
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
  if (gSystem->GetPathInfo(Form("%s/%s",fOutDir.Data(),fSubDir.Data()), dummyFileStat) == 1)
  {
    TString mkDir = Form("mkdir -p %s/%s",fOutDir.Data(),fSubDir.Data());
    gSystem->Exec(mkDir.Data());

    TString mkLin = Form("mkdir -p %s/%s/lin",fOutDir.Data(),fSubDir.Data());
    gSystem->Exec(mkLin.Data());

    TString mkLog = Form("mkdir -p %s/%s/log",fOutDir.Data(),fSubDir.Data());
    gSystem->Exec(mkLog.Data());
  }

  if (fDump)
  {
    // dump cuts used in the selection
    std::ofstream cutdump;
    cutdump.open(Form("%s/cuts.txt",fOutDir.Data()),std::ios_base::trunc);
    cutdump << "jet cuts: " << std::endl
	    << "  pT: " << fJetPtCut << std::endl
	    << "  nJets over pT cut: " << fNJetsCut << std::endl << std::endl;
    cutdump << "photon cuts: " << std::endl
	    << "  pT: " << fPhPtCut << std::endl
	    << "  HoE: " << fPhHoECut << std::endl
	    << "  Sieie: " << fPhSieieCut << std::endl
	    << "  ChgIso: " << fPhChgIsoCut << std::endl
	    << "  NeuIso: " << fPhNeuIsoCut << std::endl
	    << "  PhIso: " << fPhIsoCut << std::endl
	    << "  Smaj EB/EE: " << fPhSmajEBCut << " : " << fPhSmajEECut << std::endl
	    << "  Smin EB/EE: " << fPhSminEBCut << " : " << fPhSminEECut << std::endl
	    << "  nPhotons cut: " << fNPhsCut << std::endl;
    
    cutdump.close();

    // dump hists for stacking purposes
    fHistDump.open(Form("%s/histdump.txt",fOutDir.Data()),std::ios_base::trunc);
  }  

  // output file for plots
  fOutFile = new TFile(Form("%s/%s/plots.root",fOutDir.Data(),fSubDir.Data()),"UPDATE");
}

SkimRECO::~SkimRECO()
{
  delete fInTree;
  delete fInFile;

  delete fOutFile;
  if (fDump) fHistDump.close();
}

void SkimRECO::DoSkim(std::ofstream & effdump, const Bool_t doNm1, const Bool_t doTeff, const Bool_t doAn)
{
  SkimRECO::SetupPlots(doNm1,doTeff,doAn);
  SkimRECO::EventLoop(doNm1,doTeff,doAn);
  SkimRECO::OutputTH1Fs();
  SkimRECO::OutputTH2Fs();
  SkimRECO::OutputTEffs();
  SkimRECO::DumpEfficiency(effdump);
}

void SkimRECO::SetupPlots(const Bool_t doNm1, const Bool_t doTeff, const Bool_t doAn)
{
  if (doNm1)  SkimRECO::SetupNminus1();
  if (doTeff) SkimRECO::SetupTEffs();
  if (doAn)   SkimRECO::SetupAnalysis();
}

void SkimRECO::EventLoop(const Bool_t doNm1, const Bool_t doTeff, const Bool_t doAn)
{
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++)
  {
    fInTree->GetEntry(entry);

    // standard printout
    if (entry%fNEvCheck == 0 || entry == 0) std::cout << "Entry " << entry << " out of " << fInTree->GetEntries() << std::endl;

    /////////////////////////////////////
    // First assess individual objects //
    // passing the full selection      //
    /////////////////////////////////////
    // jets
    Int_t njetscut = SkimRECO::GetNJetsPt();
    
    // photons
    std::vector<Int_t> phs;
    SkimRECO::GetPhs(phs);
    
    ///////////////
    // N-1 Plots //
    ///////////////
    if (doNm1) SkimRECO::FillNminus1(njetscut,phs);
    
    ///////////////////////////
    // Full Efficiency Plots //
    ///////////////////////////

    // bool if event passed selection
    passed = (Int_t(phs.size()) >= fNPhsCut && njetscut >= fNJetsCut);
    //  if (fDump) passed &= hltdoubleph; // --> for trigger effs
    
    if (doTeff) SkimRECO::FillTEffs();
    if (passed) 
    { 
      fNPassed++; // event counter 
      if (doAn) SkimRECO::FillAnalysis(phs);
    }
  } // end of event loop
}

void SkimRECO::FillNminus1(const Int_t njetscut, const std::vector<Int_t> & phs)
{
  // Still need a few more n-1 vectors
  std::vector<Int_t> phspt, phshoe, phssieie, phschgiso, phsneuiso, phsiso, phsvid, phssmaj, phssmin;
  SkimRECO::GetPhsPt(phspt);
  SkimRECO::GetPhsHoE(phshoe);
  SkimRECO::GetPhsSieie(phssieie);
  SkimRECO::GetPhsChgIso(phschgiso);
  SkimRECO::GetPhsNeuIso(phsneuiso);
  SkimRECO::GetPhsIso(phsiso);
  SkimRECO::GetPhsVID(phsvid);
  SkimRECO::GetPhsSmaj(phssmaj);
  SkimRECO::GetPhsSmin(phssmin);

  // njets plots
  if (Int_t(phs.size()) >= fNPhsCut)
  {    
    fPlots["njets_nm1"]->Fill(njetscut);
    if (njetscut > 0) fPlots["jet0pt_nm1"]->Fill((*jetpt)[0]);
    if (njetscut > 1) fPlots["jet1pt_nm1"]->Fill((*jetpt)[1]);
    if (njetscut > 2) fPlots["jet2pt_nm1"]->Fill((*jetpt)[2]);
  }

  // photon plots
  if (njetscut >= fNJetsCut)
  { 
    fPlots["nphs_pt_nm1"]    ->Fill(phspt.size());
    fPlots["nphs_hoe_nm1"]   ->Fill(phshoe.size());
    fPlots["nphs_sieie_nm1"] ->Fill(phssieie.size());
    fPlots["nphs_chgiso_nm1"]->Fill(phschgiso.size());
    fPlots["nphs_neuiso_nm1"]->Fill(phsneuiso.size());
    fPlots["nphs_iso_nm1"]   ->Fill(phsiso.size());
    fPlots["nphs_vid_nm1"]   ->Fill(phsvid.size());
    fPlots["nphs_smaj_nm1"]  ->Fill(phssmaj.size());
    fPlots["nphs_smin_nm1"]  ->Fill(phssmin.size());

    SkimRECO::FillPhsPt(phspt);
    SkimRECO::FillPhsHoE(phshoe);
    SkimRECO::FillPhsSieie(phssieie);
    SkimRECO::FillPhsChgIso(phschgiso);
    SkimRECO::FillPhsNeuIso(phsneuiso);
    SkimRECO::FillPhsIso(phsiso);
    SkimRECO::FillPhsVID(phsvid);
    SkimRECO::FillPhsSmaj(phssmaj);
    SkimRECO::FillPhsSmin(phssmin);
  }
}

void SkimRECO::FillPhsPt(const std::vector<Int_t> & phspt)
{
  // photon pT plots
  if (Int_t(phspt.size()) > 0) fPlots["ph0pt_nm1"]->Fill((*phpt)[phspt[0]]);
  if (Int_t(phspt.size()) > 1) fPlots["ph1pt_nm1"]->Fill((*phpt)[phspt[1]]);
  if (Int_t(phspt.size()) > 2) fPlots["ph2pt_nm1"]->Fill((*phpt)[phspt[2]]);
}

void SkimRECO::FillPhsHoE(const std::vector<Int_t> & phshoe)
{
  // HoE plots
  if (Int_t(phshoe.size()) > 0) fPlots["ph0hoe_nm1"]->Fill((*phHoE_b)[phshoe[0]]);
  if (Int_t(phshoe.size()) > 1) fPlots["ph1hoe_nm1"]->Fill((*phHoE_b)[phshoe[1]]);
  if (Int_t(phshoe.size()) > 2) fPlots["ph2hoe_nm1"]->Fill((*phHoE_b)[phshoe[2]]);
}  

void SkimRECO::FillPhsSieie(const std::vector<Int_t> & phssieie)
{
  // Sieie plots
  if (Int_t(phssieie.size()) > 0) fPlots["ph0sieie_nm1"]->Fill((*phsieie_b)[phssieie[0]]);
  if (Int_t(phssieie.size()) > 1) fPlots["ph1sieie_nm1"]->Fill((*phsieie_b)[phssieie[1]]);
  if (Int_t(phssieie.size()) > 2) fPlots["ph2sieie_nm1"]->Fill((*phsieie_b)[phssieie[2]]);
}  

void SkimRECO::FillPhsChgIso(const std::vector<Int_t> & phschgiso)
{
  // ChgIso plots
  if (Int_t(phschgiso.size()) > 0) fPlots["ph0chgiso_nm1"]->Fill((*phChgIso_b)[phschgiso[0]]);
  if (Int_t(phschgiso.size()) > 1) fPlots["ph1chgiso_nm1"]->Fill((*phChgIso_b)[phschgiso[1]]);
  if (Int_t(phschgiso.size()) > 2) fPlots["ph2chgiso_nm1"]->Fill((*phChgIso_b)[phschgiso[2]]);
}  

void SkimRECO::FillPhsNeuIso(const std::vector<Int_t> & phsneuiso)
{
  // NeuIso plots
  if (Int_t(phsneuiso.size()) > 0) fPlots["ph0neuiso_nm1"]->Fill((*phNeuIso_b)[phsneuiso[0]]);
  if (Int_t(phsneuiso.size()) > 1) fPlots["ph1neuiso_nm1"]->Fill((*phNeuIso_b)[phsneuiso[1]]);
  if (Int_t(phsneuiso.size()) > 2) fPlots["ph2neuiso_nm1"]->Fill((*phNeuIso_b)[phsneuiso[2]]);
}  

void SkimRECO::FillPhsIso(const std::vector<Int_t> & phsiso)
{
  // Photon Iso plots
  if (Int_t(phsiso.size()) > 0) fPlots["ph0iso_nm1"]->Fill((*phPhIso_b)[phsiso[0]]);
  if (Int_t(phsiso.size()) > 1) fPlots["ph1iso_nm1"]->Fill((*phPhIso_b)[phsiso[1]]);
  if (Int_t(phsiso.size()) > 2) fPlots["ph2iso_nm1"]->Fill((*phPhIso_b)[phsiso[2]]);
}  

void SkimRECO::FillPhsVID(const std::vector<Int_t> & phsvid)
{
  // Photon VID plots
  if (Int_t(phsvid.size()) > 0) 
  {
    const Int_t ph0vid = SkimRECO::GetVID(phsvid[0]);
    if (ph0vid == 0) fPlots["ph0vid_nm1"]->Fill(0);
    if (ph0vid >= 1) fPlots["ph0vid_nm1"]->Fill(1);
    if (ph0vid >= 2) fPlots["ph0vid_nm1"]->Fill(2);
    if (ph0vid >= 3) fPlots["ph0vid_nm1"]->Fill(3);
  }
  if (Int_t(phsvid.size()) > 1) 
  {
    const Int_t ph1vid = SkimRECO::GetVID(phsvid[1]);
    if (ph1vid == 0) fPlots["ph1vid_nm1"]->Fill(0);
    if (ph1vid >= 1) fPlots["ph1vid_nm1"]->Fill(1);
    if (ph1vid >= 2) fPlots["ph1vid_nm1"]->Fill(2);
    if (ph1vid >= 3) fPlots["ph1vid_nm1"]->Fill(3);
  }
  if (Int_t(phsvid.size()) > 2) 
  {
    const Int_t ph2vid = SkimRECO::GetVID(phsvid[2]);
    if (ph2vid == 0) fPlots["ph2vid_nm1"]->Fill(0);
    if (ph2vid >= 1) fPlots["ph2vid_nm1"]->Fill(1);
    if (ph2vid >= 2) fPlots["ph2vid_nm1"]->Fill(2);
    if (ph2vid >= 3) fPlots["ph2vid_nm1"]->Fill(3);
  }
}

void SkimRECO::FillPhsSmaj(const std::vector<Int_t> & phssmaj)
{
  // Smajor plots
  if (Int_t(phssmaj.size()) > 0)
  {
    if   (std::abs((*phsceta)[phssmaj[0]]) < 1.4442) fPlots["ph0smajEB_nm1"]->Fill((*phsmaj)[phssmaj[0]]);    
    else                                             fPlots["ph0smajEE_nm1"]->Fill((*phsmaj)[phssmaj[0]]);    
  }
  if (Int_t(phssmaj.size()) > 1)
  {
    if   (std::abs((*phsceta)[phssmaj[1]]) < 1.4442) fPlots["ph1smajEB_nm1"]->Fill((*phsmaj)[phssmaj[1]]);    
    else                                             fPlots["ph1smajEE_nm1"]->Fill((*phsmaj)[phssmaj[1]]);    
  }
  if (Int_t(phssmaj.size()) > 2)
  {
    if   (std::abs((*phsceta)[phssmaj[2]]) < 1.4442) fPlots["ph2smajEB_nm1"]->Fill((*phsmaj)[phssmaj[2]]);    
    else                                             fPlots["ph2smajEE_nm1"]->Fill((*phsmaj)[phssmaj[2]]);    
  }
}  

void SkimRECO::FillPhsSmin(const std::vector<Int_t> & phssmin)
{
  // Sminor plots
  if (Int_t(phssmin.size()) > 0)
  {
    if   (std::abs((*phsceta)[phssmin[0]]) < 1.4442) fPlots["ph0sminEB_nm1"]->Fill((*phsmin)[phssmin[0]]);    
    else                                             fPlots["ph0sminEE_nm1"]->Fill((*phsmin)[phssmin[0]]);    
  }
  if (Int_t(phssmin.size()) > 1)
  {
    if   (std::abs((*phsceta)[phssmin[1]]) < 1.4442) fPlots["ph1sminEB_nm1"]->Fill((*phsmin)[phssmin[1]]);    
    else                                             fPlots["ph1sminEE_nm1"]->Fill((*phsmin)[phssmin[1]]);    
  }
  if (Int_t(phssmin.size()) > 2)
  {
    if   (std::abs((*phsceta)[phssmin[2]]) < 1.4442) fPlots["ph2sminEB_nm1"]->Fill((*phsmin)[phssmin[2]]);    
    else                                             fPlots["ph2sminEE_nm1"]->Fill((*phsmin)[phssmin[2]]);    
  }
}  

void SkimRECO::FillTEffs() 
{
  std::vector<Int_t> phsvid;
  SkimRECO::GetPhsVID(phsvid);

  if (phsvid.size() > 0)
  {
    const Int_t ph0vid = SkimRECO::GetVID(phsvid[0]);
    fTEffs["looseid_vs_ph0pt"] ->Fill(ph0vid>=1,(*phpt)[phsvid[0]]);
    fTEffs["mediumid_vs_ph0pt"]->Fill(ph0vid>=2,(*phpt)[phsvid[0]]);
    fTEffs["tightid_vs_ph0pt"] ->Fill(ph0vid>=3,(*phpt)[phsvid[0]]);
  }
  if (phsvid.size() > 1)
  {
    const Int_t ph1vid = SkimRECO::GetVID(phsvid[1]);
    fTEffs["looseid_vs_ph1pt"] ->Fill(ph1vid>=1,(*phpt)[phsvid[1]]);
    fTEffs["mediumid_vs_ph1pt"]->Fill(ph1vid>=2,(*phpt)[phsvid[1]]);
    fTEffs["tightid_vs_ph1pt"] ->Fill(ph1vid>=3,(*phpt)[phsvid[1]]);
  }
  if (phsvid.size() > 2)
  {
    const Int_t ph2vid = SkimRECO::GetVID(phsvid[2]);
    fTEffs["looseid_vs_ph2pt"] ->Fill(ph2vid>=1,(*phpt)[phsvid[2]]);
    fTEffs["mediumid_vs_ph2pt"]->Fill(ph2vid>=2,(*phpt)[phsvid[2]]);
    fTEffs["tightid_vs_ph2pt"] ->Fill(ph2vid>=3,(*phpt)[phsvid[2]]);
  }
}

void SkimRECO::FillAnalysis(const std::vector<Int_t> & phs) 
{
  if (Int_t(phs.size()) > 0) fPlots["ph0seedtime"]->Fill((*phseedtime)[phs[0]]);
  if (Int_t(phs.size()) > 1) fPlots["ph1seedtime"]->Fill((*phseedtime)[phs[1]]);
  if (Int_t(phs.size()) > 2) fPlots["ph2seedtime"]->Fill((*phseedtime)[phs[2]]);

  fPlots["MET"]->Fill(t1pfMETpt);
}

Int_t SkimRECO::GetNJetsPt()
{
  Int_t njetscut = 0;
  for (Int_t ijet = 0; ijet < njets; ijet++)
  {
    if ((*jetpt)[ijet] < fJetPtCut) break;
    njetscut++;
  }
  return njetscut;
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
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
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
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    // if passed all that, save the photon!
    phspt.push_back(iph);
  }
}

void SkimRECO::GetPhsHoE(std::vector<Int_t> & phshoe)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all other cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phshoe.push_back(iph);
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
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phssieie.push_back(iph);
  }
}

void SkimRECO::GetPhsChgIso(std::vector<Int_t> & phschgiso)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all other cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phschgiso.push_back(iph);
  }
}

void SkimRECO::GetPhsNeuIso(std::vector<Int_t> & phsneuiso)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all other cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phsneuiso.push_back(iph);
  }
}

void SkimRECO::GetPhsIso(std::vector<Int_t> & phsiso)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all other cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phsiso.push_back(iph);
  }
}

void SkimRECO::GetPhsVID(std::vector<Int_t> & phsvid)
{
  for (Int_t iph = 0; iph < nphotons; iph++)
  {
    // photon pre-selection
    if ((std::abs((*phsceta)[iph]) > 2.5 || (std::abs((*phsceta)[iph]) > 1.4442 && std::abs((*phsceta)[iph]) < 1.566))) continue;
    if ((*phseedE)[iph] < 1.f) continue;

    // all cuts
    if ((*phpt)[iph] < fPhPtCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;
    if ((isEB && (*phsmin)[iph] > fPhSminEBCut) || (!isEB && (*phsmin)[iph] > fPhSminEECut)) continue;

    phsvid.push_back(iph);
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
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
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
    if ((*phHoE_b)[iph] < fPhHoECut) continue;
    if ((*phsieie_b)[iph] < fPhSieieCut) continue;
    if ((*phChgIso_b)[iph] < fPhChgIsoCut) continue;
    if ((*phNeuIso_b)[iph] < fPhNeuIsoCut) continue;
    if ((*phPhIso_b)[iph] < fPhIsoCut) continue;
    const Bool_t isEB = (std::abs((*phsceta)[iph]) < 1.4442);
    if ((isEB && (*phsmaj)[iph] > fPhSmajEBCut) || (!isEB && (*phsmaj)[iph] > fPhSmajEECut)) continue;

    phssmin.push_back(iph);
  }
}

Int_t SkimRECO::GetVID(const Int_t iph)
{
  const Int_t tight = 3, medium = 2, loose = 1;

  if      ((*phHoE_b)[iph] >= tight  && (*phsieie_b)[iph] >= tight  && (*phChgIso_b)[iph] >= tight  && (*phNeuIso_b)[iph] >= tight  && (*phPhIso_b)[iph] >= tight ) return tight;
  else if ((*phHoE_b)[iph] >= medium && (*phsieie_b)[iph] >= medium && (*phChgIso_b)[iph] >= medium && (*phNeuIso_b)[iph] >= medium && (*phPhIso_b)[iph] >= medium) return medium;
  else if ((*phHoE_b)[iph] >= loose  && (*phsieie_b)[iph] >= loose  && (*phChgIso_b)[iph] >= loose  && (*phNeuIso_b)[iph] >= loose  && (*phPhIso_b)[iph] >= loose ) return loose;
  else    return 0;
}

void SkimRECO::SetupNminus1()
{
  // njets
  fPlots["njets_nm1"]  = SkimRECO::MakeTH1F("njets_nm1","nJets (N-1)",10,0,10,Form("nJets (p_{T} > %4.1f [GeV/c])",fJetPtCut),"Events");
  fPlots["jet0pt_nm1"] = SkimRECO::MakeTH1F("jet0pt_nm1","Jet p_{T} [GeV/c] (N-1)",100,0.f,1000.f,"Leading Jet p_{T} [GeV/c]","Events");
  fPlots["jet1pt_nm1"] = SkimRECO::MakeTH1F("jet1pt_nm1","Jet p_{T} [GeV/c] (N-1)",100,0.f,1000.f,"Subleading Jet p_{T} [GeV/c]","Events");
  fPlots["jet2pt_nm1"] = SkimRECO::MakeTH1F("jet2pt_nm1","Jet p_{T} [GeV/c] (N-1)",100,0.f,1000.f,"Subsubleading Jet p_{T} [GeV/c]","Events");

  // photons
  fPlots["nphs_pt_nm1"]     = SkimRECO::MakeTH1F("nphs_pt_nm1","nPhotons (N-1)",10,0,10,"nPhotons (p_{T} N-1)","Events");
  fPlots["nphs_hoe_nm1"]    = SkimRECO::MakeTH1F("nphs_hoe_nm1","nPhotons (N-1)",10,0,10,"nPhotons (HoverE VID N-1)","Events");
  fPlots["nphs_sieie_nm1"]  = SkimRECO::MakeTH1F("nphs_sieie_nm1","nPhotons (N-1)",10,0,10,"nPhotons (#sigma_{i#eta i#ieta} VID N-1)","Events");
  fPlots["nphs_chgiso_nm1"] = SkimRECO::MakeTH1F("nphs_chgiso_nm1","nPhotons (N-1)",10,0,10,"nPhotons (Charged Hadron Isolation VID N-1)","Events");
  fPlots["nphs_neuiso_nm1"] = SkimRECO::MakeTH1F("nphs_neuiso_nm1","nPhotons (N-1)",10,0,10,"nPhotons (Neutral Hadron Isolation VID N-1)","Events");
  fPlots["nphs_iso_nm1"]    = SkimRECO::MakeTH1F("nphs_iso_nm1","nPhotons (N-1)",10,0,10,"nPhotons (Photon Isolation VID N-1)","Events");
  fPlots["nphs_vid_nm1"]    = SkimRECO::MakeTH1F("nphs_vid_nm1","nPhotons (N-1)",10,0,10,"nPhotons (VID N-1)","Events");
  fPlots["nphs_smaj_nm1"]   = SkimRECO::MakeTH1F("nphs_smaj_nm1","nPhotons (N-1)",10,0,10,"nPhotons (S_{major} N-1)","Events");
  fPlots["nphs_smin_nm1"]   = SkimRECO::MakeTH1F("nphs_smin_nm1","nPhotons (N-1)",10,0,10,"nPhotons (S_{minor} N-1)","Events");

  fPlots["ph0pt_nm1"] = SkimRECO::MakeTH1F("ph0pt_nm1","Photon p_{T} [GeV/c] (N-1)",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]","Events");
  fPlots["ph1pt_nm1"] = SkimRECO::MakeTH1F("ph1pt_nm1","Photon p_{T} [GeV/c] (N-1)",100,0.f,2500.f,"Subleading Photon p_{T} [GeV/c]","Events");
  fPlots["ph2pt_nm1"] = SkimRECO::MakeTH1F("ph2pt_nm1","Photon p_{T} [GeV/c] (N-1)",100,0.f,2500.f,"Subsubleading Photon p_{T} [GeV/c]","Events");

  fPlots["ph0hoe_nm1"] = SkimRECO::MakeTH1F("ph0hoe_nm1","Photon HoverE VID (N-1)",4,0,4,"Leading Photon HoverE VID","Events");
  fPlots["ph1hoe_nm1"] = SkimRECO::MakeTH1F("ph1hoe_nm1","Photon HoverE VID (N-1)",4,0,4,"Subleading Photon HoverE VID","Events");
  fPlots["ph2hoe_nm1"] = SkimRECO::MakeTH1F("ph2hoe_nm1","Photon HoverE VID (N-1)",4,0,4,"Subsubleading Photon HoverE VID","Events");

  fPlots["ph0sieie_nm1"] = SkimRECO::MakeTH1F("ph0sieie_nm1","Photon #sigma_{i#ieta i#ieta} VID (N-1)",4,0,4,"Leading Photon #sigma_{i#ieta i#ieta} VID","Events");
  fPlots["ph1sieie_nm1"] = SkimRECO::MakeTH1F("ph1sieie_nm1","Photon #sigma_{i#ieta i#ieta} VID (N-1)",4,0,4,"Subleading Photon #sigma_{i#ieta i#ieta} VID","Events");
  fPlots["ph2sieie_nm1"] = SkimRECO::MakeTH1F("ph2sieie_nm1","Photon #sigma_{i#ieta i#ieta} VID (N-1)",4,0,4,"Subsubleading Photon #sigma_{i#ieta i#ieta} VID","Events");

  fPlots["ph0chgiso_nm1"] = SkimRECO::MakeTH1F("ph0chgiso_nm1","Photon Charged Hadron Isolation VID (N-1)",4,0,4,"Leading Photon Charged Hadron Isolation VID","Events");
  fPlots["ph1chgiso_nm1"] = SkimRECO::MakeTH1F("ph1chgiso_nm1","Photon Charged Hadron Isolation VID (N-1)",4,0,4,"Subleading Photon Charged Hadron Isolation VID","Events");
  fPlots["ph2chgiso_nm1"] = SkimRECO::MakeTH1F("ph2chgiso_nm1","Photon Charged Hadron Isolation VID (N-1)",4,0,4,"Subsubleading Photon Charged Hadron Isolation VID","Events");

  fPlots["ph0neuiso_nm1"] = SkimRECO::MakeTH1F("ph0neuiso_nm1","Photon Neutral Hadron Isolation VID (N-1)",4,0,4,"Leading Photon Neutral Hadron Isolation VID","Events");
  fPlots["ph1neuiso_nm1"] = SkimRECO::MakeTH1F("ph1neuiso_nm1","Photon Neutral Hadron Isolation VID (N-1)",4,0,4,"Subleading Photon Neutral Hadron Isolation VID","Events");
  fPlots["ph2neuiso_nm1"] = SkimRECO::MakeTH1F("ph2neuiso_nm1","Photon Neutral Hadron Isolation VID (N-1)",4,0,4,"Subsubleading Photon Neutral Hadron Isolation VID","Events");

  fPlots["ph0iso_nm1"] = SkimRECO::MakeTH1F("ph0iso_nm1","Photon Isolation VID (N-1)",4,0,4,"Leading Photon Isolation VID","Events");
  fPlots["ph1iso_nm1"] = SkimRECO::MakeTH1F("ph1iso_nm1","Photon Isolation VID (N-1)",4,0,4,"Subleading Isolation VID","Events");
  fPlots["ph2iso_nm1"] = SkimRECO::MakeTH1F("ph2iso_nm1","Photon Isolation VID (N-1)",4,0,4,"Subsubleading Photon Isolation VID","Events");

  fPlots["ph0vid_nm1"] = SkimRECO::MakeTH1F("ph0vid_nm1","Photon VID (N-1)",4,0,4,"Leading Photon VID","Events");
  fPlots["ph0vid_nm1"]->GetXaxis()->SetBinLabel(1,"Fail");
  fPlots["ph0vid_nm1"]->GetXaxis()->SetBinLabel(2,"Loose");
  fPlots["ph0vid_nm1"]->GetXaxis()->SetBinLabel(3,"Medium");
  fPlots["ph0vid_nm1"]->GetXaxis()->SetBinLabel(4,"Tight");
  fPlots["ph1vid_nm1"] = SkimRECO::MakeTH1F("ph1vid_nm1","Photon VID (N-1)",4,0,4,"Subleading Photon VID","Events");
  fPlots["ph1vid_nm1"]->GetXaxis()->SetBinLabel(1,"Fail");
  fPlots["ph1vid_nm1"]->GetXaxis()->SetBinLabel(2,"Loose");
  fPlots["ph1vid_nm1"]->GetXaxis()->SetBinLabel(3,"Medium");
  fPlots["ph1vid_nm1"]->GetXaxis()->SetBinLabel(4,"Tight");
  fPlots["ph2vid_nm1"] = SkimRECO::MakeTH1F("ph2vid_nm1","Photon VID (N-1)",4,0,4,"Subsubleading Photon VID","Events");
  fPlots["ph2vid_nm1"]->GetXaxis()->SetBinLabel(1,"Fail");
  fPlots["ph2vid_nm1"]->GetXaxis()->SetBinLabel(2,"Loose");
  fPlots["ph2vid_nm1"]->GetXaxis()->SetBinLabel(3,"Medium");
  fPlots["ph2vid_nm1"]->GetXaxis()->SetBinLabel(4,"Tight");

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

void SkimRECO::SetupTEffs() 
{
  fTEffs["looseid_vs_ph0pt"] = SkimRECO::MakeTEff("looseid_vs_ph0pt","Leading Photon Loose ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]","Loose ID Efficiency");
  fTEffs["mediumid_vs_ph0pt"] = SkimRECO::MakeTEff("mediumid_vs_ph0pt","Leading Photon Medium ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]","Medium ID Efficiency");
  fTEffs["tightid_vs_ph0pt"] = SkimRECO::MakeTEff("tightid_vs_ph0pt","Leading Photon Tight ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Leading Photon p_{T} [GeV/c]","Tight ID Efficiency");

  fTEffs["looseid_vs_ph1pt"] = SkimRECO::MakeTEff("looseid_vs_ph1pt","Subleading Photon Loose ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Subleading Photon p_{T} [GeV/c]","Loose ID Efficiency");
  fTEffs["mediumid_vs_ph1pt"] = SkimRECO::MakeTEff("mediumid_vs_ph1pt","Subleading Photon Medium ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Subleading Photon p_{T} [GeV/c]","Medium ID Efficiency");
  fTEffs["tightid_vs_ph1pt"] = SkimRECO::MakeTEff("tightid_vs_ph1pt","Subleading Photon Tight ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Subleading Photon p_{T} [GeV/c]","Tight ID Efficiency");

  fTEffs["looseid_vs_ph2pt"] = SkimRECO::MakeTEff("looseid_vs_ph2pt","Subsubleading Photon Loose ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Subsubleading Photon p_{T} [GeV/c]","Loose ID Efficiency");
  fTEffs["mediumid_vs_ph2pt"] = SkimRECO::MakeTEff("mediumid_vs_ph2pt","Subsubleading Photon Medium ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Subsubleading Photon p_{T} [GeV/c]","Medium ID Efficiency");
  fTEffs["tightid_vs_ph2pt"] = SkimRECO::MakeTEff("tightid_vs_ph2pt","Subsubleading Photon Tight ID Efficiency vs p_{T} [GeV/c]",100,0.f,2500.f,"Subsubleading Photon p_{T} [GeV/c]","Tight ID Efficiency");
}

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

TH2F * SkimRECO::MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle)
{
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
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
    if (fDump) fHistDump << mapiter->second->GetName() << std::endl;

    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas("canv","canv");
    canv->cd();
    mapiter->second->Draw("HIST");

    // first save as linear, then log
    canv->SetLogy(0);
    canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDir.Data(),mapiter->first.Data()));

    canv->SetLogy(1);
    canv->SaveAs(Form("%s/%s/log/%s.png",fOutDir.Data(),fSubDir.Data(),mapiter->first.Data()));

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
    canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDir.Data(),mapiter->first.Data()));

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
    canv->SaveAs(Form("%s/%s/lin/%s.png",fOutDir.Data(),fSubDir.Data(),mapiter->first.Data()));

    // first save as linear, then log
    canv->SetLogy(1);
    canv->SaveAs(Form("%s/%s/log/%s.png",fOutDir.Data(),fSubDir.Data(),mapiter->first.Data()));

    delete canv;
    delete mapiter->second;
  }
  fEffs.clear();
}

void SkimRECO::DumpEfficiency(std::ofstream & effdump)
{
  const Int_t nEntries = fInTree->GetEntries();
  const Float_t eff = Float_t(fNPassed)/Float_t(nEntries);
  const Float_t unc = std::sqrt((eff*(1.f-eff))/float(nEntries));

  std::cout << "nEntries: " << nEntries << " nPassed: " << fNPassed << " efficiency: " << eff << " +/- " << unc << std::endl;
  effdump << "sample: " << fSubDir.Data() << eff << " " << unc << " " << std::endl;

  if (fBatch)
  {
    TFile * file = TFile::Open(Form("output/phpt%i/phvid%i/plots.root",Int_t(fPhPtCut),fPhHoECut),"UPDATE");
    TH1F * hist = (TH1F*)file->Get(Form("eff_%s_njets%i",fSubDir.Data(),fNJetsCut));
  
    if (fJetPtCut == 0)
    {
      hist->SetBinContent(1,eff);
      hist->SetBinError(1,unc);
      hist->SetBinContent(2,eff);
      hist->SetBinError(2,unc);
      hist->SetBinContent(3,eff);
      hist->SetBinError(3,unc);
      hist->SetBinContent(4,eff);
      hist->SetBinError(4,unc);
    }
    else if (fJetPtCut == 20)
    {
      hist->SetBinContent(1,eff);
      hist->SetBinError(1,unc);
    }
    else if (fJetPtCut == 30)
    {
      hist->SetBinContent(2,eff);
      hist->SetBinError(2,unc);
    }
    else if (fJetPtCut == 40)
    {
      hist->SetBinContent(3,eff);
      hist->SetBinError(3,unc);
    }
    else if (fJetPtCut == 50)
    {
      hist->SetBinContent(4,eff);
      hist->SetBinError(4,unc);
    }
    hist->Write(hist->GetName(),TObject::kWriteDelete); 
    delete file;
  }
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
  if (fDump) fInTree->SetBranchAddress("hltdoubleph", &hltdoubleph, &b_hltdoubleph); //fDump is proxy for isData...
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
