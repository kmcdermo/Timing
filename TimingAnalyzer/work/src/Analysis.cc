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
  Int_t    ntimebins = 100;
  Double_t timerange = 5.0;
  Double_t fitrange  = 3.0;

  // inclusive "global" time differences by eta 
  TH1Map  inclu1DMap;
  TStrMap inclu1DSubMap; // set inside MakeTH1Plot
  inclu1DMap["tdEBEB_inclu"] = Analysis::MakeTH1Plot("tdEBEB_inclu","",ntimebins,-timerange,timerange,"Dielectron Seed Time Difference [ns] (EBEB Inclusive)","Events",inclu1DSubMap,"timing/inclusive");  
  inclu1DMap["tdEBEE_inclu"] = Analysis::MakeTH1Plot("tdEBEE_inclu","",ntimebins,-timerange,timerange,"Dielectron Seed Time Difference [ns] (EBEE Inclusive)","Events",inclu1DSubMap,"timing/inclusive");  
  inclu1DMap["tdEEEE_inclu"] = Analysis::MakeTH1Plot("tdEEEE_inclu","",ntimebins,-timerange,timerange,"Dielectron Seed Time Difference [ns] (EEEE Inclusive)","Events",inclu1DSubMap,"timing/inclusive");  

  // make 2D plots for Z variables (pT, eta)
  TH2Map  z2DMap;      
  TStrMap z2DSubMap;
  std::vector<Double_t> zptbins = {0,10,20,30,50,70,100,150,200,250,300,400,750};
  z2DMap["td_zpt"]  = Analysis::MakeTH2Plot("td_zpt","",zptbins,ntimebins,-timerange,timerange,"Z p_{T} [GeV/c]","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/zpt");  
  std::vector<Double_t> zetabins; for (int i = 0; i < 21; i++){zetabins.push_back(i/2. - 5.0);}
  z2DMap["td_zeta"] = Analysis::MakeTH2Plot("td_zeta","",zetabins,ntimebins,-timerange,timerange,"Z #eta","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/zeta");  

  // make 2D plots for effective electron pt, for EBEB, EBEE, EEEE
  TH2Map  effpt2DMap;    
  TStrMap effpt2DSubMap; 
  std::vector<Double_t> effptbins = {20,25,30,35,40,50,70,100,250}; //effective el pt bins
  effpt2DMap["tdEBEB_effpt"] = Analysis::MakeTH2Plot("tdEBEB_effpt","",effptbins,ntimebins,-timerange,timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEB)",effpt2DSubMap,"timing/effpt/EBEB");  
  effpt2DMap["tdEBEE_effpt"] = Analysis::MakeTH2Plot("tdEBEE_effpt","",effptbins,ntimebins,-timerange,timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEE)",effpt2DSubMap,"timing/effpt/EBEE");  
  effpt2DMap["tdEEEE_effpt"] = Analysis::MakeTH2Plot("tdEEEE_effpt","",effptbins,ntimebins,-timerange,timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EEEE)",effpt2DSubMap,"timing/effpt/EEEE");  

  // 2D plots for run numbers
  // read in run numbers
  ifstream input;
  input.open("txt/runs.txt",std::ios::in);
  int runno = -1;
  std::vector<int> runNos;
  while(input >> runno){
    runNos.push_back(runno);
  }
  input.close();
  
  // need this to do the wonky binning to get means/sigmas to line up exactly with run number
  std::vector<Double_t> dRunNos;
  int totalRuns = runNos.back()-runNos.front();
  int startrun  = runNos.front();
  for (int i = 0; i < totalRuns + 2; i++) { // +1 for subtraction, +1 for half offset
    dRunNos.push_back(startrun + i - 0.5);
  }
  //actually define plots
  TH2Map  runs2DMap;    
  TStrMap runs2DSubMap; 
  runs2DMap["tdEBEB_runs"] = Analysis::MakeTH2Plot("tdEBEB_runs","",dRunNos,ntimebins,-timerange,timerange,"Run Number","Dielectron Seed Time Difference [ns] (EBEB)",runs2DSubMap,"timing/runs/EBEB");  
  runs2DMap["tdEBEE_runs"] = Analysis::MakeTH2Plot("tdEBEE_runs","",dRunNos,ntimebins,-timerange,timerange,"Run Number","Dielectron Seed Time Difference [ns] (EBEE)",runs2DSubMap,"timing/runs/EBEE");  
  runs2DMap["tdEEEE_runs"] = Analysis::MakeTH2Plot("tdEEEE_runs","",dRunNos,ntimebins,-timerange,timerange,"Run Number","Dielectron Seed Time Difference [ns] (EEEE)",runs2DSubMap,"timing/runs/EEEE");  
  
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
	inclu1DMap["tdEBEB_inclu"]->Fill(time_diff);
 	effpt2DMap["tdEBEB_effpt"]->Fill(eff_dielpt,time_diff);
	runs2DMap ["tdEBEB_runs"] ->Fill(run,time_diff);
      }
      else if ( (el1eb && el2ee) || (el1ee && el2eb) ) {
	inclu1DMap["tdEBEE_inclu"]->Fill(time_diff);
 	effpt2DMap["tdEBEE_effpt"]->Fill(eff_dielpt,time_diff);
	runs2DMap ["tdEBEE_runs"] ->Fill(run,time_diff);
      }
      else if (el1ee && el2ee) {
	inclu1DMap["tdEEEE_inclu"]->Fill(time_diff);
 	effpt2DMap["tdEEEE_effpt"]->Fill(eff_dielpt,time_diff);
	runs2DMap ["tdEEEE_runs"] ->Fill(run,time_diff);
      }
    
      // Now Z based plots
      z2DMap["td_zpt"]->Fill(zeept,time_diff);
      z2DMap["td_zeta"]->Fill(zeeeta,time_diff);
    }
  }

  ////////////////////////////////////
  // Do inclusive "global" plots first
  Analysis::MakeSubDirs(inclu1DSubMap);
  Analysis::SaveTH1s(inclu1DMap,inclu1DSubMap);
  Analysis::DeleteTH1s(inclu1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do z plots second
  Analysis::MakeSubDirs(z2DSubMap);
  Analysis::SaveTH2s(z2DMap,z2DSubMap);

  // zpt
  TH1Map zpt1DMap;  TStrMap zpt1DSubMap;
  Analysis::Project2Dto1D(z2DMap["td_zpt"],z2DSubMap,zpt1DMap,zpt1DSubMap);
  Analysis::SaveTH1s(zpt1DMap,zpt1DSubMap);
  Analysis::FitandExtractTH1s(zpt1DMap,"td_zpt","Z p_{T} [GeV/c]",zptbins,fitrange,z2DSubMap["td_zpt"]);

  // zeta
  TH1Map zeta1DMap;  TStrMap zeta1DSubMap;
  Analysis::Project2Dto1D(z2DMap["td_zeta"],z2DSubMap,zeta1DMap,zeta1DSubMap);
  Analysis::SaveTH1s(zeta1DMap,zeta1DSubMap);
  Analysis::FitandExtractTH1s(zeta1DMap,"td_zeta","Z #eta",zetabins,fitrange,z2DSubMap["td_zeta"]);

  // Delete z's
  Analysis::DeleteTH2s(z2DMap);
  Analysis::DeleteTH1s(zpt1DMap);
  Analysis::DeleteTH1s(zeta1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do effective electron pt plots next
  Analysis::MakeSubDirs(effpt2DSubMap);
  Analysis::SaveTH2s(effpt2DMap,effpt2DSubMap);

  // EBEB
  TH1Map effptEBEB1DMap;  TStrMap effptEBEB1DSubMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEBEB_effpt"],effpt2DSubMap,effptEBEB1DMap,effptEBEB1DSubMap);
  Analysis::SaveTH1s(effptEBEB1DMap,effptEBEB1DSubMap);
  Analysis::FitandExtractTH1s(effptEBEB1DMap,"tdEBEB_effpt","Effective Dielectron p_{T} [GeV/c] (EBEB)",effptbins,fitrange,effpt2DSubMap["tdEBEB_effpt"]);

  // EBEE
  TH1Map effptEBEE1DMap;  TStrMap effptEBEE1DSubMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEBEE_effpt"],effpt2DSubMap,effptEBEE1DMap,effptEBEE1DSubMap);
  Analysis::SaveTH1s(effptEBEE1DMap,effptEBEE1DSubMap);
  Analysis::FitandExtractTH1s(effptEBEE1DMap,"tdEBEE_effpt","Effective Dielectron p_{T} [GeV/c] (EBEE)",effptbins,fitrange,effpt2DSubMap["tdEBEE_effpt"]);

  // EEEE
  TH1Map effptEEEE1DMap;  TStrMap effptEEEE1DSubMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEEEE_effpt"],effpt2DSubMap,effptEEEE1DMap,effptEEEE1DSubMap);
  Analysis::SaveTH1s(effptEEEE1DMap,effptEEEE1DSubMap);
  Analysis::FitandExtractTH1s(effptEEEE1DMap,"tdEEEE_effpt","Effective Dielectron p_{T} [GeV/c] (EEEE)",effptbins,fitrange,effpt2DSubMap["tdEEEE_effpt"]);

  // Delete effective electrons
  Analysis::DeleteTH2s(effpt2DMap);
  Analysis::DeleteTH1s(effptEBEB1DMap);
  Analysis::DeleteTH1s(effptEBEE1DMap);
  Analysis::DeleteTH1s(effptEEEE1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do run plots last
  Analysis::MakeSubDirs(runs2DSubMap);
  Analysis::SaveTH2s(runs2DMap,runs2DSubMap);

  // EBEB
  TH1Map runsEBEB1DMap;  TStrMap runsEBEB1DSubMap;
  Analysis::Project2Dto1D(runs2DMap["tdEBEB_runs"],runs2DSubMap,runsEBEB1DMap,runsEBEB1DSubMap);
  Analysis::SaveTH1s(runsEBEB1DMap,runsEBEB1DSubMap);
  Analysis::FitandExtractTH1s(runsEBEB1DMap,"tdEBEB_runs","Run Number (EBEB)",dRunNos,fitrange,runs2DSubMap["tdEBEB_runs"]);

  // EBEE
  TH1Map runsEBEE1DMap;  TStrMap runsEBEE1DSubMap;
  Analysis::Project2Dto1D(runs2DMap["tdEBEE_runs"],runs2DSubMap,runsEBEE1DMap,runsEBEE1DSubMap);
  Analysis::SaveTH1s(runsEBEE1DMap,runsEBEE1DSubMap);
  Analysis::FitandExtractTH1s(runsEBEE1DMap,"tdEBEE_runs","Run Number (EBEE)",dRunNos,fitrange,runs2DSubMap["tdEBEE_runs"]);

  // EEEE
  TH1Map runsEEEE1DMap;  TStrMap runsEEEE1DSubMap;
  Analysis::Project2Dto1D(runs2DMap["tdEEEE_runs"],runs2DSubMap,runsEEEE1DMap,runsEEEE1DSubMap);
  Analysis::SaveTH1s(runsEEEE1DMap,runsEEEE1DSubMap);
  Analysis::FitandExtractTH1s(runsEEEE1DMap,"tdEEEE_runs","Run Number (EEEE)",dRunNos,fitrange,runs2DSubMap["tdEEEE_runs"]);

  // Delete effective electrons
  Analysis::DeleteTH2s(runs2DMap);
  Analysis::DeleteTH1s(runsEBEB1DMap);
  Analysis::DeleteTH1s(runsEBEE1DMap);
  Analysis::DeleteTH1s(runsEEEE1DMap);
  ////////////////////////////////////
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
    // if no bins are filled, then continue to next plot
    bool isFilled = false;
    for (int j = 0; j <= hist2d->GetNbinsY() + 1; j++) {
      if (hist2d->GetBinContent(i,j) > 0) {isFilled = true; break;}
    }
    if (!isFilled) continue;

    Double_t xlow  = hist2d->GetXaxis()->GetBinLowEdge(i); 
    Double_t xhigh = hist2d->GetXaxis()->GetBinUpEdge(i);

    TString histname = "";
    // First create each histogram
    if (basename.Contains("zeta",TString::kExact)) { //ugh
      histname = Form("%s_%3.1f-%3.1f",basename.Data(),xlow,xhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %3.1f-%3.1f",ytitle.Data(),xtitle.Data(),xlow,xhigh),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }
    else if (basename.Contains("runs",TString::kExact)) { //double ugh
      int runno = (xlow+xhigh)/2;
      histname = Form("%s_%i",basename.Data(),runno);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in Run: %i",ytitle.Data(),runno),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }
    else { // "normal" filling
      int ixlow = int(xlow); 
      int ixhigh = int(xhigh); 
      histname = Form("%s_%i-%i",basename.Data(),ixlow,ixhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %i-%i",ytitle.Data(),xtitle.Data(),ixlow,ixhigh),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }

    // then fill corresponding bins from y
    for (int j = 0; j <= hist2d->GetNbinsY() + 1; j++) {
      // check to make sure not zero though...
      if ( !(hist2d->GetBinContent(i,j) < 0) ) {
	th1map[histname.Data()]->SetBinContent(j,hist2d->GetBinContent(i,j));
	th1map[histname.Data()]->SetBinError(j,hist2d->GetBinError(i,j)); 
      }
      else{
	th1map[histname.Data()]->SetBinContent(i,0);
	th1map[histname.Data()]->SetBinError(i,0);
      }
    }
  }
}

void Analysis::FitandExtractTH1s(TH1Map & th1map, TString name, TString xtitle, const std::vector<Double_t> vxbins, Float_t fitrange, TString subdir){
  // need to convert bins into array
  const Double_t * axbins = &vxbins[0]; // https://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-c

  // initialize new mean/sigma histograms
  TH1F * outhist_mean  = new TH1F(Form("%s_mean",name.Data()),"",vxbins.size()-1,axbins);
  outhist_mean->GetXaxis()->SetTitle(xtitle.Data());
  outhist_mean->GetYaxis()->SetTitle("Dielectron Seed Time Mean [ns]");

  TH1F * outhist_sigma  = new TH1F(Form("%s_sigma",name.Data()),"",vxbins.size()-1,axbins);
  outhist_sigma->GetXaxis()->SetTitle(xtitle.Data());
  outhist_sigma->GetYaxis()->SetTitle("Dielectron Seed Time Sigma [ns]");

  int i = 1; // dummy counter (ROOT starts bins at 1)
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    TF1* gausfit = new TF1("gausfit","gaus",-fitrange,fitrange);
    (*mapiter).second->Fit("gausfit","R");

    Int_t imean  = gausfit->GetParNumber("Mean");
    Int_t isigma = gausfit->GetParNumber("Sigma");

    outhist_mean->SetBinContent(i,gausfit->GetParameter(imean));
    outhist_mean->SetBinError(i,gausfit->GetParError(imean));

    outhist_sigma->SetBinContent(i,gausfit->GetParameter(isigma));
    outhist_sigma->SetBinError(i,gausfit->GetParError(isigma));

    i++; //increment after all filled
    delete gausfit;
  }

  // write output hist to file
  fOutFile->cd();
  outhist_mean->Write();
  outhist_sigma->Write();

  // save log/lin of each plot
  TCanvas * canv = new TCanvas();
  CMSLumi(canv, fLumi, "Preliminary",0);
  canv->cd();
  outhist_mean->Draw("PE");
  canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdir.Data(),outhist_mean->GetName(),fOutType.Data()));
  outhist_sigma->Draw("PE");
  canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdir.Data(),outhist_sigma->GetName(),fOutType.Data()));
  
  delete canv;
  delete outhist_sigma;
  delete outhist_mean;
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
