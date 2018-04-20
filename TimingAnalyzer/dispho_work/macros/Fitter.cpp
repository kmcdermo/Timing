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
  Fitter::SetupConfig();
  Fitter::ReadInFitConfig();
  TVirtualFitter::SetDefaultFitter("Minuit2");
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"RECREATE");
}

Fitter::~Fitter()
{
  delete fConfigPave;

  // 2D
  Fitter::Delete(HistMap2D,RooDHMap2D,RooHPdfMap2D,FracMap2D,ModelPdf2D,Workspace2D);

  // 1D -- X
  Fitter::Delete(HistMapX,RooDHMapX,RooHPdfMapX,FracMapX,ModelPdfX,WorkspaceX);

  // 1D -- Y
  Fitter::Delete(HistMapY,RooDHMapY,RooHPdfMapY,FracMapY,ModelPdfY,WorkspaceY);

  delete fY;
  delete fX;

  delete QCDHistMC_CR;
  delete GJetsHistMC_CR;
  delete QCDHistMC_SR;
  delete GJetsHistMC_SR;
  
  delete SRFile;
  delete QCDFile;
  delete GJetsFile;

  delete fOutFile;
}

void Fitter::MakeFits()
{
  // Get all the variables in place
  Fitter::PrepareFits();
  
  // Project 2D Hist
  Fitter::Project2DHistTo1D();

  // Do the fit in 2D
  FitInfo FitInfo2D(RooArgList(*fX,*fY),"2D",TwoD);
  Fitter::MakeFit(HistMap2D,RooDHMap2D,RooHPdfMap2D,FracMap2D,ModelPdf2D,Workspace2D,FitInfo2D);

  // Do the fit in 1D -- X
  FitInfo FitInfoX(RooArgList(*fX),"projX",X);
  Fitter::MakeFit(HistMapX,RooDHMapX,RooHPdfMapX,FracMapX,ModelPdfX,WorkspaceX,FitInfoX);

  // Do the fit in 1D -- Y
  FitInfo FitInfoY(RooArgList(*fY),"projY",Y);
  Fitter::MakeFit(HistMapY,RooDHMapY,RooHPdfMapY,FracMapY,ModelPdfY,WorkspaceY,FitInfoY);

  // Save MetaData
  Fitter::MakeConfigPave();
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
  GJetsFile = TFile::Open(Form("%s",gjetsfilename.Data()));
  Config::CheckValidFile(GJetsFile,gjetsfilename);

  HistMap2D[GJets] = (TH2F*)GJetsFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  GJetsHistMC_CR   = (TH2F*)GJetsFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  Config::CheckValidTH2F(HistMap2D[GJets],Config::HistNameMap[Data] ,gjetsfilename);
  Config::CheckValidTH2F(GJetsHistMC_CR  ,Config::HistNameMap[GJets],gjetsfilename);

  // QCD CR
  const TString qcdfilename = Form("%s_%s.root",fPlotName.Data(),fQCDFileBase.Data());
  QCDFile = TFile::Open(Form("%s",qcdfilename.Data()));
  Config::CheckValidFile(QCDFile,qcdfilename);

  HistMap2D[QCD] = (TH2F*)QCDFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  QCDHistMC_CR   = (TH2F*)QCDFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(HistMap2D[QCD],Config::HistNameMap[Data],qcdfilename);
  Config::CheckValidTH2F(QCDHistMC_CR  ,Config::HistNameMap[QCD] ,qcdfilename);

  // SR
  const TString srfilename = Form("%s_%s.root",fPlotName.Data(),fSRFileBase.Data()); 
  SRFile = TFile::Open(Form("%s",srfilename.Data()));
  Config::CheckValidFile(QCDFile,qcdfilename);

  HistMap2D[Data] = (TH2F*)SRFile->Get(Form("%s",Config::HistNameMap[Data].Data()));
  HistMap2D[GMSB] = (TH2F*)SRFile->Get(Form("%s",Config::HistNameMap[GMSB].Data()));
  GJetsHistMC_SR  = (TH2F*)SRFile->Get(Form("%s",Config::HistNameMap[GJets].Data()));
  QCDHistMC_SR    = (TH2F*)SRFile->Get(Form("%s",Config::HistNameMap[QCD].Data()));
  Config::CheckValidTH2F(HistMap2D[Data],Config::HistNameMap[Data] ,srfilename);
  Config::CheckValidTH2F(HistMap2D[GMSB],Config::HistNameMap[GMSB] ,srfilename);
  Config::CheckValidTH2F(GJetsHistMC_SR ,Config::HistNameMap[GJets],srfilename);
  Config::CheckValidTH2F(QCDHistMC_SR   ,Config::HistNameMap[QCD]  ,srfilename);
}  

void Fitter::GetConstants()
{
  std::cout << "Getting integral counts..." << std::endl;

  NPredMap[GJets] = HistMap2D[GJets]->Integral("widths")*GJetsHistMC_SR->Integral("widths")/GJetsHistMC_CR->Integral("widths");
  NPredMap[QCD] = HistMap2D[QCD]->Integral("widths")*QCDHistMC_SR->Integral("widths")/QCDHistMC_CR->Integral("widths");
  NPredMap[GMSB] = HistMap2D[GMSB]->Integral("widths");

  NPredTotal = 0.f;
  for (const auto & NPredPair : NPredMap) NPredTotal += NPredPair.second;
}

void Fitter::DeclareVars()
{
  std::cout << "Declaring RooFit variables..." << std::endl;

  fX = new RooRealVar("x",HistMap2D[Data]->GetXaxis()->GetTitle(),fXmin,fXmax);
  fY = new RooRealVar("y",HistMap2D[Data]->GetYaxis()->GetTitle(),fYmin,fYmax);
}

void Fitter::GetMinMax()
{
  std::cout << "Getting min and max of x,y range..." << std::endl;

  fXmin = HistMap2D[Data]->GetXaxis()->GetBinLowEdge(1);
  fXmax = HistMap2D[Data]->GetXaxis()->GetBinUpEdge(HistMap2D[Data]->GetXaxis()->GetNbins());
  fYmin = HistMap2D[Data]->GetYaxis()->GetBinLowEdge(1);
  fYmax = HistMap2D[Data]->GetYaxis()->GetBinUpEdge(HistMap2D[Data]->GetYaxis()->GetNbins());
}

void Fitter::Project2DHistTo1D()
{
  for (auto & HistPair2D : HistMap2D)
  {
    const auto & sample = HistPair2D.first;
    HistMapX[sample] = (TH1F*)HistPair2D.second->ProjectionX(Form("%s_projX",Config::HistNameMap[sample].Data()));
    HistMapY[sample] = (TH1F*)HistPair2D.second->ProjectionY(Form("%s_projY",Config::HistNameMap[sample].Data()));
  }
}

template <typename T>
void Fitter::MakeFit(const T & HistMap, RDHMap & RooDHMap, RHPMap & RooHPdfMap, RRVMap & FracMap,
		     RooAddPdf *& ModelPdf, RooWorkspace *& Workspace, const FitInfo & fitInfo)
{
  std::cout << "Doing full chain of fit for: " << fitInfo.Text_.Data() << std::endl;

  // Declare datasets with input histograms
  Fitter::DeclareDatasets(HistMap,RooDHMap,fitInfo);

  // Make pdfs from histograms
  Fitter::MakeSamplePdfs(RooDHMap,RooHPdfMap,fitInfo);

  // Declare fractions for pdfs
  Fitter::DeclareFractions(RooHPdfMap,FracMap,fitInfo);

  // Fit Model
  Fitter::FitModel(ModelPdf,RooHPdfMap,FracMap,RooDHMap,fitInfo);

  // Draw fit(s) in 1D
  if (fitInfo.Fit_ == TwoD)
  {
    Fitter::DrawFit(fX,RooDHMap,ModelPdf,"xfit",fitInfo);
    Fitter::DrawFit(fY,RooDHMap,ModelPdf,"yfit",fitInfo);
  }
  else if (fitInfo.Fit_ == X)
  {
    Fitter::DrawFit(fX,RooDHMap,ModelPdf,"fit",fitInfo);
  }
  else if (fitInfo.Fit_ == Y)
  {
    Fitter::DrawFit(fY,RooDHMap,ModelPdf,"fit",fitInfo);
  }
  else
  {
    std::cerr << "Not sure how, but you provided an incorrect enum for FitType! Exiting..." << std::endl;
    exit(1);
  }

  // Save in a workspace
  Fitter::ImportToWS(Workspace,ModelPdf,RooDHMap,fitInfo);
}

template <typename T>  
void Fitter::DeclareDatasets(const T & HistMap, RDHMap & RooDHMap, const FitInfo & fitInfo)
{
  std::cout << "Setting datasets for: " << fitInfo.Text_.Data() << std::endl;
  
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const TString name = Form("%s_RooDataHist_%s",Config::HistNameMap[sample].Data(),fitInfo.Text_.Data());
    RooDHMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList_,HistPair.second);
  }
}

void Fitter::MakeSamplePdfs(const RDHMap & RooDHMap, RHPMap & RooHPdfMap, const FitInfo & fitInfo)
{
  std::cout << "Setting Pdfs for: " << fitInfo.Text_.Data() << std::endl;

  for (const auto & RooDHPair : RooDHMap)
  {
    const auto & sample = RooDHPair.first;
    if (sample == Data) continue;
    
    const TString name = Form("%s_PDF_%s",Config::HistNameMap[sample].Data(),fitInfo.Text_.Data());
    RooHPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList_,*RooDHPair.second);
  }
}

void Fitter::DeclareFractions(const RHPMap & RooHPdfMap, RRVMap & FracMap, const FitInfo & fitInfo)
{
  std::cout << "Init fractions for: " << fitInfo.Text_.Data() << std::endl;

  for (const auto & RooHPdfPair : RooHPdfMap)
  {
    const auto & sample = RooHPdfPair.first;
    if (sample == Data) continue;
    
    const TString name = Form("%s_frac_%s",Config::HistNameMap[sample].Data(),fitInfo.Text_.Data());
    FracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),NPredMap[sample]/NPredTotal,0,1);
  }
}

void Fitter::FitModel(RooAddPdf *& ModelPdf, const RHPMap & RooHPdfMap, const RRVMap & FracMap, const RDHMap & RooDHMap, const FitInfo & fitInfo)
{
  std::cout << "Build and fit to model for: " << fitInfo.Text_.Data() << std::endl;

  const TString name = Form("modelpdf_%s",fitInfo.Text_.Data());
  ModelPdf = new RooAddPdf(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(*RooHPdfMap.at(GJets),*RooHPdfMap.at(QCD)),RooArgList(*FracMap.at(GJets),*FracMap.at(QCD)));
  ModelPdf->fitTo(*RooDHMap.at(Data));
}

void Fitter::DrawFit(RooRealVar *& var, const RDHMap & RooDHMap, RooAddPdf *& ModelPdf, const TString & title, const FitInfo & fitInfo)
{
  std::cout << "Draw fits projected into 1D for: " << fitInfo.Text_.Data() << std::endl;

  // Get Canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  
  // Draw 1D frame
  auto frame = var->frame();

  // Blind the data!
  const TString varname = var->GetName();
  if      (varname.EqualTo("x",TString::kExact)) RooDHMap.at(Data)->plotOn(frame,RooFit::Cut(Form("%s",fXCut.Data())));
  else if (varname.EqualTo("y",TString::kExact)) RooDHMap.at(Data)->plotOn(frame,RooFit::Cut(Form("%s",fYCut.Data())));
  else 
  {
    std::cerr << "How did this happen?? Specified a variable that is not predefined... exiting.." << std::endl;
    exit(1);
  }

  // Plot the fitted pdf
  ModelPdf->plotOn(frame);
  
  // make the rooplot nice
  frame->SetMinimum(1e-2);
  frame->SetMaximum(1e5);
  frame->Draw();

  // make the canvas nice and save
  Config::CMSLumi(canv);
  canv->SaveAs(Form("%s_%s.png",title.Data(),fitInfo.Text_.Data()));

  // delete it
  delete frame;
  delete canv;
}

void Fitter::ImportToWS(RooWorkspace *& Workspace, RooAddPdf *& ModelPdf, const RDHMap & RooDHMap, const FitInfo & fitInfo)
{
  std::cout << "Make workspace for " << fitInfo.Text_.Data() << std::endl;

  Workspace = new RooWorkspace(Form("workspace_%s",fitInfo.Text_.Data()),Form("workspace_%s",fitInfo.Text_.Data()));

  Workspace->import(*ModelPdf);
  Workspace->import(*RooDHMap.at(Data));

  fOutFile->cd();
  Workspace->Write(Workspace->GetName(),TObject::kWriteDelete);
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
    else 
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

template <typename T>
void Fitter::Delete(T & HistMap, RDHMap & RooDHMap, RHPMap & RooHPdfMap, RRVMap & FracMap,
		    RooAddPdf *& ModelPdf, RooWorkspace *& Workspace)
{
  Fitter::DeleteMap(HistMap);
  Fitter::DeleteMap(RooDHMap);
  Fitter::DeleteMap(RooHPdfMap);
  Fitter::DeleteMap(FracMap);
  delete ModelPdf;
  delete Workspace;
}

template <typename T>
void Fitter::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
  Map.clear();			  
}
