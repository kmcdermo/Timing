#include "../interface/CommonTypes.hh"
#include "../interface/Config.hh"
#include "../interface/CommonUtils.hh"
#include "../interface/PUReweight.hh"
#include "../interface/Analysis.hh"
#include "../interface/EACalculator.hh"
#include "../interface/PtCalculator.hh"
#include "../interface/StackDataMC.hh"
#include "../interface/StackGEDOOT.hh"

#include "TROOT.h"
#include "TSystem.h"
#include "TVirtualFitter.h"

void InitializeMain(std::ofstream & yields, TStyle *& tdrStyle) 
{
  // set TDR Style (need to force it!)
  tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // set the minimizer
  TVirtualFitter::SetDefaultFitter("Minuit2");

  // have to do this at least once!
  MakeOutDir(Config::outdir);

  // yields 
  yields.open(Form("%s/%i/yields.txt", Config::outdir.Data(), Config::year),std::ios_base::app);

  // set common vars in sample maps
  Config::mcSampleMap["gmsb"]  = true; // isMC
  Config::mcSampleMap["hvds"]  = true; // isMC
  Config::mcSampleMap["qcd"]   = true; // isMC
  Config::mcSampleMap["gjets"] = true; // isMC

  Config::mcColorMap["gmsb"]  = kBlue;
  Config::mcColorMap["hvds"]  = kRed;
  Config::mcColorMap["qcd"]   = kYellow;
  Config::mcColorMap["gjets"] = kOrange+10;

  Config::mcTitleMap["gmsb"]  = "GMSB";
  Config::mcTitleMap["hvds"]  = "HVDS";
  Config::mcTitleMap["qcd"]   = "QCD";
  Config::mcTitleMap["gjets"] = "#gamma + Jets";

  // set lists of samples
  Config::mcSampleVecMap["qcd/Pt-"] = {"15to20","20to30","30to50","50to80","80to120","120to170","170to300"};

  //////////////////////////////
  // Now set up global config //
  //////////////////////////////

  // Data
  if (Config::useDEG) Config::SampleMap["doubleeg"] = false; // !isMC
  if (Config::useSPH) Config::SampleMap["singleph"] = false; // !isMC

  // Signal MC
  if (Config::useGMSB) // --> eventually expand to grid points!
  {  
    const TString sample = "gmsb";
    Config::SampleMap[sample] = Config::mcSampleMap[sample];
    Config::ColorMap [sample] = Config::mcColorMap [sample];
    Config::TitleMap [sample] = Config::mcTitleMap [sample];
  }
  if (Config::useHVDS) // --> eventually expand to grid points!
  {  
    const TString sample = "hvds";
    Config::SampleMap[sample] = Config::mcSampleMap[sample];
    Config::ColorMap [sample] = Config::mcColorMap [sample];
    Config::TitleMap [sample] = Config::mcTitleMap [sample];
  }
  
  // Background MC
  if (Config::useQCDPt) 
  {
    const TString sample = "qcd";
    if (Config::doHadd) // Be Careful!!!!! Enable when first processing analysis
    {
      const TString label  = "/Pt-";
      for (const auto & samplebin : Config::mcSampleVecMap[sample+label])
      {
	Config::SampleMap[sample+label+samplebin] = Config::mcSampleMap[sample];
	Config::ColorMap [sample+label+samplebin] = Config::mcColorMap [sample];
	Config::TitleMap [sample+label+samplebin] = Config::mcTitleMap [sample];
      }
    }
    else // Be Careful!!!! Only do this after subsamples have gone through analysis processing + hadding
    {
      Config::SampleMap[sample] = Config::mcSampleMap[sample];
      Config::ColorMap [sample] = Config::mcColorMap [sample];
      Config::TitleMap [sample] = Config::mcTitleMap [sample];
    }
  }
  if (Config::useGJetsEM) 
  {
    const TString sample = "gjets";
    const TString label  = "-EM";
    Config::SampleMap[sample+label] = Config::mcSampleMap[sample];
    Config::ColorMap [sample+label] = Config::mcColorMap [sample];
    Config::TitleMap [sample+label] = Config::mcTitleMap [sample];
  }
  if (Config::useGJetsFlatPt) 
  {
    const TString sample = "gjets";
    const TString label  = "-flatpt";
    Config::SampleMap[sample+label] = Config::mcSampleMap[sample];
    Config::ColorMap [sample+label] = Config::mcColorMap [sample];
    Config::TitleMap [sample+label] = Config::mcTitleMap [sample];
  }
}

void DestroyMain(std::ofstream & yields, TStyle *& tdrStyle) 
{
  yields.close();
  delete tdrStyle;
}

void HaddSamples()
{
  // QCD Pt samples
  if (Config::useQCDPt) 
  {
    std::cout << "Hadding QCD Pt binned samples..." << std::endl;

    // Base strings
    const TString iodir  = Form("%s/%i/MC", Config::outdir.Data(), Config::year);
    const TString sample = "qcd";
    const TString label  = "/Pt-";
    
    // list of all files for shell
    TString listOfInputs;
    
    for (const auto & samplebin : Config::mcSampleVecMap[sample+label])
    {
      // get the files needed and append to string
      const TString subsample = sample+label+samplebin;
      const TString filename = Form("%s/%s/%s", iodir.Data(), subsample.Data(), Config::AnOutName.Data());
      
      listOfInputs += Form(" %s",filename.Data());
    }
    
    // name the output file
    const TString outname = Form("%s/%s/%s", iodir.Data(), sample.Data(), Config::AnOutName.Data());
    
    // do the Hadd
    gSystem->Exec(Form("hadd %s %s", outname.Data(), listOfInputs.Data()));

    // remove the old samples from the global maps
    for (const auto & samplebin : Config::mcSampleVecMap[sample+label])
    {
      const TString subsample = sample+label+samplebin;
      Config::SampleMap.erase(subsample);
      Config::ColorMap .erase(subsample);
      Config::TitleMap .erase(subsample);
    }

    // now add in full sample
    Config::SampleMap[sample] = Config::mcSampleMap[sample];
    Config::ColorMap [sample] = Config::mcColorMap [sample];
    Config::TitleMap [sample] = Config::mcTitleMap [sample];
  } // end of QCDPt
}

int main(int argc, const char* argv[]) 
{
  //////////////////////////////////
  // Read in command line options //
  //////////////////////////////////
  lStr_t mArgs; 
  for (int i = 1; i < argc; ++i) { mArgs.push_back(argv[i]); }
  lStr_i i  = mArgs.begin();
  while (i != mArgs.end()) 
  {
    lStr_i start = i;
    if (*i == "-h" || *i == "-help" || *i == "--help") 
    {
      printf(
        "Usage: %s [options]\n"
        "Options:\n"
	"  --outdir        <string>      name of ouput directory (def: %s)\n"
	"  --do-purw                     calculate pile-up weights (def: %s)\n"
	"  --do-analysis                 make analysis plots (def: %s)\n"
	"  --do-hadd                     hadd subsample plots after analysis (def: %s)\n"
	"  --do-EA                       calculate effective area for isolation (def: %s)\n"
	"  --do-Pt                       calculate pt scaling for isolation (def: %s)\n"
	"  --do-stacks                   stack data/MC plots (def: %s)\n"
	"  --do-phostacks                stack GED/OOT plots (def: %s)\n"
	"  --do-demo                     demo analysis (def: %s)\n"
	"  --use-DEG                     use doubleEG for data (def: %s)\n"
	"  --use-SPH                     use singlePh for data (def: %s)\n"
	"  --use-GMSB                    use GMSB with MC (def: %s)\n"
	"  --use-HVDS                    use HVDS with MC (def: %s)\n"
	"  --use-QCD-Pt                  use QCD Pt binned samples with MC (def: %s)\n"
	"  --use-GJets-HT                use Gamma+Jets HT binned samples with MC (def: %s)\n"
	"  --use-GJets-EM                use Gamma+Jets EM enriched samples with MC (def: %s)\n"
	"  --use-GJets-FlatPt            use Gamma+Jets Flat pt samples with MC (def: %s)\n"
	"  --split-pho                   split OOT and GED photon collections (def: %s)\n"
	"  --do-evstd                    make standard event validation plots (def: %s)\n"
	"  --do-phostd                   make standard photon validation plots (def: %s)\n"
	"  --use-pfIsoEA                 use effective areas for PF isolations (def: %s)\n"
	"  --use-detIsoEA                use effective areas for detector isolations (def: %s)\n"
	"  --use-pfIsoPt                 use pt corrections for PF isolations (def: %s)\n"
	"  --use-detIsoPt                use pt corrections for detector isolations (def: %s)\n"
	"  --do-iso                      make isolation plots (def: %s)\n"
	"  --do-isonvtx                  make isolation vs nvtx plots (def: %s)\n"
	"  --do-isopt                    make isolation vs pt plots (def: %s)\n"
	"  --use-mean-iso                use mean of projected histo for isolation value (def: %s)\n"
	"  --use-mean-rho                use mean of projected histo for rho (def: %s)\n"
	"  --use-mean-pt                 use mean of projected histo for iso vs pt (def: %s)\n"
	"  --q-probiso     <float>       which quantile to use for photons iso vs nvtx (def: %4.2f)\n"
	"  --q-probrho     <float>       which quantile to use for rho (def: %4.2f)\n"
	"  --q-probpt      <float>       which quantile to use for photons iso vs pt (def: %4.2f)\n"
	"  --in-year       <int>         which year to process (def: %i)\n"
	"  --save-hists                  save analysis histograms as images (def: %s)\n"
	"  --save-tmphists               save histograms used in projections to root file (def: %s)\n"
	"  --out-image     <string>      extension of file to save plots (def: %s)\n"
        ,
        argv[0],
        Config::outdir.Data(),
	PrintBool(Config::doPURW),
	PrintBool(Config::doAnalysis),
	PrintBool(Config::doHadd),
	PrintBool(Config::doEACalc),
	PrintBool(Config::doPtCalc),
	PrintBool(Config::doStacks),
	PrintBool(Config::doPhoStacks),
	PrintBool(Config::doDemo),
	PrintBool(Config::useDEG),
	PrintBool(Config::useSPH),
	PrintBool(Config::useGMSB),
	PrintBool(Config::useHVDS),
	PrintBool(Config::useQCDPt),
	PrintBool(Config::useGJetsHT),
	PrintBool(Config::useGJetsEM),
	PrintBool(Config::useGJetsFlatPt),
	PrintBool(Config::splitPho),
	PrintBool(Config::doEvStd),
	PrintBool(Config::doPhoStd),
	PrintBool(Config::pfIsoEA),
	PrintBool(Config::detIsoEA),
	PrintBool(Config::pfIsoPt),
	PrintBool(Config::detIsoPt),
	PrintBool(Config::doIso),
	PrintBool(Config::doIsoNvtx),
	PrintBool(Config::doIsoPt),
	PrintBool(Config::useMeanIso),
	PrintBool(Config::useMeanRho),
	PrintBool(Config::useMeanPt),
	Config::quantProbIso,
	Config::quantProbRho,
	Config::quantProbPt,
	Config::year,
	PrintBool(Config::saveHists),
	PrintBool(Config::saveTempHists),
	Config::outtype.Data()
      );
      exit(0);
    }
    else if (*i == "--outdir")      { next_arg_or_die(mArgs, i); Config::outdir = i->c_str(); }
    else if (*i == "--do-purw")     { Config::doPURW     = true; }
    else if (*i == "--do-analysis") { Config::doAnalysis = true; }
    else if (*i == "--do-hadd")     { Config::doHadd     = true; }
    else if (*i == "--do-EA")       { Config::doEACalc   = true;} 
    else if (*i == "--do-Pt")       { Config::doPtCalc   = true; }
    else if (*i == "--do-stacks")   { Config::doStacks   = true; }
    else if (*i == "--do-phostacks"){ Config::doPhoStacks = true; }
    else if (*i == "--do-demo")     { Config::doDemo     = true; Config::doAnalysis = true; Config::doEvStd = true; Config::doPhoStd = true; }
    else if (*i == "--use-DEG")     { Config::useDEG     = true; }
    else if (*i == "--use-SPH")     { Config::useSPH     = true; }
    else if (*i == "--use-GMSB")    { Config::useGMSB    = true; }
    else if (*i == "--use-HVDS")    { Config::useHVDS    = true; }
    else if (*i == "--use-QCD-Pt")  { Config::useQCDPt   = true; }
    else if (*i == "--use-GJets-HT"){ Config::useGJetsHT = true; }
    else if (*i == "--use-GJets-EM"){ Config::useGJetsEM = true; }
    else if (*i == "--use-GJets-FlatPt") { Config::useGJetsFlatPt = true; }
    else if (*i == "--split-pho")   { Config::splitPho   = true; Config::nPhotons   = Config::nTotalPhotons / 2; }
    else if (*i == "--do-evstd")    { Config::doAnalysis = true; Config::doEvStd    = true; }
    else if (*i == "--do-phostd")   { Config::doAnalysis = true; Config::doPhoStd   = true; }
    else if (*i == "--use-pfIsoEA") { Config::pfIsoEA    = true; }
    else if (*i == "--use-detIsoEA"){ Config::detIsoEA   = true; }
    else if (*i == "--use-pfIsoPt") { Config::pfIsoPt    = true; }
    else if (*i == "--use-detIsoPt"){ Config::detIsoPt   = true; }
    else if (*i == "--do-iso")      { Config::doAnalysis = true; Config::doIso      = true; }
    else if (*i == "--do-isonvtx")  { Config::doAnalysis = true; Config::doIsoNvtx  = true; }
    else if (*i == "--do-isopt")    { Config::doAnalysis = true; Config::doIsoPt    = true; }
    else if (*i == "--use-mean-iso"){ Config::doAnalysis = true; Config::doIsoNvtx  = true; Config::useMeanIso = true; }
    else if (*i == "--use-mean-rho"){ Config::doAnalysis = true; Config::doEvStd    = true; Config::useMeanRho = true; }
    else if (*i == "--use-mean-pt") { Config::doAnalysis = true; Config::doIsoPt    = true; Config::useMeanPt  = true; }
    else if (*i == "--q-probiso")   { next_arg_or_die(mArgs, i); Config::quantProbIso = std::atof(i->c_str()); }
    else if (*i == "--q-probrho")   { next_arg_or_die(mArgs, i); Config::quantProbRho = std::atof(i->c_str()); }
    else if (*i == "--q-probpt")    { next_arg_or_die(mArgs, i); Config::quantProbPt  = std::atof(i->c_str()); }
    else if (*i == "--in-year")     { next_arg_or_die(mArgs, i); Config::year       = std::atoi(i->c_str()); }
    else if (*i == "--save-hists")  { Config::saveHists  = true; }
    else if (*i == "--save-tmphists"){ Config::saveTempHists = true; }
    else if (*i == "--out-image")   { next_arg_or_die(mArgs, i); Config::outtype    = i->c_str(); }
    else    { std::cerr << "Error: Unknown option/argument: " << i->c_str() << " ...exiting..." << std::endl; exit(1); }
    mArgs.erase(start, ++i);
  }

  ////////////////////////////////////////
  // Checks for improper input settings //
  ////////////////////////////////////////
  if (Config::doAnalysis && Config::useQCDPt && !Config::doHadd) 
  {
    std::cerr << "Cannot do analysis on hadded QCD! Exiting..." << std::endl;
    exit(1);
  }

  ////////////////////
  // Initialization //
  ////////////////////

  // do this thing at least once!
  std::ofstream yields; TStyle * tdrStyle; 
  InitializeMain(yields,tdrStyle);

  /////////////////////////
  // Pile-up reweighting //
  /////////////////////////

  if (Config::doPURW) 
  {
    std::cout << "Calculating private pile-up weights" << std::endl;
    PUReweight reweight;
    reweight.GetPUWeights();
    std::cout << "Finished calculating pile-up weights" << std::endl;
  }
  else 
  {
    std::cout << "Skipping calculating pile-up weights" << std::endl;
  }
  std::cout << std::endl;

  ///////////////////
  // Main Analysis //
  ///////////////////

  if (Config::doAnalysis) 
  {
    std::cout << "Starting analysis section" << std::endl;
    for (const auto & samplePair : Config::SampleMap)
    {
      Analysis analysis(samplePair.first,samplePair.second);
      std::cout << "Analyzing: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
      analysis.EventLoop();
      std::cout << "Done analyzing: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
    }
    std::cout << "Finished analysis section" << std::endl;
  }
  else 
  {
    std::cout << "Skipping analysis section" << std::endl;
  }
  std::cout << std::endl;

  /////////////////////
  // Hadd SubSamples //
  /////////////////////

  if (Config::doHadd) 
  {
    std::cout << "Starting hadd-ing of subsamples" << std::endl;
    HaddSamples();
    std::cout << "Finished hadd-ing of subsamples" << std::endl;
  }
  else 
  {
    std::cout << "Skipping hadd-ing of subsamples" << std::endl;
  }
  std::cout << std::endl;

  /////////////////////////////
  // Compute Effective Areas //
  /////////////////////////////

  if (Config::doEACalc)
  {
    std::cout << "Starting EA calculation section" << std::endl;
    for (const auto & samplePair : Config::SampleMap)
    {
      EACalculator calc(samplePair.first,samplePair.second);
      std::cout << "Calculating EA: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
      calc.ExtractEA();
      std::cout << "Done calculating EA: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
    }
    std::cout << "Finished EA calculation section" << std::endl;
  }
  else 
  {
    std::cout << "Skipping EA calculation section" << std::endl;
  }
  std::cout << std::endl;

  /////////////////////////////
  // Compute Effective Areas //
  /////////////////////////////

  if (Config::doPtCalc)
  {
    std::cout << "Starting Iso Pt scaling calculation section" << std::endl;
    for (const auto & samplePair : Config::SampleMap)
    {
      PtCalculator calc(samplePair.first,samplePair.second);
      std::cout << "Calculating Pt scaling: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
      calc.ExtractPtScaling();
      std::cout << "Done calculating Pt scaling: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
    }
    std::cout << "Finished Iso Pt scaling calculation section" << std::endl;
  }
  else 
  {
    std::cout << "Skipping Iso Pt calculation section" << std::endl;
  }
  std::cout << std::endl;

  ///////////////////
  // Stack data/mc //
  ///////////////////

  if (Config::doStacks) 
  {
    std::cout << "Starting data/MC stacker" << std::endl;
    StackDataMC Stacker;
    Stacker.DoStacks(yields);
    std::cout << "Finished stacking data/MC plots" << std::endl;
  }
  else 
  {
    std::cout << "Skipping stacking data over MC" << std::endl;
  }
  std::cout << std::endl;

  ///////////////////
  // Stack GED/OOT //
  ///////////////////

  if (Config::doPhoStacks) 
  {
    std::cout << "Starting GED/OOT photon stacker section" << std::endl;
    for (const auto & samplePair : Config::SampleMap)
    {
      StackGEDOOT Stacker(samplePair.first,samplePair.second);
      std::cout << "Stacking: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
      Stacker.DoStacks();
      std::cout << "Done stacking: " << (samplePair.second?"MC":"DATA") << " sample: " << samplePair.first << std::endl;
    }
    std::cout << "Finished stacking GED/OOT photon plots" << std::endl;
  }
  else 
  {
    std::cout << "Skipping stacking GED/OOT photon plots" << std::endl;
  }
  std::cout << std::endl;

  // end of the line
  DestroyMain(yields,tdrStyle);
}
