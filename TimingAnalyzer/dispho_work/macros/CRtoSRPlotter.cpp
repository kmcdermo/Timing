// Class include
#include "CRtoSRPlotter.hh"

CRtoSRPlotter::CRtoSRPlotter(const TString & crtosrconfig, const TString & outfiletext)
  : fCRtoSRConfig(crtosrconfig), fOutFileText(outfiletext)
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

  // init configuration
  CRtoSRPlotter::SetupConfig();
  CRtoSRPlotter::SetupCRtoSRConfig();
  CRtoSRPlotter::SetupPlotConfig();
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"RECREATE");
}

CRtoSRPlotter::~CRtoSRPlotter()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete fConfigPave;

  delete Legend;
  delete OutCanv;

  for (auto & HistPair : HistMap) delete HistPair.second;

  delete fSRFile;
  delete fCRFile;;

  delete fOutFile;
  delete fTDRStyle;
}

void CRtoSRPlotter::MakeCRtoSRPlot()
{
  CRtoSRPlotter::GetInputHists();
  CRtoSRPlotter::SetupHistsStlye();
  CRtoSRPlotter::MakeMCErrs();
  CRtoSRPlotter::MakeLegend();
  CRtoSRPlotter::MakeOutCanv();

  CRtoSRPlotter::DrawOutCanv(false);
  CRtoSRPlotter::SaveOutput(false);

  CRtoSRPlotter::ScaleToUnity();
  CRtoSRPlotter::DrawOutCanv(true);
  CRtoSRPlotter::SaveOutput(true);

  CRtoSRPlotter::MakeConfigPave();
}

void CRtoSRPlotter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  // CR File
  fCRFile = TFile::Open(Form("%s",fCRFileName.Data()));
  Common::CheckValidFile(fCRFile,fCRFileName);

  HistMap["CR_Data"] = (TH1F*)fCRFile->Get(Form("%s",Common::HistNameMap["Data"].Data()));
  HistMap["CR_MC"]   = (TH1F*)fCRFile->Get(Form("%s",Common::HistNameMap[Form("%s",fSample.Data())].Data()));
  Common::CheckValidTH1F(HistMap["CR_Data"],Common::HistNameMap["Data"],fCRFileName);
  Common::CheckValidTH1F(HistMap["CR_MC"]  ,Common::HistNameMap[Form("%s",fSample.Data())],fCRFileName);

  // SR
  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);

  HistMap["SR_MC"] = (TH1F*)fSRFile->Get(Form("%s",Common::HistNameMap[Form("%s",fSample.Data())].Data()));
  Common::CheckValidTH1F(HistMap["SR_MC"],Common::HistNameMap[Form("%s",fSample.Data())],fSRFileName);

  // save to outfile
  fOutFile->cd();
  for (auto & HistPair : HistMap)
  {
    const auto & key  = HistPair.first;
    auto & hist = HistPair.second;
    hist->SetName(Form("%s",key.Data()));
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}  

void CRtoSRPlotter::SetupHistsStlye()
{
  std::cout << "Setting up hists style..." << std::endl;

  HistMap["CR_Data"]->SetLineColor(kBlack);

  for (auto & HistPair : HistMap)
  {
    const auto & key = HistPair.first;
    auto & hist = HistPair.second;

    if (key.Contains("Data",TString::kExact)) continue;

    const auto color = (key.Contains("CR",TString::kExact) ? kOrange+10 : kAzure);

    hist->SetLineColorAlpha(color,0.5);
    hist->SetFillColorAlpha(color,0.5);
    hist->SetMarkerColorAlpha(color,0.5);
    hist->SetFillStyle(1001);
  }
}

void CRtoSRPlotter::MakeMCErrs()
{
  std::cout << "Making MC Errs..." << std::endl;

  CRtoSRPlotter::MakeMCErr("CR_MC");
  CRtoSRPlotter::MakeMCErr("SR_MC");
}

void CRtoSRPlotter::MakeMCErr(const TString & key)
{
  std::cout << "Making MC Err for: " << key.Data() << "..." << std::endl;

  const auto & hist    = HistMap[key];
  const auto & key_err = key+"_Err";
    
  HistMap[key_err] = (TH1F*)hist->Clone(Form("%s_Err",hist->GetName()));
  auto & hist_err  = HistMap[key_err];

  hist_err->SetLineColorAlpha(0,1);
  hist_err->SetMarkerSize(0);
  hist_err->SetFillStyle(key.Contains("CR",TString::kExact) ? 3254 : 3245);
  hist_err->SetFillColor(hist->GetFillColor());
  
  fOutFile->cd();
  hist_err->Write(hist_err->GetName(),TObject::kWriteDelete);
}

void CRtoSRPlotter::MakeLegend()
{
  std::cout << "Creating Legend..." << std::endl;

  // instantiate the legend
  Legend = new TLegend(0.62,0.75,0.825,0.92);
  Legend->SetName("Legend");
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);
  
  // add Data
  Legend->AddEntry(HistMap["CR_Data"],Form("%s [CR Data]",fSample.Data()),"epl");

  // add MC
  Legend->AddEntry(HistMap["CR_MC"],Form("%s [CR MC]",fSample.Data()),"f");
  Legend->AddEntry(HistMap["SR_MC"],Form("%s [SR MC]",fSample.Data()),"f");

  // save to output file
  fOutFile->cd();
  Legend->Write(Legend->GetName(),TObject::kWriteDelete);
}

void CRtoSRPlotter::MakeOutCanv()
{
  OutCanv = new TCanvas("OutCanv","");
  OutCanv->cd();
}

void CRtoSRPlotter::ScaleToUnity()
{
  std::cout << "Scaling to unity..." << std::endl;

  for (auto & HistPair : HistMap)
  {
    auto & hist = HistPair.second;
    hist->Scale(1.f/hist->Integral(fXVarBins?"width":""));
  }
}

void CRtoSRPlotter::DrawOutCanv(const Bool_t isScaled)
{
  std::cout << "Drawing isScaled: " << Common::PrintBool(isScaled) << " ..." << std::endl;

   // Get and Set Global Minimum and Maximum
  CRtoSRPlotter::GetHistMinimum();
  CRtoSRPlotter::GetHistMaximum();

  OutCanv->cd();

  HistMap["CR_Data"]->SetMinimum(fMinY);
  HistMap["CR_Data"]->SetMaximum(fMaxY);
  HistMap["CR_Data"]->Draw("PE");

  // first MC, then errs
  for (const auto & HistPair : HistMap)
  {
    const auto & key  = HistPair.first;
    const auto & hist = HistPair.second;

    if (key.Contains("Data",TString::kExact)) continue;
    if (key.Contains("Err" ,TString::kExact)) continue;
    
    hist->Draw("HIST SAME");
  }

  // draw errs
  for (const auto & HistPair : HistMap)
  {
    const auto & key  = HistPair.first;
    const auto & hist = HistPair.second;

    if ( key.Contains("Data",TString::kExact)) continue;
    if (!key.Contains("Err" ,TString::kExact)) continue;
    
    hist->Draw("E2 SAME");
  }

  // And lastly draw the legend
  Legend->Draw("SAME");
}

void CRtoSRPlotter::SaveOutput(const Bool_t isScaled)
{
  std::cout << "Saving hist as png..." << std::endl;

  // Go back to the main canvas before saving and write out lumi info
  OutCanv->cd();
  Common::CMSLumi(OutCanv,0);

  // Save a log version first
  CRtoSRPlotter::PrintCanvas(isScaled,true);

  // Save a linear version second
  CRtoSRPlotter::PrintCanvas(isScaled,false);

  // save to output file
  fOutFile->cd();
  OutCanv->Write(Form("%s_%s",OutCanv->GetName(),(isScaled?"scaled":"norm")),TObject::kWriteDelete);
}  

void CRtoSRPlotter::PrintCanvas(const Bool_t isScaled, const Bool_t isLogy)
{
  // set logy
  OutCanv->cd();
  OutCanv->SetLogy(isLogy);

  auto & hist = HistMap["CR_Data"];
  
  // set min and max
  if (isLogy)
  {
    hist->SetMinimum(fMinY/1.5);
    hist->SetMaximum(fMaxY*1.5);
  }
  else 
  {
    hist->SetMinimum( fMinY > 0 ? fMinY/1.05 : fMinY*1.05 );
    hist->SetMaximum( fMaxY > 0 ? fMaxY*1.05 : fMaxY/1.05 );      
  }

  // save canvas as png
  OutCanv->cd();
  OutCanv->SaveAs(Form("%s_%s_%s.png",fOutFileText.Data(),(isScaled?"scaled":"norm"),(isLogy?"log":"lin")));
}

void CRtoSRPlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** CRtoSRPlotter Config *****");

  // fit config
  Common::AddTextFromInputConfig(fConfigPave,"CRtoSR Config",fCRtoSRConfig);

  // store last bits of misc info
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // dump in old config
  fConfigPave->AddText("***** CR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fCRFile);

  fConfigPave->AddText("***** SR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fSRFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void CRtoSRPlotter::SetupConfig()
{
  std::cout << "Setting up config..." << std::endl;

  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
}

void CRtoSRPlotter::SetupCRtoSRConfig()
{
  std::cout << "Reading CRtoSR config..." << std::endl;

  std::ifstream infile(Form("%s",fCRtoSRConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("sample=") != std::string::npos)
    {
      fSample = Common::RemoveDelim(str,"sample=");
    }
    else if (str.find("cr_file=") != std::string::npos)
    {
      fCRFileName = Common::RemoveDelim(str,"cr_file=");
    }
    else if (str.find("sr_file=") != std::string::npos)
    {
      fSRFileName = Common::RemoveDelim(str,"sr_file=");
    }
    else if (str.find("plot_config") != std::string::npos)
    {
      fPlotConfig = Common::RemoveDelim(str,"plot_config=");
    }
    else 
    {
      std::cerr << "Aye... your crtosr config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void CRtoSRPlotter::SetupPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str.find("x_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,fXVarBins);
    }
  }
}

void CRtoSRPlotter::GetHistMinimum()
{
  fMinY = 1e9;

  // need to loop through to check bin != 0
  for (const auto & HistPair : HistMap)
  {
    const auto & hist = HistPair.second;
    for (auto bin = 1; bin <= hist->GetNbinsX(); bin++)
    {
      const auto tmpmin = hist->GetBinContent(bin);
      if ((tmpmin < fMinY) && (tmpmin > 0)) fMinY = tmpmin;
    }
  }
}

void CRtoSRPlotter::GetHistMaximum()
{
  fMaxY = -1e9;

  for (const auto & HistPair : HistMap)
  {
    const auto & hist = HistPair.second;
    const auto tmpmax = hist->GetBinContent(hist->GetMaximumBin());
    if (tmpmax > fMaxY) fMaxY = tmpmax;
  }
}