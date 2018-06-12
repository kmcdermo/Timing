#include "TreePlotter2D.hh"

TreePlotter2D::TreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
			     const TString & plotconfig, const TString & miscconfig, const TString & outfiletext) 
  : fInFileName(infilename), fInSignalFileName(insignalfilename), fCutConfig(cutconfig), 
    fPlotConfig(plotconfig), fMiscConfig(miscconfig), fOutFileText(outfiletext)
{
  std::cout << "Initializing..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get main input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);

  // Get signal input file
  fInSignalFile = TFile::Open(Form("%s",fInSignalFileName.Data()));
  Common::CheckValidFile(fInSignalFile,fInSignalFileName);
 
  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // setup hists
  TreePlotter2D::SetupDefaults();
  TreePlotter2D::SetupConfig();
  TreePlotter2D::SetupPlotConfig();
  TreePlotter2D::SetupMiscConfig();
  TreePlotter2D::SetupHists();

  // output root file
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

TreePlotter2D::~TreePlotter2D()
{
  delete fConfigPave;
  delete RatioMCErrs;
  delete RatioHist;
  delete BkgdHist;
  delete DataHist;

  delete fOutFile;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete fTDRStyle;
  delete fInSignalFile;
  delete fInFile;
}

void TreePlotter2D::MakePlot()
{
  // Fill Hists from TTrees
  TreePlotter2D::MakeHistFromTrees();

  // Make Data Output
  TreePlotter2D::MakeDataOutput();

  // Make Bkgd Output
  TreePlotter2D::MakeBkgdOutput();

  // Make Ratio Output
  TreePlotter2D::MakeRatioOutput();

  // Write Out Config
  TreePlotter2D::MakeConfigPave();
}

void TreePlotter2D::MakeHistFromTrees()
{
  std::cout << "Making hists from input trees..." << std::endl;

  // loop over sample groups for each tree
  for (const auto & TreeNamePair : Common::TreeNameMap)
  {
    // Init
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;
    std::cout << "Working on tree: " << treename.Data() << std::endl;
	
    // Get infile
    auto & infile = ((Common::GroupMap[sample] != isSignal) ? fInFile : fInSignalFile);
    infile->cd();

    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
    const auto isnull = Common::IsNullTree(intree);

    if (!isnull)
    {
      std::cout << "Filling hist from tree..." << std::endl;

      // get the hist we wish to write to --> ROOT and memory residency is satanic
      auto & hist = HistMap[sample];
      hist->SetDirectory(infile);
      
      // Fill from tree
      intree->Draw(Form("%s:%s>>%s",fYVar.Data(),fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Common::CutMap[sample].Data(),Common::WeightString(sample).Data()),"goff");

      // delete tree;
      delete intree;
    }
    else
    {
      std::cout << "Skipping null tree..." << std::endl;
    }
  }

  // rescale bins by widths if variable size
  if (fXVarBins || fYVarBins)
  {
    const Bool_t isUp = false;
    for (auto & HistPair : HistMap)
    {
      auto & hist = HistPair.second;
      Common::Scale(hist,isUp,fXVarBins,fYVarBins);
    }
  }

  // save totals to output file
  fOutFile->cd();
  for (const auto & HistPair : HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}

void TreePlotter2D::MakeDataOutput()
{
  std::cout << "Making Data Output..." << std::endl;

  // Make new data hist in case we are blinded
  DataHist = TreePlotter2D::SetupHist("Data_Hist_Plotted");
  DataHist->Add(HistMap["Data"]);

  if (fBlindData)
  {
    for (const auto & Blind : fBlinds)
    {
      const auto binXlow = DataHist->GetXaxis()->FindBin(Blind.xlow);
      const auto binXup  = DataHist->GetXaxis()->FindBin(Blind.xup);
      const auto binYlow = DataHist->GetYaxis()->FindBin(Blind.ylow);
      const auto binYup  = DataHist->GetYaxis()->FindBin(Blind.yup);
      
      for (auto ibinX = binXlow; ibinX <= binXup; ibinX++) 
      {
	for (auto ibinY = binYlow; ibinY <= binYup; ibinY++) 
	{
	    DataHist->SetBinContent(ibinX,ibinY,0.f);
	    DataHist->SetBinError  (ibinX,ibinY,0.f);
	}
      }
    }
  }

  DataHist->Write(DataHist->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Make Total Bkgd Hist: for ratio and error plotting
  BkgdHist = TreePlotter2D::SetupHist("Bkgd_Hist");
  for (const auto & HistPair : HistMap)
  {
    if (Common::GroupMap[HistPair.first] == isBkgd)
    {
      BkgdHist->Add(HistPair.second);
    }
  }

  // save to output file
  fOutFile->cd();
  BkgdHist->Write(BkgdHist->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::MakeRatioOutput()
{
  std::cout << "Making Ratio Output..." << std::endl;

  // ratio value plot
  RatioHist = TreePlotter2D::SetupHist("Ratio_Hist");
  RatioHist->Add(DataHist);
  RatioHist->Divide(BkgdHist);  
  
  // ratio MC error plot
  RatioMCErrs = TreePlotter2D::SetupHist("Ratio_MCErrs");
  RatioMCErrs->Add(BkgdHist);
  RatioMCErrs->Divide(BkgdHist);

  // save to output file
  fOutFile->cd();
  RatioHist->Write(RatioHist->GetName(),TObject::kWriteDelete);
  RatioMCErrs->Write(RatioMCErrs->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** TreePlotter2D Config *****");

  // dump plot cut config first
  Common::AddTextFromInputConfig(fConfigPave,"TreePlotter2D Cut Config",fCutConfig);

  // dump plot config second
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // store last bits of info
  Common::AddTextFromInputConfig(fConfigPave,"Miscellaneous Config",fMiscConfig);

  // save name of infile, redundant
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save name of insignalfile, redundant
  fConfigPave->AddText(Form("InSignalFile name: %s",fInSignalFileName.Data()));

  // dump in old signal config
  Common::AddTextFromInputPave(fConfigPave,fInSignalFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::SetupDefaults()
{
  fXVarBins = false;
  fYVarBins = false;
  fBlindData = false;
}

void TreePlotter2D::SetupConfig()
{
  std::cout << "Setting up Config..." << std::endl;

  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupHistNames();
  Common::SetupCuts(fCutConfig);
}

void TreePlotter2D::SetupPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("plot_title=") != std::string::npos)
    {
      fTitle = Common::RemoveDelim(str,"plot_title=");
    }
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Common::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_var=") != std::string::npos)
    {
      fXVar = Common::RemoveDelim(str,"x_var=");
    }
    else if (str.find("x_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("x_labels=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_labels=");
      Common::SetupBinLabels(str,fXLabels);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Common::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_var=") != std::string::npos)
    {
      fYVar = Common::RemoveDelim(str,"y_var=");
    }
    else if (str.find("y_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_bins=");
      Common::SetupBins(str,fYBins,fYVarBins);
    }
    else if (str.find("y_labels=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_labels=");
      Common::SetupBinLabels(str,fYLabels);
    }
    else if (str.find("z_title=") != std::string::npos)
    {
      fZTitle = Common::RemoveDelim(str,"z_title=");
    }
    else if (str.find("blinding=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"blinding=");
      Common::SetupBlinding(str,fBlinds);
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TreePlotter2D::SetupMiscConfig()
{
  std::cout << "Reading miscellaneous plot config..." << std::endl;

  std::ifstream infile(Form("%s",fMiscConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("signal_to_model=") != std::string::npos) 
    {
      std::cout << "signal_to_model not implemented in 2D plotter, skipping..." << std::endl;
    }
    else if (str.find("signals_to_plot=") != std::string::npos) 
    {
      std::cout << "signals_to_plot not currently implemented in 2D plotter, skipping..." << std::endl;
    }
    else if (str.find("scale_mc_to_data=") != std::string::npos) 
    {
      std::cout << "scale_mc_to_data not currently implemented in 2D plotter, skipping..." << std::endl;
    }
    else if (str.find("scale_to_unity=") != std::string::npos)
    {
      std::cout << "scale_to_unity not currently implemented in 2D plotter, skipping..." << std::endl;
    }
    else if (str.find("blind_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"blind_data=");
      Common::SetupBool(str,fBlindData);
    }
    else if (str.find("signals_only=") != std::string::npos)
    {
      std::cout << "signals_only not currently implemented in 2D plotter, skipping..." << std::endl;
    }
    else 
    {
      std::cerr << "Aye... your miscellaneous plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TreePlotter2D::SetupHists()
{
  std::cout << "Setting up output histograms..." << std::endl;

  // instantiate each histogram
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    HistMap[HistNamePair.first] = TreePlotter2D::SetupHist(HistNamePair.second);
  }
}

TH2F * TreePlotter2D::SetupHist(const TString & name)
{
  // get the bins in a struct for ROOT
  const auto xbins = &fXBins[0];
  const auto ybins = &fYBins[0];

  // initialize new histogram
  auto hist = new TH2F(name.Data(),fTitle.Data(),fXBins.size()-1,xbins,fYBins.size()-1,ybins);

  // set axis titles
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());
  hist->GetZaxis()->SetTitle(fZTitle.Data());

  // set x-axis labels only if read in
  if (fXLabels.size() > 0)
  {
    for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
    {
      hist->GetXaxis()->SetBinLabel(ibin,fXLabels[ibin-1].Data());
    }
  }

  // set y-axis labels only if read in
  if (fYLabels.size() > 0)
  {
    for (auto ibin = 1; ibin <= hist->GetYaxis()->GetNbins(); ibin++)
    {
      hist->GetYaxis()->SetBinLabel(ibin,fYLabels[ibin-1].Data());
    }
  }

  // still do not understand why ROOT does not do this by default...
  hist->Sumw2();
  
  return hist;
}
