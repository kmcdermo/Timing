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
  trTH2Map["el2time_pt"]  = Analysis::MakeTH2Plot("el2time_pt","",100,-3.0,3.0,100,0,100,"Subleading Electron Seed Time [ns]","p_{T} [GeV/c]",trTH2SubMap,"timing");
  trTH2Map["el2time_phi"] = Analysis::MakeTH2Plot("el2time_phi","",100,-3.0,3.0,100,-3.2,3.2,"Subleading Electron Seed Time [ns]","#phi",trTH2SubMap,"timing");
  trTH2Map["el2time_eta"] = Analysis::MakeTH2Plot("el2time_eta","",100,-3.0,3.0,100,-3.2,3.2,"Subleading Electron Seed Time [ns]","#eta",trTH2SubMap,"timing");

  Analysis::MakeSubDirs(trTH1SubMap);
  Analysis::MakeSubDirs(trTH2SubMap);

  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++){
    fInTree->GetEntry(entry);
    if ( (zeemass>76. && zeemass<105.) && hltdoubleel) { // we want di-electron z's and pass hlt
      // standard "validation" plots
      trTH1Map["zeemass"]->Fill(zeemass);
      trTH1Map["el1time"]->Fill(el1time);
      trTH1Map["el2time"]->Fill(el2time);
      trTH1Map["timediff"]->Fill(el1time-el2time);
      trTH2Map["el1time_pt"]->Fill(el1time,el1pt);
      trTH2Map["el1time_phi"]->Fill(el1time,el1phi);
      trTH2Map["el1time_eta"]->Fill(el1time,el1eta);
      trTH2Map["el2time_pt"]->Fill(el2time,el2pt);
      trTH2Map["el2time_phi"]->Fill(el2time,el2phi);
      trTH2Map["el2time_eta"]->Fill(el2time,el2eta);
    }
  }

  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);

  Analysis::SaveTH2s(trTH2Map,trTH2SubMap);
  Analysis::DeleteTH2s(trTH2Map);
}

void Analysis::TriggerEffs(){
  // Set up hists to save; 
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot

  // Throw away numnerator and denominator plots
  int nBinsDEEpt = 100; float xLowDEEpt = 0.; float xHighDEEpt = 100.;
  TH1F * n_hltdoubleel_el1pt = new TH1F("numer_hltdoubleel_el1pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);
  TH1F * d_hltdoubleel_el1pt = new TH1F("denom_hltdoubleel_el1pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);

  TH1F * n_hltdoubleel_el2pt = new TH1F("numer_hltdoubleel_el2pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);
  TH1F * d_hltdoubleel_el2pt = new TH1F("denom_hltdoubleel_el2pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);

  trTH1Map["hltdoubleel_el1pt"] = Analysis::MakeTH1Plot("hltdoubleel_el1pt","Double Electron Trigger Efficiency vs. p_{T}",nBinsDEEpt,xLowDEEpt,xHighDEEpt,"Leading Electron p_{T} [GeV/c]","Efficiency",trTH1SubMap,"trigger");
  trTH1Map["hltdoubleel_el2pt"] = Analysis::MakeTH1Plot("hltdoubleel_el2pt","Double Electron Trigger Efficiency vs. p_{T}",nBinsDEEpt,xLowDEEpt,xHighDEEpt,"Subleading Electron p_{T} [GeV/c]","Efficiency",trTH1SubMap,"trigger");

  Analysis::MakeSubDirs(trTH1SubMap);

  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++){
    fInTree->GetEntry(entry);
    if ( (zeemass>76. && zeemass<105.) ){ // we want di-electron z's
      if ( hltdoubleel ) { // fill numer if passed
	n_hltdoubleel_el1pt->Fill(el1pt);
	n_hltdoubleel_el2pt->Fill(el2pt);
      }
      // always fill denom
      d_hltdoubleel_el1pt->Fill(el1pt);
      d_hltdoubleel_el2pt->Fill(el2pt);
    }
  }
  
  Analysis::ComputeRatioPlot(n_hltdoubleel_el1pt,d_hltdoubleel_el1pt,trTH1Map["hltdoubleel_el1pt"]);
  Analysis::ComputeRatioPlot(n_hltdoubleel_el2pt,d_hltdoubleel_el2pt,trTH1Map["hltdoubleel_el2pt"]);
  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);

  // delete by hand throw away plots
  delete n_hltdoubleel_el1pt;
  delete d_hltdoubleel_el1pt;
  delete n_hltdoubleel_el2pt;
  delete d_hltdoubleel_el2pt;
}

void Analysis::ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot){
  Double_t value = 0;
  Double_t err   = 0;
  for (Int_t bin = 1; bin <= ratioPlot->GetNbinsX(); bin++){
    if (denom->GetBinContent(bin)!=0){
      value = numer->GetBinContent(bin) / denom->GetBinContent(bin); 
      // Binonimal errors 
      err = sqrt( value*(1.0-value)/denom->GetBinContent(bin) );
      //Fill plots with correct values
      ratioPlot->SetBinContent(bin,value);
      ratioPlot->SetBinError(bin,err);
    }
  }
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
    MakeOutDir(Form("%s/%s/",fOutDir.Data(),(*mapiter).second.Data()));
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
  fInTree->SetBranchAddress("puwgt", &puwgt, &b_puwgt);
  fInTree->SetBranchAddress("puobs", &puobs, &b_puobs);
  fInTree->SetBranchAddress("putrue", &putrue, &b_putrue);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("hltsingleel", &hltsingleel, &b_hltsingleel);
  fInTree->SetBranchAddress("hltdoubleel", &hltdoubleel, &b_hltdoubleel);
  fInTree->SetBranchAddress("hltelnoiso", &hltelnoiso, &b_hltelnoiso);
  fInTree->SetBranchAddress("nvetoelectrons", &nvetoelectrons, &b_nvetoelectrons);
  fInTree->SetBranchAddress("nlooseelectrons", &nlooseelectrons, &b_nlooseelectrons);
  fInTree->SetBranchAddress("nmediumelectrons", &nmediumelectrons, &b_nmediumelectrons);
  fInTree->SetBranchAddress("ntightelectrons", &ntightelectrons, &b_ntightelectrons);
  fInTree->SetBranchAddress("nheepelectrons", &nheepelectrons, &b_nheepelectrons);
  fInTree->SetBranchAddress("el1pid", &el1pid, &b_el1pid);
  fInTree->SetBranchAddress("el1pt", &el1pt, &b_el1pt);
  fInTree->SetBranchAddress("el1eta", &el1eta, &b_el1eta);
  fInTree->SetBranchAddress("el1phi", &el1phi, &b_el1phi);
  fInTree->SetBranchAddress("el2pid", &el2pid, &b_el2pid);
  fInTree->SetBranchAddress("el2pt", &el2pt, &b_el2pt);
  fInTree->SetBranchAddress("el2eta", &el2eta, &b_el2eta);
  fInTree->SetBranchAddress("el2phi", &el2phi, &b_el2phi);
  fInTree->SetBranchAddress("el1time", &el1time, &b_el1time);
  fInTree->SetBranchAddress("el2time", &el2time, &b_el2time);
  fInTree->SetBranchAddress("zeemass", &zeemass, &b_zeemass);
  fInTree->SetBranchAddress("zeept", &zeept, &b_zeept);
  fInTree->SetBranchAddress("zeeeta", &zeeeta, &b_zeeeta);
  fInTree->SetBranchAddress("zeephi", &zeephi, &b_zeephi);
}
