#include "TreePlotter2D.hh"

TreePlotter2D::TreePlotter2D(const TString & cutconfig, const TString & plotconfig, const TString & pdname, const TString & outfilename) 
  : fCutConfig(cutconfig), fPlotConfig(plotconfig), fPDName(pdname), fOutFileName(outfilename)
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
  TreePlotter2D::SetupConfig();
  TreePlotter2D::SetupHists();

  // output root file
  fOutFile = TFile::Open(Form("%s",fOutFileName.Data()),"UPDATE");
}

TreePlotter2D::~TreePlotter2D()
{
  delete fConfigPave;
  delete RatioMCErrs;
  delete RatioHist;
  delete BkgdHist;
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
  TreePlotter2D::MakeConfigPave();
}

void TreePlotter2D::MakeHistFromTrees()
{
  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    std::cout << "Working on input: " << input.Data() << std::endl;

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
    tree->Draw(Form("%s:%s>>%s",fYVar.Data(),fXVar.Data(),hist->GetName()),Form("(%s) * (%s)",Config::CutMap[sample].Data(),Config::WeightString(input,sample).Data()),"goff");
    
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

  // rescale bins by widths if variable size
  if (fXVarBins || fYVarBins)
  {
    for (auto & HistPair : HistMap)
    {
      auto & hist = HistPair.second;
      const UInt_t nbinsX = hist->GetXaxis()->GetNbins();
      const UInt_t nbinsY = hist->GetYaxis()->GetNbins();
      for (UInt_t ibinX = 1; ibinX <= nbinsX; ibinX++)
      {
	const Float_t binwidthX = hist->GetXaxis()->GetBinWidth(ibinX);
	for (UInt_t ibinY = 1; ibinY <= nbinsY; ibinY++)
        {
	  const Float_t binwidthY = hist->GetYaxis()->GetBinWidth(ibinY);
	  
	  Float_t divisor = 1.f;
	  if (fXVarBins) divisor *= binwidthX;
	  if (fYVarBins) divisor *= binwidthY;

	  hist->SetBinContent(ibinX,ibinY,(hist->GetBinContent(ibinX,ibinY)/divisor));
	  hist->SetBinError  (ibinX,ibinY,(hist->GetBinError  (ibinX,ibinY)/divisor));
	}
      }
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

void TreePlotter2D::MakeBkgdOutput()
{
  std::cout << "Making Bkgd Output..." << std::endl;

  // Make Total Bkgd Hist: for ratio and error plotting
  BkgdHist = TreePlotter2D::SetupHist("Bkgd_Hist");
  for (const auto & HistPair : HistMap)
  {
    if (Config::GroupMap[HistPair.first] == isBkgd)
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
  RatioHist->Add(HistMap[Data]);
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

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName("Config");
  std::string str; // tmp string
  
  // dump cut config first
  fConfigPave->AddText("Cut Config");
  std::ifstream cutfile(Form("%s",fCutConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump plot config second
  fConfigPave->AddText("Plot Config");
  std::ifstream plotfile(Form("%s",fPlotConfig.Data()),std::ios::in);
  while (std::getline(plotfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump with PD
  fConfigPave->AddText(Form("Primary Dataset: %s",fPDName.Data()));

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TreePlotter2D::SetupConfig()
{
  Config::SetupPrimaryDataset(fPDName);
  Config::SetupSamples();
  Config::SetupHistNames();
  Config::SetupCuts(fCutConfig);
}

void TreePlotter2D::ReadPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
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
    else if (str.find("x_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"x_bins=");
      Config::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("x_labels=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"x_labels=");
      Config::SetupBinLabels(str,fXLabels);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Config::RemoveDelim(str,"y_title=");
    }
    else if (str.find("y_var=") != std::string::npos)
    {
      fYVar = Config::RemoveDelim(str,"y_var=");
    }
    else if (str.find("y_bins=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"y_bins=");
      Config::SetupBins(str,fYBins,fYVarBins);
    }
    else if (str.find("y_labels=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"y_labels=");
      Config::SetupBinLabels(str,fYLabels);
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
  TreePlotter2D::ReadPlotConfig();

  // instantiate each histogram
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    HistMap[HistNamePair.first] = SetupHist(HistNamePair.second);
  }
}

TH2F * TreePlotter2D::SetupHist(const TString & name)
{
  // get the bins in a struct for ROOT
  const Double_t * xbins = &fXBins[0];
  const Double_t * ybins = &fYBins[0];

  // initialize new histogram
  TH2F * hist = new TH2F(name.Data(),fTitle.Data(),fXBins.size()-1,xbins,fYBins.size()-1,ybins);

  // set axis titles
  hist->GetXaxis()->SetTitle(fXTitle.Data());
  hist->GetYaxis()->SetTitle(fYTitle.Data());

  // set x-axis labels only if read in
  if (fXLabels.size() > 0)
  {
    for (Int_t ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
    {
      hist->GetXaxis()->SetBinLabel(ibin,fXLabels[ibin-1].Data());
    }
  }

  // set y-axis labels only if read in
  if (fYLabels.size() > 0)
  {
    for (Int_t ibin = 1; ibin <= hist->GetYaxis()->GetNbins(); ibin++)
    {
      hist->GetYaxis()->SetBinLabel(ibin,fYLabels[ibin-1].Data());
    }
  }

  // still do not understand why ROOT does not do this by default...
  hist->Sumw2();
  
  return hist;
}
