#include "TreePlotter2D.hh"

TreePlotter2D::TreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
			     const TString & plotconfig, const TString & miscconfig, const TString & era,
			     const Bool_t savemetadata, const TString & outfiletext) 
  : fInFileName(infilename), fInSignalFileName(insignalfilename), fCutConfig(cutconfig),
    fPlotConfig(plotconfig), fMiscConfig(miscconfig), fEra(era), fOutFileText(outfiletext)
{
  std::cout << "Initializing..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // setup hists
  TreePlotter2D::SetupDefaults();
  TreePlotter2D::SetupSaveMetaData(savemetadata);
  TreePlotter2D::SetupCommon();
  TreePlotter2D::SetupPlotConfig(fPlotConfig);
  TreePlotter2D::SetupMiscConfig(fMiscConfig);
  if (fSkipData)   Common::RemoveGroup(SampleGroup::isData);
  if (fSkipBkgdMC) Common::RemoveGroup(SampleGroup::isBkgd);
  TreePlotter2D::SetupHists();

  // Get main input file
  if (!fSkipData || !fSkipBkgdMC)
  {
    fInFile = TFile::Open(Form("%s",fInFileName.Data()));
    Common::CheckValidFile(fInFile,fInFileName);
  }

  // Get signal input file
  fInSignalFile = TFile::Open(Form("%s",fInSignalFileName.Data()));
  Common::CheckValidFile(fInSignalFile,fInSignalFileName);
 
  // output root file
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

void TreePlotter2D::MakeTreePlot2D()
{
  // Fill Hists from TTrees
  TreePlotter2D::MakeHistFromTrees(fInFile,fInSignalFile);

  // Make Data Output
  if (!fSkipData) TreePlotter2D::MakeDataOutput();

  if (!fSkipBkgdMC)
  {
    // Make Bkgd Output
    TreePlotter2D::MakeBkgdOutput();
    
    // Make Ratio Output
    if (!fSkipData) TreePlotter2D::MakeRatioOutput();
  }

  // Write Out Config
  if (fSaveMetaData) TreePlotter2D::MakeConfigPave();

  // Delete Memory
  TreePlotter2D::DeleteMemory(true);
}

void TreePlotter2D::MakeHistFromTrees(TFile *& inFile, TFile *& inSignalFile)
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
    auto & infile = ((Common::GroupMap[sample] != SampleGroup::isSignal) ? inFile : inSignalFile);
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
      intree->Draw(Form("%s:%s>>%s",Common::YVarMap[sample].Data(),Common::XVarMap[sample].Data(),hist->GetName()),Form("%s",Common::CutWgtMap[sample].Data()),"goff");

      // delete tree;
      delete intree;
    }
    else
    {
      std::cout << "Skipping null tree..." << std::endl;
    }
  }

  // rescale bins by widths if variable size
  if (fScaleToBinWidths && (fXVarBins || fYVarBins))
  {
    const auto isUp = false;
    for (auto & HistPair : HistMap)
    {
      auto & hist = HistPair.second;
      Common::Scale(hist,isUp,fXVarBins,fYVarBins);
    }
  }

  // save totals to output file
  Common::WriteMap(fOutFile,HistMap);
}

void TreePlotter2D::MakeDataOutput()
{
  std::cout << "Making Data Output..." << std::endl;

  // Make new data hist in case we are blinded
  DataHist = TreePlotter2D::SetupHist(Form("%s_Plotted",Common::HistNameMap["Data"].Data()));
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

  Common::Write(fOutFile,DataHist);
}

void TreePlotter2D::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Extra Hists
  BkgdHist = TreePlotter2D::SetupHist(Form("%s",Common::BkgdHistName.Data())); // Make Total Bkgd Hist: for ratio and error plotting
  EWKHist  = TreePlotter2D::SetupHist(Form("%s",Common::EWKHistName.Data())); // Make EWK Histogram for posterity : do not plot it though

  // Loop over histograms, adding up bkgd as needed
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist = HistPair.second;

    if (Common::GroupMap[sample] == SampleGroup::isBkgd)
    {
      // add all MC bkgds for bkgd hist 
      BkgdHist->Add(hist);
      
      // ewk hist excludes GJets and QCD MC
      if (!Common::IsCR(sample))
      {
	EWKHist->Add(hist);
      }
    }
  }

  // save to output file
  Common::Write(fOutFile,BkgdHist);
  Common::Write(fOutFile,EWKHist);
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
  Common::Write(fOutFile,RatioHist);
  Common::Write(fOutFile,RatioMCErrs);
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

  // dump plot config
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // store last bits of info
  Common::AddTextFromInputConfig(fConfigPave,"Miscellaneous Config",fMiscConfig);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // save name of infile, redundant
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save name of insignalfile, redundant
  fConfigPave->AddText(Form("InSignalFile name: %s",fInSignalFileName.Data()));

  // dump in old signal config
  Common::AddTextFromInputPave(fConfigPave,fInSignalFile);

  // save to output file
  Common::Write(fOutFile,fConfigPave);
}

void TreePlotter2D::DeleteMemory(const Bool_t deleteInternal)
{
  if (fSaveMetaData) delete fConfigPave;

  if (!fSkipBkgdMC)
  {
    delete RatioMCErrs;
    delete RatioHist;
    delete EWKHist;
    delete BkgdHist;
  }
  if (!fSkipData) delete DataHist; 

  Common::DeleteMap(HistMap);

  delete fOutFile;

  if (deleteInternal)
  {
    delete fInSignalFile;
    if (!fSkipData || !fSkipBkgdMC) delete fInFile;
  }

  delete fTDRStyle;
}

void TreePlotter2D::SetupDefaults()
{
  fScaleToBinWidths = true;
  fXVarBins = false;
  fYVarBins = false;
  fBlindData = false;
  fSkipData = false;
  fSkipBkgdMC = false;
  fSaveMetaData = false;
}

void TreePlotter2D::SetupSaveMetaData(const Bool_t savemetadata)
{
  fSaveMetaData = savemetadata;
}

void TreePlotter2D::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupHistNames();
  Common::SetupCuts(fCutConfig);
  Common::SetupEraCuts(fEra);
  Common::SetupWeights();
  Common::SetupEraWeights(fEra);
}

void TreePlotter2D::SetupPlotConfig(const TString & plotconfig)
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",plotconfig.Data()),std::ios::in);
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
      str = Common::RemoveDelim(str,"x_var=");
      Common::SetVar(str,Variable::X);
    }    
    else if (str.find("x_var_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var_data=");
      Common::SetVarMod(str,Variable::X,SampleGroup::isData);
    }
    else if (str.find("x_var_bkgd=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var_bkgd=");
      Common::SetVarMod(str,Variable::X,SampleGroup::isBkgd);
    }
    else if (str.find("x_var_sign=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_var_sign=");
      Common::SetVarMod(str,Variable::X,SampleGroup::isSignal);
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
      str = Common::RemoveDelim(str,"y_var=");
      Common::SetVar(str,Variable::Y);
    }    
    else if (str.find("y_var_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_var_data=");
      Common::SetVarMod(str,Variable::Y,SampleGroup::isData);
    }
    else if (str.find("y_var_bkgd=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_var_bkgd=");
      Common::SetVarMod(str,Variable::Y,SampleGroup::isBkgd);
    }
    else if (str.find("y_var_sign=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_var_sign=");
      Common::SetVarMod(str,Variable::Y,SampleGroup::isSignal);
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
      Common::SetupBlockRanges(str,fBlinds);
    }
    else 
    {
      std::cerr << "Aye... your plot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TreePlotter2D::SetupMiscConfig(const TString & miscconfig)
{
  std::cout << "Reading miscellaneous plot config..." << std::endl;

  std::ifstream infile(Form("%s",miscconfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("scale_to_bin_widths=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"scale_to_bin_widths");
      Common::SetupBool(str,fScaleToBinWidths);
    }    
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
    else if (str.find("skip_bkgd_mc=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"skip_bkgd_mc=");
      Common::SetupBool(str,fSkipBkgdMC);
    }  
    else if (str.find("skip_data=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"skip_data=");
      Common::SetupBool(str,fSkipData);
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
