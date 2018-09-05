// Class include
#include "TimeAdjuster.hh"

TimeAdjuster::TimeAdjuster(const TString & skimfilename, const TString & infilesconfig, const TString & outfiletext)
  : fSkimFileName(skimfilename), fInFilesConfig(infilesconfig), fOutFileText(outfiletext)
{
  std::cout << "Initializing TimeAdjuster..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // init configuration
  TimeAdjuster::SetupCommon();
  TimeAdjuster::SetupInFiles();

  // open skim file
  fSkimFile = TFile::Open(Form("%s",fSkimFileName.Data()),"UPDATE");
  Common::CheckValidFile(fSkimFile,fSkimFileName);
  fSkimFile->cd();
}

TimeAdjuster::~TimeAdjuster()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete fConfigPave;
  TimeAdjuster::DeleteMap(fInFileMap);
  delete fSkimFile;
}

void TimeAdjuster::AdjustTime();
{
  std::cout << "Adjusting time..." << std::endl;

  // prepare for time adjustments: data
  FitStruct DataInfo("Data");
  TimeAdjuster::PrepAdjustments(DataInfo);

  // prepare for time adjustments: mc
  FitStruct MCInfo("MC");
  TimeAdjuster::PrepAdjustments(MCInfo);

  // Correct data
  TimeAdjuster::CorrectData(DataInfo);

  // Correct MC
  TimeAdjuster::CorrectMC(DataInfo,MCInfo);

  // Save fits in proper input files
  TimeAdjuster::SaveSigmaFits(DataInfo,MCInfo);
  
  // dump meta info
  TimeAdjuster::MakeConfigPave();

  // delete info
  TimeAdjuster::DeleteInfo(DataInfo);
  TimeAdjuster::DeleteInfo(MCInfo);
}

void TimeAdjuster::DeleteInfo(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Deleting info for: " << label.Data() << std::endl;

  TimeAdjuster::DeleteMap(FitInfo.MuHistMap);
  TimeAdjuster::DeleteMap(FitInfo.SigmaHistMap);
  TimeAdjuster::DeleteMap(FitInfo.SigmaFitMap);
}

void TimeAdjuster::PrepAdjustments(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Preparing time adjustments for: " << label.Data() << std::endl;

  // Get the input hits first
  TimeAdjuster::GetInputHists(FitInfo);

  // Prep fits for sigma
  TimeAdjuster::PrepSigmaFits(FitInfo);
  
  // Fit sigma hists
  TimeAdjuster::FitSigmaHists(FitInfo);
}

void TimeAdjuster::CorrectData(FitStruct & DataInfo)
{
  std::cout << "Correcting data!" << std::endl;

  // variables for looping
  Event ev;
  std::vector<Photon> phos(Common::nPhotons);

  // get tree
  fSkimFile->cd();
  auto tree = (TTree*)fSkimFile->Get(Form("%s",Common::TreeNameMap["Data"].Data()));
  
  // set input branches
  tree->SetBranchAddress(Form("%s",ev.s_run.Data(),&ev.run,&ev.b_run);
  tree->SetBranchAddress(Form("%s",ev.s_nphotons.Data(),&ev.nphotons,&ev.b_nphotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    // get pho
    auto & pho = phos[ipho];

    // set old
    tree->SetBranchAddress(Form("%s_%i",pho.s_pt.Data(),ipho),&pho.pt,&pho.b_pt);
    tree->SetBranchAddress(Form("%s_%i",pho.s_isEB.Data(),ipho),&pho.isEB,&pho.b_isEB);

    // make new
    pho.b_seedtimeSHIFT = tree->SetBranchAddress(Form("%s_%i",pho.s_seedtimeSHIFT.Data(),ipho),&pho.seedtimeSHIFT,Form("%s/F",s_seedtimeSHIFT.Data()));
  }

  // loop over entries in tree
  const auto nEntries = tree->GetEntries();
  for (auto entry = 0U; entry < nEntries; entry++)
  {
    // only need the entry for the single branches
    ev.b_run->GetEntry(entry);
    ev.b_nphotons->GetEntry(entry);

    // get era
    const auto era = std::find_if(Common::EraMap.begin(),Common::EraMap.end(),
				  [=](const std::pair<TString,EraStruct> & EraPair)
				  {
				    return ( (EraPair.first != "Full") && ((run >= EraPair.second.startRun) && (run <= EraPair.second.endRun)) );
				  });
    
    // loop over nphotons
    const auto nphos = std::min(nphotons,Common::nPhotons);
    for (auto ipho = 0; ipho < nphos; ipho++)
    {
      auto & pho = phos[ipho];
      
      // get branch
      pho.b_pt->GetEntry(entry);
      pho.b_isEB->GetEntry(entry);

      // get the correction
      DataInfo.
    }

    // store blank info
    for (auto ipho = nphos; ipho < Common::nPhotons)

    // get weight from ratio hist
    varwgt = hist->GetBinContent(hist->FindBin(var));
  
    // fill single branch with weight
    b_varwgt->Fill();
  }

  // overwrite tree
  tree->Write(tree->GetName(),TObject::kWriteDelete);

  // delete it all
  delete tree;
}
}

void TimeAdjuster::CorrectMC(FitStruct & DataInfo, FitInfo & MCInfo)
{
  std::cout << "Correcting MC!" << std::endl;
}

void TimeAdjuster::SaveSigmaFits(FitStruct & DataInfo, FitStruct & MCInfo)
{
  std::cout << "Saving fits back into original files..." << std::endl;
}

void TimeAdjuster::GetInputHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Getting input histograms for: " << label.Data() << std::endl;

  // get inputs to be set
  auto & MuHistMap    = FitInfo.MuHistMap;
  auto & SigmaHistMap = FitInfo.SigmaHistMap;

  // tmp names for getting hists
  const TString muname    = label+"_mu";
  const TString sigmaname = label+"_sigma";

  // loop over files, read in hists, then rename
  for (auto & InFilePair : fInFileMap)
  {
    const auto & key = InFilePair.first;
    auto & file = InFilePair.second;

    auto & MuHist    = MuHistMap   [key];
    auto & SigmaHist = SigmaHistMap[key];

    // grab hists
    MuHist    = (TH1F*)file->Get(muname.Data());
    SigmaHist = (TH1F*)file->Get(sigmaname.Data());

    // check to make sure ok
    Common::CheckValidHist(MuHist   ,muname   ,file->GetName());
    Common::CheckValidHist(SigmaHist,sigmaname,file->GetName());

    // rename
    MuHist   ->SetName(Form("%s_%s_Hist",muname   ,key.Data()));
    SigmaHist->SetName(Form("%s_%s_Hist",sigmaname,key.Data()));
  }
}

void TimeAdjuster::PrepSigmaFits(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Prepping fits for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  const auto & SigmaHistMap = FitInfo.SigmaHistMap;
  const auto & SigmaFitMap  = FitInfo.SigmaFitMap;

  // loop over sigma hists
  for (const auto & SigmaHistPair : SigmaHistMap)
  {
    const auto & key  = SigmaHistPair.first;
    const auto & hist = SigmaHistPair.second;

    // get range
    const auto x_low = hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetFirst());
    const auto x_up  = hist->GetXaxis()->GetBinUpEdge (hist->GetXaxis()->GetLast());

    // set names
    const TString formname = label+"_"+key+"_form";
    const TString fitname  = label+"_"+key+"_fit";

    // set formula
    TFormula form(formname.Data(),Form("sqrt((([0]*[0])/(x*x))+(2*[1]*[1]))"));
    
    // get and set fit
    auto & fit = SigmaFitMap[key];
    fit = new TF1(fitname.Data(),formname.Data(),x_low,x_up);

    // init params
    fit->SetParameter(0,5000.f);
    fit->SetParLimits(0,0.f,20000.f);
    fit->SetParameter(1,150.f);
    fit->SetParLimits(1,0.f,500.f);
  }
}

void TimeAdjuster::FitSigmaHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Fitting sigma hists for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  const auto & SigmaHistMap = FitInfo.SigmaHistMap;
  const auto & SigmaFitMap  = FitInfo.SigmaFitMap;

  // loop over sigma hists and fit!
  for (const auto & SigmaHistPair : SigmaHistMap)
  {
    const auto & key = SigmaHistPair.first;
    auto & hist = SigmaHistPair.second;
    auto & fit  = SigmaFitMap[key];

    hist->Fit(fit->GetName(),"RBQ0");
  }
}


void TimeAdjuster::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fSkimFile->cd();
  fConfigPave = (TPaveText*)fSkimFile->Get(Form("%s",Common::pavename.Data()));

  // add some padding to new stuff
  Common::AddPaddingToPave(fConfigPave,3);

  // give grand title
  fConfigPave->AddText("***** TimeAdjuster Config *****");
  Common::AddTextFromInputConfig(fConfigPave,"VarWgt Config",fVarWgtConfig);

  // dump in old config
  fConfigPave->AddText("***** CR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fCRFile);

  fConfigPave->AddText("***** SR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fSRFile);

  // save to output file
  fSkimFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TimeAdjuster::SetupVarWgtConfig()
{
  std::cout << "Reading VarWgt config..." << std::endl;

  std::ifstream infile(Form("%s",fVarWgtConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("sample=") != std::string::npos)
    {
      fSample = Common::RemoveDelim(str,"sample=");
    }
    else if (str.find("var=") != std::string::npos)
    {
      fVar = Common::RemoveDelim(str,"var=");
    }
    else if (str.find("plot_config=") != std::string::npos)
    {
      fPlotConfig = Common::RemoveDelim(str,"plot_config=");
    }
    else if (str.find("cr_file=") != std::string::npos)
    {
      fCRFileName = Common::RemoveDelim(str,"cr_file=");
    }
    else if (str.find("sr_file=") != std::string::npos)
    {
      fSRFileName = Common::RemoveDelim(str,"sr_file=");
    }
    else if (str.find("skim_file=") != std::string::npos)
    {
      fSkimFileName = Common::RemoveDelim(str,"skim_file=");
    }
    else 
    {
      std::cerr << "Aye... your varwgt config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TimeAdjuster::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
}

void TimeAdjuster::SetupInFiles()
{
  std::cout << "Setting up in files..." << std::endl;

  // read in filenames
  std::ifstream infile(Form("%s",fInFilesConfig.Data()),std::ios::in);
  TString line = "";
  while (infile >> line)
  {
    if (line.EqualTo("",TString::kExact)) continue; 
    
    // dissect the file name
    const TString match = "=";
    auto middle = line.Index(match);
    
    const TString key(line(0,middle));
    const TString infilename(line(middle+1,line.Length()-middle-1));

    fInFileNameMap[key] = infilename;
  }

  // now open the files!
  for (const auto & InFileNamePair : fInFileNameMap)
  {
    const auto & key      = InFileNamePair.first;
    const auto & filename = InFileNamePair.second;

    fInFileMap[key] = TFile::Open(filename.Data());
    Common::CheckValidFile(fInFileMap[key],filename);
  }
}

template <typename T>
void TimeAdjuster::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
}
