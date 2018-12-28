// Class include
#include "VarWeighter.hh"

VarWeighter::VarWeighter(const TString & varwgtconfig)
  : fVarWgtConfig(varwgtconfig)
{
  std::cout << "Initializing VarWeighter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // init configuration
  VarWeighter::SetupVarWgtConfig();
  VarWeighter::SetupCommon();
}

VarWeighter::~VarWeighter()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete fConfigPave;

  for (auto & HistPair : HistMap) delete HistPair.second;

  delete fSkimFile;
  delete fSRFile;
  delete fCRFile;
}

void VarWeighter::MakeVarWeights()
{
  // setup hists for making ratio
  VarWeighter::GetInputHists();

  // scale input hists to unity 
  VarWeighter::ScaleInputToUnity();

  // make the MC SR/CR hist
  VarWeighter::MakeRatioHist();

  // make new weight branches from ratio hist
  VarWeighter::MakeWeightBranches();

  // dump meta info
  VarWeighter::MakeConfigPave();
}

void VarWeighter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  const TString mcname = Common::HistNameMap[fSample]+"_Plotted";

  // CR File
  fCRFile = TFile::Open(Form("%s",fCRFileName.Data()));
  Common::CheckValidFile(fCRFile,fCRFileName);

  HistMap["CR_MC"] = (TH1F*)fCRFile->Get(mcname.Data());
  Common::CheckValidHist(HistMap["CR_MC"],mcname,fCRFileName);

  // SR
  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);

  HistMap["SR_MC"] = (TH1F*)fSRFile->Get(mcname.Data());
  Common::CheckValidHist(HistMap["SR_MC"],mcname,fSRFileName);
}  

void VarWeighter::ScaleInputToUnity()
{
  std::cout << "Scaling input hists to unity..." << std::endl;

  for (auto & HistPair : HistMap)
  {
    auto & hist = HistPair.second;
    hist->Scale(1.f/hist->Integral(fXVarBins?"width":""));
  }
}

void VarWeighter::MakeRatioHist()
{
  std::cout << "Computing SR/CR histogram..." << std::endl;

  // Open the skim file as we want to save the hist there
  fSkimFile = TFile::Open(Form("%s",fSkimFileName.Data()),"UPDATE");
  Common::CheckValidFile(fSkimFile,fSkimFileName);
  fSkimFile->cd();
  
  // make the histogram SR/CR MC
  const TString histname = fSample+"_"+fVar+"_Ratio";
  
  HistMap["Ratio"] = (TH1F*)HistMap["SR_MC"]->Clone(Form("%s",histname.Data()));
  HistMap["Ratio"]->Divide(HistMap["CR_MC"]);

  // save it 
  fSkimFile->cd();
  HistMap["Ratio"]->Write(Form("%s",histname.Data()),TObject::kWriteDelete);
}

void VarWeighter::MakeWeightBranches()
{
  std::cout << "Making weight branches..." << std::endl;

  // MC in CR
  VarWeighter::MakeWeightBranch(Common::TreeNameMap[fSample]);

  // Data in CR
  VarWeighter::MakeWeightBranch(Common::TreeNameMap["Data"]);
}

void VarWeighter::MakeWeightBranch(const TString & treename)
{
  std::cout << "Working on adding weights for tree: " << treename.Data() << std::endl;

  // make const ref
  const auto & hist = HistMap["Ratio"];

  // get tree
  fSkimFile->cd();
  auto tree = (TTree*)fSkimFile->Get(Form("%s",treename.Data()));
  
  // get input branch and var: assumes float!!!!!
  Float_t var = 0.f;
  TBranch * b_var = 0;
  tree->SetBranchAddress(Form("%s",fVar.Data()),&var,&b_var);

  // set new branches: assumes float!!!!!
  Float_t varwgt = 0.f;
  const TString s_varwgt = fVar+"_wgt";
  TBranch * b_varwgt = tree->Branch(Form("%s",s_varwgt.Data()),&varwgt,Form("%s/F",s_varwgt.Data()));

  // loop over entries in tree
  const auto nEntries = tree->GetEntries();
  for (auto entry = 0U; entry < nEntries; entry++)
  {
    // only need the entry for the single branch
    b_var->GetEntry(entry);

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

void VarWeighter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fSkimFile->cd();
  fConfigPave = (TPaveText*)fSkimFile->Get(Form("%s",Common::pavename.Data()));

  // add some padding to new stuff
  Common::AddPaddingToPave(fConfigPave,3);

  // give grand title
  fConfigPave->AddText("***** VarWeighter Config *****");
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

void VarWeighter::SetupVarWgtConfig()
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

void VarWeighter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupHistNames();
  Common::SetupVarBinsBool("x_bins=",fPlotConfig,fXVarBins);
}
