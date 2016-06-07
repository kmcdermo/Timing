#include "../interface/Analysis.hh"

#include "TH1D.h"

inline Float_t rad2(Float_t x, Float_t y){
  return x*x + y*y;
}

Analysis::Analysis(TString sample, Bool_t isMC) : fSample(sample), fIsMC(isMC) {
  // Set input
  TString filename = Form("input/%s/%s/%s", (fIsMC?"MC":"DATA"), fSample.Data(), (Config::useFull?"tree.root":"skimmedtree.root"));
  fInFile  = TFile::Open(filename.Data());
  CheckValidFile(fInFile,filename);

  TString treename = "tree/tree";
  fInTree = (TTree*)fInFile->Get(treename.Data());
  CheckValidTree(fInTree,treename,filename);
  InitTree();
  
  // Set Output Stuff
  fOutDir = Form("%s/%s/%s",Config::outdir.Data(), (fIsMC?"MC":"DATA"), fSample.Data());
  MakeOutDir(fOutDir);
  fOutFile = new TFile(Form("%s/plots.root",fOutDir.Data()),"UPDATE");
  fColor = (fIsMC?Config::colorMap[fSample]:kBlack);
  
  // extra setup for data and MC
  if (fIsMC) { 
    // Get pile-up weights
    TString purwfname = "input/PU/pileupWeights.root";
    TFile * purwfile  = TFile::Open(purwfname.Data());
    CheckValidFile(purwfile,purwfname);

    TH1D * gen_pu    = (TH1D*) purwfile->Get("generated_pu");
    CheckValidTH1D(gen_pu,"generated_pu",purwfname);
    TH1D * puweights = (TH1D*) purwfile->Get("weights");
    CheckValidTH1D(puweights,"weights",purwfname);

    TH1D* weightedPU= (TH1D*)gen_pu->Clone("weightedPU");
    weightedPU->Multiply(puweights);
    TH1D* weights = (TH1D*)puweights->Clone("rescaledWeights");
    weights->Scale( gen_pu->Integral(1,Config::nvtxbins) / weightedPU->Integral(1,Config::nvtxbins) );

    for (Int_t i = 1; i <= Config::nvtxbins; i++){
      fPUweights.push_back(weights->GetBinContent(i));
    }

    delete gen_pu;
    delete puweights;
    delete weightedPU;
    delete weights;
    delete purwfile;
    // end getting pile-up weights

    // set sample xsec + wgtsum
    fXsec   = Config::SampleXsecMap[fSample];
    fWgtsum = Config::SampleWgtsumMap[fSample];
  }  
  else{
    fTH1Dump.open(Form("%s/%s",Config::outdir.Data(),Config::plotdumpname.Data()),std::ios_base::trunc); // do this once, and just do it for data
  }
}

Analysis::~Analysis(){
  delete fInTree;
  delete fInFile;
  delete fOutFile;
  if (!fIsMC) fTH1Dump.close();
}

void Analysis::StandardPlots(){
  // Set up hists first --> first in map is histo name, by design!
  TH1Map  trTH1Map;
  TStrMap trTH1SubMap; // set inside MakeTH1Plot
  trTH1Map["nvtx"]   = Analysis::MakeTH1Plot("nvtx","",Config::nvtxbins,0.,Double_t(Config::nvtxbins),"nVertices","Events",trTH1SubMap,"standard");
  trTH1Map["zmass"]  = Analysis::MakeTH1Plot("zmass","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EBEB"]  = Analysis::MakeTH1Plot("zmass_EBEB","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EBEB)","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EEEE"]  = Analysis::MakeTH1Plot("zmass_EEEE","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EEEE)","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EBEE"]  = Analysis::MakeTH1Plot("zmass_EBEE","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EBEE)","Events",trTH1SubMap,"standard");
  trTH1Map["zpt"]    = Analysis::MakeTH1Plot("zpt","",100,0.,750.,"Dielectron p_{T} [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zeta"]   = Analysis::MakeTH1Plot("zeta","",100,-10.0,10.0,"Dielectron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["zphi"]   = Analysis::MakeTH1Plot("zphi","",100,-3.2,3.2,"Dielectron #phi","Events",trTH1SubMap,"standard");

  trTH1Map["timediff"]   = Analysis::MakeTH1Plot("timediff","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["eff_dielpt"] = Analysis::MakeTH1Plot("eff_dielpt","",100,0.,250.,"Effective Dielectron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");

  trTH1Map["el1pt"]  = Analysis::MakeTH1Plot("el1pt","",100,0.,700.,"Leading Electron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
  trTH1Map["el1eta"] = Analysis::MakeTH1Plot("el1eta","",100,-3.0,3.0,"Leading Electron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el1phi"] = Analysis::MakeTH1Plot("el1phi","",100,-3.2,3.2,"Leading Electron #phi","Events",trTH1SubMap,"standard");
  trTH1Map["el2pt"]  = Analysis::MakeTH1Plot("el2pt","",100,0.,300.,"Subleading Electron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
  trTH1Map["el2eta"] = Analysis::MakeTH1Plot("el2eta","",100,-3.0,3.0,"Subleading Electron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el2phi"] = Analysis::MakeTH1Plot("el2phi","",100,-3.2,3.2,"Subleading Electron #phi","Events",trTH1SubMap,"standard");

  trTH1Map["el1time"]    = Analysis::MakeTH1Plot("el1time","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Seed Time [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el1time_EB"] = Analysis::MakeTH1Plot("el1time_EB","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EB Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el1time_EE"] = Analysis::MakeTH1Plot("el1time_EE","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EE Inclusive)","Events",trTH1SubMap,"standard");  

  trTH1Map["el2time"]    = Analysis::MakeTH1Plot("el2time","",Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Seed Time [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el2time_EB"] = Analysis::MakeTH1Plot("el2time_EB","",Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Time [ns] (EB Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el2time_EE"] = Analysis::MakeTH1Plot("el2time_EE","",Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Time [ns] (EE Inclusive)","Events",trTH1SubMap,"standard");  

  MakeSubDirs(trTH1SubMap,fOutDir);

  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++){
    fInTree->GetEntry(entry);
    if ((zmass>Config::zlow && zmass<Config::zhigh) && hltdoubleel && (el1pid == -el2pid)) { // extra selection over skimmed samples
      Float_t weight = -1.;
      if   (fIsMC) {weight = (fXsec * Config::lumi * wgt / fWgtsum) * fPUweights[putrue];}
      else         {weight = 1.0;}

      Bool_t el1eb = false; Bool_t el1ee = false;
      if      (std::abs(el1eta) < Config::etaEB)                                            { el1eb = true; }
      else if (std::abs(el1eta) > Config::etaEElow && std::abs(el1eta) < Config::etaEEhigh) { el1ee = true; }

      Bool_t el2eb = false; Bool_t el2ee = false;
      if      (std::abs(el2eta) < Config::etaEB)                                            { el2eb = true; }
      else if (std::abs(el2eta) > Config::etaEElow && std::abs(el2eta) < Config::etaEEhigh) { el2ee = true; }

      // standard "validation" plots
      trTH1Map["nvtx"]->Fill(nvtx,weight);
      trTH1Map["zmass"]->Fill(zmass,weight);
      trTH1Map["zpt"]->Fill(zpt,weight);
      trTH1Map["zeta"]->Fill(zeta,weight);
      trTH1Map["zphi"]->Fill(zphi,weight);

      trTH1Map["timediff"]->Fill(el1time-el2time,weight);
      Float_t eff_dielpt = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));
      trTH1Map["eff_dielpt"]->Fill(eff_dielpt,weight);

      trTH1Map["el1pt"]->Fill(el1pt,weight);
      trTH1Map["el1eta"]->Fill(el1eta,weight);
      trTH1Map["el1phi"]->Fill(el1phi,weight);
      trTH1Map["el2pt"]->Fill(el2pt,weight);
      trTH1Map["el2eta"]->Fill(el2eta,weight);
      trTH1Map["el2phi"]->Fill(el2phi,weight); 

      trTH1Map["el1time"]->Fill(el1time,weight);
      if      (el1eb) trTH1Map["el1time_EB"]->Fill(el1time,weight);
      else if (el1ee) trTH1Map["el1time_EE"]->Fill(el1time,weight);
      trTH1Map["el2time"]->Fill(el2time,weight);
      if      (el2eb) trTH1Map["el2time_EB"]->Fill(el2time,weight);
      else if (el2ee) trTH1Map["el2time_EE"]->Fill(el2time,weight);


      if      (el1eb && el2eb)                       { trTH1Map["zmass_EBEB"]->Fill(zmass,weight); }
      else if (el1ee && el2ee)                       { trTH1Map["zmass_EEEE"]->Fill(zmass,weight); }
      else if ((el1eb && el2ee) || (el1ee && el2eb)) { trTH1Map["zmass_EBEE"]->Fill(zmass,weight); }
    }
  }

  Analysis::SaveTH1s(trTH1Map,trTH1SubMap);
  if (!fIsMC) Analysis::DumpTH1Names(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);
}

void Analysis::TimeResPlots(){
  // inclusive "global" time differences by eta + single electron times
  TH1Map  inclu1DMap;
  TStrMap inclu1DSubMap; // set inside MakeTH1Plot
  inclu1DMap["tdEBEB_inclusive"] = Analysis::MakeTH1Plot("tdEBEB_inclusive","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns] (EBEB Inclusive)","Events",inclu1DSubMap,"timing/effpt/EBEB");  
  inclu1DMap["tdEBEE_inclusive"] = Analysis::MakeTH1Plot("tdEBEE_inclusive","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns] (EBEE Inclusive)","Events",inclu1DSubMap,"timing/effpt/EBEE");  
  inclu1DMap["tdEEEE_inclusive"] = Analysis::MakeTH1Plot("tdEEEE_inclusive","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns] (EEEE Inclusive)","Events",inclu1DSubMap,"timing/effpt/EEEE");  

  // make 2D plots for Z variables (pT, eta)
  TH2Map  z2DMap;      
  TStrMap z2DSubMap;
  DblVec zptbins = {0,10,20,30,50,70,100,150,200,250,300,750};
  z2DMap["td_zpt"]  = Analysis::MakeTH2Plot("td_zpt","",zptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Z p_{T} [GeV/c]","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/z/pt");  

  DblVec zetabins; for (Int_t i = 0; i < 21; i++){zetabins.push_back(i/2. - 5.0);}
  z2DMap["td_zeta"] = Analysis::MakeTH2Plot("td_zeta","",zetabins,Config::ntimebins,-Config::timerange,Config::timerange,"Z #eta","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/z/eta");  

  DblVec abszetabins; for (Int_t i = 0; i < 21; i++){abszetabins.push_back(i/4.);}
  z2DMap["td_abszeta"] = Analysis::MakeTH2Plot("td_abszeta","",abszetabins,Config::ntimebins,-Config::timerange,Config::timerange,"Z abs(#eta)","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/z/abseta");  

  DblVec zphibins; for (Int_t i = 0; i < 21; i++){zphibins.push_back(i*0.32 - 3.2);}
  z2DMap["td_zphi"] = Analysis::MakeTH2Plot("td_zphi","",zphibins,Config::ntimebins,-Config::timerange,Config::timerange,"Z #phi","Dielectron Seed Time Difference [ns]",z2DSubMap,"timing/z/phi");  

  // make 2D plots for effective electron pt, for EBEB, EBEE, EEEE
  TH2Map  effpt2DMap;    
  TStrMap effpt2DSubMap; 
  DblVec  effptbins = {20,25,30,35,40,50,70,250}; //effective el pt bins
  effpt2DMap["tdEBEB_effpt"] = Analysis::MakeTH2Plot("tdEBEB_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEB)",effpt2DSubMap,"timing/effpt/EBEB");  
  effpt2DMap["tdEBEE_effpt"] = Analysis::MakeTH2Plot("tdEBEE_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEE)",effpt2DSubMap,"timing/effpt/EBEE");  
  effpt2DMap["tdEEEE_effpt"] = Analysis::MakeTH2Plot("tdEEEE_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EEEE)",effpt2DSubMap,"timing/effpt/EEEE");  

  // make 2D plots for single electron timing pt, for two categories
  TH2Map  el1pt2DMap;    
  TStrMap el1pt2DSubMap; 
  DblVec  el1ptbins = {20,25,30,35,40,50,70,100,150,200,750}; // el1 pt bins
  el1pt2DMap["el1pt_EB"] = Analysis::MakeTH2Plot("el1pt_EB","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EB)",el1pt2DSubMap,"timing/el1/pt/EB");  
  el1pt2DMap["el1pt_EE"] = Analysis::MakeTH2Plot("el1pt_EE","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EE)",el1pt2DSubMap,"timing/el1/pt/EE");  

  TH2Map  el2pt2DMap;    
  TStrMap el2pt2DSubMap; 
  DblVec  el2ptbins = {20,25,30,35,40,50,70,100,300}; // el2 pt bins
  el2pt2DMap["el2pt_EB"] = Analysis::MakeTH2Plot("el2pt_EB","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EB)",el2pt2DSubMap,"timing/el2/pt/EB");  
  el2pt2DMap["el2pt_EE"] = Analysis::MakeTH2Plot("el2pt_EE","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EE)",el2pt2DSubMap,"timing/el2/pt/EE");  

  // and also electron eta
  TH2Map  eleta2DMap;    
  TStrMap eleta2DSubMap; 
  DblVec  eletabins; for (Int_t i = 0; i < 21; i++){eletabins.push_back(i/4. - 2.5);}
  eleta2DMap["el1eta_time"] = Analysis::MakeTH2Plot("el1eta_time","",eletabins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron #eta","Leading Electron Seed Time [ns]",eleta2DSubMap,"timing/el1/eta");  
  eleta2DMap["el2eta_time"] = Analysis::MakeTH2Plot("el2eta_time","",eletabins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron #eta","Subleading Electron Seed Time [ns]",eleta2DSubMap,"timing/el2/eta");  

  // 2D plots for run numbers
  // read in run numbers
  std::ifstream input;
  input.open(Config::runs.Data(),std::ios::in);
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
  if (!fIsMC && !Config::skipRuns) { // only do for data... declarations are small, don't bother with check
    runs2DMap["tdEBEB_runs"] = Analysis::MakeTH2Plot("tdEBEB_runs","",dRunNos,Config::ntimebins,-Config::timerange,Config::timerange,"Run Number","Dielectron Seed Time Difference [ns] (EBEB)",runs2DSubMap,"timing/runs/EBEB");  
    runs2DMap["tdEBEE_runs"] = Analysis::MakeTH2Plot("tdEBEE_runs","",dRunNos,Config::ntimebins,-Config::timerange,Config::timerange,"Run Number","Dielectron Seed Time Difference [ns] (EBEE)",runs2DSubMap,"timing/runs/EBEE");  
    runs2DMap["tdEEEE_runs"] = Analysis::MakeTH2Plot("tdEEEE_runs","",dRunNos,Config::ntimebins,-Config::timerange,Config::timerange,"Run Number","Dielectron Seed Time Difference [ns] (EEEE)",runs2DSubMap,"timing/runs/EEEE");  
  }  

  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++){
    fInTree->GetEntry(entry);
    if ((zmass>Config::zlow && zmass<Config::zhigh) && hltdoubleel && (el1pid == -el2pid)) { // extra selection over skims
      Float_t weight = -1.;
      if   (fIsMC) {weight = (fXsec * Config::lumi * wgt / fWgtsum) * fPUweights[putrue];}
      else         {weight = 1.0;}

      Float_t time_diff  = el1time-el2time;
      Float_t eff_dielpt = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));

      // Electron based bins first
      Bool_t el1eb = false; Bool_t el1ee = false;
      if ( std::abs(el1eta) < Config::etaEB ) {
	el1eb = true;
      }
      else if (std::abs(el1eta) > Config::etaEElow && std::abs(el1eta) < Config::etaEEhigh) {
	el1ee = true;
      }

      Bool_t el2eb = false; Bool_t el2ee = false;
      if ( std::abs(el2eta) < Config::etaEB ) {
	el2eb = true;
      }
      else if (std::abs(el2eta) > Config::etaEElow && std::abs(el2eta) < Config::etaEEhigh) {
	el2ee = true;
      }
      
      // single electron etas
      eleta2DMap["el1eta_time"]->Fill(el1eta,el1time,weight);
      eleta2DMap["el2eta_time"]->Fill(el2eta,el2time,weight);

      // single electron categories
      if      (el1eb) el1pt2DMap["el1pt_EB"]->Fill(el1pt,el1time,weight);
      else if (el1ee) el1pt2DMap["el1pt_EE"]->Fill(el1pt,el1time,weight);

      if      (el2eb) el2pt2DMap["el2pt_EB"]->Fill(el2pt,el2time,weight);
      else if (el2ee) el2pt2DMap["el2pt_EE"]->Fill(el2pt,el2time,weight);

      // electron eta categories 
      if (el1eb && el2eb) {
	inclu1DMap["tdEBEB_inclusive"]->Fill(time_diff,weight);
 	effpt2DMap["tdEBEB_effpt"]->Fill(eff_dielpt,time_diff,weight);
	if (!fIsMC && !Config::skipRuns) {runs2DMap["tdEBEB_runs"]->Fill(run,time_diff,weight);}
      }
      else if ( (el1eb && el2ee) || (el1ee && el2eb) ) {
	inclu1DMap["tdEBEE_inclusive"]->Fill(time_diff,weight);
 	effpt2DMap["tdEBEE_effpt"]->Fill(eff_dielpt,time_diff,weight);
	if (!fIsMC && !Config::skipRuns) {runs2DMap["tdEBEE_runs"]->Fill(run,time_diff,weight);}
      }
      else if (el1ee && el2ee) {
	inclu1DMap["tdEEEE_inclusive"]->Fill(time_diff,weight);
 	effpt2DMap["tdEEEE_effpt"]->Fill(eff_dielpt,time_diff,weight);
	if (!fIsMC && !Config::skipRuns) {runs2DMap["tdEEEE_runs"]->Fill(run,time_diff,weight);}
      }
    
      // Now Z based plots
      z2DMap["td_zpt"]->Fill(zpt,time_diff,weight);
      z2DMap["td_zeta"]->Fill(zeta,time_diff,weight);
      z2DMap["td_abszeta"]->Fill(std::abs(zeta),time_diff,weight);
      z2DMap["td_zphi"]->Fill(zphi,time_diff,weight);
    }
  }

  ////////////////////////////////////
  // Do inclusive "global" plots first
  MakeSubDirs(inclu1DSubMap,fOutDir);
  Analysis::SaveTH1s(inclu1DMap,inclu1DSubMap);
  if (!fIsMC) Analysis::DumpTH1Names(inclu1DMap,inclu1DSubMap);
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
  TH1Map zpt1DMap; TStrMap zpt1DSubMap; TStrIntMap zpt1DbinMap;
  Analysis::Project2Dto1D(z2DMap["td_zpt"],z2DSubMap,zpt1DMap,zpt1DSubMap,zpt1DbinMap);
  Analysis::ProduceMeanSigma(zpt1DMap,zpt1DbinMap,"td_zpt","Z p_{T} [GeV/c]",zptbins,z2DSubMap["td_zpt"]);

  // zeta
  TH1Map zeta1DMap; TStrMap zeta1DSubMap; TStrIntMap zeta1DbinMap;
  Analysis::Project2Dto1D(z2DMap["td_zeta"],z2DSubMap,zeta1DMap,zeta1DSubMap,zeta1DbinMap);
  Analysis::ProduceMeanSigma(zeta1DMap,zeta1DbinMap,"td_zeta","Z #eta",zetabins,z2DSubMap["td_zeta"]);

  // abszeta
  TH1Map abszeta1DMap; TStrMap abszeta1DSubMap; TStrIntMap abszeta1DbinMap;
  Analysis::Project2Dto1D(z2DMap["td_abszeta"],z2DSubMap,abszeta1DMap,abszeta1DSubMap,abszeta1DbinMap);
  Analysis::ProduceMeanSigma(abszeta1DMap,abszeta1DbinMap,"td_abszeta","Z #eta",abszetabins,z2DSubMap["td_abszeta"]);

  // zphi
  TH1Map zphi1DMap; TStrMap zphi1DSubMap; TStrIntMap zphi1DbinMap;
  Analysis::Project2Dto1D(z2DMap["td_zphi"],z2DSubMap,zphi1DMap,zphi1DSubMap,zphi1DbinMap);
  Analysis::ProduceMeanSigma(zphi1DMap,zphi1DbinMap,"td_zphi","Z #phi",zphibins,z2DSubMap["td_zphi"]);

  // Delete z's
  Analysis::DeleteTH2s(z2DMap);
  Analysis::DeleteTH1s(zpt1DMap);
  Analysis::DeleteTH1s(zeta1DMap);
  Analysis::DeleteTH1s(abszeta1DMap);
  Analysis::DeleteTH1s(zphi1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do effective electron pt plots next
  MakeSubDirs(effpt2DSubMap,fOutDir);
  Analysis::SaveTH2s(effpt2DMap,effpt2DSubMap);

  // EBEB
  TH1Map effptEBEB1DMap; TStrMap effptEBEB1DSubMap; TStrIntMap effptEBEB1DbinMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEBEB_effpt"],effpt2DSubMap,effptEBEB1DMap,effptEBEB1DSubMap,effptEBEB1DbinMap);
  Analysis::ProduceMeanSigma(effptEBEB1DMap,effptEBEB1DbinMap,"tdEBEB_effpt","Effective Dielectron p_{T} [GeV/c] (EBEB)",effptbins,effpt2DSubMap["tdEBEB_effpt"]);

  // EBEE
  TH1Map effptEBEE1DMap; TStrMap effptEBEE1DSubMap; TStrIntMap effptEBEE1DbinMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEBEE_effpt"],effpt2DSubMap,effptEBEE1DMap,effptEBEE1DSubMap,effptEBEE1DbinMap);
  Analysis::ProduceMeanSigma(effptEBEE1DMap,effptEBEE1DbinMap,"tdEBEE_effpt","Effective Dielectron p_{T} [GeV/c] (EBEE)",effptbins,effpt2DSubMap["tdEBEE_effpt"]);

  // EEEE
  TH1Map effptEEEE1DMap; TStrMap effptEEEE1DSubMap; TStrIntMap effptEEEE1DbinMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEEEE_effpt"],effpt2DSubMap,effptEEEE1DMap,effptEEEE1DSubMap,effptEEEE1DbinMap);
  Analysis::ProduceMeanSigma(effptEEEE1DMap,effptEEEE1DbinMap,"tdEEEE_effpt","Effective Dielectron p_{T} [GeV/c] (EEEE)",effptbins,effpt2DSubMap["tdEEEE_effpt"]);

  // Delete effective electron pt plots
  Analysis::DeleteTH2s(effpt2DMap);
  Analysis::DeleteTH1s(effptEBEB1DMap);
  Analysis::DeleteTH1s(effptEBEE1DMap);
  Analysis::DeleteTH1s(effptEEEE1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron pt time - el1 first
  MakeSubDirs(el1pt2DSubMap,fOutDir);
  Analysis::SaveTH2s(el1pt2DMap,el1pt2DSubMap);

  // EB
  TH1Map el1ptEB1DMap; TStrMap el1ptEB1DSubMap; TStrIntMap el1ptEB1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_EB"],el1pt2DSubMap,el1ptEB1DMap,el1ptEB1DSubMap,el1ptEB1DbinMap);
  Analysis::ProduceMeanSigma(el1ptEB1DMap,el1ptEB1DbinMap,"el1pt_EB","Leading Electron p_{T} [GeV/c] (EB)",el1ptbins,el1pt2DSubMap["el1pt_EB"]);

  // EE
  TH1Map el1ptEE1DMap; TStrMap el1ptEE1DSubMap; TStrIntMap el1ptEE1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_EE"],el1pt2DSubMap,el1ptEE1DMap,el1ptEE1DSubMap,el1ptEE1DbinMap);
  Analysis::ProduceMeanSigma(el1ptEE1DMap,el1ptEE1DbinMap,"el1pt_EE","Leading Electron p_{T} [GeV/c] (EE)",el1ptbins,el1pt2DSubMap["el1pt_EE"]);

  // Delete el1 pt plots
  Analysis::DeleteTH2s(el1pt2DMap);
  Analysis::DeleteTH1s(el1ptEB1DMap);
  Analysis::DeleteTH1s(el1ptEE1DMap);

  // el2 second
  MakeSubDirs(el2pt2DSubMap,fOutDir);
  Analysis::SaveTH2s(el2pt2DMap,el2pt2DSubMap);

  // EB
  TH1Map el2ptEB1DMap; TStrMap el2ptEB1DSubMap; TStrIntMap el2ptEB1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_EB"],el2pt2DSubMap,el2ptEB1DMap,el2ptEB1DSubMap,el2ptEB1DbinMap);
  Analysis::ProduceMeanSigma(el2ptEB1DMap,el2ptEB1DbinMap,"el2pt_EB","Leading Electron p_{T} [GeV/c] (EB)",el2ptbins,el2pt2DSubMap["el2pt_EB"]);

  // EE
  TH1Map el2ptEE1DMap; TStrMap el2ptEE1DSubMap; TStrIntMap el2ptEE1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_EE"],el2pt2DSubMap,el2ptEE1DMap,el2ptEE1DSubMap,el2ptEE1DbinMap);
  Analysis::ProduceMeanSigma(el2ptEE1DMap,el2ptEE1DbinMap,"el2pt_EE","Leading Electron p_{T} [GeV/c] (EE)",el2ptbins,el2pt2DSubMap["el2pt_EE"]);

  // Delete el2 pt plots
  Analysis::DeleteTH2s(el2pt2DMap);
  Analysis::DeleteTH1s(el2ptEB1DMap);
  Analysis::DeleteTH1s(el2ptEE1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron eta plots
  MakeSubDirs(eleta2DSubMap,fOutDir);
  Analysis::SaveTH2s(eleta2DMap,eleta2DSubMap);

  TH1Map el1eta1DMap; TStrMap el1eta1DSubMap; TStrIntMap el1eta1DbinMap;
  Analysis::Project2Dto1D(eleta2DMap["el1eta_time"],el1pt2DSubMap,el1eta1DMap,el1eta1DSubMap,el1eta1DbinMap);
  Analysis::ProduceMeanSigma(el1eta1DMap,el1eta1DbinMap,"el1eta_time","Leading Electron #eta",eletabins,eleta2DSubMap["el1eta_time"]);

  TH1Map el2eta1DMap; TStrMap el2eta1DSubMap; TStrIntMap el2eta1DbinMap;
  Analysis::Project2Dto1D(eleta2DMap["el2eta_time"],el2pt2DSubMap,el2eta1DMap,el2eta1DSubMap,el2eta1DbinMap);
  Analysis::ProduceMeanSigma(el2eta1DMap,el2eta1DbinMap,"el2eta_time","Subleading Electron #eta",eletabins,eleta2DSubMap["el2eta_time"]);

  // Delete el1 pt plots
  Analysis::DeleteTH2s(eleta2DMap);
  Analysis::DeleteTH1s(el1eta1DMap);
  Analysis::DeleteTH1s(el2eta1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do run plots last
  if (!fIsMC && !Config::skipRuns) {
    MakeSubDirs(runs2DSubMap,fOutDir);
    Analysis::SaveTH2s(runs2DMap,runs2DSubMap);

    // EBEB
    TH1Map runsEBEB1DMap; TStrMap runsEBEB1DSubMap; TStrIntMap runsEBEB1DbinMap;
    Analysis::Project2Dto1D(runs2DMap["tdEBEB_runs"],runs2DSubMap,runsEBEB1DMap,runsEBEB1DSubMap,runsEBEB1DbinMap);
    Analysis::ProduceMeanSigma(runsEBEB1DMap,runsEBEB1DbinMap,"tdEBEB_runs","Run Number (EBEB)",dRunNos,runs2DSubMap["tdEBEB_runs"]);

    // EBEE
    TH1Map runsEBEE1DMap; TStrMap runsEBEE1DSubMap; TStrIntMap runsEBEE1DbinMap;
    Analysis::Project2Dto1D(runs2DMap["tdEBEE_runs"],runs2DSubMap,runsEBEE1DMap,runsEBEE1DSubMap,runsEBEE1DbinMap);
    Analysis::ProduceMeanSigma(runsEBEE1DMap,runsEBEE1DbinMap,"tdEBEE_runs","Run Number (EBEE)",dRunNos,runs2DSubMap["tdEBEE_runs"]);

    // EEEE
    TH1Map runsEEEE1DMap; TStrMap runsEEEE1DSubMap; TStrIntMap runsEEEE1DbinMap;
    Analysis::Project2Dto1D(runs2DMap["tdEEEE_runs"],runs2DSubMap,runsEEEE1DMap,runsEEEE1DSubMap,runsEEEE1DbinMap);
    Analysis::ProduceMeanSigma(runsEEEE1DMap,runsEEEE1DbinMap,"tdEEEE_runs","Run Number (EEEE)",dRunNos,runs2DSubMap["tdEEEE_runs"]);

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
    if ( (zmass>Config::zlow && zmass<Config::zhigh) && (el1pid == -el2pid) ){ // we want di-electron z's
      Float_t weight = -1.;
      if   (fIsMC) {weight = (fXsec * Config::lumi * wgt / fWgtsum) * fPUweights[putrue];}
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
  if (!fIsMC) Analysis::DumpTH1Names(trTH1Map,trTH1SubMap);
  Analysis::DeleteTH1s(trTH1Map);

  // delete by hand throw away plots
  delete n_hltdoubleel_el1pt;
  delete d_hltdoubleel_el1pt;
  delete n_hltdoubleel_el2pt;
  delete d_hltdoubleel_el2pt;
}

void Analysis::Project2Dto1D(TH2F *& hist2d, TStrMap & subdir2dmap, TH1Map & th1map, TStrMap & subdir1dmap, TStrIntMap & th1binmap) {
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
    for (Int_t j = 1; j <= hist2d->GetNbinsY(); j++) {
      if (hist2d->GetBinContent(i,j) > 0) {isFilled = true; break;}
    }
    if (!isFilled) continue;

    Double_t xlow  = hist2d->GetXaxis()->GetBinLowEdge(i); 
    Double_t xhigh = hist2d->GetXaxis()->GetBinUpEdge(i);

    TString histname = "";
    // First create each histogram
    if     (basename.Contains("td_zeta",TString::kExact)) { //ugh
      histname = Form("%s_%3.1f-%3.1f",basename.Data(),xlow,xhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %3.1f-%3.1f",ytitle.Data(),xtitle.Data(),xlow,xhigh),
							 "Events",subdir1dmap,subdir2dmap[basename.Data()]);
    }
    else if ((basename.Contains("zphi",TString::kExact))   || (basename.Contains("abszeta",TString::kExact)) ||
	     (basename.Contains("el1eta",TString::kExact)) || (basename.Contains("el2eta",TString::kExact))) { //triple ugh
      histname = Form("%s_%4.2f-%4.2f",basename.Data(),xlow,xhigh);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in %s bin: %4.2f-%4.2f",ytitle.Data(),xtitle.Data(),xlow,xhigh),
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
    th1binmap[histname.Data()] = i; // universal pairing

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

void Analysis::ProduceMeanSigma(TH1Map & th1map, TStrIntMap & th1binmap, TString name, TString xtitle, const DblVec vxbins, TString subdir){
  // need to convert bins into array
  const Double_t * axbins = &vxbins[0]; // https://stackoverflow.com/questions/2923272/how-to-convert-vector-to-array-c

  // initialize new mean/sigma histograms
  TH1F * outhist_mean  = new TH1F(Form("%s_mean_%s",name.Data(),Config::formname.Data()),"",vxbins.size()-1,axbins);
  outhist_mean->GetXaxis()->SetTitle(xtitle.Data());
  if (!name.Contains("el1",TString::kExact) && !name.Contains("el2",TString::kExact)) {
    outhist_mean->GetYaxis()->SetTitle("Dielectron Seed Time Difference Bias [ns]");
  }
  else if (name.Contains("el1",TString::kExact)) {
    outhist_mean->GetYaxis()->SetTitle("Leading Electron Seed Time Bias [ns]");
  }
  else if (name.Contains("el2",TString::kExact)) {
    outhist_mean->GetYaxis()->SetTitle("Subleading Electron Seed Time Bias [ns]");
  }
  outhist_mean->SetLineColor(fColor);
  outhist_mean->SetMarkerColor(fColor);

  TH1F * outhist_sigma  = new TH1F(Form("%s_sigma_%s",name.Data(),Config::formname.Data()),"",vxbins.size()-1,axbins);
  outhist_sigma->GetXaxis()->SetTitle(xtitle.Data());
  if (!name.Contains("el1",TString::kExact) && !name.Contains("el2",TString::kExact)) {
    outhist_sigma->GetYaxis()->SetTitle("Dielectron Seed Time Difference Resolution [ns]");
  }
  else if (name.Contains("el1",TString::kExact)) {
    outhist_sigma->GetYaxis()->SetTitle("Leading Electron Seed Time Resolution [ns]");
  }
  else if (name.Contains("el2",TString::kExact)) {
    outhist_sigma->GetYaxis()->SetTitle("Subleading Electron Seed Time Resolution [ns]");
  }
  outhist_sigma->SetLineColor(fColor);
  outhist_sigma->SetMarkerColor(fColor);

  // use this to store runs that by themselves produce bad fits
  TH1Map tempmap; // a bit hacky I admit...
  Int_t  sumevents = 0; // also a bit hacky...

  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) { 
    Int_t bin = th1binmap[(*mapiter).first]; // returns which bin each th1 corresponds to one the new plot
    
    // only do this for run number plots --> check each plot has enough entries to do fit
    if ( name.Contains("runs",TString::kExact) ) {
      if ( ((*mapiter).second->Integral() + sumevents) < Config::nEventsCut ) { 
	
	// store the plot to be added later
	TString tempname  = Form("%s_tmp",(*mapiter).first.Data());
	tempmap[tempname] = (TH1F*)(*mapiter).second->Clone(tempname.Data()); 

	// record the number of events to exceed cut
	sumevents += tempmap[tempname]->Integral();

	continue; // don't do anything more, just go to next run
      } 
      
      if ( tempmap.size() > 0 ) { // since we passed the last check, see if we had any bad runs -- if so, add to this one

	// set bin to weighted average of events
	Int_t numer = 0;
	numer += bin * (*mapiter).second->Integral();
	Int_t denom = sumevents + (*mapiter).second->Integral();

	// add the bad histos to the good one
	for (TH1MapIter tempmapiter = tempmap.begin(); tempmapiter != tempmap.end(); ++tempmapiter) {
	  (*mapiter).second->Add((*tempmapiter).second);
	  numer = th1binmap[(*tempmapiter).first] * (*tempmapiter).second->Integral();
	}
	
	// set "effective" bin number
	bin = numer / denom; 

	// now delete everything in temp map to avoid leaking
	for (TH1MapIter tempmapiter = tempmap.begin(); tempmapiter != tempmap.end(); ++tempmapiter) {
	  delete ((*tempmapiter).second);
	}
	tempmap.clear();
	sumevents = 0; // reset sum events, too
      }
    } // end check over "runs"

    // declare fit, prep it, then use it for binned plots
    TF1 * fit; 
    Analysis::PrepFit(fit,(*mapiter).second);
    (*mapiter).second->Fit(fit->GetName(),"R");

    // need to capture the mean and sigma
    Float_t mean,  emean;
    Float_t sigma, esigma;
    Analysis::GetMeanSigma(fit,mean,emean,sigma,esigma);

    outhist_mean->SetBinContent(bin,mean);
    outhist_mean->SetBinError(bin,emean);

    outhist_sigma->SetBinContent(bin,sigma);
    outhist_sigma->SetBinError(bin,esigma);

    // save a copy of the fitted histogram with the fit
    Analysis::SaveTH1andFit((*mapiter).second,subdir,fit);
  }

  // write output hist to file
  fOutFile->cd();
  outhist_mean->Write();
  outhist_sigma->Write();

  // and want to dump them too (for stacking)!
  if (!fIsMC && !name.Contains("runs",TString::kExact)) {fTH1Dump << outhist_mean->GetName()  << " " << subdir.Data() << std::endl;}
  if (!fIsMC && !name.Contains("runs",TString::kExact)) {fTH1Dump << outhist_sigma->GetName() << " " << subdir.Data() << std::endl;}

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

void Analysis::PrepFit(TF1 *& fit, TH1F *& hist) {
  TF1 * tempfit = new TF1("temp","gaus",-Config::fitrange,Config::fitrange);
  hist->Fit("temp","R");
  const Float_t tempp0 = tempfit->GetParameter("Constant");
  const Float_t tempp1 = tempfit->GetParameter("Mean");
  const Float_t tempp2 = tempfit->GetParameter("Sigma");

  if (Config::formname.EqualTo("gaus1",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2);
  }
  else if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[4])/[5])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,0,tempp2*4);
  }
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,tempp2*4);
  }
  else if (Config::formname.EqualTo("gauslin",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-(abs(x)-[1])/[2])");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,0,1);
  }
  else {
    std::cerr << "Yikes, you picked a function that we made that does not even exist ...exiting... " << std::endl;
    exit(1);
  }

  delete tempfit;
}

void Analysis::GetMeanSigma(TF1 *& fit, Float_t & mean, Float_t & emean, Float_t & sigma, Float_t & esigma) {
  if (Config::formname.EqualTo("gaus1",TString::kExact)) {
    mean   = fit->GetParameter(1);
    emean  = fit->GetParError (1);
    sigma  = fit->GetParameter(2);
    esigma = fit->GetParError (2);
  }
  else if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom =  const1 + const2;

    mean   = (const1*fit->GetParameter(1) + const2*fit->GetParameter(4))/denom;
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(5))/denom;

    emean  = rad2(const1*fit->GetParError(1),const2*fit->GetParError(4));
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(5));

    emean  = std::sqrt(emean) /denom;
    esigma = std::sqrt(esigma)/denom;
  }
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom =  const1 + const2;

    mean   = fit->GetParameter(1);
    sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4))/denom;

    emean  = fit->GetParError(1);
    esigma = rad2(const1*fit->GetParError(2),const2*fit->GetParError(4));

    esigma = std::sqrt(esigma)/denom;
  }
  else if (Config::formname.EqualTo("gauslin",TString::kExact)) {
    std::cout << "currently broken, don't use this" << std::endl;
    exit(1);
  }
}

void Analysis::DrawSubComp(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2) {
  if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-Config::fitrange,Config::fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-Config::fitrange,Config::fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(4),fit->GetParameter(5));
  } 
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    sub1 = new TF1("sub1","gaus(0)",-Config::fitrange,Config::fitrange);
    sub1->SetParameters(fit->GetParameter(0),fit->GetParameter(1),fit->GetParameter(2));
  
    sub2 = new TF1("sub2","gaus(0)",-Config::fitrange,Config::fitrange);
    sub2->SetParameters(fit->GetParameter(3),fit->GetParameter(1),fit->GetParameter(4));
  } 
  else { // do not do anything in this function
    return;
  }

  canv->cd();

  sub1->SetLineColor(kBlue+1);
  sub1->SetLineWidth(1);
  sub1->SetLineStyle(7);
  sub1->Draw("same");

  sub2->SetLineColor(kGreen+2);
  sub2->SetLineWidth(1);
  sub2->SetLineStyle(7);
  sub2->Draw("same");
}

TH1F * Analysis::MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh, TString xtitle,
			     TString ytitle, TStrMap& subdirmap, TString subdir) {
  TH1F * hist = new TH1F(hname.Data(),htitle.Data(),nbins,xlow,xhigh);
  hist->SetLineColor(kBlack);
  if (fIsMC) {
    hist->SetFillColor(fColor);
    hist->SetMarkerColor(fColor);
  }
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

  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) { 
    // save to output file
    (*mapiter).second->Write(); // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    TCanvas * canv = new TCanvas();
    canv->cd();
    (*mapiter).second->Draw( fIsMC ? "HIST" : "PE" );
    
    // first save as linear, then log
    canv->SetLogy(0);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/lin/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));

    canv->SetLogy(1);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/log/%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));

    delete canv;

    // Draw and save normalized clone; then take original and fit it with some copy/past code
    TString xtitle = (*mapiter).second->GetXaxis()->GetTitle();
    if (xtitle.Contains("Time",TString::kExact)) {
      // first clone th1, then normalize it, then draw + save it
      TH1F * normhist = (TH1F*)(*mapiter).second->Clone(Form("%s_norm",(*mapiter).first.Data()));
      normhist->Scale(1./normhist->Integral());
      fOutFile->cd();
      normhist->Write();

      TCanvas * normcanv = new TCanvas();
      normcanv->cd();
      normhist->Draw( fIsMC ? "HIST" : "PE" );
      
      normcanv->SetLogy(0);
      CMSLumi(normcanv);
      normcanv->SaveAs(Form("%s/%s/lin/%s_norm.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));
      
      normcanv->SetLogy(1);
      CMSLumi(normcanv);
      normcanv->SaveAs(Form("%s/%s/log/%s_norm.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));

      if (!fIsMC) {fTH1Dump << normhist->GetName()  << " " << subdirmap[(*mapiter).first].Data() << std::endl;}

      delete normhist;
      delete normcanv;

      // fit th1s with time in the name --> could factor out this copy-paste...

      // make it a "graph" with the right colors
      (*mapiter).second->SetLineColor(fColor);
      (*mapiter).second->SetMarkerColor(fColor);
      
      // declare fit, then pass it to prepper along with hist for prefitting
      TF1 * fit; 
      Analysis::PrepFit(fit,(*mapiter).second);
      (*mapiter).second->Fit(fit->GetName(),"R");
      
      TCanvas * fitcanv = new TCanvas();
      fitcanv->cd();
      (*mapiter).second->Draw("PE");
      fit->SetLineWidth(2);
      fit->Draw("same");
      
      // draw sub components of fit it applies
      TF1 * sub1; TF1 * sub2;
      Analysis::DrawSubComp(fit,fitcanv,sub1,sub2);

      // first save as linear, then log
      fitcanv->SetLogy(0);
      CMSLumi(fitcanv);
      fitcanv->SaveAs(Form("%s/%s/lin/%s_%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),fit->GetName(),Config::outtype.Data()));
      
      fitcanv->SetLogy(1);
      CMSLumi(fitcanv);
      fitcanv->SaveAs(Form("%s/%s/log/%s_%s.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),fit->GetName(),Config::outtype.Data()));
      
      delete fitcanv;
      Analysis::DeleteFit(fit,sub1,sub2);
    }
  }
}

void Analysis::SaveTH1andFit(TH1F *& hist, TString subdir, TF1 *& fit) {
  fOutFile->cd();
  hist->Write(); 

  // now draw onto canvas to save as png
  TCanvas * canv = new TCanvas();
  canv->cd();
  hist->Draw("PE");
  fit->SetLineWidth(2);
  fit->Draw("same");

  // draw subcomponents, too, if they apply
  TF1 * sub1; TF1 * sub2;
  Analysis::DrawSubComp(fit,canv,sub1,sub2);
    
  // first save as linear, then log
  canv->SetLogy(0);
  CMSLumi(canv);
  canv->SaveAs(Form("%s/%s/lin/%s_%s.%s",fOutDir.Data(),subdir.Data(),hist->GetName(),fit->GetName(),Config::outtype.Data()));
  
  canv->SetLogy(1);
  CMSLumi(canv);
  canv->SaveAs(Form("%s/%s/log/%s_%s.%s",fOutDir.Data(),subdir.Data(),hist->GetName(),fit->GetName(),Config::outtype.Data()));

  delete canv;
  Analysis::DeleteFit(fit,sub1,sub2); // now that the fitting is done being used, delete it (as well as sub component gaussians)
}

void Analysis::SaveTH2s(TH2Map & th2map, TStrMap & subdirmap) {
  fOutFile->cd();

  TCanvas * canv = new TCanvas();
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) { 
    
    (*mapiter).second->Write(); // map is map["hist name",TH1D*]

    // now draw onto canvas to save as png
    canv->cd();
    (*mapiter).second->Draw("colz");
    
    // only save as linear
    canv->SetLogy(0);
    CMSLumi(canv);
    canv->SaveAs(Form("%s/%s/%s_2D.%s",fOutDir.Data(),subdirmap[(*mapiter).first].Data(),(*mapiter).first.Data(),Config::outtype.Data()));
  }

  delete canv;
}

void Analysis::DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap) {
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) { 
    fTH1Dump << (*mapiter).first.Data()  << " " <<  subdirmap[(*mapiter).first].Data() << std::endl;
  }
}

void Analysis::DeleteFit(TF1 *& fit, TF1 *& sub1, TF1 *& sub2) {
  delete fit;
  if (!Config::formname.EqualTo("gaus1",TString::kExact)) { delete sub1; }
  if (!Config::formname.EqualTo("gaus1",TString::kExact)) { delete sub2; }
}

void Analysis::DeleteTH1s(TH1Map & th1map) {
  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) { 
    delete ((*mapiter).second);
  }
  th1map.clear();
}

void Analysis::DeleteTH2s(TH2Map & th2map) {
  for (TH2MapIter mapiter = th2map.begin(); mapiter != th2map.end(); ++mapiter) { 
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
  }
}
