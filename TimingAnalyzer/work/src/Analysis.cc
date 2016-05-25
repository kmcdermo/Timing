#include "../interface/Analysis.hh"
#include "../interface/Common.hh"

#include "TCanvas.h"
#include "TF1.h"

#include <iostream>
#include <fstream>

inline float rad2(float x, float y){
  return x*x + y*y;
}

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
  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
}

Analysis::~Analysis(){
  delete fTDRStyle;
  delete fInTree;
  delete fInFile;
  delete fOutFile;
}

void Analysis::StandardPlots(){
  // Set up hists first --> first in map is histo name, by design!
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot
  trTH1Map["zeemass"]  = Analysis::MakeTH1Plot("zeemass","",100,60.,120.,"Dielectron invariant mass [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zeept"]    = Analysis::MakeTH1Plot("zeept","",100,0.,750.,"Dielectron p_{T} [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zeeeta"]   = Analysis::MakeTH1Plot("zeeeta","",100,-10.0,10.0,"Dielectron #eta","Events",trTH1SubMap,"standard");

  trTH1Map["timediff"]   = Analysis::MakeTH1Plot("timediff","",100,-5.0,5.0,"Dielectron Seed Time Difference [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el1time"]    = Analysis::MakeTH1Plot("el1time","",100,-5.0,5.0,"Leading Electron Seed Time [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el1pt"]      = Analysis::MakeTH1Plot("el1pt","",100,0.,700.,"Leading Electron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
  trTH1Map["el1eta"]     = Analysis::MakeTH1Plot("el1eta","",100,-3.0,3.0,"Leading Electron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el1phi"]     = Analysis::MakeTH1Plot("el1phi","",100,-3.2,3.2,"Leading Electron #phi","Events",trTH1SubMap,"standard");
  trTH1Map["el2time"]    = Analysis::MakeTH1Plot("el2time","",100,-5.0,5.0,"Subleading Electron Seed Time [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el2pt"]      = Analysis::MakeTH1Plot("el2pt","",100,0.,300.,"Subleading Electron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
  trTH1Map["el2eta"]     = Analysis::MakeTH1Plot("el2eta","",100,-3.0,3.0,"Subleading Electron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el2phi"]     = Analysis::MakeTH1Plot("el2phi","",100,-3.2,3.2,"Subleading Electron #phi","Events",trTH1SubMap,"standard");
  trTH1Map["eff_dielpt"] = Analysis::MakeTH1Plot("eff_dielpt","",100,0.,250.,"Effective Dielectron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");

  Analysis::MakeSubDirs(trTH1SubMap);

  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++){
    fInTree->GetEntry(entry);
    if ( (zeemass>76. && zeemass<106.) && hltdoubleel) { // we want di-electron z's and pass hlt
      // standard "validation" plots
      trTH1Map["zeemass"]->Fill(zeemass);
      trTH1Map["zeept"]->Fill(zeept);
      trTH1Map["zeeeta"]->Fill(zeeeta);

      trTH1Map["timediff"]->Fill(el1time-el2time);
      trTH1Map["el1time"]->Fill(el1time);
      trTH1Map["el1pt"]->Fill(el1pt);
      trTH1Map["el1eta"]->Fill(el1eta);
      trTH1Map["el1phi"]->Fill(el1phi);
      trTH1Map["el2time"]->Fill(el2time);
      trTH1Map["el2pt"]->Fill(el2pt);
      trTH1Map["el2eta"]->Fill(el2eta);
      trTH1Map["el2phi"]->Fill(el2phi); 

      float eff_dielpt = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));
      trTH1Map["eff_dielpt"]->Fill(eff_dielpt);
    }
  }

  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);
}

void Analysis::TimeResPlots(){
  // read in run numbers
  ifstream input;
  input.open("txt/runs.txt",std::ios::in);
  int runno = -1;
  std::vector<int> runNos;
  while(input >> runno){
    runNos.push_back(runno);
  }
  input.close();

  std::vector<Double_t> dRunNos; // need this to do the wonky binning to get means/sigmas to line up exactly with run number
  int totalRuns = runNos.back()-runNos.front();
  int startrun  = runNos.front();
  for (int i = 0; i < totalRuns + 2; i++) { // +1 for subtraction, +1 for half offset
    dRunNos.push_back(startrun + i - 0.5);
  }

  // td    = time difference (el1 - el2)
  // effpt = effective pt 
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot
  trTH1Map["tdEBEB_allpt"] = Analysis::MakeTH1Plot("tdEBEB_allpt","",100,-5.0,5.0,"Dielectron Seed Time Difference [ns] (EBEB all p_{T})","Events",trTH1SubMap,"timing/allpt");  
  trTH1Map["tdEBEE_allpt"] = Analysis::MakeTH1Plot("tdEBEE_allpt","",100,-5.0,5.0,"Dielectron Seed Time Difference [ns] (EBEE all p_{T})","Events",trTH1SubMap,"timing/allpt");  
  trTH1Map["tdEEEE_allpt"] = Analysis::MakeTH1Plot("tdEEEE_allpt","",100,-5.0,5.0,"Dielectron Seed Time Difference [ns] (EEEE all p_{T})","Events",trTH1SubMap,"timing/allpt");  

  // pt bins
  std::vector<Double_t> elptbins = {20,25,30,35,40,50,70,100,250}; 
  // make in 2D first, then decompose them into 1D histograms (saves on inner loops over events)
  TH2Map  trTH2Map;
  TStrMap trTH2SubMap;
  trTH2Map["tdEBEB_ptbins"] = Analysis::MakeTH2Plot("tdEBEB_ptbins","",elptbins,100,-5.0,5.0,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEB)",trTH2SubMap,"timing/elpt/ebeb");  
  trTH2Map["tdEBEE_ptbins"] = Analysis::MakeTH2Plot("tdEBEE_ptbins","",elptbins,100,-5.0,5.0,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEE)",trTH2SubMap,"timing/elpt/ebee");  
  trTH2Map["tdEEEE_ptbins"] = Analysis::MakeTH2Plot("tdEEEE_ptbins","",elptbins,100,-5.0,5.0,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EEEE)",trTH2SubMap,"timing/elpt/eeee");  

  // make a plot in EBEB, EBEE, EEEE for each run
  for (int i = 0; i < runNos.size(); i++){
    trTH1Map[Form("tdEBEB_%i",runNos[i])] = Analysis::MakeTH1Plot(Form("tdEBEB_%i",runNos[i]),"",100,-5.0,5.0,Form("Dielectron Seed Time Difference [ns] (EBEB run %i)",runNos[i]),"Events",trTH1SubMap,"timing/runs/ebeb");  
    trTH1Map[Form("tdEBEE_%i",runNos[i])] = Analysis::MakeTH1Plot(Form("tdEBEE_%i",runNos[i]),"",100,-5.0,5.0,Form("Dielectron Seed Time Difference [ns] (EBEE run %i)",runNos[i]),"Events",trTH1SubMap,"timing/runs/ebee");  
    trTH1Map[Form("tdEEEE_%i",runNos[i])] = Analysis::MakeTH1Plot(Form("tdEEEE_%i",runNos[i]),"",100,-5.0,5.0,Form("Dielectron Seed Time Difference [ns] (EEEE run %i)",runNos[i]),"Events",trTH1SubMap,"timing/runs/eeee");  
  }

  // make 2D plots for Z variables (pT, eta)
  std::vector<Double_t> zptbins = {0,10,20,30,50,70,100,150,200,250,300,400,750};
  trTH2Map["td_zpt"]  = Analysis::MakeTH2Plot("td_zpt","",zptbins,100,-5.0,5.0,"Z p_{T} [GeV/c]","Dielectron Seed Time Difference [ns]",trTH2SubMap,"timing/zpt");  
  trTH2Map["td_zeta"] = Analysis::MakeTH2Plot("td_zeta","",100,-5.0,5.0,100,-5.0,5.0,"Z #eta","Dielectron Seed Time Difference [ns]",trTH2SubMap,"timing/zeta");  

  for (UInt_t entry = 0; entry < 100000; entry++){
    //  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++){
    fInTree->GetEntry(entry);
    if ( (zeemass>76. && zeemass<106.) && hltdoubleel) { // we want di-electron z's and pass hlt
      float time_diff  = el1time-el2time;
      float eff_dielpt = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));

      // Electron based bins first
      bool el1eb = false; bool el1ee = false;
      if ( std::abs(el1eta) < 1.4442 ) {
	el1eb = true;
      }
      else if (std::abs(el1eta) > 1.566 && std::abs(el1eta) < 2.5) {
	el1ee = true;
      }

      bool el2eb = false; bool el2ee = false;
      if ( std::abs(el2eta) < 1.4442 ) {
	el2eb = true;
      }
      else if (std::abs(el2eta) > 1.566 && std::abs(el2eta) < 2.5) {
	el2ee = true;
      }

      // electron eta categories 
      if (el1eb && el2eb) {
	trTH1Map["tdEBEB_allpt"]       ->Fill(time_diff);
 	trTH1Map[Form("tdEBEB_%i",run)]->Fill(time_diff);
 	trTH2Map["tdEBEB_ptbins"]      ->Fill(eff_dielpt,time_diff);
      }
      else if ( (el1eb && el2ee) || (el1ee && el2eb) ) {
	trTH1Map["tdEBEE_allpt"]       ->Fill(time_diff);
	std::cout << run << std::endl;
 	trTH1Map[Form("tdEBEE_%i",run)]->Fill(time_diff);
 	trTH2Map["tdEBEE_ptbins"]      ->Fill(eff_dielpt,time_diff);
      }
      else if (el1ee && el2ee) {
	trTH1Map["tdEEEE_allpt"]       ->Fill(time_diff);
 	trTH1Map[Form("tdEEEE_%i",run)]->Fill(time_diff);
 	trTH2Map["tdEEEE_ptbins"]      ->Fill(eff_dielpt,time_diff);
      }
    
      // Now Z based plots
      trTH2Map["td_zpt"]->Fill(zeept,time_diff);
      trTH2Map["td_zeta"]->Fill(zeeeta,time_diff);
    }
  }

  // Now project out each of the 2D plots
  Analysis::Project2Dto1D(trTH2Map["td_zpt"],trTH2SubMap,trTH1Map,trTH1SubMap);
  Analysis::Project2Dto1D(trTH2Map["td_zeta"],trTH2SubMap,trTH1Map,trTH1SubMap);
  Analysis::Project2Dto1D(trTH2Map["tdEBEB_ptbins"],trTH2SubMap,trTH1Map,trTH1SubMap);
  Analysis::Project2Dto1D(trTH2Map["tdEBEE_ptbins"],trTH2SubMap,trTH1Map,trTH1SubMap);
  Analysis::Project2Dto1D(trTH2Map["tdEEEE_ptbins"],trTH2SubMap,trTH1Map,trTH1SubMap);

  // Fit each timing plot to a simple gaussian
  Analysis::FitandExtractTH1s(trTH1Map,3.0);

  Analysis::MakeSubDirs(trTH1SubMap);
  Analysis::MakeSubDirs(trTH2SubMap);

  Analysis::SaveTH2s(trTH2Map,trTH2SubMap);
  Analysis::DeleteTH2s(trTH2Map);

  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);
}

void Analysis::TriggerEffs(){
  // Set up hists to save; 
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot

  int nBinsDEEpt = 100; float xLowDEEpt = 15.; float xHighDEEpt = 20.;
  TH1F * n_hltdoubleel_el1pt = new TH1F("numer_hltdoubleel_el1pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);
  TH1F * d_hltdoubleel_el1pt = new TH1F("denom_hltdoubleel_el1pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);

  TH1F * n_hltdoubleel_el2pt = new TH1F("numer_hltdoubleel_el2pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);
  TH1F * d_hltdoubleel_el2pt = new TH1F("denom_hltdoubleel_el2pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);

  trTH1Map["hltdoubleel_el1pt"] = Analysis::MakeTH1Plot("hltdoubleel_el1pt","Double Electron Trigger Efficiency vs. p_{T}",nBinsDEEpt,xLowDEEpt,xHighDEEpt,"Leading Electron p_{T} [GeV/c]","Efficiency",trTH1SubMap,"trigger_zoomest");
  trTH1Map["hltdoubleel_el2pt"] = Analysis::MakeTH1Plot("hltdoubleel_el2pt","Double Electron Trigger Efficiency vs. p_{T}",nBinsDEEpt,xLowDEEpt,xHighDEEpt,"Subleading Electron p_{T} [GeV/c]","Efficiency",trTH1SubMap,"trigger_zoomest");

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

TH2F * Analysis::MakeTH2Plot(TString hname, TString htitle, Int_t nbinsx, Double_t xlow, Double_t xhigh, Int_t nbinsy, Double_t ylow, Double_t yhigh,
			     TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) {
  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),nbinsx,xlow,xhigh,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

TH2F * Analysis::MakeTH2Plot(TString hname, TString htitle, const std::vector<Double_t> vxbins, Int_t nbinsy, Double_t ylow, Double_t yhigh,
			     TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) {
  // need to convert vectors into arrays per ROOT
  const Double_t * axbins = &vxbins[0]; // https://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-c

  TH2F * hist = new TH2F(hname.Data(),htitle.Data(),vxbins.size()-1,axbins,nbinsy,ylow,yhigh);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

void Analysis::Project2Dto1D(TH2F *& hist2d, TStrMap & subdir2dmap, TH1Map & th1map, TStrMap & subdir1dmap){
  // y bins same width, x bins are variable

  TString  basename = hist2d->GetName();
  Int_t    nybins   = hist2d->GetNbinsY();
  Double_t ylow     = hist2d->GetYaxis()->GetXmin();
  Double_t yhigh    = hist2d->GetYaxis()->GetXmax();
  TString  xtitle   = hist2d->GetXaxis()->GetTitle();
  TString  ytitle   = hist2d->GetYaxis()->GetTitle();

  // loop over all x bins to project out
  for (int i = 1; i <= hist2d->GetNbinsX(); i++){  
    Double_t xlow  = hist2d->GetXaxis()->GetBinLowEdge(i); int ixlow  = int(xlow); 
    Double_t xhigh = hist2d->GetXaxis()->GetBinUpEdge(i);  int ixhigh = int(xhigh); 

    TString histname = "";
    // First create each histogram
    if (!basename.Contains("zeta",TString::kExact)) { //ugh
      histname = Form("%s_%i-%i",basename.Data(),ixlow,ixhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %i-%i",ytitle.Data(),xtitle.Data(),ixlow,ixhigh),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }
    else {
      histname = Form("%s_%3.1f-%3.1f",basename.Data(),xlow,xhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %3.1f-%3.1f",ytitle.Data(),xtitle.Data(),xlow,xhigh),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }

    // then fill corresponding bins from y
    for (int j = 0; j <= hist2d->GetNbinsY() + 1; j++) {
      th1map[histname.Data()]->SetBinContent(j,hist2d->GetBinContent(i,j));
      th1map[histname.Data()]->SetBinError(j,hist2d->GetBinError(i,j)); 
    }
  }
}

void Analysis::FitandExtractTH1s(TH1Map & th1map, Float_t fitrange, const std::vector<Double_t> bins){

  TH1F * outhist = new TH1F;

  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    TF1* gausfit = new TF1("gausfit","gaus",-fitrange,fitrange);
    (*mapiter).second->Fit("gausfit","R");

    Double_t mean  = gausfit->GetParameter("Mean");
    Double_t sigma = gausfit->GetParameter("Sigma");

    delete gausfit;
  }

  // write output hist to file
  fOutFile->cd();
  outhist->Write();

  // save log/lin of each plot
  TCanvas * canv = new TCanvas();
  CMSLumi(canv, fLumi, "Preliminary",0);
  canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),outhist->GetName(),fOutType.Data()));
  
  canv->SetLogy(1);
  CMSLumi(canv, fLumi, "Preliminary",0);
  canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),outhist->GetName(),fOutType.Data()));
  
  delete canv;
  delete outhist;
}

void Analysis::MakeSubDirs(TStrMap & subdirmap){
  for (TStrMapIter mapiter = subdirmap.begin(); mapiter != subdirmap.end(); mapiter++) { 
    MakeOutDir(Form("%s/%s/",fOutDir.Data(),(*mapiter).second.Data()));
    MakeOutDir(Form("%s/%s/lin/",fOutDir.Data(),(*mapiter).second.Data()));
    MakeOutDir(Form("%s/%s/log/",fOutDir.Data(),(*mapiter).second.Data()));
  }
}

void Analysis::SaveTH1s(TH1Map & th1map, TStrMap & subdirmap){
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

void Analysis::SaveTH2s(TH2Map & th2map, TStrMap & subdirmap){
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
    canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),fOutType.Data()));
  }

  delete canv;
}

void Analysis::DeleteTH1s(TH1Map & th1map) {
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    delete ((*mapiter).second);
  }
  th1map.clear();
}

void Analysis::DeleteTH2s(TH2Map & th2map) {
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
