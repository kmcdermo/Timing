#include "../interface/Analysis.hh"
#include "TROOT.h"

#include "TH1D.h"

inline Float_t rad2(Float_t x, Float_t y){
  return x*x + y*y;
}

inline Float_t theta(float r, float z){
  return std::atan2(r,z);
}

inline Float_t eta(Float_t x, Float_t y, Float_t z){
  return -1.0f * std::log( std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f) );  
}

inline Float_t TOF(Float_t x, Float_t y, Float_t z, Float_t vx, Float_t vy, Float_t vz, Float_t time){
  return time + (std::sqrt(z*z + x*x + y*y)-std::sqrt((z-vz)*(z-vz) + (x-vx)*(x-vx) + (y-vy)*(y-vy)))/Config::sol;
}

Analysis::Analysis(TString sample, Bool_t isMC) : fSample(sample), fIsMC(isMC) {
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

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
    weights->Scale( gen_pu->Integral(1,Config::nbinsvtx) / weightedPU->Integral(1,Config::nbinsvtx) );

    for (Int_t i = 1; i <= Config::nbinsvtx; i++){
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
  trTH1Map["nvtx"]   = Analysis::MakeTH1Plot("nvtx","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx),"nVertices","Events",trTH1SubMap,"standard");
  trTH1Map["zmass"]  = Analysis::MakeTH1Plot("zmass","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EBEB"]  = Analysis::MakeTH1Plot("zmass_EBEB","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EBEB)","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EEEE"]  = Analysis::MakeTH1Plot("zmass_EEEE","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EEEE)","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EEMEEM"]  = Analysis::MakeTH1Plot("zmass_EEMEEM","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EE-EE-)","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EEPEEP"]  = Analysis::MakeTH1Plot("zmass_EEPEEP","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EE+EE+)","Events",trTH1SubMap,"standard");
  //  trTH1Map["zmass_EEMEEP"]  = Analysis::MakeTH1Plot("zmass_EEMEEP","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EE-EE+)","Events",trTH1SubMap,"standard");
  trTH1Map["zmass_EBEE"]  = Analysis::MakeTH1Plot("zmass_EBEE","",100,Config::zlow,Config::zhigh,"Dielectron invariant mass [GeV/c^{2}] (EBEE)","Events",trTH1SubMap,"standard");
  trTH1Map["zpt"]    = Analysis::MakeTH1Plot("zpt","",100,0.,750.,"Dielectron p_{T} [GeV/c^{2}]","Events",trTH1SubMap,"standard");
  trTH1Map["zeta"]   = Analysis::MakeTH1Plot("zeta","",100,-10.0,10.0,"Dielectron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["zphi"]   = Analysis::MakeTH1Plot("zphi","",100,-3.2,3.2,"Dielectron #phi","Events",trTH1SubMap,"standard");

  trTH1Map["timediff"]   = Analysis::MakeTH1Plot("timediff","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns]","Events",trTH1SubMap,"standard");
  if (!Config::normE) {
    trTH1Map["eff_dielpt"]  = Analysis::MakeTH1Plot("eff_dielpt","",100,0.,250.,"Effective Dielectron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
    trTH1Map["eff_dielE"]   = Analysis::MakeTH1Plot("eff_dielE","",100,0.,250.,"Effective Dielectron Energy [GeV]","Events",trTH1SubMap,"standard");
    trTH1Map["eff_diseedE"] = Analysis::MakeTH1Plot("eff_diseedE","",100,0.,250.,"Effective Dielectron Seed Energy [GeV]","Events",trTH1SubMap,"standard");
  }
  else {
    trTH1Map["eff_dielpt"]  = Analysis::MakeTH1Plot("eff_dielpt","",100,0.,250./Config::nE,"Effective Dielectron p_{T}/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["eff_dielE"]   = Analysis::MakeTH1Plot("eff_dielE","",100,0.,250./Config::nE,"Effective Dielectron Energy/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["eff_diseedE"] = Analysis::MakeTH1Plot("eff_diseedE","",100,0.,250./Config::nE,"Effective Dielectron Seed Energy/#sigma_{n}","Events",trTH1SubMap,"standard");
  }

  if (!Config::normE) {
    trTH1Map["el1pt"]  = Analysis::MakeTH1Plot("el1pt","",100,0.,700.,"Leading Electron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
    trTH1Map["el1p"]   = Analysis::MakeTH1Plot("el1p","",100,0.,700.,"Leading Electron p [GeV/c]","Events",trTH1SubMap,"standard");
    trTH1Map["el1E"]   = Analysis::MakeTH1Plot("el1E","",100,0.,700.,"Leading Electron Energy [GeV]","Events",trTH1SubMap,"standard");
  }
  else {
    trTH1Map["el1pt"]  = Analysis::MakeTH1Plot("el1pt","",100,0.,700./Config::nE,"Leading Electron p_{T}/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["el1p"]   = Analysis::MakeTH1Plot("el1p","",100,0.,700./Config::nE,"Leading Electron p/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["el1E"]   = Analysis::MakeTH1Plot("el1E","",100,0.,700./Config::nE,"Leading Electron Energy/#sigma_{n}","Events",trTH1SubMap,"standard");
  }
  trTH1Map["el1eta"] = Analysis::MakeTH1Plot("el1eta","",100,-3.0,3.0,"Leading Electron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el1phi"] = Analysis::MakeTH1Plot("el1phi","",100,-3.2,3.2,"Leading Electron #phi","Events",trTH1SubMap,"standard");

  if (!Config::normE) {
    trTH1Map["el2pt"]  = Analysis::MakeTH1Plot("el2pt","",100,0.,300.,"Subleading Electron p_{T} [GeV/c]","Events",trTH1SubMap,"standard");
    trTH1Map["el2p"]   = Analysis::MakeTH1Plot("el2p","",100,0.,300.,"Subleading Electron p [GeV/c]","Events",trTH1SubMap,"standard");
    trTH1Map["el2E"]   = Analysis::MakeTH1Plot("el2E","",100,0.,300.,"Subleading Electron Energy [GeV]","Events",trTH1SubMap,"standard");
  }
  else {
    trTH1Map["el2pt"]  = Analysis::MakeTH1Plot("el2pt","",100,0.,300./Config::nE,"Subleading Electron p_{T}/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["el2p"]   = Analysis::MakeTH1Plot("el2p","",100,0.,300./Config::nE,"Subleading Electron p/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["el2E"]   = Analysis::MakeTH1Plot("el2E","",100,0.,300./Config::nE,"Subleading Electron Energy/#sigma_{n}","Events",trTH1SubMap,"standard");
  }
  trTH1Map["el2eta"] = Analysis::MakeTH1Plot("el2eta","",100,-3.0,3.0,"Subleading Electron #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el2phi"] = Analysis::MakeTH1Plot("el2phi","",100,-3.2,3.2,"Subleading Electron #phi","Events",trTH1SubMap,"standard");

  trTH1Map["el1seedeta"] = Analysis::MakeTH1Plot("el1seedeta","",100,-3.0,3.0,"Leading Electron Seed RecHit #eta","Events",trTH1SubMap,"standard");
  trTH1Map["el2seedeta"] = Analysis::MakeTH1Plot("el2seedeta","",100,-3.0,3.0,"Subleading Electron Seed RecHit #eta","Events",trTH1SubMap,"standard");
  if (!Config::normE) {
    trTH1Map["el1seedE"]   = Analysis::MakeTH1Plot("el1seedE","",100,0.,700.,"Leading Electron Seed RecHit Energy [GeV]","Events",trTH1SubMap,"standard");
    trTH1Map["el2seedE"]   = Analysis::MakeTH1Plot("el2seedE","",100,0.,300.,"Subleading Electron Seed RecHit Energy [GeV]","Events",trTH1SubMap,"standard");
  }
  else {
    trTH1Map["el1seedE"]   = Analysis::MakeTH1Plot("el1seedE","",100,0.,700./Config::nE,"Leading Electron Seed RecHit Energy/#sigma_{n}","Events",trTH1SubMap,"standard");
    trTH1Map["el2seedE"]   = Analysis::MakeTH1Plot("el2seedE","",100,0.,300./Config::nE,"Subleading Electron Seed RecHit Energy/#sigma_{n}","Events",trTH1SubMap,"standard");
  }

  trTH1Map["el1time"]    = Analysis::MakeTH1Plot("el1time","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Seed Time [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el1time_EB"] = Analysis::MakeTH1Plot("el1time_EB","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EB Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el1time_EE"] = Analysis::MakeTH1Plot("el1time_EE","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EE Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el1time_EEP"] = Analysis::MakeTH1Plot("el1time_EEP","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EE+ Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el1time_EEM"] = Analysis::MakeTH1Plot("el1time_EEM","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EE- Inclusive)","Events",trTH1SubMap,"standard");  

  trTH1Map["el2time"]    = Analysis::MakeTH1Plot("el2time","",Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Seed Time [ns]","Events",trTH1SubMap,"standard");
  trTH1Map["el2time_EB"] = Analysis::MakeTH1Plot("el2time_EB","",Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Time [ns] (EB Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el2time_EE"] = Analysis::MakeTH1Plot("el2time_EE","",Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Time [ns] (EE Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el2time_EEP"] = Analysis::MakeTH1Plot("el2time_EEP","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EE+ Inclusive)","Events",trTH1SubMap,"standard");  
  trTH1Map["el2time_EEM"] = Analysis::MakeTH1Plot("el2time_EEM","",Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Time [ns] (EE- Inclusive)","Events",trTH1SubMap,"standard");  

  MakeSubDirs(trTH1SubMap,fOutDir);

  for (UInt_t entry = 0; entry < (!Config::doDemo?fInTree->GetEntries():Config::demoNum); entry++){
    fInTree->GetEntry(entry);
    if ((zmass>Config::zlow && zmass<Config::zhigh) && hltdoubleel && (el1pid == -el2pid)) { // extra selection over skimmed samples
      Float_t weight = -1.;
      if   (fIsMC) {weight = (fXsec * Config::lumi * wgt / fWgtsum) * fPUweights[putrue];}
      else         {weight = 1.0;}

      const Float_t el1seedeta = eta(el1seedX,el1seedY,el1seedZ);
      const Float_t el2seedeta = eta(el2seedX,el2seedY,el2seedZ);

      Bool_t el1eb = false; Bool_t el1ee = false;
      if      (std::abs(el1seedeta) < Config::etaEB)                                                { el1eb = true; }
      else if (std::abs(el1seedeta) > Config::etaEElow && std::abs(el1seedeta) < Config::etaEEhigh) { el1ee = true; }
      Bool_t el1eep = false; Bool_t el1eem = false;
      if (el1ee) {
	if   (el1seedZ>0) {el1eep = true;}
	else              {el1eem = true;}
      }

      Bool_t el2eb = false; Bool_t el2ee = false;
      if      (std::abs(el2seedeta) < Config::etaEB)                                                { el2eb = true; }
      else if (std::abs(el2seedeta) > Config::etaEElow && std::abs(el2seedeta) < Config::etaEEhigh) { el2ee = true; }
      Bool_t el2eep = false; Bool_t el2eem = false;
      if (el2ee) {
	if   (el2seedZ>0) {el2eep = true;}
	else              {el2eem = true;}
      }

      // standard "validation" plots
      trTH1Map["nvtx"]->Fill(nvtx,weight);
      trTH1Map["zmass"]->Fill(zmass,weight);
      trTH1Map["zpt"]->Fill(zpt,weight);
      trTH1Map["zeta"]->Fill(zeta,weight);
      trTH1Map["zphi"]->Fill(zphi,weight);

      Float_t el1time = -99.0;
      if (Config::applyTOF && !Config::wgtedtime){
	el1time = TOF(el1seedX,el1seedY,el1seedZ,vtxX,vtxY,vtxZ,el1seedtime);
      }
      else {
	el1time = el1seedtime;
      }

      Float_t el2time = -99.0;
      if (Config::applyTOF && !Config::wgtedtime){
	el2time = TOF(el2seedX,el2seedY,el2seedZ,vtxX,vtxY,vtxZ,el2seedtime);
      }
      else {
	el2time = el2seedtime;
      }
      
      trTH1Map["timediff"]->Fill(el1time-el2time,weight);

      if (Config::normE) {
	el1pt /= Config::nE;
	el2pt /= Config::nE;

	el1E /= Config::nE;
	el2E /= Config::nE;

	el1p /= Config::nE;
	el2p /= Config::nE;
      }

      const Float_t eff_dielpt  = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));
      const Float_t eff_dielE   = el1E*el2E/std::sqrt(rad2(el1E,el2E));
      const Float_t eff_diseedE = el1seedE*el2seedE/std::sqrt(rad2(el1seedE,el2seedE));
      trTH1Map["eff_dielpt"]->Fill(eff_dielpt,weight);
      trTH1Map["eff_dielE"]->Fill(eff_dielE,weight);
      trTH1Map["eff_diseedE"]->Fill(eff_diseedE,weight);

      trTH1Map["el1pt"]->Fill(el1pt,weight);
      trTH1Map["el1eta"]->Fill(el1eta,weight);
      trTH1Map["el1phi"]->Fill(el1phi,weight);
      trTH1Map["el1p"]->Fill(el1p,weight);
      trTH1Map["el1E"]->Fill(el1E,weight);
      trTH1Map["el1seedeta"]->Fill(el1seedeta,weight);
      trTH1Map["el1seedE"]->Fill(el1seedE,weight);

      trTH1Map["el2pt"]->Fill(el2pt,weight);
      trTH1Map["el2eta"]->Fill(el2eta,weight);
      trTH1Map["el2phi"]->Fill(el2phi,weight); 
      trTH1Map["el2p"]->Fill(el2p,weight);
      trTH1Map["el2E"]->Fill(el2E,weight);
      trTH1Map["el2seedeta"]->Fill(el2seedeta,weight);
      trTH1Map["el2seedE"]->Fill(el2seedE,weight);

      trTH1Map["el1time"]->Fill(el1time,weight);
      if      (el1eb) {trTH1Map["el1time_EB"]->Fill(el1time,weight);}
      else if (el1ee) {
	trTH1Map["el1time_EE"]->Fill(el1time,weight);
	if (el1eep) {
	  trTH1Map["el1time_EEP"]->Fill(el1time,weight);
	}
	else {
	  trTH1Map["el1time_EEM"]->Fill(el1time,weight);
	}
      }

      trTH1Map["el2time"]->Fill(el2time,weight);
      if      (el2eb) {trTH1Map["el2time_EB"]->Fill(el2time,weight);}
      else if (el2ee) {
	trTH1Map["el2time_EE"]->Fill(el2time,weight);
	if (el2eep) {
	  trTH1Map["el2time_EEP"]->Fill(el2time,weight);
	}
	else {
	  trTH1Map["el2time_EEM"]->Fill(el2time,weight);
	}
      }

      if      (el1eb && el2eb) {
	trTH1Map["zmass_EBEB"]->Fill(zmass,weight); 
      }
      else if (el1ee && el2ee) {
	trTH1Map["zmass_EEEE"]->Fill(zmass,weight); 
	if (el1eem && el2eem){
	  trTH1Map["zmass_EEMEEM"]->Fill(zmass,weight); 
	}
	else if (el1eep && el2eep){
	  trTH1Map["zmass_EEPEEP"]->Fill(zmass,weight); 
	}
// 	else {
// 	  trTH1Map["zmass_EEMEEP"]->Fill(zmass,weight); 
// 	}
      }
      else if ((el1eb && el2ee) || (el1ee && el2eb)) { 
	trTH1Map["zmass_EBEE"]->Fill(zmass,weight); 
      }
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
  inclu1DMap["tdEEPEEP_inclusive"] = Analysis::MakeTH1Plot("tdEEPEEP_inclusive","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns] (EEPEEP Inclusive)","Events",inclu1DSubMap,"timing/effpt/EEPEEP");  
  inclu1DMap["tdEEMEEM_inclusive"] = Analysis::MakeTH1Plot("tdEEMEEM_inclusive","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns] (EEMEEM Inclusive)","Events",inclu1DSubMap,"timing/effpt/EEMEEM");  
  //  inclu1DMap["tdEEMEEP_inclusive"] = Analysis::MakeTH1Plot("tdEEMEEP_inclusive","",Config::ntimebins,-Config::timerange,Config::timerange,"Dielectron Seed Time Difference [ns] (EEMEEP Inclusive)","Events",inclu1DSubMap,"timing/effpt/EEMEEP");  

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
  effpt2DMap["td_effpt_inclusive"] = Analysis::MakeTH2Plot("td_effpt_inclusive","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (inclusive)",effpt2DSubMap,"timing/effpt/inclusive");  
  effpt2DMap["tdEBEB_effpt"] = Analysis::MakeTH2Plot("tdEBEB_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEB)",effpt2DSubMap,"timing/effpt/EBEB");  
  effpt2DMap["tdEBEE_effpt"] = Analysis::MakeTH2Plot("tdEBEE_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EBEE)",effpt2DSubMap,"timing/effpt/EBEE");  
  effpt2DMap["tdEEEE_effpt"] = Analysis::MakeTH2Plot("tdEEEE_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EEEE)",effpt2DSubMap,"timing/effpt/EEEE");  
  effpt2DMap["tdEEPEEP_effpt"] = Analysis::MakeTH2Plot("tdEEPEEP_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EE+EE+)",effpt2DSubMap,"timing/effpt/EEPEEP");  
  effpt2DMap["tdEEMEEM_effpt"] = Analysis::MakeTH2Plot("tdEEMEEM_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EE-EE-)",effpt2DSubMap,"timing/effpt/EEMEEM");  
  //  effpt2DMap["tdEEMEEP_effpt"] = Analysis::MakeTH2Plot("tdEEMEEP_effpt","",effptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective p_{T} [GeV/c]","Dielectron Seed Time Difference [ns] (EE-EE+)",effpt2DSubMap,"timing/effpt/EEMEEP");  

  // make 2D plots for effective electron E, for EBEB, EBEE, EEEE
  TH2Map  effelE2DMap;    
  TStrMap effelE2DSubMap; 
  DblVec  effelEbins = {20,25,30,35,40,50,70,250}; //effective el E bins
  effelE2DMap["td_effelE_inclusive"] = Analysis::MakeTH2Plot("td_effelE_inclusive","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (inclusive)",effelE2DSubMap,"timing/effelE/inclusive");  
  effelE2DMap["tdEBEB_effelE"] = Analysis::MakeTH2Plot("tdEBEB_effelE","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (EBEB)",effelE2DSubMap,"timing/effelE/EBEB");  
  effelE2DMap["tdEBEE_effelE"] = Analysis::MakeTH2Plot("tdEBEE_effelE","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (EBEE)",effelE2DSubMap,"timing/effelE/EBEE");  
  effelE2DMap["tdEEEE_effelE"] = Analysis::MakeTH2Plot("tdEEEE_effelE","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (EEEE)",effelE2DSubMap,"timing/effelE/EEEE");  
  effelE2DMap["tdEEPEEP_effelE"] = Analysis::MakeTH2Plot("tdEEPEEP_effelE","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (EE+EE+)",effelE2DSubMap,"timing/effelE/EEPEEP");  
  effelE2DMap["tdEEMEEM_effelE"] = Analysis::MakeTH2Plot("tdEEMEEM_effelE","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (EE-EE-)",effelE2DSubMap,"timing/effelE/EEMEEM");  
  //  effelE2DMap["tdEEMEEP_effelE"] = Analysis::MakeTH2Plot("tdEEMEEP_effelE","",effelEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Electron Energy [GeV]","Dielectron Seed Time Difference [ns] (EE-EE+)",effelE2DSubMap,"timing/effelE/EEMEEP");  

  // make 2D plots for effective electron seed E, for EBEB, EBEE, EEEE
  TH2Map  effseedE2DMap;    
  TStrMap effseedE2DSubMap; 
  DblVec  effseedEbins = {20,25,30,35,40,50,70,250}; //effective el E bins
  effseedE2DMap["td_effseedE_inclusive"] = Analysis::MakeTH2Plot("td_effseedE_inclusive","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (inclusive)",effseedE2DSubMap,"timing/effseedE/inclusive");  
  effseedE2DMap["tdEBEB_effseedE"] = Analysis::MakeTH2Plot("tdEBEB_effseedE","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (EBEB)",effseedE2DSubMap,"timing/effseedE/EBEB");  
  effseedE2DMap["tdEBEE_effseedE"] = Analysis::MakeTH2Plot("tdEBEE_effseedE","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (EBEE)",effseedE2DSubMap,"timing/effseedE/EBEE");  
  effseedE2DMap["tdEEEE_effseedE"] = Analysis::MakeTH2Plot("tdEEEE_effseedE","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (EEEE)",effseedE2DSubMap,"timing/effseedE/EEEE");  
  effseedE2DMap["tdEEPEEP_effseedE"] = Analysis::MakeTH2Plot("tdEEPEEP_effseedE","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (EE+EE+)",effseedE2DSubMap,"timing/effseedE/EEPEEP");  
  effseedE2DMap["tdEEMEEM_effseedE"] = Analysis::MakeTH2Plot("tdEEMEEM_effseedE","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (EE-EE-)",effseedE2DSubMap,"timing/effseedE/EEMEEM");  
  //  effseedE2DMap["tdEEMEEP_effseedE"] = Analysis::MakeTH2Plot("tdEEMEEP_effseedE","",effseedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Effective Seed Energy [GeV]","Dielectron Seed Time Difference [ns] (EE-EE+)",effseedE2DSubMap,"timing/effseedE/EEMEEP");  

  // make 2D plots for td vs nvtx, for EBEB, EBEE, EEEE
  TH2Map  nvtx2DMap;    
  TStrMap nvtx2DSubMap; 
  DblVec  nvtxbins;
  for (int iv = 0; iv <= Config::nbinsvtx; iv++){
    nvtxbins.push_back(Double_t(iv));
  }
  nvtx2DMap["td_nvtx_inclusive"] = Analysis::MakeTH2Plot("td_nvtx_inclusive","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (inclusive)",nvtx2DSubMap,"timing/nvtx/inclusive");  
  nvtx2DMap["tdEBEB_nvtx"] = Analysis::MakeTH2Plot("tdEBEB_nvtx","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (EBEB)",nvtx2DSubMap,"timing/nvtx/EBEB");  
  nvtx2DMap["tdEBEE_nvtx"] = Analysis::MakeTH2Plot("tdEBEE_nvtx","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (EBEE)",nvtx2DSubMap,"timing/nvtx/EBEE");  
  nvtx2DMap["tdEEEE_nvtx"] = Analysis::MakeTH2Plot("tdEEEE_nvtx","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (EEEE)",nvtx2DSubMap,"timing/nvtx/EEEE");  
  nvtx2DMap["tdEEPEEP_nvtx"] = Analysis::MakeTH2Plot("tdEEPEEP_nvtx","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (EE+EE+)",nvtx2DSubMap,"timing/nvtx/EEPEEP");  
  nvtx2DMap["tdEEMEEM_nvtx"] = Analysis::MakeTH2Plot("tdEEMEEM_nvtx","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (EE-EE-)",nvtx2DSubMap,"timing/nvtx/EEMEEM");  
  //  nvtx2DMap["tdEEMEEP_nvtx"] = Analysis::MakeTH2Plot("tdEEMEEP_nvtx","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (EE-EE+)",nvtx2DSubMap,"timing/nvtx/EEMEEP");  

  // make 2D plots for single electron timing pt, for two categories
  TH2Map  el1pt2DMap;    
  TStrMap el1pt2DSubMap; 
  DblVec  el1ptbins = {20,25,30,35,40,50,70,100,150,200,750}; // el1 pt bins
  el1pt2DMap["el1pt_inclusive"] = Analysis::MakeTH2Plot("el1pt_inclusive","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (inclusive)",el1pt2DSubMap,"timing/el1/pt/inclusive");  
  el1pt2DMap["el1pt_EB"] = Analysis::MakeTH2Plot("el1pt_EB","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EB)",el1pt2DSubMap,"timing/el1/pt/EB");  
  el1pt2DMap["el1pt_EE"] = Analysis::MakeTH2Plot("el1pt_EE","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EE)",el1pt2DSubMap,"timing/el1/pt/EE");  
  el1pt2DMap["el1pt_EEP"] = Analysis::MakeTH2Plot("el1pt_EEP","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EEP)",el1pt2DSubMap,"timing/el1/pt/EEP");  
  el1pt2DMap["el1pt_EEM"] = Analysis::MakeTH2Plot("el1pt_EEM","",el1ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron p_{T} [GeV/c]","Leading Electron Seed Time [ns] (EEM)",el1pt2DSubMap,"timing/el1/pt/EEM");  

  TH2Map  el1E2DMap;    
  TStrMap el1E2DSubMap; 
  DblVec  el1Ebins = {20,25,30,35,40,50,70,100,150,200,750}; // el1 E bins
  el1E2DMap["el1E_inclusive"] = Analysis::MakeTH2Plot("el1E_inclusive","",el1Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (inclusive)",el1E2DSubMap,"timing/el1/E/inclusive");  
  el1E2DMap["el1E_EB"] = Analysis::MakeTH2Plot("el1E_EB","",el1Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EB)",el1E2DSubMap,"timing/el1/E/EB");  
  el1E2DMap["el1E_EE"] = Analysis::MakeTH2Plot("el1E_EE","",el1Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EE)",el1E2DSubMap,"timing/el1/E/EE");  
  el1E2DMap["el1E_EEP"] = Analysis::MakeTH2Plot("el1E_EEP","",el1Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EEP)",el1E2DSubMap,"timing/el1/E/EEP");  
  el1E2DMap["el1E_EEM"] = Analysis::MakeTH2Plot("el1E_EEM","",el1Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EEM)",el1E2DSubMap,"timing/el1/E/EEM");  

  TH2Map  el1seedE2DMap;    
  TStrMap el1seedE2DSubMap; 
  DblVec  el1seedEbins = {20,25,30,35,40,50,70,100,150,200,750}; // el1 seedE bins
  el1seedE2DMap["el1seedE_inclusive"] = Analysis::MakeTH2Plot("el1seedE_inclusive","",el1seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (inclusive)",el1seedE2DSubMap,"timing/el1/E/inclusive");  
  el1seedE2DMap["el1seedE_EB"] = Analysis::MakeTH2Plot("el1seedE_EB","",el1seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EB)",el1seedE2DSubMap,"timing/el1/E/EB");  
  el1seedE2DMap["el1seedE_EE"] = Analysis::MakeTH2Plot("el1seedE_EE","",el1seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EE)",el1seedE2DSubMap,"timing/el1/E/EE");  
  el1seedE2DMap["el1seedE_EEP"] = Analysis::MakeTH2Plot("el1seedE_EEP","",el1seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EEP)",el1seedE2DSubMap,"timing/el1/E/EEP");  
  el1seedE2DMap["el1seedE_EEM"] = Analysis::MakeTH2Plot("el1seedE_EEM","",el1seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Energy [GeV]","Leading Electron Seed Time [ns] (EEM)",el1seedE2DSubMap,"timing/el1/E/EEM");  

  // make 2D plots for el1 vs nvtx, for EBEB, EBEE, EEEE
  TH2Map  el1nvtx2DMap;    
  TStrMap el1nvtx2DSubMap; 
  el1nvtx2DMap["el1_nvtx_inclusive"] = Analysis::MakeTH2Plot("el1_nvtx_inclusive","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (inclusive)",el1nvtx2DSubMap,"timing/el1/nvtx/inclusive");  
  el1nvtx2DMap["el1_nvtx_EB"] = Analysis::MakeTH2Plot("el1_nvtx_EB","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Leading Electron Seed Time [ns] (EB)",el1nvtx2DSubMap,"timing/el1/nvtx/EB");  
  el1nvtx2DMap["el1_nvtx_EE"] = Analysis::MakeTH2Plot("el1_nvtx_EE","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Leading Electron Seed Time [ns] (EE)",el1nvtx2DSubMap,"timing/el1/nvtx/EE");  
  el1nvtx2DMap["el1_nvtx_EEP"] = Analysis::MakeTH2Plot("el1_nvtx_EEP","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Leading Electron Seed Time [ns] (EE+)",el1nvtx2DSubMap,"timing/el1/nvtx/EEP");  
  el1nvtx2DMap["el1_nvtx_EEM"] = Analysis::MakeTH2Plot("el1_nvtx_EEM","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Leading Electron Seed Time [ns] (EE-)",el1nvtx2DSubMap,"timing/el1/nvtx/EEM");  

  TH2Map  el2pt2DMap;    
  TStrMap el2pt2DSubMap; 
  DblVec  el2ptbins = {20,25,30,35,40,50,70,100,300}; // el2 pt bins
  el2pt2DMap["el2pt_inclusive"] = Analysis::MakeTH2Plot("el2pt_inclusive","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Subleading Electron Seed Time [ns] (inclusive)",el2pt2DSubMap,"timing/el2/pt/inclusive");  
  el2pt2DMap["el2pt_EB"] = Analysis::MakeTH2Plot("el2pt_EB","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Subleading Electron Seed Time [ns] (EB)",el2pt2DSubMap,"timing/el2/pt/EB");  
  el2pt2DMap["el2pt_EE"] = Analysis::MakeTH2Plot("el2pt_EE","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Subleading Electron Seed Time [ns] (EE)",el2pt2DSubMap,"timing/el2/pt/EE");  
  el2pt2DMap["el2pt_EEP"] = Analysis::MakeTH2Plot("el2pt_EEP","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Subleading Electron Seed Time [ns] (EEP)",el2pt2DSubMap,"timing/el2/pt/EEP");  
  el2pt2DMap["el2pt_EEM"] = Analysis::MakeTH2Plot("el2pt_EEM","",el2ptbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron p_{T} [GeV/c]","Subleading Electron Seed Time [ns] (EEM)",el2pt2DSubMap,"timing/el2/pt/EEM");  

  TH2Map  el2E2DMap;    
  TStrMap el2E2DSubMap; 
  DblVec  el2Ebins = {20,25,30,35,40,50,70,100,150,200,750}; // el2 E bins
  el2E2DMap["el2E_inclusive"] = Analysis::MakeTH2Plot("el2E_inclusive","",el2Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (inclusive)",el2E2DSubMap,"timing/el2/E/inclusive");  
  el2E2DMap["el2E_EB"] = Analysis::MakeTH2Plot("el2E_EB","",el2Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EB)",el2E2DSubMap,"timing/el2/E/EB");  
  el2E2DMap["el2E_EE"] = Analysis::MakeTH2Plot("el2E_EE","",el2Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EE)",el2E2DSubMap,"timing/el2/E/EE");  
  el2E2DMap["el2E_EEP"] = Analysis::MakeTH2Plot("el2E_EEP","",el2Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EEP)",el2E2DSubMap,"timing/el2/E/EEP");  
  el2E2DMap["el2E_EEM"] = Analysis::MakeTH2Plot("el2E_EEM","",el2Ebins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EEM)",el2E2DSubMap,"timing/el2/E/EEM");  

  TH2Map  el2seedE2DMap;    
  TStrMap el2seedE2DSubMap; 
  DblVec  el2seedEbins = {20,25,30,35,40,50,70,100,150,200,750}; // el2 seedE bins
  el2seedE2DMap["el2seedE_inclusive"] = Analysis::MakeTH2Plot("el2seedE_inclusive","",el2seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (inclusive)",el2seedE2DSubMap,"timing/el2/E/inclusive");  
  el2seedE2DMap["el2seedE_EB"] = Analysis::MakeTH2Plot("el2seedE_EB","",el2seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EB)",el2seedE2DSubMap,"timing/el2/E/EB");  
  el2seedE2DMap["el2seedE_EE"] = Analysis::MakeTH2Plot("el2seedE_EE","",el2seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EE)",el2seedE2DSubMap,"timing/el2/E/EE");  
  el2seedE2DMap["el2seedE_EEP"] = Analysis::MakeTH2Plot("el2seedE_EEP","",el2seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EEP)",el2seedE2DSubMap,"timing/el2/E/EEP");  
  el2seedE2DMap["el2seedE_EEM"] = Analysis::MakeTH2Plot("el2seedE_EEM","",el2seedEbins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Energy [GeV]","Subleading Electron Seed Time [ns] (EEM)",el2seedE2DSubMap,"timing/el2/E/EEM");  

  // make 2D plots for el1 vs nvtx, for EBEB, EBEE, EEEE
  TH2Map  el2nvtx2DMap;    
  TStrMap el2nvtx2DSubMap; 
  el2nvtx2DMap["el2_nvtx_inclusive"] = Analysis::MakeTH2Plot("el2_nvtx_inclusive","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Dielectron Seed Time Difference [ns] (inclusive)",el2nvtx2DSubMap,"timing/el2/nvtx/inclusive");  
  el2nvtx2DMap["el2_nvtx_EB"] = Analysis::MakeTH2Plot("el2_nvtx_EB","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Subleading Electron Seed Time [ns] (EB)",el2nvtx2DSubMap,"timing/el2/nvtx/EB");  
  el2nvtx2DMap["el2_nvtx_EE"] = Analysis::MakeTH2Plot("el2_nvtx_EE","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Subleading Electron Seed Time [ns] (EE)",el2nvtx2DSubMap,"timing/el2/nvtx/EE");  
  el2nvtx2DMap["el2_nvtx_EEP"] = Analysis::MakeTH2Plot("el2_nvtx_EEP","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Subleading Electron Seed Time [ns] (EE+)",el2nvtx2DSubMap,"timing/el2/nvtx/EEP");  
  el2nvtx2DMap["el2_nvtx_EEM"] = Analysis::MakeTH2Plot("el2_nvtx_EEM","",nvtxbins,Config::ntimebins,-Config::timerange,Config::timerange,"nPV","Subleading Electron Seed Time [ns] (EE-)",el2nvtx2DSubMap,"timing/el2/nvtx/EEM");  

  // and also electron eta
  TH2Map  eleta2DMap;    
  TStrMap eleta2DSubMap; 
  DblVec  eletabins; for (Int_t i = 0; i < 21; i++){eletabins.push_back(i/4. - 2.5);}
  eleta2DMap["el1eta_time"] = Analysis::MakeTH2Plot("el1eta_time","",eletabins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron #eta","Leading Electron Seed Time [ns]",eleta2DSubMap,"timing/el1/eta");  
  eleta2DMap["el2eta_time"] = Analysis::MakeTH2Plot("el2eta_time","",eletabins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron #eta","Subleading Electron Seed Time [ns]",eleta2DSubMap,"timing/el2/eta");  

  // and also electron seed eta
  TH2Map  elseedeta2DMap;    
  TStrMap elseedeta2DSubMap; 
  DblVec  elseedetabins; for (Int_t i = 0; i < 21; i++){elseedetabins.push_back(i/4. - 2.5);}
  elseedeta2DMap["el1seedeta_time"] = Analysis::MakeTH2Plot("el1seedeta_time","",elseedetabins,Config::ntimebins,-Config::timerange,Config::timerange,"Leading Electron Seed RecHit #eta","Leading Electron Seed Time [ns]",elseedeta2DSubMap,"timing/el1/seedeta");  
  elseedeta2DMap["el2seedeta_time"] = Analysis::MakeTH2Plot("el2seedeta_time","",elseedetabins,Config::ntimebins,-Config::timerange,Config::timerange,"Subleading Electron Seed RecHit #eta","Subleading Electron Seed Time [ns]",elseedeta2DSubMap,"timing/el2/seedeta");  

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
    runs2DMap["td_runs_inclusive"] = Analysis::MakeTH2Plot("td_runs_inclusive","",dRunNos,Config::ntimebins,-Config::timerange,Config::timerange,"Run Number","Dielectron Seed Time Difference [ns] (inclusive)",runs2DSubMap,"timing/runs/inclusive");  
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

      Float_t el1time = -99.0;
      if (Config::applyTOF && !Config::wgtedtime){
	el1time = TOF(el1seedX,el1seedY,el1seedZ,vtxX,vtxY,vtxZ,el1seedtime);
      }
      else {
	el1time = el1seedtime;
      }

      Float_t el2time = -99.0;
      if (Config::applyTOF && !Config::wgtedtime){
	el2time = TOF(el2seedX,el2seedY,el2seedZ,vtxX,vtxY,vtxZ,el2seedtime);
      }
      else {
	el2time = el2seedtime;
      }

      if (Config::normE) {
	el1pt /= Config::nE;
	el2pt /= Config::nE;

	el1E /= Config::nE;
	el2E /= Config::nE;

	el1p /= Config::nE;
	el2p /= Config::nE;
      }

      const Float_t time_diff   = el1time-el2time;
      const Float_t eff_dielpt  = el1pt*el2pt/std::sqrt(rad2(el1pt,el2pt));
      const Float_t eff_dielE   = el1E*el2E/std::sqrt(rad2(el1E,el2E));
      const Float_t eff_diseedE = el1seedE*el2seedE/std::sqrt(rad2(el1seedE,el2seedE));

      // Electron based bins first
      const Float_t el1seedeta = eta(el1seedX,el1seedY,el1seedZ);
      const Float_t el2seedeta = eta(el2seedX,el2seedY,el2seedZ);

      Bool_t el1eb = false; Bool_t el1ee = false;
      if      (std::abs(el1seedeta) < Config::etaEB)                                                { el1eb = true; }
      else if (std::abs(el1seedeta) > Config::etaEElow && std::abs(el1seedeta) < Config::etaEEhigh) { el1ee = true; }
      Bool_t el1eep = false; Bool_t el1eem = false;
      if (el1ee) {
	if   (el1seedZ>0) {el1eep = true;}
	else              {el1eem = true;}
      }

      Bool_t el2eb = false; Bool_t el2ee = false;
      if      (std::abs(el2seedeta) < Config::etaEB)                                                { el2eb = true; }
      else if (std::abs(el2seedeta) > Config::etaEElow && std::abs(el2seedeta) < Config::etaEEhigh) { el2ee = true; }
      Bool_t el2eep = false; Bool_t el2eem = false;
      if (el2ee) {
	if   (el2seedZ>0) {el2eep = true;}
	else              {el2eem = true;}
      }
      
      // single electron etas
      eleta2DMap["el1eta_time"]->Fill(el1eta,el1time,weight);
      eleta2DMap["el2eta_time"]->Fill(el2eta,el2time,weight);
      elseedeta2DMap["el1seedeta_time"]->Fill(el1seedeta,el1time,weight);
      elseedeta2DMap["el2seedeta_time"]->Fill(el2seedeta,el2time,weight);

      // single electron categories
      el1pt2DMap   ["el1pt_inclusive"]   ->Fill(el1pt,el1time,weight);
      el1E2DMap    ["el1E_inclusive"]    ->Fill(el1E,el1time,weight);
      el1seedE2DMap["el1seedE_inclusive"]->Fill(el1seedE,el1time,weight);
      el1nvtx2DMap ["el1_nvtx_inclusive"]->Fill(nvtx,el1time,weight);
      if      (el1eb) {
	el1pt2DMap   ["el1pt_EB"]   ->Fill(el1pt,   el1time,weight);
	el1E2DMap    ["el1E_EB"]    ->Fill(el1E,    el1time,weight);
	el1seedE2DMap["el1seedE_EB"]->Fill(el1seedE,el1time,weight);
	el1nvtx2DMap ["el1_nvtx_EB"]->Fill(nvtx,el1time,weight);
      }
      else if (el1ee) {
	el1pt2DMap   ["el1pt_EE"]   ->Fill(el1pt,   el1time,weight);
	el1E2DMap    ["el1E_EE"]    ->Fill(el1E,    el1time,weight);
	el1seedE2DMap["el1seedE_EE"]->Fill(el1seedE,el1time,weight);
	el1nvtx2DMap ["el1_nvtx_EB"]->Fill(nvtx,el1time,weight);
	if (el1eep) {
	  el1pt2DMap   ["el1pt_EEP"]   ->Fill(el1pt,   el1time,weight);
	  el1E2DMap    ["el1E_EEP"]    ->Fill(el1E,    el1time,weight);
	  el1seedE2DMap["el1seedE_EEP"]->Fill(el1seedE,el1time,weight);
	  el1nvtx2DMap ["el1_nvtx_EEP"]->Fill(nvtx,el1time,weight);
	}
	else {
	  el1pt2DMap   ["el1pt_EEM"]   ->Fill(el1pt,   el1time,weight);
	  el1E2DMap    ["el1E_EEM"]    ->Fill(el1E,    el1time,weight);
	  el1seedE2DMap["el1seedE_EEM"]->Fill(el1seedE,el1time,weight);
	  el1nvtx2DMap ["el1_nvtx_EEM"]->Fill(nvtx,el1time,weight);
	}
      }

      el2pt2DMap   ["el2pt_inclusive"]   ->Fill(el2pt,el2time,weight);
      el2E2DMap    ["el2E_inclusive"]    ->Fill(el2E,el2time,weight);
      el2seedE2DMap["el2seedE_inclusive"]->Fill(el2seedE,el2time,weight);
      el2nvtx2DMap ["el2_nvtx_inclusive"]->Fill(nvtx,el2time,weight);
      if      (el2eb) {
	el2pt2DMap   ["el2pt_EB"]   ->Fill(el2pt,   el2time,weight);
	el2E2DMap    ["el2E_EB"]    ->Fill(el2E,    el2time,weight);
	el2seedE2DMap["el2seedE_EB"]->Fill(el2seedE,el2time,weight);
	el2nvtx2DMap ["el2_nvtx_EB"]->Fill(nvtx,el2time,weight);
      }
      else if (el2ee) {
	el2pt2DMap   ["el2pt_EE"]   ->Fill(el2pt,   el2time,weight);
	el2E2DMap    ["el2E_EE"]    ->Fill(el2E,    el2time,weight);
	el2seedE2DMap["el2seedE_EE"]->Fill(el2seedE,el2time,weight);
	el2nvtx2DMap ["el2_nvtx_EB"]->Fill(nvtx,el2time,weight);
	if (el2eep) {
	  el2pt2DMap   ["el2pt_EEP"]   ->Fill(el2pt,   el2time,weight);
	  el2E2DMap    ["el2E_EEP"]    ->Fill(el2E,    el2time,weight);
	  el2seedE2DMap["el2seedE_EEP"]->Fill(el2seedE,el2time,weight);
	  el2nvtx2DMap ["el2_nvtx_EEP"]->Fill(nvtx,el2time,weight);
	}
	else {
	  el2pt2DMap   ["el2pt_EEM"]   ->Fill(el2pt,   el2time,weight);
	  el2E2DMap    ["el2E_EEM"]    ->Fill(el2E,    el2time,weight);
	  el2seedE2DMap["el2seedE_EEM"]->Fill(el2seedE,el2time,weight);
	  el2nvtx2DMap ["el2_nvtx_EEM"]->Fill(nvtx,el2time,weight);
	}
      }

      // time diff studies
      effpt2DMap["td_effpt_inclusive"]->Fill(eff_dielpt,time_diff,weight);
      effelE2DMap["td_effelE_inclusive"]->Fill(eff_dielE,time_diff,weight);
      effseedE2DMap["td_effseedE_inclusive"]->Fill(eff_diseedE,time_diff,weight);
      nvtx2DMap["td_nvtx_inclusive"]->Fill(nvtx,time_diff,weight);
      if (!fIsMC && !Config::skipRuns) {runs2DMap["td_runs_inclusive"]->Fill(run,time_diff,weight);}

      if (el1eb && el2eb) {
	inclu1DMap["tdEBEB_inclusive"]->Fill(time_diff,weight);
 	effpt2DMap["tdEBEB_effpt"]->Fill(eff_dielpt,time_diff,weight);
 	effelE2DMap["tdEBEB_effelE"]->Fill(eff_dielE,time_diff,weight);
 	effseedE2DMap["tdEBEB_effseedE"]->Fill(eff_diseedE,time_diff,weight);
	nvtx2DMap["tdEBEB_nvtx"]->Fill(nvtx,time_diff,weight);
	if (!fIsMC && !Config::skipRuns) {runs2DMap["tdEBEB_runs"]->Fill(run,time_diff,weight);}
      }
      else if ( (el1eb && el2ee) || (el1ee && el2eb) ) {
	inclu1DMap["tdEBEE_inclusive"]->Fill(time_diff,weight);
 	effpt2DMap["tdEBEE_effpt"]->Fill(eff_dielpt,time_diff,weight);
 	effelE2DMap["tdEBEE_effelE"]->Fill(eff_dielE,time_diff,weight);
 	effseedE2DMap["tdEBEE_effseedE"]->Fill(eff_diseedE,time_diff,weight);
	nvtx2DMap["tdEBEE_nvtx"]->Fill(nvtx,time_diff,weight);
	if (!fIsMC && !Config::skipRuns) {runs2DMap["tdEBEE_runs"]->Fill(run,time_diff,weight);}
      }
      else if (el1ee && el2ee) {
	inclu1DMap["tdEEEE_inclusive"]->Fill(time_diff,weight);
 	effpt2DMap["tdEEEE_effpt"]->Fill(eff_dielpt,time_diff,weight);
 	effelE2DMap["tdEEEE_effelE"]->Fill(eff_dielE,time_diff,weight);
 	effseedE2DMap["tdEEEE_effseedE"]->Fill(eff_diseedE,time_diff,weight);
	nvtx2DMap["tdEEEE_nvtx"]->Fill(nvtx,time_diff,weight);
	if (!fIsMC && !Config::skipRuns) {runs2DMap["tdEEEE_runs"]->Fill(run,time_diff,weight);}
	if (el1eem && el2eem){
	  inclu1DMap["tdEEMEEM_inclusive"]->Fill(time_diff,weight);
	  effpt2DMap["tdEEMEEM_effpt"]->Fill(eff_dielpt,time_diff,weight);
	  effelE2DMap["tdEEMEEM_effelE"]->Fill(eff_dielE,time_diff,weight);
	  effseedE2DMap["tdEEMEEM_effseedE"]->Fill(eff_diseedE,time_diff,weight);
	  nvtx2DMap["tdEEMEEM_nvtx"]->Fill(nvtx,time_diff,weight);
	}
	else if (el1eep && el2eep){
	  inclu1DMap["tdEEPEEP_inclusive"]->Fill(time_diff,weight);
	  effpt2DMap["tdEEPEEP_effpt"]->Fill(eff_dielpt,time_diff,weight);
	  effelE2DMap["tdEEPEEP_effelE"]->Fill(eff_dielE,time_diff,weight);
	  effseedE2DMap["tdEEPEEP_effseedE"]->Fill(eff_diseedE,time_diff,weight);
	  nvtx2DMap["tdEEPEEP_nvtx"]->Fill(nvtx,time_diff,weight);
	}
// 	else {
// 	  inclu1DMap["tdEEMEEP_inclusive"]->Fill(time_diff,weight);
// 	  effpt2DMap["tdEEMEEP_effpt"]->Fill(eff_dielpt,time_diff,weight);
// 	  effelE2DMap["tdEEMEEP_effelE"]->Fill(eff_dielE,time_diff,weight);
// 	  effseedE2DMap["tdEEMEEP_effseedE"]->Fill(eff_diseedE,time_diff,weight);
// 	  nvtx2DMap["tdEEMEEP_nvtx"]->Fill(nvtx,time_diff,weight);
//	}
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

  // Inclusive
  TH1Map effptinclusive1DMap; TStrMap effptinclusive1DSubMap; TStrIntMap effptinclusive1DbinMap;
  Analysis::Project2Dto1D(effpt2DMap["td_effpt_inclusive"],effpt2DSubMap,effptinclusive1DMap,effptinclusive1DSubMap,effptinclusive1DbinMap);
  Analysis::ProduceMeanSigma(effptinclusive1DMap,effptinclusive1DbinMap,"td_effpt_inclusive","Effective Dielectron p_{T} [GeV/c] (inclusive)",effptbins,effpt2DSubMap["td_effpt_inclusive"]);

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

  // EEMEEM
  TH1Map effptEEMEEM1DMap; TStrMap effptEEMEEM1DSubMap; TStrIntMap effptEEMEEM1DbinMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEEMEEM_effpt"],effpt2DSubMap,effptEEMEEM1DMap,effptEEMEEM1DSubMap,effptEEMEEM1DbinMap);
  Analysis::ProduceMeanSigma(effptEEMEEM1DMap,effptEEMEEM1DbinMap,"tdEEMEEM_effpt","Effective Dielectron p_{T} [GeV/c] (EE-EE-)",effptbins,effpt2DSubMap["tdEEMEEM_effpt"]);

  // EEPEEP
  TH1Map effptEEPEEP1DMap; TStrMap effptEEPEEP1DSubMap; TStrIntMap effptEEPEEP1DbinMap;
  Analysis::Project2Dto1D(effpt2DMap["tdEEPEEP_effpt"],effpt2DSubMap,effptEEPEEP1DMap,effptEEPEEP1DSubMap,effptEEPEEP1DbinMap);
  Analysis::ProduceMeanSigma(effptEEPEEP1DMap,effptEEPEEP1DbinMap,"tdEEPEEP_effpt","Effective Dielectron p_{T} [GeV/c] (EE+EE+)",effptbins,effpt2DSubMap["tdEEPEEP_effpt"]);

  // // EEMEEP
//   TH1Map effptEEMEEP1DMap; TStrMap effptEEMEEP1DSubMap; TStrIntMap effptEEMEEP1DbinMap;
//   Analysis::Project2Dto1D(effpt2DMap["tdEEMEEP_effpt"],effpt2DSubMap,effptEEMEEP1DMap,effptEEMEEP1DSubMap,effptEEMEEP1DbinMap);
//   Analysis::ProduceMeanSigma(effptEEMEEP1DMap,effptEEMEEP1DbinMap,"tdEEMEEP_effpt","Effective Dielectron p_{T} [GeV/c] (EE-EE+)",effptbins,effpt2DSubMap["tdEEMEEP_effpt"]);

  // Delete effective electron pt plots
  Analysis::DeleteTH2s(effpt2DMap);
  Analysis::DeleteTH1s(effptinclusive1DMap);
  Analysis::DeleteTH1s(effptEBEB1DMap);
  Analysis::DeleteTH1s(effptEBEE1DMap);
  Analysis::DeleteTH1s(effptEEEE1DMap);
  Analysis::DeleteTH1s(effptEEMEEM1DMap);
  Analysis::DeleteTH1s(effptEEPEEP1DMap);
  //  Analysis::DeleteTH1s(effptEEMEEP1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do effective electron E plots next
  MakeSubDirs(effelE2DSubMap,fOutDir);
  Analysis::SaveTH2s(effelE2DMap,effelE2DSubMap);

  // Inclusive
  TH1Map effelEinclusive1DMap; TStrMap effelEinclusive1DSubMap; TStrIntMap effelEinclusive1DbinMap;
  Analysis::Project2Dto1D(effelE2DMap["td_effelE_inclusive"],effelE2DSubMap,effelEinclusive1DMap,effelEinclusive1DSubMap,effelEinclusive1DbinMap);
  Analysis::ProduceMeanSigma(effelEinclusive1DMap,effelEinclusive1DbinMap,"td_effelE_inclusive","Effective Dielectron Energy [GeV] (inclusive)",effelEbins,effelE2DSubMap["td_effelE_inclusive"]);

  // EBEB
  TH1Map effelEEBEB1DMap; TStrMap effelEEBEB1DSubMap; TStrIntMap effelEEBEB1DbinMap;
  Analysis::Project2Dto1D(effelE2DMap["tdEBEB_effelE"],effelE2DSubMap,effelEEBEB1DMap,effelEEBEB1DSubMap,effelEEBEB1DbinMap);
  Analysis::ProduceMeanSigma(effelEEBEB1DMap,effelEEBEB1DbinMap,"tdEBEB_effelE","Effective Dielectron Energy [GeV] (EBEB)",effelEbins,effelE2DSubMap["tdEBEB_effelE"]);

  // EBEE
  TH1Map effelEEBEE1DMap; TStrMap effelEEBEE1DSubMap; TStrIntMap effelEEBEE1DbinMap;
  Analysis::Project2Dto1D(effelE2DMap["tdEBEE_effelE"],effelE2DSubMap,effelEEBEE1DMap,effelEEBEE1DSubMap,effelEEBEE1DbinMap);
  Analysis::ProduceMeanSigma(effelEEBEE1DMap,effelEEBEE1DbinMap,"tdEBEE_effelE","Effective Dielectron Energy [GeV] (EBEE)",effelEbins,effelE2DSubMap["tdEBEE_effelE"]);

  // EEEE
  TH1Map effelEEEEE1DMap; TStrMap effelEEEEE1DSubMap; TStrIntMap effelEEEEE1DbinMap;
  Analysis::Project2Dto1D(effelE2DMap["tdEEEE_effelE"],effelE2DSubMap,effelEEEEE1DMap,effelEEEEE1DSubMap,effelEEEEE1DbinMap);
  Analysis::ProduceMeanSigma(effelEEEEE1DMap,effelEEEEE1DbinMap,"tdEEEE_effelE","Effective Dielectron Energy [GeV] (EEEE)",effelEbins,effelE2DSubMap["tdEEEE_effelE"]);

  // EEMEEM
  TH1Map effelEEEMEEM1DMap; TStrMap effelEEEMEEM1DSubMap; TStrIntMap effelEEEMEEM1DbinMap;
  Analysis::Project2Dto1D(effelE2DMap["tdEEMEEM_effelE"],effelE2DSubMap,effelEEEMEEM1DMap,effelEEEMEEM1DSubMap,effelEEEMEEM1DbinMap);
  Analysis::ProduceMeanSigma(effelEEEMEEM1DMap,effelEEEMEEM1DbinMap,"tdEEMEEM_effelE","Effective Dielectron Energy [GeV] (EE-EE-)",effelEbins,effelE2DSubMap["tdEEMEEM_effelE"]);

  // EEPEEP
  TH1Map effelEEEPEEP1DMap; TStrMap effelEEEPEEP1DSubMap; TStrIntMap effelEEEPEEP1DbinMap;
  Analysis::Project2Dto1D(effelE2DMap["tdEEPEEP_effelE"],effelE2DSubMap,effelEEEPEEP1DMap,effelEEEPEEP1DSubMap,effelEEEPEEP1DbinMap);
  Analysis::ProduceMeanSigma(effelEEEPEEP1DMap,effelEEEPEEP1DbinMap,"tdEEPEEP_effelE","Effective Dielectron Energy [GeV] (EE+EE+)",effelEbins,effelE2DSubMap["tdEEPEEP_effelE"]);

//   // EEMEEP
//   TH1Map effelEEEMEEP1DMap; TStrMap effelEEEMEEP1DSubMap; TStrIntMap effelEEEMEEP1DbinMap;
//   Analysis::Project2Dto1D(effelE2DMap["tdEEMEEP_effelE"],effelE2DSubMap,effelEEEMEEP1DMap,effelEEEMEEP1DSubMap,effelEEEMEEP1DbinMap);
//   Analysis::ProduceMeanSigma(effelEEEMEEP1DMap,effelEEEMEEP1DbinMap,"tdEEMEEP_effelE","Effective Dielectron Energy [GeV] (EE-EE+)",effelEbins,effelE2DSubMap["tdEEMEEP_effelE"]);

  // Delete effective electron elE plots
  Analysis::DeleteTH2s(effelE2DMap);
  Analysis::DeleteTH1s(effelEinclusive1DMap);
  Analysis::DeleteTH1s(effelEEBEB1DMap);
  Analysis::DeleteTH1s(effelEEBEE1DMap);
  Analysis::DeleteTH1s(effelEEEEE1DMap);
  Analysis::DeleteTH1s(effelEEEMEEM1DMap);
  Analysis::DeleteTH1s(effelEEEPEEP1DMap);
  //  Analysis::DeleteTH1s(effelEEEMEEP1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do effective electron seed E plots next
  MakeSubDirs(effseedE2DSubMap,fOutDir);
  Analysis::SaveTH2s(effseedE2DMap,effseedE2DSubMap);

  // Inclusive
  TH1Map effseedEinclusive1DMap; TStrMap effseedEinclusive1DSubMap; TStrIntMap effseedEinclusive1DbinMap;
  Analysis::Project2Dto1D(effseedE2DMap["td_effseedE_inclusive"],effseedE2DSubMap,effseedEinclusive1DMap,effseedEinclusive1DSubMap,effseedEinclusive1DbinMap);
  Analysis::ProduceMeanSigma(effseedEinclusive1DMap,effseedEinclusive1DbinMap,"td_effseedE_inclusive","Effective Dielectron Energy [GeV] (inclusive)",effseedEbins,effseedE2DSubMap["td_effseedE_inclusive"]);

  // EBEB
  TH1Map effseedEEBEB1DMap; TStrMap effseedEEBEB1DSubMap; TStrIntMap effseedEEBEB1DbinMap;
  Analysis::Project2Dto1D(effseedE2DMap["tdEBEB_effseedE"],effseedE2DSubMap,effseedEEBEB1DMap,effseedEEBEB1DSubMap,effseedEEBEB1DbinMap);
  Analysis::ProduceMeanSigma(effseedEEBEB1DMap,effseedEEBEB1DbinMap,"tdEBEB_effseedE","Effective Dielectron Energy [GeV] (EBEB)",effseedEbins,effseedE2DSubMap["tdEBEB_effseedE"]);

  // EBEE
  TH1Map effseedEEBEE1DMap; TStrMap effseedEEBEE1DSubMap; TStrIntMap effseedEEBEE1DbinMap;
  Analysis::Project2Dto1D(effseedE2DMap["tdEBEE_effseedE"],effseedE2DSubMap,effseedEEBEE1DMap,effseedEEBEE1DSubMap,effseedEEBEE1DbinMap);
  Analysis::ProduceMeanSigma(effseedEEBEE1DMap,effseedEEBEE1DbinMap,"tdEBEE_effseedE","Effective Dielectron Energy [GeV] (EBEE)",effseedEbins,effseedE2DSubMap["tdEBEE_effseedE"]);

  // EEEE
  TH1Map effseedEEEEE1DMap; TStrMap effseedEEEEE1DSubMap; TStrIntMap effseedEEEEE1DbinMap;
  Analysis::Project2Dto1D(effseedE2DMap["tdEEEE_effseedE"],effseedE2DSubMap,effseedEEEEE1DMap,effseedEEEEE1DSubMap,effseedEEEEE1DbinMap);
  Analysis::ProduceMeanSigma(effseedEEEEE1DMap,effseedEEEEE1DbinMap,"tdEEEE_effseedE","Effective Dielectron Energy [GeV] (EEEE)",effseedEbins,effseedE2DSubMap["tdEEEE_effseedE"]);

  // EEMEEM
  TH1Map effseedEEEMEEM1DMap; TStrMap effseedEEEMEEM1DSubMap; TStrIntMap effseedEEEMEEM1DbinMap;
  Analysis::Project2Dto1D(effseedE2DMap["tdEEMEEM_effseedE"],effseedE2DSubMap,effseedEEEMEEM1DMap,effseedEEEMEEM1DSubMap,effseedEEEMEEM1DbinMap);
  Analysis::ProduceMeanSigma(effseedEEEMEEM1DMap,effseedEEEMEEM1DbinMap,"tdEEMEEM_effseedE","Effective Dielectron Energy [GeV] (EE-EE-)",effseedEbins,effseedE2DSubMap["tdEEMEEM_effseedE"]);

  // EEPEEP
  TH1Map effseedEEEPEEP1DMap; TStrMap effseedEEEPEEP1DSubMap; TStrIntMap effseedEEEPEEP1DbinMap;
  Analysis::Project2Dto1D(effseedE2DMap["tdEEPEEP_effseedE"],effseedE2DSubMap,effseedEEEPEEP1DMap,effseedEEEPEEP1DSubMap,effseedEEEPEEP1DbinMap);
  Analysis::ProduceMeanSigma(effseedEEEPEEP1DMap,effseedEEEPEEP1DbinMap,"tdEEPEEP_effseedE","Effective Dielectron Energy [GeV] (EE+EE+)",effseedEbins,effseedE2DSubMap["tdEEPEEP_effseedE"]);

//   // EEMEEP
//   TH1Map effseedEEEMEEP1DMap; TStrMap effseedEEEMEEP1DSubMap; TStrIntMap effseedEEEMEEP1DbinMap;
//   Analysis::Project2Dto1D(effseedE2DMap["tdEEMEEP_effseedE"],effseedE2DSubMap,effseedEEEMEEP1DMap,effseedEEEMEEP1DSubMap,effseedEEEMEEP1DbinMap);
//   Analysis::ProduceMeanSigma(effseedEEEMEEP1DMap,effseedEEEMEEP1DbinMap,"tdEEMEEP_effseedE","Effective Dielectron Energy [GeV] (EE-EE+)",effseedEbins,effseedE2DSubMap["tdEEMEEP_effseedE"]);

  // Delete effective electron seedE plots
  Analysis::DeleteTH2s(effseedE2DMap);
  Analysis::DeleteTH1s(effseedEinclusive1DMap);
  Analysis::DeleteTH1s(effseedEEBEB1DMap);
  Analysis::DeleteTH1s(effseedEEBEE1DMap);
  Analysis::DeleteTH1s(effseedEEEEE1DMap);
  Analysis::DeleteTH1s(effseedEEEMEEM1DMap);
  Analysis::DeleteTH1s(effseedEEEPEEP1DMap);
  //  Analysis::DeleteTH1s(effseedEEEMEEP1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do effective electron seed E plots next
  MakeSubDirs(nvtx2DSubMap,fOutDir);
  Analysis::SaveTH2s(nvtx2DMap,nvtx2DSubMap);

  // Inclusive
  TH1Map nvtxinclusive1DMap; TStrMap nvtxinclusive1DSubMap; TStrIntMap nvtxinclusive1DbinMap;
  Analysis::Project2Dto1D(nvtx2DMap["td_nvtx_inclusive"],nvtx2DSubMap,nvtxinclusive1DMap,nvtxinclusive1DSubMap,nvtxinclusive1DbinMap);
  Analysis::ProduceMeanSigma(nvtxinclusive1DMap,nvtxinclusive1DbinMap,"td_nvtx_inclusive","nPV (inclusive)",nvtxbins,nvtx2DSubMap["td_nvtx_inclusive"]);

  // EBEB
  TH1Map nvtxEBEB1DMap; TStrMap nvtxEBEB1DSubMap; TStrIntMap nvtxEBEB1DbinMap;
  Analysis::Project2Dto1D(nvtx2DMap["tdEBEB_nvtx"],nvtx2DSubMap,nvtxEBEB1DMap,nvtxEBEB1DSubMap,nvtxEBEB1DbinMap);
  Analysis::ProduceMeanSigma(nvtxEBEB1DMap,nvtxEBEB1DbinMap,"tdEBEB_nvtx","nPV (EBEB)",nvtxbins,nvtx2DSubMap["tdEBEB_nvtx"]);

  // EBEE
  TH1Map nvtxEBEE1DMap; TStrMap nvtxEBEE1DSubMap; TStrIntMap nvtxEBEE1DbinMap;
  Analysis::Project2Dto1D(nvtx2DMap["tdEBEE_nvtx"],nvtx2DSubMap,nvtxEBEE1DMap,nvtxEBEE1DSubMap,nvtxEBEE1DbinMap);
  Analysis::ProduceMeanSigma(nvtxEBEE1DMap,nvtxEBEE1DbinMap,"tdEBEE_nvtx","nPV (EBEE)",nvtxbins,nvtx2DSubMap["tdEBEE_nvtx"]);

  // EEEE
  TH1Map nvtxEEEE1DMap; TStrMap nvtxEEEE1DSubMap; TStrIntMap nvtxEEEE1DbinMap;
  Analysis::Project2Dto1D(nvtx2DMap["tdEEEE_nvtx"],nvtx2DSubMap,nvtxEEEE1DMap,nvtxEEEE1DSubMap,nvtxEEEE1DbinMap);
  Analysis::ProduceMeanSigma(nvtxEEEE1DMap,nvtxEEEE1DbinMap,"tdEEEE_nvtx","nPV (EEEE)",nvtxbins,nvtx2DSubMap["tdEEEE_nvtx"]);

  // EEMEEM
  TH1Map nvtxEEMEEM1DMap; TStrMap nvtxEEMEEM1DSubMap; TStrIntMap nvtxEEMEEM1DbinMap;
  Analysis::Project2Dto1D(nvtx2DMap["tdEEMEEM_nvtx"],nvtx2DSubMap,nvtxEEMEEM1DMap,nvtxEEMEEM1DSubMap,nvtxEEMEEM1DbinMap);
  Analysis::ProduceMeanSigma(nvtxEEMEEM1DMap,nvtxEEMEEM1DbinMap,"tdEEMEEM_nvtx","nPV (EE-EE-)",nvtxbins,nvtx2DSubMap["tdEEMEEM_nvtx"]);

  // EEPEEP
  TH1Map nvtxEEPEEP1DMap; TStrMap nvtxEEPEEP1DSubMap; TStrIntMap nvtxEEPEEP1DbinMap;
  Analysis::Project2Dto1D(nvtx2DMap["tdEEPEEP_nvtx"],nvtx2DSubMap,nvtxEEPEEP1DMap,nvtxEEPEEP1DSubMap,nvtxEEPEEP1DbinMap);
  Analysis::ProduceMeanSigma(nvtxEEPEEP1DMap,nvtxEEPEEP1DbinMap,"tdEEPEEP_nvtx","nPV (EE+EE+)",nvtxbins,nvtx2DSubMap["tdEEPEEP_nvtx"]);

//   // EEMEEP
//   TH1Map nvtxEEMEEP1DMap; TStrMap nvtxEEMEEP1DSubMap; TStrIntMap nvtxEEMEEP1DbinMap;
//   Analysis::Project2Dto1D(nvtx2DMap["tdEEMEEP_nvtx"],nvtx2DSubMap,nvtxEEMEEP1DMap,nvtxEEMEEP1DSubMap,nvtxEEMEEP1DbinMap);
//   Analysis::ProduceMeanSigma(nvtxEEMEEP1DMap,nvtxEEMEEP1DbinMap,"tdEEMEEP_nvtx","nPV (EE-EE+)",nvtxbins,nvtx2DSubMap["tdEEMEEP_nvtx"]);

  // Delete effective electron seedE plots
  Analysis::DeleteTH2s(nvtx2DMap);
  Analysis::DeleteTH1s(nvtxinclusive1DMap);
  Analysis::DeleteTH1s(nvtxEBEB1DMap);
  Analysis::DeleteTH1s(nvtxEBEE1DMap);
  Analysis::DeleteTH1s(nvtxEEEE1DMap);
  Analysis::DeleteTH1s(nvtxEEMEEM1DMap);
  Analysis::DeleteTH1s(nvtxEEPEEP1DMap);
  //  Analysis::DeleteTH1s(nvtxEEMEEP1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron pt time - el1 first
  MakeSubDirs(el1pt2DSubMap,fOutDir);
  Analysis::SaveTH2s(el1pt2DMap,el1pt2DSubMap);

  // inclusive
  TH1Map el1ptinclusive1DMap; TStrMap el1ptinclusive1DSubMap; TStrIntMap el1ptinclusive1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_inclusive"],el1pt2DSubMap,el1ptinclusive1DMap,el1ptinclusive1DSubMap,el1ptinclusive1DbinMap);
  Analysis::ProduceMeanSigma(el1ptinclusive1DMap,el1ptinclusive1DbinMap,"el1pt_inclusive","Leading Electron p_{T} [GeV/c] (inclusive)",el1ptbins,el1pt2DSubMap["el1pt_inclusive"]);

  // EB
  TH1Map el1ptEB1DMap; TStrMap el1ptEB1DSubMap; TStrIntMap el1ptEB1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_EB"],el1pt2DSubMap,el1ptEB1DMap,el1ptEB1DSubMap,el1ptEB1DbinMap);
  Analysis::ProduceMeanSigma(el1ptEB1DMap,el1ptEB1DbinMap,"el1pt_EB","Leading Electron p_{T} [GeV/c] (EB)",el1ptbins,el1pt2DSubMap["el1pt_EB"]);

  // EE
  TH1Map el1ptEE1DMap; TStrMap el1ptEE1DSubMap; TStrIntMap el1ptEE1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_EE"],el1pt2DSubMap,el1ptEE1DMap,el1ptEE1DSubMap,el1ptEE1DbinMap);
  Analysis::ProduceMeanSigma(el1ptEE1DMap,el1ptEE1DbinMap,"el1pt_EE","Leading Electron p_{T} [GeV/c] (EE)",el1ptbins,el1pt2DSubMap["el1pt_EE"]);

  // EEP
  TH1Map el1ptEEP1DMap; TStrMap el1ptEEP1DSubMap; TStrIntMap el1ptEEP1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_EEP"],el1pt2DSubMap,el1ptEEP1DMap,el1ptEEP1DSubMap,el1ptEEP1DbinMap);
  Analysis::ProduceMeanSigma(el1ptEEP1DMap,el1ptEEP1DbinMap,"el1pt_EEP","Leading Electron p_{T} [GeV/c] (EE+)",el1ptbins,el1pt2DSubMap["el1pt_EEP"]);

  // EEM
  TH1Map el1ptEEM1DMap; TStrMap el1ptEEM1DSubMap; TStrIntMap el1ptEEM1DbinMap;
  Analysis::Project2Dto1D(el1pt2DMap["el1pt_EEM"],el1pt2DSubMap,el1ptEEM1DMap,el1ptEEM1DSubMap,el1ptEEM1DbinMap);
  Analysis::ProduceMeanSigma(el1ptEEM1DMap,el1ptEEM1DbinMap,"el1pt_EEM","Leading Electron p_{T} [GeV/c] (EE-)",el1ptbins,el1pt2DSubMap["el1pt_EEM"]);

  // Delete el1 pt plots
  Analysis::DeleteTH2s(el1pt2DMap);
  Analysis::DeleteTH1s(el1ptinclusive1DMap);
  Analysis::DeleteTH1s(el1ptEB1DMap);
  Analysis::DeleteTH1s(el1ptEE1DMap);
  Analysis::DeleteTH1s(el1ptEEP1DMap);
  Analysis::DeleteTH1s(el1ptEEM1DMap);

  // el2 second
  MakeSubDirs(el2pt2DSubMap,fOutDir);
  Analysis::SaveTH2s(el2pt2DMap,el2pt2DSubMap);

  // inclusive
  TH1Map el2ptinclusive1DMap; TStrMap el2ptinclusive1DSubMap; TStrIntMap el2ptinclusive1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_inclusive"],el2pt2DSubMap,el2ptinclusive1DMap,el2ptinclusive1DSubMap,el2ptinclusive1DbinMap);
  Analysis::ProduceMeanSigma(el2ptinclusive1DMap,el2ptinclusive1DbinMap,"el2pt_inclusive","Leading Electron p_{T} [GeV/c] (inclusive)",el2ptbins,el2pt2DSubMap["el2pt_inclusive"]);

  // EB
  TH1Map el2ptEB1DMap; TStrMap el2ptEB1DSubMap; TStrIntMap el2ptEB1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_EB"],el2pt2DSubMap,el2ptEB1DMap,el2ptEB1DSubMap,el2ptEB1DbinMap);
  Analysis::ProduceMeanSigma(el2ptEB1DMap,el2ptEB1DbinMap,"el2pt_EB","Leading Electron p_{T} [GeV/c] (EB)",el2ptbins,el2pt2DSubMap["el2pt_EB"]);

  // EE
  TH1Map el2ptEE1DMap; TStrMap el2ptEE1DSubMap; TStrIntMap el2ptEE1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_EE"],el2pt2DSubMap,el2ptEE1DMap,el2ptEE1DSubMap,el2ptEE1DbinMap);
  Analysis::ProduceMeanSigma(el2ptEE1DMap,el2ptEE1DbinMap,"el2pt_EE","Leading Electron p_{T} [GeV/c] (EE)",el2ptbins,el2pt2DSubMap["el2pt_EE"]);

  // EEP
  TH1Map el2ptEEP1DMap; TStrMap el2ptEEP1DSubMap; TStrIntMap el2ptEEP1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_EEP"],el2pt2DSubMap,el2ptEEP1DMap,el2ptEEP1DSubMap,el2ptEEP1DbinMap);
  Analysis::ProduceMeanSigma(el2ptEEP1DMap,el2ptEEP1DbinMap,"el2pt_EEP","Leading Electron p_{T} [GeV/c] (EE+)",el2ptbins,el2pt2DSubMap["el2pt_EEP"]);

  // EEM
  TH1Map el2ptEEM1DMap; TStrMap el2ptEEM1DSubMap; TStrIntMap el2ptEEM1DbinMap;
  Analysis::Project2Dto1D(el2pt2DMap["el2pt_EEM"],el2pt2DSubMap,el2ptEEM1DMap,el2ptEEM1DSubMap,el2ptEEM1DbinMap);
  Analysis::ProduceMeanSigma(el2ptEEM1DMap,el2ptEEM1DbinMap,"el2pt_EEM","Leading Electron p_{T} [GeV/c] (EE-)",el2ptbins,el2pt2DSubMap["el2pt_EEM"]);

  // Delete el2 pt plots
  Analysis::DeleteTH2s(el2pt2DMap);
  Analysis::DeleteTH1s(el2ptinclusive1DMap);
  Analysis::DeleteTH1s(el2ptEB1DMap);
  Analysis::DeleteTH1s(el2ptEE1DMap);
  Analysis::DeleteTH1s(el2ptEEP1DMap);
  Analysis::DeleteTH1s(el2ptEEM1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron E time - el1 first
  MakeSubDirs(el1E2DSubMap,fOutDir);
  Analysis::SaveTH2s(el1E2DMap,el1E2DSubMap);

  // inclusive
  TH1Map el1Einclusive1DMap; TStrMap el1Einclusive1DSubMap; TStrIntMap el1Einclusive1DbinMap;
  Analysis::Project2Dto1D(el1E2DMap["el1E_inclusive"],el1E2DSubMap,el1Einclusive1DMap,el1Einclusive1DSubMap,el1Einclusive1DbinMap);
  Analysis::ProduceMeanSigma(el1Einclusive1DMap,el1Einclusive1DbinMap,"el1E_inclusive","Leading Electron Energy [GeV] (inclusive)",el1Ebins,el1E2DSubMap["el1E_inclusive"]);

  // EB
  TH1Map el1EEB1DMap; TStrMap el1EEB1DSubMap; TStrIntMap el1EEB1DbinMap;
  Analysis::Project2Dto1D(el1E2DMap["el1E_EB"],el1E2DSubMap,el1EEB1DMap,el1EEB1DSubMap,el1EEB1DbinMap);
  Analysis::ProduceMeanSigma(el1EEB1DMap,el1EEB1DbinMap,"el1E_EB","Leading Electron Energy [GeV] (EB)",el1Ebins,el1E2DSubMap["el1E_EB"]);

  // EE
  TH1Map el1EEE1DMap; TStrMap el1EEE1DSubMap; TStrIntMap el1EEE1DbinMap;
  Analysis::Project2Dto1D(el1E2DMap["el1E_EE"],el1E2DSubMap,el1EEE1DMap,el1EEE1DSubMap,el1EEE1DbinMap);
  Analysis::ProduceMeanSigma(el1EEE1DMap,el1EEE1DbinMap,"el1E_EE","Leading Electron Energy [GeV] (EE)",el1Ebins,el1E2DSubMap["el1E_EE"]);

  // EEP
  TH1Map el1EEEP1DMap; TStrMap el1EEEP1DSubMap; TStrIntMap el1EEEP1DbinMap;
  Analysis::Project2Dto1D(el1E2DMap["el1E_EEP"],el1E2DSubMap,el1EEEP1DMap,el1EEEP1DSubMap,el1EEEP1DbinMap);
  Analysis::ProduceMeanSigma(el1EEEP1DMap,el1EEEP1DbinMap,"el1E_EEP","Leading Electron Energy [GeV] (EE+)",el1Ebins,el1E2DSubMap["el1E_EEP"]);

  // EEM
  TH1Map el1EEEM1DMap; TStrMap el1EEEM1DSubMap; TStrIntMap el1EEEM1DbinMap;
  Analysis::Project2Dto1D(el1E2DMap["el1E_EEM"],el1E2DSubMap,el1EEEM1DMap,el1EEEM1DSubMap,el1EEEM1DbinMap);
  Analysis::ProduceMeanSigma(el1EEEM1DMap,el1EEEM1DbinMap,"el1E_EEM","Leading Electron Energy [GeV] (EE-)",el1Ebins,el1E2DSubMap["el1E_EEM"]);

  // Delete el1 E plots
  Analysis::DeleteTH2s(el1E2DMap);
  Analysis::DeleteTH1s(el1Einclusive1DMap);
  Analysis::DeleteTH1s(el1EEB1DMap);
  Analysis::DeleteTH1s(el1EEE1DMap);
  Analysis::DeleteTH1s(el1EEEP1DMap);
  Analysis::DeleteTH1s(el1EEEM1DMap);

  // el2 second
  MakeSubDirs(el2E2DSubMap,fOutDir);
  Analysis::SaveTH2s(el2E2DMap,el2E2DSubMap);

  // inclusive
  TH1Map el2Einclusive1DMap; TStrMap el2Einclusive1DSubMap; TStrIntMap el2Einclusive1DbinMap;
  Analysis::Project2Dto1D(el2E2DMap["el2E_inclusive"],el2E2DSubMap,el2Einclusive1DMap,el2Einclusive1DSubMap,el2Einclusive1DbinMap);
  Analysis::ProduceMeanSigma(el2Einclusive1DMap,el2Einclusive1DbinMap,"el2E_inclusive","Leading Electron Energy [GeV] (inclusive)",el2Ebins,el2E2DSubMap["el2E_inclusive"]);

  // EB
  TH1Map el2EEB1DMap; TStrMap el2EEB1DSubMap; TStrIntMap el2EEB1DbinMap;
  Analysis::Project2Dto1D(el2E2DMap["el2E_EB"],el2E2DSubMap,el2EEB1DMap,el2EEB1DSubMap,el2EEB1DbinMap);
  Analysis::ProduceMeanSigma(el2EEB1DMap,el2EEB1DbinMap,"el2E_EB","Leading Electron Energy [GeV] (EB)",el2Ebins,el2E2DSubMap["el2E_EB"]);

  // EE
  TH1Map el2EEE1DMap; TStrMap el2EEE1DSubMap; TStrIntMap el2EEE1DbinMap;
  Analysis::Project2Dto1D(el2E2DMap["el2E_EE"],el2E2DSubMap,el2EEE1DMap,el2EEE1DSubMap,el2EEE1DbinMap);
  Analysis::ProduceMeanSigma(el2EEE1DMap,el2EEE1DbinMap,"el2E_EE","Leading Electron Energy [GeV] (EE)",el2Ebins,el2E2DSubMap["el2E_EE"]);

  // EEP
  TH1Map el2EEEP1DMap; TStrMap el2EEEP1DSubMap; TStrIntMap el2EEEP1DbinMap;
  Analysis::Project2Dto1D(el2E2DMap["el2E_EEP"],el2E2DSubMap,el2EEEP1DMap,el2EEEP1DSubMap,el2EEEP1DbinMap);
  Analysis::ProduceMeanSigma(el2EEEP1DMap,el2EEEP1DbinMap,"el2E_EEP","Leading Electron Energy [GeV] (EE+)",el2Ebins,el2E2DSubMap["el2E_EEP"]);

  // EEM
  TH1Map el2EEEM1DMap; TStrMap el2EEEM1DSubMap; TStrIntMap el2EEEM1DbinMap;
  Analysis::Project2Dto1D(el2E2DMap["el2E_EEM"],el2E2DSubMap,el2EEEM1DMap,el2EEEM1DSubMap,el2EEEM1DbinMap);
  Analysis::ProduceMeanSigma(el2EEEM1DMap,el2EEEM1DbinMap,"el2E_EEM","Leading Electron Energy [GeV] (EE-)",el2Ebins,el2E2DSubMap["el2E_EEM"]);

  // Delete el2 E plots
  Analysis::DeleteTH2s(el2E2DMap);
  Analysis::DeleteTH1s(el2Einclusive1DMap);
  Analysis::DeleteTH1s(el2EEB1DMap);
  Analysis::DeleteTH1s(el2EEE1DMap);
  Analysis::DeleteTH1s(el2EEEP1DMap);
  Analysis::DeleteTH1s(el2EEEM1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron seedE time - el1 first
  MakeSubDirs(el1seedE2DSubMap,fOutDir);
  Analysis::SaveTH2s(el1seedE2DMap,el1seedE2DSubMap);

  // inclusive
  TH1Map el1seedEinclusive1DMap; TStrMap el1seedEinclusive1DSubMap; TStrIntMap el1seedEinclusive1DbinMap;
  Analysis::Project2Dto1D(el1seedE2DMap["el1seedE_inclusive"],el1seedE2DSubMap,el1seedEinclusive1DMap,el1seedEinclusive1DSubMap,el1seedEinclusive1DbinMap);
  Analysis::ProduceMeanSigma(el1seedEinclusive1DMap,el1seedEinclusive1DbinMap,"el1seedE_inclusive","Leading Electron Energy [GeV] (inclusive)",el1seedEbins,el1seedE2DSubMap["el1seedE_inclusive"]);

  // EB
  TH1Map el1seedEEB1DMap; TStrMap el1seedEEB1DSubMap; TStrIntMap el1seedEEB1DbinMap;
  Analysis::Project2Dto1D(el1seedE2DMap["el1seedE_EB"],el1seedE2DSubMap,el1seedEEB1DMap,el1seedEEB1DSubMap,el1seedEEB1DbinMap);
  Analysis::ProduceMeanSigma(el1seedEEB1DMap,el1seedEEB1DbinMap,"el1seedE_EB","Leading Electron Energy [GeV] (EB)",el1seedEbins,el1seedE2DSubMap["el1seedE_EB"]);

  // EE
  TH1Map el1seedEEE1DMap; TStrMap el1seedEEE1DSubMap; TStrIntMap el1seedEEE1DbinMap;
  Analysis::Project2Dto1D(el1seedE2DMap["el1seedE_EE"],el1seedE2DSubMap,el1seedEEE1DMap,el1seedEEE1DSubMap,el1seedEEE1DbinMap);
  Analysis::ProduceMeanSigma(el1seedEEE1DMap,el1seedEEE1DbinMap,"el1seedE_EE","Leading Electron Energy [GeV] (EE)",el1seedEbins,el1seedE2DSubMap["el1seedE_EE"]);

  // EEP
  TH1Map el1seedEEEP1DMap; TStrMap el1seedEEEP1DSubMap; TStrIntMap el1seedEEEP1DbinMap;
  Analysis::Project2Dto1D(el1seedE2DMap["el1seedE_EEP"],el1seedE2DSubMap,el1seedEEEP1DMap,el1seedEEEP1DSubMap,el1seedEEEP1DbinMap);
  Analysis::ProduceMeanSigma(el1seedEEEP1DMap,el1seedEEEP1DbinMap,"el1seedE_EEP","Leading Electron Energy [GeV] (EE+)",el1seedEbins,el1seedE2DSubMap["el1seedE_EEP"]);

  // EEM
  TH1Map el1seedEEEM1DMap; TStrMap el1seedEEEM1DSubMap; TStrIntMap el1seedEEEM1DbinMap;
  Analysis::Project2Dto1D(el1seedE2DMap["el1seedE_EEM"],el1seedE2DSubMap,el1seedEEEM1DMap,el1seedEEEM1DSubMap,el1seedEEEM1DbinMap);
  Analysis::ProduceMeanSigma(el1seedEEEM1DMap,el1seedEEEM1DbinMap,"el1seedE_EEM","Leading Electron Energy [GeV] (EE-)",el1seedEbins,el1seedE2DSubMap["el1seedE_EEM"]);

  // Delete el1 E plots
  Analysis::DeleteTH2s(el1seedE2DMap);
  Analysis::DeleteTH1s(el1seedEinclusive1DMap);
  Analysis::DeleteTH1s(el1seedEEB1DMap);
  Analysis::DeleteTH1s(el1seedEEE1DMap);
  Analysis::DeleteTH1s(el1seedEEEP1DMap);
  Analysis::DeleteTH1s(el1seedEEEM1DMap);

  // el2 second
  MakeSubDirs(el2seedE2DSubMap,fOutDir);
  Analysis::SaveTH2s(el2seedE2DMap,el2seedE2DSubMap);

  // inclusive
  TH1Map el2seedEinclusive1DMap; TStrMap el2seedEinclusive1DSubMap; TStrIntMap el2seedEinclusive1DbinMap;
  Analysis::Project2Dto1D(el2seedE2DMap["el2seedE_inclusive"],el2seedE2DSubMap,el2seedEinclusive1DMap,el2seedEinclusive1DSubMap,el2seedEinclusive1DbinMap);
  Analysis::ProduceMeanSigma(el2seedEinclusive1DMap,el2seedEinclusive1DbinMap,"el2seedE_inclusive","Leading Electron Energy [GeV] (inclusive)",el2seedEbins,el2seedE2DSubMap["el2seedE_inclusive"]);

  // EB
  TH1Map el2seedEEB1DMap; TStrMap el2seedEEB1DSubMap; TStrIntMap el2seedEEB1DbinMap;
  Analysis::Project2Dto1D(el2seedE2DMap["el2seedE_EB"],el2seedE2DSubMap,el2seedEEB1DMap,el2seedEEB1DSubMap,el2seedEEB1DbinMap);
  Analysis::ProduceMeanSigma(el2seedEEB1DMap,el2seedEEB1DbinMap,"el2seedE_EB","Leading Electron Energy [GeV] (EB)",el2seedEbins,el2seedE2DSubMap["el2seedE_EB"]);

  // EE
  TH1Map el2seedEEE1DMap; TStrMap el2seedEEE1DSubMap; TStrIntMap el2seedEEE1DbinMap;
  Analysis::Project2Dto1D(el2seedE2DMap["el2seedE_EE"],el2seedE2DSubMap,el2seedEEE1DMap,el2seedEEE1DSubMap,el2seedEEE1DbinMap);
  Analysis::ProduceMeanSigma(el2seedEEE1DMap,el2seedEEE1DbinMap,"el2seedE_EE","Leading Electron Energy [GeV] (EE)",el2seedEbins,el2seedE2DSubMap["el2seedE_EE"]);

  // EEP
  TH1Map el2seedEEEP1DMap; TStrMap el2seedEEEP1DSubMap; TStrIntMap el2seedEEEP1DbinMap;
  Analysis::Project2Dto1D(el2seedE2DMap["el2seedE_EEP"],el2seedE2DSubMap,el2seedEEEP1DMap,el2seedEEEP1DSubMap,el2seedEEEP1DbinMap);
  Analysis::ProduceMeanSigma(el2seedEEEP1DMap,el2seedEEEP1DbinMap,"el2seedE_EEP","Leading Electron Energy [GeV] (EE+)",el2seedEbins,el2seedE2DSubMap["el2seedE_EEP"]);

  // EEM
  TH1Map el2seedEEEM1DMap; TStrMap el2seedEEEM1DSubMap; TStrIntMap el2seedEEEM1DbinMap;
  Analysis::Project2Dto1D(el2seedE2DMap["el2seedE_EEM"],el2seedE2DSubMap,el2seedEEEM1DMap,el2seedEEEM1DSubMap,el2seedEEEM1DbinMap);
  Analysis::ProduceMeanSigma(el2seedEEEM1DMap,el2seedEEEM1DbinMap,"el2seedE_EEM","Leading Electron Energy [GeV] (EE-)",el2seedEbins,el2seedE2DSubMap["el2seedE_EEM"]);

  // Delete el2 E plots
  Analysis::DeleteTH2s(el2seedE2DMap);
  Analysis::DeleteTH1s(el2seedEinclusive1DMap);
  Analysis::DeleteTH1s(el2seedEEB1DMap);
  Analysis::DeleteTH1s(el2seedEEE1DMap);
  Analysis::DeleteTH1s(el2seedEEEP1DMap);
  Analysis::DeleteTH1s(el2seedEEEM1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron nvtx time - el1 first
  MakeSubDirs(el1nvtx2DSubMap,fOutDir);
  Analysis::SaveTH2s(el1nvtx2DMap,el1nvtx2DSubMap);

  // inclusive
  TH1Map el1nvtxinclusive1DMap; TStrMap el1nvtxinclusive1DSubMap; TStrIntMap el1nvtxinclusive1DbinMap;
  Analysis::Project2Dto1D(el1nvtx2DMap["el1_nvtx_inclusive"],el1nvtx2DSubMap,el1nvtxinclusive1DMap,el1nvtxinclusive1DSubMap,el1nvtxinclusive1DbinMap);
  Analysis::ProduceMeanSigma(el1nvtxinclusive1DMap,el1nvtxinclusive1DbinMap,"el1_nvtx_inclusive","Leading Electron p_{T} [GeV/c] (inclusive)",nvtxbins,el1nvtx2DSubMap["el1_nvtx_inclusive"]);

  // EB
  TH1Map el1nvtxEB1DMap; TStrMap el1nvtxEB1DSubMap; TStrIntMap el1nvtxEB1DbinMap;
  Analysis::Project2Dto1D(el1nvtx2DMap["el1_nvtx_EB"],el1nvtx2DSubMap,el1nvtxEB1DMap,el1nvtxEB1DSubMap,el1nvtxEB1DbinMap);
  Analysis::ProduceMeanSigma(el1nvtxEB1DMap,el1nvtxEB1DbinMap,"el1_nvtx_EB","Leading Electron p_{T} [GeV/c] (EB)",nvtxbins,el1nvtx2DSubMap["el1_nvtx_EB"]);

  // EE
  TH1Map el1nvtxEE1DMap; TStrMap el1nvtxEE1DSubMap; TStrIntMap el1nvtxEE1DbinMap;
  Analysis::Project2Dto1D(el1nvtx2DMap["el1_nvtx_EE"],el1nvtx2DSubMap,el1nvtxEE1DMap,el1nvtxEE1DSubMap,el1nvtxEE1DbinMap);
  Analysis::ProduceMeanSigma(el1nvtxEE1DMap,el1nvtxEE1DbinMap,"el1_nvtx_EE","Leading Electron p_{T} [GeV/c] (EE)",nvtxbins,el1nvtx2DSubMap["el1_nvtx_EE"]);

  // EEP
  TH1Map el1nvtxEEP1DMap; TStrMap el1nvtxEEP1DSubMap; TStrIntMap el1nvtxEEP1DbinMap;
  Analysis::Project2Dto1D(el1nvtx2DMap["el1_nvtx_EEP"],el1nvtx2DSubMap,el1nvtxEEP1DMap,el1nvtxEEP1DSubMap,el1nvtxEEP1DbinMap);
  Analysis::ProduceMeanSigma(el1nvtxEEP1DMap,el1nvtxEEP1DbinMap,"el1_nvtx_EEP","Leading Electron p_{T} [GeV/c] (EE+)",nvtxbins,el1nvtx2DSubMap["el1_nvtx_EEP"]);

  // EEM
  TH1Map el1nvtxEEM1DMap; TStrMap el1nvtxEEM1DSubMap; TStrIntMap el1nvtxEEM1DbinMap;
  Analysis::Project2Dto1D(el1nvtx2DMap["el1_nvtx_EEM"],el1nvtx2DSubMap,el1nvtxEEM1DMap,el1nvtxEEM1DSubMap,el1nvtxEEM1DbinMap);
  Analysis::ProduceMeanSigma(el1nvtxEEM1DMap,el1nvtxEEM1DbinMap,"el1_nvtx_EEM","Leading Electron p_{T} [GeV/c] (EE-)",nvtxbins,el1nvtx2DSubMap["el1_nvtx_EEM"]);

  // Delete el1 nvtx plots
  Analysis::DeleteTH2s(el1nvtx2DMap);
  Analysis::DeleteTH1s(el1nvtxinclusive1DMap);
  Analysis::DeleteTH1s(el1nvtxEB1DMap);

  Analysis::DeleteTH1s(el1nvtxEE1DMap);
  Analysis::DeleteTH1s(el1nvtxEEP1DMap);
  Analysis::DeleteTH1s(el1nvtxEEM1DMap);

  // el2 second
  MakeSubDirs(el2nvtx2DSubMap,fOutDir);
  Analysis::SaveTH2s(el2nvtx2DMap,el2nvtx2DSubMap);

  // inclusive
  TH1Map el2nvtxinclusive1DMap; TStrMap el2nvtxinclusive1DSubMap; TStrIntMap el2nvtxinclusive1DbinMap;
  Analysis::Project2Dto1D(el2nvtx2DMap["el2_nvtx_inclusive"],el2nvtx2DSubMap,el2nvtxinclusive1DMap,el2nvtxinclusive1DSubMap,el2nvtxinclusive1DbinMap);
  Analysis::ProduceMeanSigma(el2nvtxinclusive1DMap,el2nvtxinclusive1DbinMap,"el2_nvtx_inclusive","Leading Electron p_{T} [GeV/c] (inclusive)",nvtxbins,el2nvtx2DSubMap["el2_nvtx_inclusive"]);

  // EB
  TH1Map el2nvtxEB1DMap; TStrMap el2nvtxEB1DSubMap; TStrIntMap el2nvtxEB1DbinMap;
  Analysis::Project2Dto1D(el2nvtx2DMap["el2_nvtx_EB"],el2nvtx2DSubMap,el2nvtxEB1DMap,el2nvtxEB1DSubMap,el2nvtxEB1DbinMap);
  Analysis::ProduceMeanSigma(el2nvtxEB1DMap,el2nvtxEB1DbinMap,"el2_nvtx_EB","Leading Electron p_{T} [GeV/c] (EB)",nvtxbins,el2nvtx2DSubMap["el2_nvtx_EB"]);

  // EE
  TH1Map el2nvtxEE1DMap; TStrMap el2nvtxEE1DSubMap; TStrIntMap el2nvtxEE1DbinMap;
  Analysis::Project2Dto1D(el2nvtx2DMap["el2_nvtx_EE"],el2nvtx2DSubMap,el2nvtxEE1DMap,el2nvtxEE1DSubMap,el2nvtxEE1DbinMap);
  Analysis::ProduceMeanSigma(el2nvtxEE1DMap,el2nvtxEE1DbinMap,"el2_nvtx_EE","Leading Electron p_{T} [GeV/c] (EE)",nvtxbins,el2nvtx2DSubMap["el2_nvtx_EE"]);

  // EEP
  TH1Map el2nvtxEEP1DMap; TStrMap el2nvtxEEP1DSubMap; TStrIntMap el2nvtxEEP1DbinMap;
  Analysis::Project2Dto1D(el2nvtx2DMap["el2_nvtx_EEP"],el2nvtx2DSubMap,el2nvtxEEP1DMap,el2nvtxEEP1DSubMap,el2nvtxEEP1DbinMap);
  Analysis::ProduceMeanSigma(el2nvtxEEP1DMap,el2nvtxEEP1DbinMap,"el2_nvtx_EEP","Leading Electron p_{T} [GeV/c] (EE+)",nvtxbins,el2nvtx2DSubMap["el2_nvtx_EEP"]);

  // EEM
  TH1Map el2nvtxEEM1DMap; TStrMap el2nvtxEEM1DSubMap; TStrIntMap el2nvtxEEM1DbinMap;
  Analysis::Project2Dto1D(el2nvtx2DMap["el2_nvtx_EEM"],el2nvtx2DSubMap,el2nvtxEEM1DMap,el2nvtxEEM1DSubMap,el2nvtxEEM1DbinMap);
  Analysis::ProduceMeanSigma(el2nvtxEEM1DMap,el2nvtxEEM1DbinMap,"el2_nvtx_EEM","Leading Electron p_{T} [GeV/c] (EE-)",nvtxbins,el2nvtx2DSubMap["el2_nvtx_EEM"]);

  // Delete el2 nvtx plots
  Analysis::DeleteTH2s(el2nvtx2DMap);
  Analysis::DeleteTH1s(el2nvtxinclusive1DMap);
  Analysis::DeleteTH1s(el2nvtxEB1DMap);
  Analysis::DeleteTH1s(el2nvtxEE1DMap);
  Analysis::DeleteTH1s(el2nvtxEEP1DMap);
  Analysis::DeleteTH1s(el2nvtxEEM1DMap);
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

  // Delete eta plots
  Analysis::DeleteTH2s(eleta2DMap);
  Analysis::DeleteTH1s(el1eta1DMap);
  Analysis::DeleteTH1s(el2eta1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do single electron seed eta plots
  MakeSubDirs(elseedeta2DSubMap,fOutDir);
  Analysis::SaveTH2s(elseedeta2DMap,elseedeta2DSubMap);

  TH1Map el1seedeta1DMap; TStrMap el1seedeta1DSubMap; TStrIntMap el1seedeta1DbinMap;
  Analysis::Project2Dto1D(elseedeta2DMap["el1seedeta_time"],el1pt2DSubMap,el1seedeta1DMap,el1seedeta1DSubMap,el1seedeta1DbinMap);
  Analysis::ProduceMeanSigma(el1seedeta1DMap,el1seedeta1DbinMap,"el1seedeta_time","Leading Electron #eta",elseedetabins,elseedeta2DSubMap["el1seedeta_time"]);

  TH1Map el2seedeta1DMap; TStrMap el2seedeta1DSubMap; TStrIntMap el2seedeta1DbinMap;
  Analysis::Project2Dto1D(elseedeta2DMap["el2seedeta_time"],el2pt2DSubMap,el2seedeta1DMap,el2seedeta1DSubMap,el2seedeta1DbinMap);
  Analysis::ProduceMeanSigma(el2seedeta1DMap,el2seedeta1DbinMap,"el2seedeta_time","Subleading Electron #eta",elseedetabins,elseedeta2DSubMap["el2seedeta_time"]);

  // Delete el1 pt plots
  Analysis::DeleteTH2s(elseedeta2DMap);
  Analysis::DeleteTH1s(el1seedeta1DMap);
  Analysis::DeleteTH1s(el2seedeta1DMap);
  ////////////////////////////////////

  ////////////////////////////////////
  // Do run plots last
  if (!fIsMC && !Config::skipRuns) {
    MakeSubDirs(runs2DSubMap,fOutDir);
    Analysis::SaveTH2s(runs2DMap,runs2DSubMap);

    // Inclusive
    TH1Map runsinclusive1DMap; TStrMap runsinclusive1DSubMap; TStrIntMap runsinclusive1DbinMap;
    Analysis::Project2Dto1D(runs2DMap["td_runs_inclusive"],runs2DSubMap,runsinclusive1DMap,runsinclusive1DSubMap,runsinclusive1DbinMap);
    Analysis::ProduceMeanSigma(runsinclusive1DMap,runsinclusive1DbinMap,"td_runs_inclusive","Run Number (inclusive)",dRunNos,runs2DSubMap["td_runs_inclusive"]);

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
    Analysis::DeleteTH1s(runsinclusive1DMap);
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
    else if ((basename.Contains("zphi",TString::kExact))       || (basename.Contains("abszeta",TString::kExact))     ||
	     (basename.Contains("el1eta",TString::kExact))     || (basename.Contains("el2eta",TString::kExact))      || 
	     (basename.Contains("el1seedeta",TString::kExact)) || (basename.Contains("el2seedeta",TString::kExact))) { //triple ugh
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
    else if (basename.Contains("nvtx",TString::kExact)) { //triple ugh
      Int_t ivtx = (xlow+xhigh)/2;
      histname = Form("%s_%i",basename.Data(),ivtx);
      th1map[histname.Data()] = Analysis::MakeTH1Plot(histname.Data(),"",nybins,ylow,yhigh,Form("%s in nPV: %i",ytitle.Data(),ivtx),
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
  outhist_mean->GetYaxis()->SetTitleOffset(outhist_mean->GetYaxis()->GetTitleOffset() * Config::TitleFF);
  outhist_mean->Sumw2();

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
  outhist_sigma->GetYaxis()->SetTitleOffset(outhist_sigma->GetYaxis()->GetTitleOffset() * Config::TitleFF);
  outhist_sigma->Sumw2();

  // use this to store runs that by themselves produce bad fits
  TH1Map tempmap; // a bit hacky I admit...
  Int_t  sumevents = 0; // also a bit hacky...

  for (TH1MapIter mapiter = th1map.begin(); mapiter != th1map.end(); ++mapiter) { 
    Int_t bin = th1binmap[(*mapiter).first]; // returns which bin each th1 corresponds to one the new plot
    
    // only do this for run number plots --> check each plot has enough entries to do fit
    if ( name.Contains("runs",TString::kExact) ) {
      if ( ((*mapiter).second->Integral() + sumevents) < Config::nEventsCut ) { 
	
	// store the plot to be added later
	tempmap[(*mapiter).first] = (TH1F*)(*mapiter).second->Clone(Form("%s_tmp",(*mapiter).first.Data()));

	// record the number of events to exceed cut
	sumevents += tempmap[(*mapiter).first]->Integral();
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
	  numer += th1binmap[(*tempmapiter).first] * (*tempmapiter).second->Integral();
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
    (*mapiter).second->Fit(fit->GetName(),"RBQ0");

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
  } // end loop over th1s

  TString xruntitle = outhist_mean->GetXaxis()->GetTitle();
  if (name.Contains("runs",TString::kExact) && xruntitle.Contains("EBEB",TString::kExact)) {
    outhist_mean->SetMaximum(  0.04 );
    outhist_mean->SetMinimum( -0.04 );

    outhist_sigma->SetMaximum( 0.50 );
    outhist_sigma->SetMinimum( 0.25 );
  }

  // write output hist to file
  fOutFile->cd();
  outhist_mean->Write();
  outhist_sigma->Write();

  // and want to dump them too (for stacking)!
  if (!fIsMC && !name.Contains("runs",TString::kExact)) {fTH1Dump << outhist_mean->GetName()  << " " << subdir.Data() << std::endl;}
  if (!fIsMC && !name.Contains("runs",TString::kExact)) {fTH1Dump << outhist_sigma->GetName() << " " << subdir.Data() << std::endl;}

  // save log/lin of each plot
  TCanvas * canv = new TCanvas("canv","canv");
  canv->cd();

  outhist_mean->Draw("PE");
  CMSLumi(canv);
  canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdir.Data(),outhist_mean->GetName(),Config::outtype.Data()));

  Float_t min = 1e9;
  for (Int_t i = 1; i <= outhist_sigma->GetNbinsX(); i++){
    Float_t tmpmin = outhist_sigma->GetBinContent(i);
    if (tmpmin < min && tmpmin != 0){ min = tmpmin; }
  }
  outhist_sigma->SetMinimum( min / 1.1 );
  outhist_sigma->Draw("PE");
  CMSLumi(canv);
  canv->SaveAs(Form("%s/%s/%s.%s",fOutDir.Data(),subdir.Data(),outhist_sigma->GetName(),Config::outtype.Data()));
  
  delete canv;
  delete outhist_sigma;
  delete outhist_mean;
}

void Analysis::PrepFit(TF1 *& fit, TH1F *& hist) {
  TF1 * tempfit = new TF1("temp","gaus(0)",-Config::fitrange,Config::fitrange);
  tempfit->SetParLimits(2,0,10);
  hist->Fit("temp","RQ0B");
  const Float_t tempp0 = tempfit->GetParameter(0); // constant
  const Float_t tempp1 = tempfit->GetParameter(1); // mean
  const Float_t tempp2 = tempfit->GetParameter(2); // sigma

  if (Config::formname.EqualTo("gaus1",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2);
    fit->SetParLimits(2,0,10);
  }
  else if (Config::formname.EqualTo("gaus2",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[4])/[5])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,0,tempp2*4);
    fit->SetParLimits(2,0,10);
    fit->SetParLimits(5,0,10);
  }
  else if (Config::formname.EqualTo("gaus2fm",TString::kExact)) {
    TFormula form(Config::formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(Form("%s_fit",Config::formname.Data()),Config::formname.Data(),-Config::fitrange,Config::fitrange);
    fit->SetParameters(tempp0,tempp1,tempp2,tempp0/10,tempp2*4);
    fit->SetParLimits(2,0,10);
    fit->SetParLimits(4,0,10);
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

  fit->SetLineColor(kMagenta-3); //kViolet-6
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

  sub1->SetLineColor(kRed) ;  // kgreen-3
  sub1->SetLineWidth(2);
  sub1->SetLineStyle(7);
  sub1->Draw("same");

  sub2->SetLineColor(kBlue); // kViolet-3
  sub2->SetLineWidth(2);
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
  hist->GetYaxis()->SetTitleOffset(hist->GetYaxis()->GetTitleOffset() * Config::TitleFF);
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
    TCanvas * canv = new TCanvas("canv","canv");
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

      TCanvas * normcanv = new TCanvas("normcanv","normcanv");
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
      (*mapiter).second->Fit(fit->GetName(),"RBQ0");

      TCanvas * fitcanv = new TCanvas("fitcanv","fitcanv");
      fitcanv->cd();
      (*mapiter).second->Draw("PE");
      fit->SetLineWidth(3);
      fit->Draw("same");
      
      // draw sub components of fit it applies
      TF1 * sub1; TF1 * sub2;
      Analysis::DrawSubComp(fit,fitcanv,sub1,sub2);
      (*mapiter).second->Draw("PE SAME"); // redraw to put points on top

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
  TCanvas * canv = new TCanvas("canv","canv");
  canv->cd();
  hist->Draw("PE");
  fit->SetLineWidth(3);
  fit->Draw("same");

  // draw subcomponents, too, if they apply
  TF1 * sub1; TF1 * sub2;
  Analysis::DrawSubComp(fit,canv,sub1,sub2);
  hist->Draw("PE SAME"); // redraw to get data points on top

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

  TCanvas * canv = new TCanvas("canv","canv");
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
  fInTree->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
  fInTree->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
  fInTree->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
  fInTree->SetBranchAddress("nvetoelectrons", &nvetoelectrons, &b_nvetoelectrons);
  fInTree->SetBranchAddress("nlooseelectrons", &nlooseelectrons, &b_nlooseelectrons);
  fInTree->SetBranchAddress("nmediumelectrons", &nmediumelectrons, &b_nmediumelectrons);
  fInTree->SetBranchAddress("ntightelectrons", &ntightelectrons, &b_ntightelectrons);
  fInTree->SetBranchAddress("nheepelectrons", &nheepelectrons, &b_nheepelectrons);
  fInTree->SetBranchAddress("el1pid", &el1pid, &b_el1pid);
  fInTree->SetBranchAddress("el1pt", &el1pt, &b_el1pt);
  fInTree->SetBranchAddress("el1eta", &el1eta, &b_el1eta);
  fInTree->SetBranchAddress("el1phi", &el1phi, &b_el1phi);
  fInTree->SetBranchAddress("el1E", &el1E, &b_el1E);
  fInTree->SetBranchAddress("el1p", &el1p, &b_el1p);
  fInTree->SetBranchAddress("el2pid", &el2pid, &b_el2pid);
  fInTree->SetBranchAddress("el2pt", &el2pt, &b_el2pt);
  fInTree->SetBranchAddress("el2eta", &el2eta, &b_el2eta);
  fInTree->SetBranchAddress("el2phi", &el2phi, &b_el2phi);
  fInTree->SetBranchAddress("el2E", &el2E, &b_el2E);
  fInTree->SetBranchAddress("el2p", &el2p, &b_el2p);
  fInTree->SetBranchAddress("el1scX", &el1scX, &b_el1scX);
  fInTree->SetBranchAddress("el1scY", &el1scY, &b_el1scY);
  fInTree->SetBranchAddress("el1scZ", &el1scZ, &b_el1scZ);
  fInTree->SetBranchAddress("el1scE", &el1scE, &b_el1scE);
  fInTree->SetBranchAddress("el2scX", &el2scX, &b_el2scX);
  fInTree->SetBranchAddress("el2scY", &el2scY, &b_el2scY);
  fInTree->SetBranchAddress("el2scZ", &el2scZ, &b_el2scZ);
  fInTree->SetBranchAddress("el2scE", &el2scE, &b_el2scE);
  fInTree->SetBranchAddress("el1rhXs", &el1rhXs, &b_el1rhXs);
  fInTree->SetBranchAddress("el1rhYs", &el1rhYs, &b_el1rhYs);
  fInTree->SetBranchAddress("el1rhZs", &el1rhZs, &b_el1rhZs);
  fInTree->SetBranchAddress("el1rhtimes", &el1rhtimes, &b_el1rhtimes);
  fInTree->SetBranchAddress("el2rhXs", &el2rhXs, &b_el2rhXs);
  fInTree->SetBranchAddress("el2rhYs", &el2rhYs, &b_el2rhYs);
  fInTree->SetBranchAddress("el2rhZs", &el2rhZs, &b_el2rhZs);
  fInTree->SetBranchAddress("el2rhtimes", &el2rhtimes, &b_el2rhtimes);
  fInTree->SetBranchAddress("el1seedX", &el1seedX, &b_el1seedX);
  fInTree->SetBranchAddress("el1seedY", &el1seedY, &b_el1seedY);
  fInTree->SetBranchAddress("el1seedZ", &el1seedZ, &b_el1seedZ);
  fInTree->SetBranchAddress("el1seedE", &el1seedE, &b_el1seedE);
  fInTree->SetBranchAddress("el1seedtime", &el1seedtime, &b_el1seedtime);
  fInTree->SetBranchAddress("el2seedX", &el2seedX, &b_el2seedX);
  fInTree->SetBranchAddress("el2seedY", &el2seedY, &b_el2seedY);
  fInTree->SetBranchAddress("el2seedZ", &el2seedZ, &b_el2seedZ);
  fInTree->SetBranchAddress("el2seedE", &el2seedE, &b_el2seedE);
  fInTree->SetBranchAddress("el2seedtime", &el2seedtime, &b_el2seedtime);
  fInTree->SetBranchAddress("el1nrh", &el1nrh, &b_el1nrh);
  fInTree->SetBranchAddress("el2nrh", &el2nrh, &b_el2nrh);
  fInTree->SetBranchAddress("zmass", &zmass, &b_zmass);
  fInTree->SetBranchAddress("zpt", &zpt, &b_zpt);
  fInTree->SetBranchAddress("zeta", &zeta, &b_zeta);
  fInTree->SetBranchAddress("zphi", &zphi, &b_zphi);
  fInTree->SetBranchAddress("zE", &zE, &b_zE);
  fInTree->SetBranchAddress("zp", &zp, &b_zp);
  
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
