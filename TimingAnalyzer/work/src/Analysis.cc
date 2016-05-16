#include "../interface/Analysis.hh"
#include "../interface/Common.hh"

#include "TCanvas.h"

#include <iostream>

Analysis::Analysis(TString infilename, TString outdir, TString outtype, Float_t lumi) : 
  fOutDir(outdir), fOutType(outtype), fLumi(lumi) {
  
  fInFile = TFile::Open(infilename.Data());
  fInTree = (TTree*)fInFile->Get("tree/tree");
  InitTree();

  // boilerplate tdr style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  // output stuff
  MakeOutDir(fOutDir);
  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"RECREATE");
}

Analysis::~Analysis(){
  delete fTDRStyle;
  delete fInTree;
  delete fInFile;
  delete fOutFile;
}

void Analysis::TimeResPlots(){
  // Set up hists first --> first in map is histo name, by design!
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot
  trTH1Map["zeemass"]  = Analysis::MakeTH1Plot("zeemass","",100,75.,105.,"Dielectron invariant mass [GeV/c^{2}]","Events",trTH1SubMap,"basics");
  trTH1Map["el1time"]  = Analysis::MakeTH1Plot("el1time","",100,-3.0,3.0,"Leading Electron Seed Time [ns]","Events",trTH1SubMap,"timing");
  trTH1Map["el2time"]  = Analysis::MakeTH1Plot("el2time","",100,-3.0,3.0,"Subleading Electron Seed Time [ns]","Events",trTH1SubMap,"timing");
  trTH1Map["timediff"] = Analysis::MakeTH1Plot("timediff","",100,-3.0,3.0,"Dielectron Seed Time Difference [ns]","Events",trTH1SubMap,"timing");

  TH2Map  trTH2Map;
  TStrMap trTH2SubMap; // set inside MakeTH2Plot
  trTH2Map["el1time_pt"]  = Analysis::MakeTH2Plot("el1time_pt","",100,-3.0,3.0,100,0,100,"Leading Electron Seed Time [ns]","p_{T} [GeV/c]",trTH2SubMap,"timing");
  trTH2Map["el1time_phi"] = Analysis::MakeTH2Plot("el1time_phi","",100,-3.0,3.0,100,-3.2,3.2,"Leading Electron Seed Time [ns]","#phi",trTH2SubMap,"timing");
  trTH2Map["el1time_eta"] = Analysis::MakeTH2Plot("el1time_eta","",100,-3.0,3.0,100,-3.2,3.2,"Leading Electron Seed Time [ns]","#eta",trTH2SubMap,"timing");

  Analysis::MakeSubDirs(trTH1SubMap);
  Analysis::MakeSubDirs(trTH2SubMap);

  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++){
    fInTree->GetEntry(entry);
    if ( (zeemass>76. && zeemass<105.) ){ // we want di-electron z's
      // standard "validation" plots
      trTH1Map["zeemass"]->Fill(zeemass);
      trTH1Map["el1time"]->Fill(el1time);
      //      trTH1Map["el2time"]->Fill(el2time);
      //      trTH1Map["timediff"]->Fill(el1time-el2time);
      trTH2Map["el1time_pt"]->Fill(el1time,el1pt);
      trTH2Map["el1time_phi"]->Fill(el1time,el1phi);
      trTH2Map["el1time_eta"]->Fill(el1time,el1eta);
    }
  }

  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);

  Analysis::SaveTH2s(trTH2Map,trTH2SubMap);
  Analysis::DeleteTH2s(trTH2Map);
}

TH1F * Analysis::MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) {
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbins,xlow,xhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

TH2F * Analysis::MakeTH2Plot(TString hname, TString htitle, Int_t nbinsx, Double_t xlow, Double_t xhigh, Int_t nbinsy, Double_t ylow, Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) {
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

void Analysis::MakeSubDirs(TStrMap subdirmap){
  for (TStrMapIter mapiter = subdirmap.begin(); mapiter != subdirmap.end(); mapiter++) { 
    MakeOutDir(Form("%s/%s",fOutDir.Data(),(*mapiter).second.Data()));
    MakeOutDir(Form("%s/%s/lin/",fOutDir.Data(),(*mapiter).second.Data()));
    MakeOutDir(Form("%s/%s/log/",fOutDir.Data(),(*mapiter).second.Data()));
  }
}

void Analysis::SaveTH1s(TH1Map th1map, TStrMap subdirmap){
  fOutFile->cd();

  TCanvas * canv = new TCanvas();
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    
    (*mapiter).second->Write(); // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    canv->cd();
    (*mapiter).second->Draw("PE");
    
    // first save as linear, then log
    canv->SetLogy(0);
    CMSLumi(canv, fLumi, "Preliminary",0);
    canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),fOutType.Data()));

    canv->SetLogy(1);
    CMSLumi(canv, fLumi, "Preliminary",0);
    canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),fOutType.Data()));
  }

  delete canv;
}

void Analysis::SaveTH2s(TH2Map th2map, TStrMap subdirmap){
  fOutFile->cd();

  TCanvas * canv = new TCanvas();
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); mapiter++) { 
    
    (*mapiter).second->Write(); // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    canv->cd();
    (*mapiter).second->Draw("colz");
    
    // only save as linear
    canv->SetLogy(0);
    CMSLumi(canv, fLumi, "Preliminary",0);
    canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),fOutType.Data()));
  }

  delete canv;
}

void Analysis::DeleteTH1s(TH1Map th1map) {
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    delete ((*mapiter).second);
  }
  th1map.clear();
}

void Analysis::DeleteTH2s(TH2Map th2map) {
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); mapiter++) { 
    delete ((*mapiter).second);
  }
  th2map.clear();
}

void Analysis::InitTree(){
  // Set branch addresses and branch pointers

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("xsec", &xsec, &b_xsec);
  fInTree->SetBranchAddress("wgt", &wgt, &b_wgt);
  fInTree->SetBranchAddress("pswgt", &pswgt, &b_pswgt);
  fInTree->SetBranchAddress("puwgt", &puwgt, &b_puwgt);
  fInTree->SetBranchAddress("puobs", &puobs, &b_puobs);
  fInTree->SetBranchAddress("putrue", &putrue, &b_putrue);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("hltphoton165", &hltphoton165, &b_hltphoton165);
  fInTree->SetBranchAddress("hltphoton175", &hltphoton175, &b_hltphoton175);
  fInTree->SetBranchAddress("hltphoton120", &hltphoton120, &b_hltphoton120);
  fInTree->SetBranchAddress("hltdoubleel", &hltdoubleel, &b_hltdoubleel);
  fInTree->SetBranchAddress("hltsingleel", &hltsingleel, &b_hltsingleel);
  fInTree->SetBranchAddress("nelectrons", &nelectrons, &b_nelectrons);
  fInTree->SetBranchAddress("ntightelectrons", &ntightelectrons, &b_ntightelectrons);
  fInTree->SetBranchAddress("nheepelectrons", &nheepelectrons, &b_nheepelectrons);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fInTree->SetBranchAddress("el1pid", &el1pid, &b_el1pid);
  fInTree->SetBranchAddress("el1pt", &el1pt, &b_el1pt);
  fInTree->SetBranchAddress("el1eta", &el1eta, &b_el1eta);
  fInTree->SetBranchAddress("el1phi", &el1phi, &b_el1phi);
  fInTree->SetBranchAddress("el1id", &el1id, &b_el1id);
  fInTree->SetBranchAddress("el1idl", &el1idl, &b_el1idl);
  fInTree->SetBranchAddress("el2pid", &el2pid, &b_el2pid);
  fInTree->SetBranchAddress("el2pt", &el2pt, &b_el2pt);
  fInTree->SetBranchAddress("el2eta", &el2eta, &b_el2eta);
  fInTree->SetBranchAddress("el2phi", &el2phi, &b_el2phi);
  fInTree->SetBranchAddress("el2id", &el2id, &b_el2id);
  fInTree->SetBranchAddress("el2idl", &el2idl, &b_el2idl);
  fInTree->SetBranchAddress("el1time", &el1time, &b_el1time);
  //  fInTree->SetBranchAddress("el2time", &el2time, &b_el2time);
  fInTree->SetBranchAddress("zeemass", &zeemass, &b_zeemass);
  fInTree->SetBranchAddress("zeept", &zeept, &b_zeeept);
  fInTree->SetBranchAddress("zeeeta", &zeeeta, &b_zeeeta);
  fInTree->SetBranchAddress("zeephi", &zeephi, &b_zeephi);
  fInTree->SetBranchAddress("phidl", &phidl, &b_phidl);
  fInTree->SetBranchAddress("phidm", &phidm, &b_phidm);
  fInTree->SetBranchAddress("phidt", &phidt, &b_phidt);
  fInTree->SetBranchAddress("phidh", &phidh, &b_phidh);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("wzid", &wzid, &b_wzid);
  fInTree->SetBranchAddress("wzmass", &wzmass, &b_wzmass);
  fInTree->SetBranchAddress("wzpt", &wzpt, &b_wzpt);
  fInTree->SetBranchAddress("wzeta", &wzeta, &b_wzeta);
  fInTree->SetBranchAddress("wzphi", &wzphi, &b_wzphi);
  fInTree->SetBranchAddress("l1id", &l1id, &b_l1id);
  fInTree->SetBranchAddress("l1pt", &l1pt, &b_l1pt);
  fInTree->SetBranchAddress("l1eta", &l1eta, &b_l1eta);
  fInTree->SetBranchAddress("l1phi", &l1phi, &b_l1phi);
  fInTree->SetBranchAddress("l2id", &l2id, &b_l2id);
  fInTree->SetBranchAddress("l2pt", &l2pt, &b_l2pt);
  fInTree->SetBranchAddress("l2eta", &l2eta, &b_l2eta);
  fInTree->SetBranchAddress("l2phi", &l2phi, &b_l2phi);
  fInTree->SetBranchAddress("parid", &parid, &b_parid);
  fInTree->SetBranchAddress("parpt", &parpt, &b_parpt);
  fInTree->SetBranchAddress("pareta", &pareta, &b_pareta);
  fInTree->SetBranchAddress("parphi", &parphi, &b_parphi);
  fInTree->SetBranchAddress("ancid", &ancid, &b_ancid);
  fInTree->SetBranchAddress("ancpt", &ancpt, &b_ancpt);
  fInTree->SetBranchAddress("anceta", &anceta, &b_anceta);
  fInTree->SetBranchAddress("ancphi", &ancphi, &b_ancphi);
}
