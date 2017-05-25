#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>

#include "TChain.h"
#include "TString.h"
#include "TCut.h"
#include "TEntryList.h"
#include "TDirectory.h"
#include "TMath.h"

int    VERBOSE = 1; // full verbosity = 3
bool   TEST    = false; // to test with NTEST entries per process
int    NTEST   = 1000000;
double CONVERT = 1e-36;

///////////////////////
// Declare functions //
///////////////////////

Double_t GetXSec(TString process);
Int_t    DefineChains(std::vector<TString> &processes, std::vector<TChain*> &chains);
TCut     DefineCut(TString process, UInt_t minPU, UInt_t maxPU);
Int_t    DefineCuts(std::vector<TString> processes, std::vector<TCut> &cuts, UInt_t minPU, UInt_t maxPU);
Int_t    DefinePaths(std::vector<TString> &paths);

///////////////////
// Main function //
///////////////////

Int_t rateMC(TString minpu=54, TString maxpu=58, TString ps="2e34")
{
  UInt_t minPU = minpu.Atoi();
  UInt_t maxPU = maxpu.Atoi();
  Double_t lumi = ps.Atof();
  TString input="All";

  // Define the chains
  std::vector<TChain*> chains;
  std::vector<TString> processes;
  DefineChains(processes, chains);

  // Define the cuts
  std::vector<TCut> cuts;
  DefineCuts(processes, cuts, minPU, maxPU);

  // Define paths to study
  std::vector<TString> paths;
  DefinePaths(paths);

  // Check std::vector sizes
  const UInt_t nP    = paths    .size();
  const UInt_t nCh   = chains   .size();
  const UInt_t nProc = processes.size();
  const UInt_t nCuts = cuts     .size();
  if( nCh!=nProc || nCh!=nCuts || nProc!=nCuts ) {
    std::cout << "ERROR: sizes do not match!" << std::endl;
    return -1;
  }

  // Check if the chains get defined correctly
  std::cout << "- Defined " << nProc << " processes." << std::endl;
  if(VERBOSE>1) {
    for(UInt_t iP=0; iP<nProc; iP++) {
      std::cout << processes[iP]     << std::endl
	   << "   => Entries: " << chains[iP]->GetEntries() << std::endl
	   << "   => Cut: "     << cuts[iP] << std::endl;
    }
    std::cout << std::endl << std::endl;
  }

  // Print-out paths
  if(VERBOSE>0) {
    std::cout << "- Paths to be studied:" << std::endl;
    for(UInt_t iP=0; iP<paths.size(); iP++) {
      std::cout << "-- " << paths[iP] << std::endl;
    }
    std::cout << std::endl << std::endl;
  }

  // Initialize counters
  Long64_t den[nCh];
  Long64_t num[nCh][nP];
  for(UInt_t iCh=0; iCh<nCh; iCh++) {
    den[iCh]=0;
    for(UInt_t iP=0; iP<nP; iP++) {
      num[iCh][iP]=0;
    }
  }

  // Count entries
  TEntryList* skim;
  TString     tskim;
  TCut        thePath;
  //
  std::cout << "- Start looping over chains to count events" << std::endl;
  //
  for(UInt_t iCh=0; iCh<nCh; iCh++) {

    // look only at the requested process
    // unless "All" is specified for parameter "input"
    if(input!="All" && input!=processes[iCh]) continue; 

    std::cout << "-- process: " << processes[iCh] << std::endl;

    // Skim the chain with denominator events only
    tskim    = "skim_"+processes[iCh];
    //
    if(TEST) {
      den[iCh] = chains[iCh]->Draw(">>+"+tskim, cuts[iCh], "entrylist", NTEST); // test with NTEST entries
    }
    else {
      den[iCh] = chains[iCh]->Draw(">>+"+tskim, cuts[iCh], "entrylist");
    }
    //
    skim     = (TEntryList*)gDirectory->Get(tskim);
    //
    if(skim) { 
      chains[iCh]->SetEntryList(skim); 
      if(VERBOSE>2) std::cout << "-- skim size: " << skim->GetN() << " entries ; den[iCh]=" << den[iCh] << std::endl;
    }

    // Get one numerator per path
    for(UInt_t iP=0; iP<nP; iP++) {
      thePath = TCut(paths[iP]);
      num[iCh][iP] = chains[iCh]->GetEntries( thePath );
    }

  } // end loop over chains

  // Initialize rates
  Double_t totalRate[nP];
  Double_t totalErr[nP];
  Double_t rate[nCh][nP];
  Double_t rateErr[nCh][nP];
  Double_t fraction, xsec;
  //
  fraction = xsec = 0;
  for(UInt_t iP=0; iP<nP; iP++) {
    totalRate[iP]=0;
    totalErr[iP] =0;
    for(UInt_t iCh=0; iCh<nCh; iCh++) {
      rate   [iCh][iP]=0;
      rateErr[iCh][iP]=0;
    }
  }

  // Evaluate rates
  for(UInt_t iP=0; iP<nP; iP++) {
    for(UInt_t iCh=0; iCh<nCh; iCh++) {
      xsec          = GetXSec(processes[iCh]);
      fraction      = den[iCh]!=0 ? num[iCh][iP] / ((Double_t)den[iCh]) : 0;
      //
      rate   [iCh][iP] = xsec * CONVERT * lumi * fraction;
      //rateErr[iCh][iP] = den[iCh]!=0 ? (xsec * CONVERT * lumi * std::sqrt( num[iCh][iP] ) / ((Double_t)den[iCh])) : 0;
      rateErr[iCh][iP] = den[iCh]!=0 ? (xsec * CONVERT * lumi * 1.96 * std::sqrt( fraction * (1.f - fraction) / den[iCh] )) : 0;
      //
      totalRate[iP]+= rate[iCh][iP];
      totalErr[iP] += std::pow(rateErr[iCh][iP], 2);
    }
    totalErr[iP] = std::sqrt( totalErr[iP] );
  }

  // save rates in text file
  std::ofstream outtxt;
  outtxt.open(Form("rates_%s_%sto%s.txt",ps.Data(),minpu.Data(),maxpu.Data()),std::ios_base::trunc);

  // Print-out rates
  std::cout << std::endl << "=== RATES ===" << std::endl;
  for(UInt_t iP=0; iP<nP; iP++) {
    std::cout << "- " << paths[iP] << " : " << totalRate[iP] << " +/- " << totalErr[iP] << std::endl;
    outtxt << paths[iP].Data() << " " << totalRate[iP] << " " << totalErr[iP] << std::endl;
  }
  std::cout << "=============" << std::endl;

  outtxt.close();

  return 0;
}

// Define list of trigger paths to be studied //
Int_t DefinePaths(std::vector<TString> &paths)
{

  paths.push_back("HLT_PFHT350MinPFJet15_v1");
  paths.push_back("HLT_Photon60_R9Id90_CaloIdL_IsoL_v1");
  paths.push_back("HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_v1");
  paths.push_back("HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_PFHT350MinPFJet15_v1");

  return 0;
}

// Define TChains and add input root files //
Int_t DefineChains(std::vector<TString> &processes, std::vector<TChain*> &chains)
{

  if(VERBOSE>0) std::cout << "- Launch DefineChains()" << std::endl;

  // Define processes
  processes.push_back("QCDEM15");    
  processes.push_back("QCDEM20");    
  processes.push_back("QCDEM30");    
  processes.push_back("QCDEM50");    
  processes.push_back("QCDEM80");    
  processes.push_back("QCDEM120");   

  // List subdirectories for each process
  std::map< TString , std::vector<TString> > subdir;
  //
  subdir["QCDEM15"   ].push_back("pt15to20"); 
  subdir["QCDEM20"   ].push_back("pt20to30"); 
  subdir["QCDEM30"   ].push_back("pt30to50"); 
  subdir["QCDEM50"   ].push_back("pt50to80"); 
  subdir["QCDEM80"   ].push_back("pt80to120"); 
  subdir["QCDEM120"  ].push_back("pt120to170"); 

  // Make the chains
  TChain* chTemp;
  TString filePath;
  TString pathEOS;
  std::vector<TString> vTemp;
  //
  for(UInt_t iP=0; iP<processes.size(); iP++) { 

    vTemp  = subdir[processes[iP]];
    chTemp = new TChain("HltTree");

    if(VERBOSE>0) std::cout << "-- Filling chain for process: " << processes[iP] << std::endl;

    // Loop over subdirectories for process iP and add all root files
    for(UInt_t iS=0; iS<vTemp.size(); iS++) {

      filePath = vTemp[iS]+"/hltbits.root";

      chTemp->Add(filePath);
      if(VERBOSE>2) std::cout << "--- added: " << filePath << std::endl;
    }

    if(VERBOSE>2) std::cout << "-- entries: " << chTemp->GetEntries() << std::endl;

    chains.push_back(chTemp);
  }

  return 0;
}

// Get the cross section for each process //
Double_t GetXSec(TString process)
{

  // - generic case: returns xsec * McM efficiency
  // - particular case: for processes that have an extended dataset,
  //   add a third factor to reweight wrt #generated events

  // When processing NTEST events for all processes, 
  // the third factor is 0.5 as the same #events is used 
  // in both versions of the dataset
  if(TEST) {
    if     (process=="QCDEM15")    return 1279000000.0 *0.0018 *0.5;
    else if(process=="QCDEM30")    return 136000000.0  *0.073  *0.5;
  }

  if     (process=="QCDEM15")    return 1270000000.0 *0.001099;
  else if(process=="QCDEM20")    return 560700000.0  *0.01038;
  else if(process=="QCDEM30")    return 139200000.0  *0.05073;
  else if(process=="QCDEM50")    return 19300000.0   *0.1103;
  else if(process=="QCDEM80")    return 2805000.0    *0.1771;
  else if(process=="QCDEM120")   return 484600.0     *0.1521;

  else return -1;
}

// Define the cut to be used when counting denominator/numerator events //
TCut DefineCut(TString process, UInt_t minPU, UInt_t maxPU)
{

  // PU filter
  TCut cut_PU = "HLT_RemovePileUpDominatedEventsGen_v1";

  // GEN filter
  TCut cut_GEN = "";

  if(process.Contains("QCD")) {

    if(process.Contains("QCDMu")) {
      cut_GEN = "HLT_MuFilter_v1 && !(!HLT_BCToEFilter_v1 && HLT_EmFilter_v1)";
    }

    else if(process.Contains("QCDEM")) {
      cut_GEN = "!HLT_BCToEFilter_v1 && HLT_EmFilter_v1";
    }

    else if( !process.Contains("QCD170") && !process.Contains("QCD300") && !process.Contains("QCD470") ) {
      cut_GEN = "!HLT_MuFilter_v1 && !(!HLT_BCToEFilter_v1 && HLT_EmFilter_v1)";
    }

  }

  // PU range
  std::ostringstream oss_minPU("");
  oss_minPU << minPU;
  TString string_minPU = oss_minPU.str();
  //
  std::ostringstream oss_maxPU("");
  oss_maxPU << maxPU;
  TString string_maxPU = oss_maxPU.str();
  //
  TString string_RangePU = "NPUTrueBX0>"+string_minPU+" && NPUTrueBX0<"+string_maxPU;
  TCut cut_RangePU(string_RangePU);

  return cut_PU * cut_GEN * cut_RangePU;
}

// Launch DefineCut() for each process //
Int_t DefineCuts(std::vector<TString> processes, std::vector<TCut> &cuts, UInt_t minPU, UInt_t maxPU)
{

  TCut theCut;
  for(UInt_t iP=0; iP<processes.size(); iP++) {
    theCut = DefineCut(processes[iP], minPU, maxPU);
    cuts.push_back(theCut);
  }
  
  return 0;
}
