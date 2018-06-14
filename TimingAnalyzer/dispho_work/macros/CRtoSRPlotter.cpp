// Class include
#include "CRtoSRPlotter.hh"

CRtoSRPlotter::CRtoSRPlotter(const TString & crtosrconfig, const TString & plotconfig, const TString & outfiletext)
  : fCRtoSRConfig(crtosrconfig), fPlotConfig(plotconfig), fOutFileText(outfiletext)
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

  // init configuration, set minimizer
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
  CRtoSRPlotter::SetHistsStlye();
  CRtoSRPlotter::MakeLegend();

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
  HistMap["CR_Data"]->Write(HistMap["CR_Data"]->GetName(),TObject::kWriteDelete);
  HistMap["CR_MC"]->Write(HistMap["CR_MC"]->GetName(),TObject::kWriteDelete);
  HistMap["SR_MC"]->Write(HistMap["SR_MC"]->GetName(),TObject::kWriteDelete);
}  

void CRtoSRPlotter::SetHistsStlye()
{
  HistMap["CR_MC"]->SetLineColor(kRed);
  HistMap["CR_MC"]->SetFillColorAlpha(kRed,0.5);
  HistMap["CR_MC"]->SetMarkerColor(kRed);
  HistMap["CR_MC"]->SetFillStyle(1001);

  HistMap["SR_MC"]->SetLineColor(kBlue);
  HistMap["SR_MC"]->SetFillColorAlpha(kBlue,0.5);
  HistMap["SR_MC"]->SetMarkerColor(kBlue);
  HistMap["SR_MC"]->SetFillStyle(1001);

  HistMap["CR_Data"]->SetLineColor(kBlack);
}

void CRtoSRPlotter::MakeLegend()
{
  std::cout << "Creating Legend..." << std::endl;

  // instantiate the legend
  Legend = new TLegend(0.55,0.65,0.825,0.92);
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

void CRtoSRPlotter::ScaleToUnity()
{
  HistMap["CR_Data"]->Scale(1.f/HistMap["CR_Data"]->Integral(fXVarBins?"width":""));
  HistMap["CR_MC"]->Scale(1.f/HistMap["CR_MC"]->Integral(fXVarBins?"width":""));
  HistMap["SR_MC"]->Scale(1.f/HistMap["SR_MC"]->Integral(fXVarBins?"width":""));
}

void CRtoSRPlotter::DrawOutCanv(const Bool_t isScaled)
{
  std::cout << "Drawing ..." << std::endl;

  OutCanv->cd();
  
  // Get and Set Global Minimum and Maximum
  CRtoSRPlotter::GetHistMinimum();
  CRtoSRPlotter::GetHistMaximum();

  HistMap["CR_Data"]->SetMinimum(fMinY);
  HistMap["CR_Data"]->SetMaximum(fMaxY);
  HistMap["CR_Data"]->Draw("PE");

  HistMap["CR_MC"]->Draw("HIST SAME");
  HistMap["CR_MC"]->SetLineColorAlpha(0,1);
  HistMap["CR_MC"]->SetMarkerSize(0);
  HistMap["CR_MC"]->SetFillStyle(3254);
  HistMap["CR_MC"]->SetFillColor(kGray+3);
  HistMap["CR_MC"]->Draw("E2 SAME");

  HistMap["SR_MC"]->Draw("HIST SAME");
  HistMap["SR_MC"]->SetLineColorAlpha(0,1);
  HistMap["SR_MC"]->SetMarkerSize(0);
  HistMap["SR_MC"]->SetFillStyle(3254);
  HistMap["SR_MC"]->SetFillColor(kGray+3);
  HistMap["SR_MC"]->Draw("E2 SAME");

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
  CRtoSRPlotter::PrintCanvas(fOutFileText,isScaled,true);

  // Save a linear version second
  CRtoSRPlotter::PrintCanvas(fOutFileText,isScaled,false);

  // save to output file
  fOutFile->cd();
  OutCanv->Write(Form("%s_%s",OutCanv->GetName(),(isScaled?"norm":"scaled")),TObject::kWriteDelete);
}  

void CRtoSRPlotter::PrintCanvas(const Bool_t isScaled, const Bool_t isLogy)
{
  OutCanv->cd();
  OutCanv->SetLogy(isLogy);
  
  // set min and max
  if (isLogy)
  {
    HistMap["CR_Data"]->SetMinimum(fMinY/1.5);
    HistMap["CR_Data"]->SetMaximum(fMaxY*1.5);
  }
  else 
  {
    HistMap["CR_Data"]->SetMinimum( fMinY > 0 ? fMinY/1.05 : fMinY*1.05 );
    HistMap["CR_Data"]->SetMaximum( fMaxY > 0 ? fMaxY*1.05 : fMaxY/1.05 );      
  }

  // save canvas as png
  OutCanv->cd();
  OutCanv->SaveAs(Form("%s_%s_%s.png",fOutFileText.Data(),(isScaled?"norm":"scaled"),(isLogy?"log":"lin")));
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
    if ((tmpmax > fMaxY) tmpmax = fMaxY;
  }
}
