// Class include
#include "Fitter.hh"

// ROOT+RooFit includes
#include "TVirtualFitter.h"
#include "RooPlot.h"

Fitter::Fitter(const TString & infilename, const TString & outfilename) 
  : fInFileName(infilename), fOutFileName(outfilename)
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
  Fitter::InitConfig();
  TVirtualFitter::SetDefaultFitter("Minuit2");

  // get input root file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Config::CheckValidFile(fInFile,fInFileName);
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s",fOutFileName.Data()),"RECREATE");
}

Fitter::~Fitter()
{
  // 2D
  Fitter::Delete(HistMap2D,RooDHMap2D,RooHPdfMap2D,FracMap2D,ModelPdf2D,Workspace2D);

  // 1D -- X
  Fitter::Delete(HistMapX,RooDHMapX,RooHPdfMapX,FracMapX,ModelPdfX,WorkspaceX);

  // 1D -- Y
  Fitter::Delete(HistMapY,RooDHMapY,RooHPdfMapY,FracMapY,ModelPdfY,WorkspaceY);

  delete fY;
  delete fX;

  delete fInFile;
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
}

void Fitter::PrepareFits()
{
  // Get the input 2D histograms
  Fitter::GetInputHists();

  // Get mins and maxes
  Fitter::GetMinMax();

  // Declare variable of interest
  Fitter::DeclareVars();
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

void Fitter::GetInputHists()
{
  std::cout << "Getting input histograms..." << std::endl;

  for (const auto & HistNamePair : Config::HistNameMap)
  {
    const auto & sample = HistNamePair.first;
    const TString histname = HistNamePair.second;
    HistMap2D[sample] = (TH2F*)fInFile->Get(Form("%s",histname.Data())); 
    Config::CheckValidTH2F(HistMap2D[sample],histname,fInFileName);
  }
}  

void Fitter::GetMinMax()
{
  std::cout << "Getting min and max of x,y range..." << std::endl;

  fXmin = HistMap2D[Data]->GetXaxis()->GetBinLowEdge(1);
  fXmax = HistMap2D[Data]->GetXaxis()->GetBinUpEdge(HistMap2D[Data]->GetXaxis()->GetNbins());
  fYmin = HistMap2D[Data]->GetYaxis()->GetBinLowEdge(1);
  fYmax = HistMap2D[Data]->GetYaxis()->GetBinUpEdge(HistMap2D[Data]->GetYaxis()->GetNbins());
}

void Fitter::DeclareVars()
{
  std::cout << "Declaring RooFit variables..." << std::endl;

  fX = new RooRealVar("x",HistMap2D[Data]->GetXaxis()->GetTitle(),fXmin,fXmax);
  fY = new RooRealVar("y",HistMap2D[Data]->GetYaxis()->GetTitle(),fYmin,fYmax);
}

template <typename T>  
void Fitter::DeclareDatasets(const T & HistMap, RDHMap & RooDHMap, const FitInfo & fitInfo)
{
  std::cout << "Setting datasets..." << std::endl;
  
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const TString name = Form("%s_RooDataHist_%s",Config::HistNameMap[sample].Data(),fitInfo.Text_.Data());
    RooDHMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList_,HistPair.second);
  }
}

void Fitter::MakeSamplePdfs(const RDHMap & RooDHMap, RHPMap & RooHPdfMap, const FitInfo & fitInfo)
{
  std::cout << "Setting Pdfs..." << std::endl;

  for (const auto & RooDHPair : RooDHMap)
  {
    const auto & sample = RooDHPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_PDF_%s",Config::HistNameMap[sample].Data(),fitInfo.Text_.Data());
    RooHPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),fitInfo.ArgList_,*RooDHPair.second);
  }
}

void Fitter::DeclareFractions(const RHPMap & RooHPdfMap, RRVMap & FracMap, const FitInfo & fitInfo)
{
  std::cout << "Init fractions..." << std::endl;

  for (const auto & RooHPdfPair : RooHPdfMap)
  {
    const auto & sample = RooHPdfPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_frac_%s",Config::HistNameMap[sample].Data(),fitInfo.Text_.Data());
    FracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),0.5,0,1);
  }
}

void Fitter::FitModel(RooAddPdf *& ModelPdf, const RHPMap & RooHPdfMap, const RRVMap & FracMap, const RDHMap & RooDHMap, const FitInfo & fitInfo)
{
  std::cout << "Build and fit to model..." << std::endl;

  const TString name = Form("modelpdf_%s",fitInfo.Text_.Data());
  ModelPdf = new RooAddPdf(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(*RooHPdfMap.at(GJets),*RooHPdfMap.at(QCD)),RooArgList(*FracMap.at(GJets),*FracMap.at(QCD)));
  ModelPdf->fitTo(*RooDHMap.at(Data));
}

void Fitter::DrawFit(RooRealVar *& var, const RDHMap & RooDHMap, RooAddPdf *& ModelPdf, const TString & title, const FitInfo & fitInfo)
{
  std::cout << "Draw fits projected into 1D..." << std::endl;

  // Get Canvas
  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogy();

  // Draw 1D stuff
  RooPlot * frame = var->frame();
  //  RooDHMap.at(Data)->reduce(RooFit::Cut(Form("%s",var->GetName())));
  RooDHMap.at(Data)->plotOn(frame);
  ModelPdf->plotOn(frame);
  frame->Draw();
  Config::CMSLumi(canv);
  canv->SaveAs(Form("%s_%s.png",title.Data(),fitInfo.Text_.Data()));

  // delete it
  delete frame;
  delete canv;
}

void Fitter::ImportToWS(RooWorkspace *& Workspace, RooAddPdf *& ModelPdf, const RDHMap & RooDHMap, const FitInfo & fitInfo)
{
  std::cout << "Make workspace..." << std::endl;

  Workspace = new RooWorkspace(Form("workspace_%s",fitInfo.Text_.Data()),Form("workspace_%s",fitInfo.Text_.Data()));

  Workspace->import(*ModelPdf);
  Workspace->import(*RooDHMap.at(Data));

  Workspace->writeToFile(fOutFileName.Data());
}

void Fitter::InitConfig()
{
  Config::SetupHistNames();
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
