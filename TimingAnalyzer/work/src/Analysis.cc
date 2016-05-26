#include "../interface/Analysis.hh"

inline Float_t rad2(Float_t x, Float_t y){
  return x*x + y*y;
}

Analysis::Analysis(TString sample, Bool_t isMC, ColorMap colormap) : fSample(sample), fIsMC(isMC) {
  TString filename = "";
  if (fIsMC) {
    filename = Form("input/MC/%s/treewgtsum.root",fSample.Data());
    fInFile  = TFile::Open(filename.Data());
    CheckValidFile(fInFile,filename);
    
    // Get pile-up weights
    TString purwfname = Form("%s/purw/PURW.root",Config::outdir.Data());   
    TFile * purwfile  = TFile::Open(purwfname.Data());
    CheckValidFile(purwfile,purwfname);
    
    TString purwpname = "nvtx_dataOverMC";
    TH1F *  purwplot  = (TH1F*)purwfile->Get(purwpname.Data());
    CheckValidTH1F(purwplot,purwpname,purwfname);
    
    for (Int_t i = 1; i <= purwplot->GetNbinsX(); i++){
      fPUweights.push_back(purwplot->GetBinContent(i));
    }

    delete purwplot;
    delete purwfile;
    // end getting pile-up weights

    fOutDir = Form("%s/MC/%s",Config::outdir.Data(),fSample.Data());
    fColor  = colormap[fSample.Data()];
  }
  else {
    filename = Form("input/DATA/%s/tree.root",fSample.Data());
    fInFile  = TFile::Open(filename.Data());
    CheckValidFile(fInFile,filename);

    fOutDir  = Form("%s/DATA/%s",Config::outdir.Data(),fSample.Data());
    fColor   = kBlack;
  }
  
  TString treename = "tree/tree";
  fInTree = (TTree*)fInFile->Get(treename.Data());
  CheckValidTree(fInTree,treename,filename);
  InitTree();

  // output stuff
  MakeOutDir(fOutDir);
  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
  
  // dump th1 names and subdirs
  fTH1Dump.open(Form("%s/dumpplots.txt",fOutDir.Data()),std::ios_base::trunc);
}

Analysis::~Analysis(){
  delete fInTree;
  delete fInFile;
  delete fOutFile;
  fTH1Dump.close();
}

void Analysis::StandardPlots(){
  // Set up hists first --> first in map is histo name, by design!
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot
  trTH1Map["nvtx"]   = Analysis::MakeTH1Plot("nvtx","",Config::nvtxbins,0.,Double_t(Config::nvtxbins),"nVertices","Events",trTH1SubMap,"standard");
  trTH1Map["zmass"]  = Analysis::MakeTH1Plot("zmass","",100,60.,120.,"Dielectron invariant mass [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zpt"]    = Analysis::MakeTH1Plot("zpt","",100,0.,750.,"Dielectron p_{T} [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zeta"]   = Analysis::MakeTH1Plot("zeta","",100,-10.0,10.0,"Dielectron #eta","Events",trTH1SubMap,"standard");

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

  MakeSubDirs(trTH1SubMap,fOutDir);

  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++){
    fInTree->GetEntry(entry);
    if (zmass>Config::zlow && zmass<Config::zup) { // extra selection over skimmed samples
      Float_t weight = -1.;
      if   (fIsMC) {weight = (xsec * Config::lumi * wgt / wgtsum) * fPUweights[nvtx];}
      else         {weight = 1.0;}

      // standard "validation" plots
      trTH1Map["nvtx"]->Fill(nvtx,weight);
      trTH1Map["zmass"]->Fill(zmass,weight);
      trTH1Map["zpt"]->Fill(zpt,weight);
      trTH1Map["zeta"]->Fill(zeta,weight);

      trTH1Map["timediff"]->Fill(el1time-el2time,weight);
      trTH1Map["el1time"]->Fill(el1time,weight);
      trTH1Map["el1pt"]->Fill(el1pt,weight);
      trTH1Map["el1eta"]->Fill(el1eta,weight);
      trTH1Map["el1phi"]->Fill(el1phi,weight);
      trTH1Map["el2time"]->Fill(el2time,weight);
      trTH1Map["el2pt"]->Fill(el2pt,weight);
      trTH1Map["el2eta"]->Fill(el2eta,weight);
      trTH1Map["el2phi"]->Fill(el2phi,weight); 

      Float_t eff_dielpt = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));
      trTH1Map["eff_dielpt"]->Fill(eff_dielpt,weight);
    }
  }

  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DumpTH1Names(trTH1Map,trTH1SubMap);
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
  DblVec zptbins = {0,10,20,30,50,70,100,150,200,250,300,400,750};
  z2DMap["td_zpt"]  = Analysis::MakeTH2Plot("td_zpt","",zptbins,ntimebins,-timerange,timerange,"Z p_{T} [GeV/c]","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/zpt");  
  DblVec zetabins; for (Int_t i = 0; i < 21; i++){zetabins.push_back(i/2. - 5.0);}
  z2DMap["td_zeta"] = Analysis::MakeTH2Plot("td_zeta","",zetabins,ntimebins,-timerange,timerange,"Z #eta","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/zeta");  

  // make 2D plots for effective electron pt, for EBEB, EBEE, EEEE
  TH2Map  effpt2DMap;    
  TStrMap effpt2DSubMap; 
  DblVec effptbins = {20,25,30,35,40,50,70,100,250}; //effective el pt bins
  effpt2DMap["tdEBEB_effpt"] = Analysis::MakeTH2Plot("tdEBEB_effpt","",effptbins,ntimebins,-timerange,timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEB)",effpt2DSubMap,"timing/effpt/EBEB");  
  effpt2DMap["tdEBEE_effpt"] = Analysis::MakeTH2Plot("tdEBEE_effpt","",effptbins,ntimebins,-timerange,timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEE)",effpt2DSubMap,"timing/effpt/EBEE");  
  effpt2DMap["tdEEEE_effpt"] = Analysis::MakeTH2Plot("tdEEEE_effpt","",effptbins,ntimebins,-timerange,timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EEEE)",effpt2DSubMap,"timing/effpt/EEEE");  

  // 2D plots for run numbers
  // read in run numbers
  ifstream input;
  input.open("config/runs.txt",std::ios::in);
  Int_t runno = -1;
  IntVec runNos;
  while(input >> runno){
    runNos.push_back(runno);
  }
  input.close();
  
  // need this to do the wonky binning to get means/sigmas to line up exactly with run number
  DblVec dRunNos;
  Int_t totalRuns = runNos.back()-runNos.front();
  Int_t startrun  = runNos.front();
  for (Int_t i = 0; i < totalRuns + 2; i++) { // +1 for subtraction, +1 for half offset
    dRunNos.push_back(startrun + i - 0.5);
  }
  //actually define plots
  TH2Map  runs2DMap;    
  TStrMap runs2DSubMap; 
  if (!fIsMC) { // only do for data... declarations are small, don't bother with check
    runs2DMap["tdEBEB_runs"] = Analysis::MakeTH2Plot("tdEBEB_runs","",dRunNos,ntimebins,-timerange,timerange,"Run Number","Dielectron Seed Time Difference [ns] (EBEB)",runs2DSubMap,"timing/runs/EBEB");  
    runs2DMap["tdEBEE_runs"] = Analysis::MakeTH2Plot("tdEBEE_runs","",dRunNos,ntimebins,-timerange,timerange,"Run Number","Dielectron Seed Time Difference [ns] (EBEE)",runs2DSubMap,"timing/runs/EBEE");  
    runs2DMap["tdEEEE_runs"] = Analysis::MakeTH2Plot("tdEEEE_runs","",dRunNos,ntimebins,-timerange,timerange,"Run Number","Dielectron Seed Time Difference [ns] (EEEE)",runs2DSubMap,"timing/runs/EEEE");  
  }  

  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++){
    fInTree->GetEntry(entry);
    if (zmass>Config::zlow && zmass<Config::zup) { // extra selection over skims
      Float_t weight = -1.;
      if   (fIsMC) {weight = (xsec * Config::lumi * wgt / wgtsum) * fPUweights[nvtx];}
      else         {weight = 1.0;}

      Float_t time_diff  = el1time-el2time;
      Float_t eff_dielpt = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));

      // Electron based bins first
      Bool_t el1eb = false; Bool_t el1ee = false;
      if ( std::abs(el1eta) < 1.4442 ) {
	el1eb = true;
      }
      else if (std::abs(el1eta) > 1.566 && std::abs(el1eta) < 2.5) {
	el1ee = true;
      }

      Bool_t el2eb = false; Bool_t el2ee = false;
      if ( std::abs(el2eta) < 1.4442 ) {
	el2eb = true;
      }
      else if (std::abs(el2eta) > 1.566 && std::abs(el2eta) < 2.5) {
	el2ee = true;
      }

      // electron eta categories 
      if (el1eb && el2eb) {
	inclu1DMap["tdEBEB_inclu"]->Fill(time_diff,weight);
 	effpt2DMap["tdEBEB_effpt"]->Fill(eff_dielpt,time_diff,weight);
	if (!fIsMC) {runs2DMap["tdEBEB_runs"]->Fill(run,time_diff,weight);}
      }
      else if ( (el1eb && el2ee) || (el1ee && el2eb) ) {
	inclu1DMap["tdEBEE_inclu"]->Fill(time_diff,weight);
 	effpt2DMap["tdEBEE_effpt"]->Fill(eff_dielpt,time_diff,weight);
	if (!fIsMC) {runs2DMap["tdEBEE_runs"]->Fill(run,time_diff,weight);}
      }
      else if (el1ee && el2ee) {
	inclu1DMap["tdEEEE_inclu"]->Fill(time_diff,weight);
 	effpt2DMap["tdEEEE_effpt"]->Fill(eff_dielpt,time_diff,weight);
	if (!fIsMC) {runs2DMap["tdEEEE_runs"]->Fill(run,time_diff,weight);}
      }
    
      // Now Z based plots
      z2DMap["td_zpt"]->Fill(zpt,time_diff,weight);
      z2DMap["td_zeta"]->Fill(zeta,time_diff,weight);
    }
  }

  ////////////////////////////////////
  // Do inclusive "global" plots first
  MakeSubDirs(inclu1DSubMap,fOutDir);
  Analysis::SaveTH1s(inclu1DMap,inclu1DSubMap);
  Analysis::DeleteTH1s(inclu1DMap);
  ////////////////////////////////////

  ////////////////////////////////////////////////////////
  // ProduceMeanSigma implicitly saves undlerlying TH1 //
  ///////////////////////////////////////////////////////

  ////////////////////////////////////
  // Do z plots second
  MakeSubDirs(z2DSubMap,fOutDir);
  Analysis::SaveTH2s(z2DMap,z2DSubMap);

  // zpt
  TH1Map zpt1DMap;  TStrMap zpt1DSubMap;
  Analysis::Project2Dto1D(z2DMap["td_zpt"],z2DSubMap,zpt1DMap,zpt1DSubMap);
  Analysis::ProduceMeanSigma(zpt1DMap,"td_zpt","Z p_{T} [GeV/c]",zptbins,fitrange,z2DSubMap["td_zpt"]);

  // zeta
  TH1Map zeta1DMap;  TStrMap zeta1DSubMap;
  Analysis::Project2Dto1D(z2DMap["td_zeta"],z2DSubMap,zeta1DMap,zeta1DSubMap);
  Analysis::ProduceMeanSigma(zeta1DMap,"td_zeta","Z #eta",zetabins,fitrange,z2DSubMap["td_zeta"]);

  // Delete z's
  Analysis::DeleteTH2s(z2DMap);
  Analysis::DeleteTH1s(zpt1DMap);
  Analysis::DeleteTH1s(zeta1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do effective electron pt plots next
  MakeSubDirs(effpt2DSubMap,fOutDir);
  Analysis::SaveTH2s(effpt2DMap,effpt2DSubMap);

  // EBEB
  TH1Map effptEBEB1DMap;  TStrMap effptEBEB1DSubMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEBEB_effpt"],effpt2DSubMap,effptEBEB1DMap,effptEBEB1DSubMap);
  Analysis::ProduceMeanSigma(effptEBEB1DMap,"tdEBEB_effpt","Effective Dielectron p_{T} [GeV/c] (EBEB)",effptbins,fitrange,effpt2DSubMap["tdEBEB_effpt"]);

  // EBEE
  TH1Map effptEBEE1DMap;  TStrMap effptEBEE1DSubMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEBEE_effpt"],effpt2DSubMap,effptEBEE1DMap,effptEBEE1DSubMap);
  Analysis::ProduceMeanSigma(effptEBEE1DMap,"tdEBEE_effpt","Effective Dielectron p_{T} [GeV/c] (EBEE)",effptbins,fitrange,effpt2DSubMap["tdEBEE_effpt"]);

  // EEEE
  TH1Map effptEEEE1DMap;  TStrMap effptEEEE1DSubMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEEEE_effpt"],effpt2DSubMap,effptEEEE1DMap,effptEEEE1DSubMap);
  Analysis::ProduceMeanSigma(effptEEEE1DMap,"tdEEEE_effpt","Effective Dielectron p_{T} [GeV/c] (EEEE)",effptbins,fitrange,effpt2DSubMap["tdEEEE_effpt"]);

  // Delete effective electron pt plots
  Analysis::DeleteTH2s(effpt2DMap);
  Analysis::DeleteTH1s(effptEBEB1DMap);
  Analysis::DeleteTH1s(effptEBEE1DMap);
  Analysis::DeleteTH1s(effptEEEE1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do run plots last
  if (!fIsMC) {
    MakeSubDirs(runs2DSubMap,fOutDir);
    Analysis::SaveTH2s(runs2DMap,runs2DSubMap);

    // EBEB
    TH1Map runsEBEB1DMap;  TStrMap runsEBEB1DSubMap;
    Analysis::Project2Dto1D(runs2DMap["tdEBEB_runs"],runs2DSubMap,runsEBEB1DMap,runsEBEB1DSubMap);
    Analysis::ProduceMeanSigma(runsEBEB1DMap,"tdEBEB_runs","Run Number (EBEB)",dRunNos,fitrange,runs2DSubMap["tdEBEB_runs"]);

    // EBEE
    TH1Map runsEBEE1DMap;  TStrMap runsEBEE1DSubMap;
    Analysis::Project2Dto1D(runs2DMap["tdEBEE_runs"],runs2DSubMap,runsEBEE1DMap,runsEBEE1DSubMap);
    Analysis::ProduceMeanSigma(runsEBEE1DMap,"tdEBEE_runs","Run Number (EBEE)",dRunNos,fitrange,runs2DSubMap["tdEBEE_runs"]);

    // EEEE
    TH1Map runsEEEE1DMap;  TStrMap runsEEEE1DSubMap;
    Analysis::Project2Dto1D(runs2DMap["tdEEEE_runs"],runs2DSubMap,runsEEEE1DMap,runsEEEE1DSubMap);
    Analysis::ProduceMeanSigma(runsEEEE1DMap,"tdEEEE_runs","Run Number (EEEE)",dRunNos,fitrange,runs2DSubMap["tdEEEE_runs"]);

    // Delete run plots
    Analysis::DeleteTH2s(runs2DMap);
    Analysis::DeleteTH1s(runsEBEB1DMap);
    Analysis::DeleteTH1s(runsEBEE1DMap);
    Analysis::DeleteTH1s(runsEEEE1DMap);
  }
  ////////////////////////////////////
}

void Analysis::TriggerEffs(){
  // Set up hists to save; 
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot

  Int_t nBinsDEEpt = 100; Float_t xLowDEEpt = 20.; Float_t xHighDEEpt = 120.;
  TH1F * n_hltdoubleel_el1pt = new TH1F("numer_hltdoubleel_el1pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);
  TH1F * d_hltdoubleel_el1pt = new TH1F("denom_hltdoubleel_el1pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);

  TH1F * n_hltdoubleel_el2pt = new TH1F("numer_hltdoubleel_el2pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);
  TH1F * d_hltdoubleel_el2pt = new TH1F("denom_hltdoubleel_el2pt","",nBinsDEEpt,xLowDEEpt,xHighDEEpt);

  trTH1Map["hltdoubleel_el1pt"] = Analysis::MakeTH1Plot("hltdoubleel_el1pt","Double Electron Trigger Efficiency vs. p_{T}",nBinsDEEpt,xLowDEEpt,xHighDEEpt,"Leading Electron p_{T} [GeV/c]","Efficiency",trTH1SubMap,"trigger_zoomest");
  trTH1Map["hltdoubleel_el2pt"] = Analysis::MakeTH1Plot("hltdoubleel_el2pt","Double Electron Trigger Efficiency vs. p_{T}",nBinsDEEpt,xLowDEEpt,xHighDEEpt,"Subleading Electron p_{T} [GeV/c]","Efficiency",trTH1SubMap,"trigger_zoomest");

  MakeSubDirs(trTH1SubMap,fOutDir);

  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++){
    fInTree->GetEntry(entry);
    if ( (zmass>Config::zlow && zmass<Config::zup) ){ // we want di-electron z's
      Float_t weight = -1.;
      if   (fIsMC) {weight = (xsec * Config::lumi * wgt / wgtsum) * fPUweights[nvtx];}
      else         {weight = 1.0;}

      if ( hltdoubleel ) { // fill numer if passed
	n_hltdoubleel_el1pt->Fill(el1pt,weight);
	n_hltdoubleel_el2pt->Fill(el2pt,weight);
      }
      // always fill denom
      d_hltdoubleel_el1pt->Fill(el1pt,weight);
      d_hltdoubleel_el2pt->Fill(el2pt,weight);
    }
  }
  
  ComputeRatioPlot(n_hltdoubleel_el1pt,d_hltdoubleel_el1pt,trTH1Map["hltdoubleel_el1pt"]);
  ComputeRatioPlot(n_hltdoubleel_el2pt,d_hltdoubleel_el2pt,trTH1Map["hltdoubleel_el2pt"]);
  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  Analysis::DumpTH1Names(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);

  // delete by hand throw away plots
  delete n_hltdoubleel_el1pt;
  delete d_hltdoubleel_el1pt;
  delete n_hltdoubleel_el2pt;
  delete d_hltdoubleel_el2pt;
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
  for (Int_t i = 1; i <= hist2d->GetNbinsX(); i++){  
    // if no bins are filled, then continue to next plot
    Bool_t isFilled = false;
    for (Int_t j = 0; j <= hist2d->GetNbinsY() + 1; j++) {
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
      Int_t runno = (xlow+xhigh)/2;
      histname = Form("%s_%i",basename.Data(),runno);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in Run: %i",ytitle.Data(),runno),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }
    else { // "normal" filling
      Int_t ixlow  = Int_t(xlow); 
      Int_t ixhigh = Int_t(xhigh); 
      histname = Form("%s_%i-%i",basename.Data(),ixlow,ixhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %i-%i",ytitle.Data(),xtitle.Data(),ixlow,ixhigh),
						      "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }

    // then fill corresponding bins from y
    for (Int_t j = 0; j <= hist2d->GetNbinsY() + 1; j++) {
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

void Analysis::ProduceMeanSigma(TH1Map & th1map, TString name, TString xtitle, const DblVec vxbins, Float_t fitrange, TString subdir){
  // need to convert bins into array
  const Double_t * axbins = &vxbins[0]; // https://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-c

  // initialize new mean/sigma histograms
  TH1F * outhist_mean  = new TH1F(Form("%s_mean",name.Data()),"",vxbins.size()-1,axbins);
  outhist_mean->GetXaxis()->SetTitle(xtitle.Data());
  outhist_mean->GetYaxis()->SetTitle("Dielectron Seed Time Mean [ns]");
  outhist_mean->SetLineColor(fColor);

  TH1F * outhist_sigma  = new TH1F(Form("%s_sigma",name.Data()),"",vxbins.size()-1,axbins);
  outhist_sigma->GetXaxis()->SetTitle(xtitle.Data());
  outhist_sigma->GetYaxis()->SetTitle("Dielectron Seed Time Sigma [ns]");
  outhist_sigma->SetLineColor(fColor);

  Int_t i = 1; // dummy counter (ROOT starts bins at 1)
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    TF1* gausfit = new TF1("gausfit","gaus",-fitrange,fitrange);
    (*mapiter).second->Fit("gausfit","R");

    Analysis::SaveTH1andFit((*mapiter).second,subdir,gausfit);

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
  CMSLumi(canv);
  canv->cd();

  outhist_mean->Draw("PE");
  canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdir.Data(),outhist_mean->GetName(),Config::outtype.Data()));

  outhist_sigma->Draw("PE");
  canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdir.Data(),outhist_sigma->GetName(),Config::outtype.Data()));
  
  delete canv;
  delete outhist_sigma;
  delete outhist_mean;
}

TH1F * Analysis::MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh, TString xtitle,
			     TString ytitle, TStrMap& subdirmap, TString subdir) {
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbins,xlow,xhigh);
  hist->SetLineColor(kBlack);
  if (fIsMC) hist->SetFillColor(fColor);
  hist->GetXaxis()->SetTitle(xtitle.Data());
  hist->GetYaxis()->SetTitle(ytitle.Data());
  hist->Sumw2();

  // cheat a bit and set subdir map here
  subdirmap[hname] = subdir;
  
  return hist;
}

TH2F * Analysis::MakeTH2Plot(TString hname, TString htitle, const DblVec vxbins, Int_t nbinsy, Double_t ylow, 
			     Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir) {
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

void Analysis::SaveTH1s(TH1Map & th1map, TStrMap & subdirmap) {
  fOutFile->cd();

  TCanvas * canv = new TCanvas();
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    
    (*mapiter).second->Write(); // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    canv->cd();
    (*mapiter).second->Draw( fIsMC ? "HIST" : "PE" );
    
    // first save as linear, then log
    canv->SetLogy(0);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));

    canv->SetLogy(1);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));
  }

  delete canv;
}

void Analysis::SaveTH1andFit(TH1F * hist, TString subdir, TF1 * fit) {
  fOutFile->cd();

  TCanvas * canv = new TCanvas();
  hist->Write(); 

  // now draw onto canvas to save as png
  canv->cd();
  hist->Draw("PE");
  fit->Draw("same");
    
  // first save as linear, then log
  canv->SetLogy(0);
  CMSLumi(canv);
  canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdir.Data(),hist->GetName(),Config::outtype.Data()));
  
  canv->SetLogy(1);
  CMSLumi(canv);
  canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdir.Data(),hist->GetName(),Config::outtype.Data()));

  delete canv;
}

void Analysis::SaveTH2s(TH2Map & th2map, TStrMap & subdirmap) {
  fOutFile->cd();

  TCanvas * canv = new TCanvas();
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); mapiter++) { 
    
    (*mapiter).second->Write(); // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    canv->cd();
    (*mapiter).second->Draw("colz");
    
    // only save as linear
    canv->SetLogy(0);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));
  }

  delete canv;
}

void Analysis::DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap) {
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); mapiter++) { 
    fTH1Dump << (*mapiter).first.Data()  << " " <<  subdirmap[(*mapiter).first].Data() << std::endl;
  }
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

void Analysis::InitTree() {
  // Set branch addresses and branch pointers
  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("hltsingleel", &hltsingleel, &b_hltsingleel);
  fInTree->SetBranchAddress("hltdoubleel", &hltdoubleel, &b_hltdoubleel);
  fInTree->SetBranchAddress("hltelnoiso", &hltelnoiso, &b_hltelnoiso);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
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
  fInTree->SetBranchAddress("zmass", &zmass, &b_zmass);
  fInTree->SetBranchAddress("zpt", &zpt, &b_zpt);
  fInTree->SetBranchAddress("zeta", &zeta, &b_zeta);
  fInTree->SetBranchAddress("zphi", &zphi, &b_zphi);

  if (fIsMC){ // initialize extra branches if MC
    fInTree->SetBranchAddress("puobs", &puobs, &b_puobs);
    fInTree->SetBranchAddress("putrue", &putrue, &b_putrue);
    fInTree->SetBranchAddress("xsec", &xsec, &b_xsec);
    fInTree->SetBranchAddress("wgt", &wgt, &b_wgt);
    fInTree->SetBranchAddress("genzpid", &genzpid, &b_genzpid);
    fInTree->SetBranchAddress("genzpt", &genzpt, &b_genzpt);
    fInTree->SetBranchAddress("genzeta", &genzeta, &b_genzeta);
    fInTree->SetBranchAddress("genzphi", &genzphi, &b_genzphi);
    fInTree->SetBranchAddress("genzmass", &genzmass, &b_genzmass);
    fInTree->SetBranchAddress("genel1pid", &genel1pid, &b_genel1pid);
    fInTree->SetBranchAddress("genel1pt", &genel1pt, &b_genel1pt);
    fInTree->SetBranchAddress("genel1eta", &genel1eta, &b_genel1eta);
    fInTree->SetBranchAddress("genel1phi", &genel1phi, &b_genel1phi);
    fInTree->SetBranchAddress("genel2pid", &genel2pid, &b_genel2pid);
    fInTree->SetBranchAddress("genel2pt", &genel2pt, &b_genel2pt);
    fInTree->SetBranchAddress("genel2eta", &genel2eta, &b_genel2eta);
    fInTree->SetBranchAddress("genel2phi", &genel2phi, &b_genel2phi);
    fInTree->SetBranchAddress("wgtsum", &wgtsum, &b_wgtsum);
  }
}
