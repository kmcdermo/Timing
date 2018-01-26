#include "TreePlotter2D.hh"

TreePlotter2D::TreePlotter2D(const TString & cutconfig, const TString & plotconfig, const TString & outfilename) 
  : fCutConfig(cutconfig), fPlotConfig(plotconfig), fOutFileName(outfilename)
{
  std::cout << "Initializing..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Config::SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  // setup hists
  TreePlotter2D::SetupDump();
  TreePlotter2D::InitConfig();
  TreePlotter2D::ReadPlotConfig();
  TreePlotter2D::SetupHists();

  // output root file
  fOutFile = TFile::Open(Form("%s",fOutFileName.Data()),"UPDATE");
}

TreePlotter2D::~TreePlotter2D()
{
  delete RatioMCErrs;
  delete RatioHist;
  delete BkgdHist;
  delete fConfigPave;
  delete fOutFile;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete fTDRStyle;
}

void TreePlotter2D::MakePlot()
{
  // Fill Hists from TTrees
  TreePlotter2D::MakeHistFromTrees();

  // Make Bkgd Output
  TreePlotter2D::MakeBkgdOutput();

  // Make Ratio Output
  TreePlotter2D::MakeRatioOutput();

  // Write Out Config
  TreePlotter2D::WriteDump();
}

void TreePlotter2D::MakeHistFromTrees()
{
  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    const Bool_t isMC = (sample != Data);
    std::cout << "Working on " << (isMC?"MC":"DATA") << " sample: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s",Config::eosDir.Data(),Config::baseDir.Data(),input.Data(),Config::tupleFileName.Data());
    TFile * file = TFile::Open(Form("%s",filename.Data()));
    Config::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    TTree * tree = (TTree*)file->Get(Form("%s",Config::disphotreename.Data()));
    Config::CheckValidTree(tree,Config::disphotreename,filename);

    // Make temp hist
    TString histname = Config::ReplaceSlashWithUnderscore(input);
    TH2F * hist = TreePlotter2D::SetupHist(Form("%s_Hist",histname.Data()));
    
    // Fill from tree
    tree->Draw(Form("%s:%s>>%s",fYVar.Data(),fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(isMC).Data()),"goff");
    
    // Add to main hists
    HistMap[sample]->Add(hist);

    // save to output file
    fOutFile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);

    // delete everything
    delete hist;
    delete tree;
    delete file;
  }

  // save totals to output file
  fOutFile->cd();
  for (const auto & HistPair : HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}

void TreePlotter2D::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Make Total Bkgd Hist: for ratio and error plotting
  BkgdHist = TreePlotter2D::SetupHist("Bkgd_Hist");
  BkgdHist->Add(HistMap[GJets]);
  BkgdHist->Add(HistMap[QCD]);
  BkgdHist->SetMarkerSize(0);
  BkgdHist->SetFillStyle(3254);
  BkgdHist->SetFillColor(kGray+3);

  // save to output file
  fOutFile->cd();
  BkgdHist->Write(BkgdHist->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::MakeRatioOutput()
{
  std::cout << "Making Ratio Output..." << std::endl;

  // ratio value plot
  RatioHist = TreePlotter2D::SetupHist("Ratio_Hist");
  RatioHist->Add(HistMap[Data]);
  RatioHist->Divide(BkgdHist);  
  RatioHist->SetStats(0);      // No statistics on lower plot
  
  // ratio MC error plot
  RatioMCErrs = TreePlotter2D::SetupHist("Ratio_MCErrs");
  RatioMCErrs->Add(BkgdHist);
  RatioMCErrs->Divide(BkgdHist);

  // save to output file
  fOutFile->cd();
  RatioHist->Write(RatioHist->GetName(),TObject::kWriteDelete);
  RatioMCErrs->Write(RatioMCErrs->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::WriteDump()
{
  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::SetupDump()
{
  fConfigPave = new TPaveText();
  fConfigPave->SetName("Config");
}

void TreePlotter2D::InitConfig()
{
  Config::SetupSamples();
  Config::SetupHistNames();
  
  TreePlotter2D::ReadCutConfig();
  Config::SetupCuts();
}

void TreePlotter2D::ReadCutConfig()
{
  std::cout << "Reading cut config..." << std::endl;
  fConfigPave->AddText("Cut Config");

  std::ifstream infile(Form("%s",fCutConfig.Data()),std::ios::in);
  std::string tmp;
  while (std::getline(infile,tmp))
  {
    fConfigPave->AddText(tmp.c_str());

    if (str == "") continue;
    else if (str.find("common_cut=") != std::string::npos)
    {
      fCommonCut = Config::RemoveDelim(str,"common_cut=");
    }
    else if (str.find("bkgd_cut=") != std::string::npos)
    {
      fBkgdCut = Config::RemoveDelim(str,"bkgd_cut=");
    }
    else if (str.find("sign_cut=") != std::string::npos)
    {
      fSignCut = Config::RemoveDelim(str,"sign_cut=");
    }
    else if (str.find("data_cut=") != std::string::npos)
    {
      fDataCut = Config::RemoveDelim(str,"data_cut=");
    }
    else 
    {
      std::cerr << "Aye... your cut config is messed up, try again!" << std::endl;
      exit(1);
    }
  }
}

void TreePlotter2D::ReadPLotConfig()
{
  std::cout << "Reading plot config..." << std::endl;
  fConfigPave->AddText("Plot Config");

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string tmp;
  while (std::getline(infile,tmp))
  {
    fConfigPave->AddText(tmp.c_str());

    if (str == "") continue;
    else if (str.find("plot_title=") != std::string::npos)
    {
      fTitle = Config::RemoveDelim(str,"plot_title=");
    }
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Config::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_var=") != std::string::npos)
    {
      fXVar = Config::RemoveDelim(str,"x_var=");
    }
    else if (str.Contains("x_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"x_bins=");
      std::stringstream ss(str);
      Float_t bin_edge;
      while (ss >> bin_edge) fXBins.push_back(bin_edge);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Config::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_var=") != std::string::npos)
    {
      fYVar = Config::RemoveDelim(str,"y_var=");
    }
    else if (str.Contains("y_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"y_bins=");
      std::stringstream ss(str);
      Float_t bin_edge;
      while (ss >> bin_edge) fYBins.push_back(bin_edge);
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      exit(1);
    }
  }
}

void TreePlotter2D::SetupHists()
{
  HistMap[QCD]   = SetupHist(Config::HistNameMap[QCD]);
  HistMap[GJets] = SetupHist(Config::HistNameMap[GJets]);
  HistMap[GMSB]  = SetupHist(Config::HistNameMap[GMSB]);
  HistMap[Data]  = SetupHist(Config::HistNameMap[Data]);
}

TH2F * TreePlotter2D::SetupHist(const TString & name)
{
  const Double_t * xbins = &fXBins[0];
  const Double_t * ybins = &fYBins[0];

  TH2F * hist = new TH2F(name.Data(),fTitle.Data(),fXBins.size()-1,xbins,fYBins.size()-1,ybins);
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());
  hist->Sumw2();
  
  return hist;
}
