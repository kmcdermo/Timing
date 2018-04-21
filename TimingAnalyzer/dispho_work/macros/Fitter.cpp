// Class include
#include "Fitter.hh"

// ROOT+RooFit includes
#include "TVirtualFitter.h"
#include "RooPlot.h"

Fitter::Fitter(const TString & fitconfig, const TString & outfiletext)
  : fFitConfig(fitconfig), fOutFileText(outfiletext)
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

  // init configuration, set minimizer
  Fitter::SetupDefaultBools();
  Fitter::SetupConfig();
  Fitter::ReadInFitConfig();
  TVirtualFitter::SetDefaultFitter("Minuit2");
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"RECREATE");
}

Fitter::~Fitter()
{
  delete fConfigPave;

  delete fY;
  delete fX;

  delete fNPredSign;
  delete fNPredBkgd;
  Fitter::DeleteMap(fFracMap);

  Fitter::DeleteMap(fHistMapY);
  Fitter::DeleteMap(fHistMapX);
  Fitter::DeleteMap(fHistMap2D);

  delete fQCDHistMC_CR;
  delete fGJetsHistMC_CR;
  delete fQCDHistMC_SR;
  delete fGJetsHistMC_SR;
  
  delete fSRFile;
  delete fQCDFile;
  delete fGJetsFile;

  delete fOutFile;
  delete fTDRStyle;
}

void Fitter::MakeFits()
{
  // Get all the variables in place
  Fitter::PrepareFits();
  
  // Do the fit in 2D
  FitInfo FitInfo2D(RooArgList(*fX,*fY),"2D",TwoD);
  Fitter::MakeFit(fHistMap2D,FitInfo2D);

  // Project 2D Hist to 1D
  Fitter::Project2DHistTo1D();

  // Do the fit in 1D -- X
  FitInfo FitInfoX(RooArgList(*fX),"projX",X);
  Fitter::MakeFit(fHistMapX,FitInfoX);

  // Do the fit in 1D -- Y
  FitInfo FitInfoY(RooArgList(*fY),"projY",Y);
  Fitter::MakeFit(fHistMapY,FitInfoY);

  // Save MetaData
  Fitter::MakeConfigPave();

  // Delete stuff
  Fitter::DeleteFitInfo(FitInfo2D);
  Fitter::DeleteFitInfo(FitInfoX);
  Fitter::DeleteFitInfo(FitInfoY);
}

void Fitter::PrepareFits()
{
  // Get the input 2D histograms
  Fitter::GetInputHists();

  // Get constants as needed
  Fitter::GetConstants();

  // Get mins and maxes
  Fitter::GetMinMax();

  // Declare variable of interest
  Fitter::DeclareVars();
}

void Fitter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  // GJets CR
  const TString gjetsfilename = Form("%s_%s.root",fPlotName.Data(),fGJetsFileBase.Data());
  fGJetsFile = TFile::Open(Form("%s",gjetsfilename.Data()));
  Config::CheckValidFile(fGJetsFile,gjetsfilename);

  fHistMap2D[GJets] = (TH2F*)fGJetsFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  fGJetsHistMC_CR   = (TH2F*)fGJetsFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  Config::CheckValidTH2F(fHistMap2D[GJets],Config::HistNameMap[Data] ,gjetsfilename);
  Config::CheckValidTH2F(fGJetsHistMC_CR  ,Config::HistNameMap[GJets],gjetsfilename);

  // QCD CR
  const TString qcdfilename = Form("%s_%s.root",fPlotName.Data(),fQCDFileBase.Data());
  fQCDFile = TFile::Open(Form("%s",qcdfilename.Data()));
  Config::CheckValidFile(fQCDFile,qcdfilename);

  fHistMap2D[QCD] = (TH2F*)fQCDFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  fQCDHistMC_CR   = (TH2F*)fQCDFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(fHistMap2D[QCD],Config::HistNameMap[Data],qcdfilename);
  Config::CheckValidTH2F(fQCDHistMC_CR  ,Config::HistNameMap[QCD] ,qcdfilename);

  // SR
  const TString srfilename = Form("%s_%s.root",fPlotName.Data(),fSRFileBase.Data()); 
  fSRFile = TFile::Open(Form("%s",srfilename.Data()));
  Config::CheckValidFile(fQCDFile,qcdfilename);

  fGJetsHistMC_SR = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  fQCDHistMC_SR   = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(fGJetsHistMC_SR,Config::HistNameMap[GJets],srfilename);
  Config::CheckValidTH2F(fQCDHistMC_SR  ,Config::HistNameMap[QCD]  ,srfilename);

  // use signal sample?
  if (!fBkgdOnly) 
  {
    fHistMap2D[GMSB] = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[GMSB].Data()));
    Config::CheckValidTH2F(fHistMap2D[GMSB],Config::HistNameMap[GMSB],srfilename);
  }

  // use real data?
  if (!fGenData)
  {
    fHistMap2D[Data] = (TH2F*)fSRFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
    Config::CheckValidTH2F(fHistMap2D[Data],Config::HistNameMap[Data],srfilename);
  }
}  

void Fitter::GetConstants()
{
  std::cout << "Getting integral counts..." << std::endl;

  // Count up background first
  std::map<SampleType,Float_t> nBkgdMap;
  nBkgdMap[GJets] = ((fHistMap2D[GJets]->Integral("widths"))*(fGJetsHistMC_SR->Integral("widths")))/(fGJetsHistMC_CR->Integral("widths"));
  nBkgdMap[QCD]   = ((fHistMap2D[QCD]  ->Integral("widths"))*(fQCDHistMC_SR  ->Integral("widths")))/(fQCDHistMC_CR  ->Integral("widths"));
 
  fNBkgdTotal = 0.f;
  for (const auto & nBkgdPair : nBkgdMap) fNBkgdTotal += nBkgdPair.second;

  for (auto & nBkgdPair : nBkgdMap)
  {
    const auto & sample = nBkgdPair.first;
    const auto   nbkgd  = nBkgdPair.second;

    const TString name = Form("%s_frac",Config::HistNameMap[sample].Data());
    fFracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),nbkgd/fNBkgdTotal);
  }

  // Count signal
  fNSignTotal = (fBkgdOnly ? 0.f : fHistMap2D[GMSB]->Integral("widths"));

  // make vars for varying extended PDFs
  fNPredBkgd = new RooRealVar("nbkgd","nbkgd",fInitRange*fNBkgdTotal,(1+fInitRange)*fNBkgdTotal);
  fNPredSign = new RooRealVar("nsign","nsign",fInitRange*fNSignTotal,(1+fInitRange)*fNSignTotal);
}

void Fitter::GetMinMax()
{
  std::cout << "Getting min and max of x,y range..." << std::endl;

  fXmin = fHistMap2D[GJets]->GetXaxis()->GetBinLowEdge(1);
  fXmax = fHistMap2D[GJets]->GetXaxis()->GetBinUpEdge(fHistMap2D[GJets]->GetXaxis()->GetNbins());
  fYmin = fHistMap2D[GJets]->GetYaxis()->GetBinLowEdge(1);
  fYmax = fHistMap2D[GJets]->GetYaxis()->GetBinUpEdge(fHistMap2D[GJets]->GetYaxis()->GetNbins());
}

void Fitter::DeclareVars()
{
  std::cout << "Declaring RooFit variables..." << std::endl;

  fX = new RooRealVar("x",fHistMap2D[GJets]->GetXaxis()->GetTitle(),fXmin,fXmax);
  fY = new RooRealVar("y",fHistMap2D[GJets]->GetYaxis()->GetTitle(),fYmin,fYmax);
}

template <typename T>
void Fitter::MakeFit(const T & HistMap, FitInfo & fitInfo)
{
  std::cout << "Doing full chain of fit for: " << fitInfo.Text.Data() << std::endl;

  // Declare datasets with input histograms (when using real data as input)
  Fitter::DeclareDatasets(HistMap,fitInfo);

  // Make pdfs from histograms
  Fitter::MakeSamplePdfs(fitInfo);

  // Build Model
  Fitter::BuildModel(fitInfo);

  // Construct asimov dataset from PDFS
  if (fGenData) Fitter::GenerateData(fitInfo);

  // Fit Model to Data
  Fitter::FitModel(fitInfo);

  // Draw fit(s) in 1D
  if (fitInfo.Fit == TwoD)
  {
    Fitter::DrawFit(fX,"xfit",fitInfo);
    Fitter::DrawFit(fY,"yfit",fitInfo);
  }
  else if (fitInfo.Fit == X)
  {
    Fitter::DrawFit(fX,"fit",fitInfo);
  }
  else if (fitInfo.Fit == Y)
  {
    Fitter::DrawFit(fY,"fit",fitInfo);
  }
  else
  {
    std::cerr << "Not sure how, but you provided an incorrect enum for FitType! Exiting..." << std::endl;
    exit(1);
  }

  // Save in a workspace
  Fitter::ImportToWS(fitInfo);
}

template <typename T>  
void Fitter::DeclareDatasets(const T & HistMap, FitInfo & fitInfo)
{
  std::cout << "Setting datasets for: " << fitInfo.Text.Data() << std::endl;
  
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const auto & hist   = HistPair.second;

    const TString name = Form("%s_RooDataHist_%s",Config::HistNameMap[sample].Data(),fitInfo.Text.Data());
    fitInfo.DataHistMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList,hist);
  }
}

void Fitter::MakeSamplePdfs(FitInfo & fitInfo)
{
  std::cout << "Setting Pdfs for: " << fitInfo.Text.Data() << std::endl;

  for (const auto & DataHistPair : fitInfo.DataHistMap)
  {
    // build background and signal pdfs
    const auto & sample   = DataHistPair.first;
    const auto & datahist = DataHistPair.second;
    if (Config::GroupMap[sample] == isData) continue;
    
    const TString name = Form("%s_PDF_%s",Config::HistNameMap[sample].Data(),fitInfo.Text.Data());
    fitInfo.HistPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList,*datahist);
  }
}

void Fitter::BuildModel(FitInfo & fitInfo)
{
  std::cout << "Build model for: " << fitInfo.Text.Data() << std::endl;

  // Declare strings for naming pdfs
  const TString bkgdname  = Form("bkgdpdf_%s",fitInfo.Text.Data());
  const TString ebkgdname = Form("ebkgdpdf_%s",fitInfo.Text.Data());
  const TString signname  = Form("signpdf_%s",fitInfo.Text.Data());
  const TString esignname = Form("esignpdf_%s",fitInfo.Text.Data());
  const TString modelname = Form("modelpdf_%s",fitInfo.Text.Data());

  // get members of fitInfo
  const auto & pdfmap = fitInfo.HistPdfMap;

  // Build Bkgd-Only Pdfs first
  fitInfo.BkgdPdf = new RooAddPdf(Form("%s",bkgdname.Data()),Form("%s",bkgdname.Data()),RooArgList(*pdfmap.at(GJets),*pdfmap.at(QCD)),RooArgList(*fFracMap.at(GJets),*fFracMap.at(QCD)));
  fitInfo.EBkgdPdf = new RooExtendPdf(Form("%s",bkgdname.Data()),Form("%s",bkgdname.Data()),*fitInfo.BkgdPdf,*fNPredBkgd);

  // use signal?
  if (!fBkgdOnly)
  {
    fitInfo.ESignPdf = new RooExtendPdf(Form("%s",signname.Data()),Form("%s",signname.Data()),*pdfmap.at(GMSB),*fNPredSign);
    fitInfo.ModelPdf = new RooAddPdf(Form("%s",modelname.Data()),Form("%s",modelname.Data()),RooArgList(*fitInfo.EBkgdPdf,*fitInfo.ESignPdf));
  }
  else
  {
    fitInfo.ModelPdf = new RooAddPdf(Form("%s",modelname.Data()),Form("%s",modelname.Data()),RooArgList(*fitInfo.EBkgdPdf));
  }
}

void Fitter::GenerateData(FitInfo & fitInfo)
{
  std::cout << "Generating Asimov data for: " << fitInfo.Text.Data() << std::endl;
  fitInfo.DataHistMap[Data] = fitInfo.ModelPdf->generateBinned(fitInfo.ArgList,(fNBkgdTotal+fNSignTotal),RooFit::Asimov());
}

void Fitter::FitModel(FitInfo & fitInfo)
{
  std::cout << "Fit model for: " << fitInfo.Text.Data() << std::endl;
  fitInfo.ModelPdf->fitTo(*fitInfo.DataHistMap.at(Data),RooFit::SumW2Error(true),RooFit::PrintLevel(2));
}

void Fitter::DrawFit(RooRealVar *& var, const TString & title, const FitInfo & fitInfo)
{
  std::cout << "Draw fits projected into 1D for: " << fitInfo.Text.Data() << std::endl;

  // Get Canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  
  // Draw 1D frame
  auto frame = var->frame();

  // Blind the data!
  const TString varname = var->GetName();
  if      (varname.EqualTo("x",TString::kExact)) fitInfo.DataHistMap.at(Data)->plotOn(frame,RooFit::Cut(Form("%s",fXCut.Data())));
  else if (varname.EqualTo("y",TString::kExact)) fitInfo.DataHistMap.at(Data)->plotOn(frame,RooFit::Cut(Form("%s",fYCut.Data())));
  else 
  {
    std::cerr << "How did this happen?? Specified a variable that is not predefined... exiting.." << std::endl;
    exit(1);
  }

  // Plot the fitted pdf
  fitInfo.ModelPdf->plotOn(frame);
  frame->Draw();

  // make the canvas nice and save
  Config::CMSLumi(canv);
  canv->SaveAs(Form("%s_%s.png",title.Data(),fitInfo.Text.Data()));

  // delete it
  delete frame;
  delete canv;
}

void Fitter::ImportToWS(FitInfo & fitInfo)
{
  std::cout << "Make workspace for " << fitInfo.Text.Data() << std::endl;

  fitInfo.Workspace = new RooWorkspace(Form("workspace_%s",fitInfo.Text.Data()),Form("workspace_%s",fitInfo.Text.Data()));

  fitInfo.Workspace->import(*fitInfo.ModelPdf);
  fitInfo.Workspace->import(*fitInfo.DataHistMap.at(Data));

  fOutFile->cd();
  fitInfo.Workspace->Write(fitInfo.Workspace->GetName(),TObject::kWriteDelete);
}

void Fitter::Project2DHistTo1D()
{
  for (auto & HistPair2D : fHistMap2D)
  {
    const auto & sample = HistPair2D.first;
    fHistMapX[sample] = (TH1F*)HistPair2D.second->ProjectionX(Form("%s_projX",Config::HistNameMap[sample].Data()));
    fHistMapY[sample] = (TH1F*)HistPair2D.second->ProjectionY(Form("%s_projY",Config::HistNameMap[sample].Data()));
  }
}

void Fitter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName("Config");
  std::string str; // tmp string
  
  // fit config
  fConfigPave->AddText("Fit Config");
  std::ifstream cutfile(Form("%s",fFitConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void Fitter::SetupDefaultBools()
{
  fBkgdOnly = false;
  fGenData  = false;
}

void Fitter::SetupConfig()
{
  Config::SetupGroups();
  Config::SetupHistNames();
}

void Fitter::ReadInFitConfig()
{
  std::cout << "Reading fit config..." << std::endl;

  std::ifstream infile(Form("%s",fFitConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("CR_GJets_In=") != std::string::npos)
    {
      fGJetsFileBase = Config::RemoveDelim(str,"CR_GJets_In=");
    }
    else if (str.find("CR_QCD_In=") != std::string::npos)
    {
      fQCDFileBase = Config::RemoveDelim(str,"CR_QCD_In=");
    }
    else if (str.find("SR_In=") != std::string::npos)
    {
      fSRFileBase = Config::RemoveDelim(str,"SR_In=");
    }
    else if (str.find("Range=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"Range=");
      fInitRange = std::atof(str.c_str());
    }
    else if (str.find("Plot=") != std::string::npos)
    {
      fPlotName = Config::RemoveDelim(str,"Plot=");
    }
    else if (str.find("x_cut=") != std::string::npos)
    {
      fXCut = Config::RemoveDelim(str,"x_cut=");
    }
    else if (str.find("y_cut=") != std::string::npos)
    {
      fYCut = Config::RemoveDelim(str,"y_cut=");
    }
    else if (str.find("bkgd_only=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"bkgd_only=");
      Config::SetupBool(str,fBkgdOnly);
    }
    else if (str.find("gen_data=") != std::string::npos)
    {
      str = Config::RemoveDelim(str,"gen_data=");
      Config::SetupBool(str,fGenData);
    }
    else 
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void Fitter::DeleteFitInfo(FitInfo & fitInfo)
{
  Fitter::DeleteMap(fitInfo.DataHistMap);
  Fitter::DeleteMap(fitInfo.HistPdfMap);

  delete fitInfo.BkgdPdf;
  delete fitInfo.EBkgdPdf;
  delete fitInfo.ESignPdf;
  delete fitInfo.ModelPdf;
  delete fitInfo.Workspace;
}

template <typename T>
void Fitter::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
  Map.clear();			  
}
