// Class include
#include "CRtoSRPlotter.hh"

CRtoSRPlotter::CRtoSRPlotter(const TString & crtosrconfig, const TString & era, const TString & outfiletext)
  : fCRtoSRConfig(crtosrconfig), fEra(era), fOutFileText(outfiletext)
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
  CRtoSRPlotter::SetupDefaults();
  CRtoSRPlotter::SetupCRtoSRConfig();
  CRtoSRPlotter::SetupCommon();

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
  delete fCRFile;

  delete fOutFile;
  delete fTDRStyle;
}

void CRtoSRPlotter::MakeCRtoSRPlot()
{
  // setup hists/canv/legend for plotting
  CRtoSRPlotter::GetInputHists();
  CRtoSRPlotter::SetupHistsStlye();
  CRtoSRPlotter::SaveOutHists();
  CRtoSRPlotter::MakeMCErrs();
  CRtoSRPlotter::MakeLegend();
  CRtoSRPlotter::MakeOutCanv();

  // draw canvas normalized
  if (fDrawNorm)
  {
    CRtoSRPlotter::DrawOutCanv(false);
    CRtoSRPlotter::SaveOutput(false);
  }

  // draw canvas normalized to 1.0
  if (fDrawScaled)
  {
    CRtoSRPlotter::ScaleToUnity();
    CRtoSRPlotter::DrawOutCanv(true);
    CRtoSRPlotter::SaveOutput(true);
  }

  // dump meta info
  CRtoSRPlotter::MakeConfigPave();
}

void CRtoSRPlotter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  const TString dataname = Common::HistNameMap["Data"] +"_Plotted";
  const TString mcname   = Common::HistNameMap[fSample]+"_Plotted";

  // CR File
  fCRFile = TFile::Open(Form("%s",fCRFileName.Data()));
  Common::CheckValidFile(fCRFile,fCRFileName);

  HistMap["CR_Data"] = (TH1F*)fCRFile->Get(dataname.Data());
  HistMap["CR_MC"]   = (TH1F*)fCRFile->Get(mcname  .Data());
  Common::CheckValidHist(HistMap["CR_Data"],dataname,fCRFileName);
  Common::CheckValidHist(HistMap["CR_MC"]  ,mcname  ,fCRFileName);

  // SR
  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);

  HistMap["SR_MC"]   = (TH1F*)fSRFile->Get(mcname  .Data());
  Common::CheckValidHist(HistMap["SR_MC"]  ,mcname  ,fSRFileName);
}  

void CRtoSRPlotter::SetupHistsStlye()
{
  std::cout << "Setting up hists style..." << std::endl;

  for (auto & HistPair : HistMap)
  {
    const auto & key = HistPair.first;
    auto & hist = HistPair.second;

    if (key.Contains("Data",TString::kExact))
    {
      hist->SetLineColor(kBlack);
      hist->SetMarkerColor(kBlack);
    }
    else
    {
      const auto color = (key.Contains("CR",TString::kExact) ? kOrange+10 : kAzure);

      hist->SetLineColorAlpha(color,0.5);
      hist->SetFillColorAlpha(color,0.5);
      hist->SetMarkerColorAlpha(color,0.5);
      hist->SetFillStyle(1001);
    }
  }
}

void CRtoSRPlotter::SaveOutHists()
{
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
  Legend = new TLegend(0.57,0.75,0.825,0.92);
  Legend->SetName("Legend");
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);
  
  // add Data
  Legend->AddEntry(HistMap["CR_Data"],Form("Data [%s CR]",fSample.Data()),"epl");

  // add MC
  Legend->AddEntry(HistMap["SR_MC"],Form("%s MC [SR]",fSample.Data()),"f");
  Legend->AddEntry(HistMap["CR_MC"],Form("%s MC [%s CR]",fSample.Data(),fSample.Data()),"f");

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

  // zeroth, draw data
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

  // draw errs on top
  for (const auto & HistPair : HistMap)
  {
    const auto & key  = HistPair.first;
    const auto & hist = HistPair.second;

    if ( key.Contains("Data",TString::kExact)) continue;
    if (!key.Contains("Err" ,TString::kExact)) continue;
    
    hist->Draw("E2 SAME");
  }

  // then, redraw data to appear on top
  HistMap["CR_Data"]->Draw("PE SAME");

  // And lastly draw the legend
  Legend->Draw("SAME");
}

void CRtoSRPlotter::SaveOutput(const Bool_t isScaled)
{
  std::cout << "Saving hist as images..." << std::endl;

  // Go back to the main canvas before saving and write out lumi info
  OutCanv->cd();
  Common::CMSLumi(OutCanv,0,fEra);

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

 // set min/max to this hist as it is the first to be drawn
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

  // save canvas as images
  Common::SaveAs(OutCanv,Form("%s_%s_%s",fOutFileText.Data(),(isScaled?"scaled":"norm"),(isLogy?"log":"lin")));
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

  // Add era info
  Common::AddEraInfoToPave(fConfigPave,fEra);

  // CR to SR plot config
  Common::AddTextFromInputConfig(fConfigPave,"CRtoSR Config",fCRtoSRConfig);

  // a bit redundant as this is in CR and SR config, but store plot config anyway
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);
  
  // dump in old config
  fConfigPave->AddText("***** CR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fCRFile);

  fConfigPave->AddText("***** SR Config *****");
  Common::AddTextFromInputPave(fConfigPave,fSRFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void CRtoSRPlotter::SetupDefaults()
{
  fDrawNorm = false;
  fDrawScaled = false;
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
    else if (str.find("plot_config=") != std::string::npos)
    {
      fPlotConfig = Common::RemoveDelim(str,"plot_config=");
    }
    else if (str.find("draw_norm=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"draw_norm=");
      Common::SetupBool(str,fDrawNorm);
    }
    else if (str.find("draw_scaled=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"draw_scaled=");
      Common::SetupBool(str,fDrawScaled);
    }
    else 
    {
      std::cerr << "Aye... your crtosr config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void CRtoSRPlotter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
  Common::SetupVarBinsBool("x_bins=",fPlotConfig,fXVarBins);
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
