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
  Fitter::DeleteMap(RooDHMap2D);
  Fitter::DeleteMap(RooHPdfMap2D);
  Fitter::DeleteMap(FracMap2D);
  Fitter::DeleteMap(HistMap2D);
  
  delete fY;
  delete fX;

  delete fInFile;

  delete Workspace2D;
  delete fOutFile;
}

void Fitter::MakeFit()
{
  // Get all the variables in place
  Fitter::PrepareFits();
  
  // Do the 2D fit
  Fitter::Fit2D();
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

void Fitter::Fit2D()
{
  // Common vars
  const RooArgList arglist2D(*fX,*fY);
  const TString text2D   = "2D";
  const TString plottext = "projfit";

  // Declare datasets with input histograms
  Fitter::DeclareDatasets(HistMap2D,RooDHMap2D,arglist2D,text2D);

  // Make pdfs from histograms
  Fitter::MakeSamplePdfs(RooDHMap2D,RooHPdfMap2D,arglist2D,text2D);

  // Declare fractions for pdfs
  Fitter::DeclareFractions(RooHPdfMap2D,FracMap2D,text2D);

  // Fit Model
  Fitter::FitModel(ModelPdf2D,RooHPdfMap2D,FracMap2D,RooDHMap2D,text2D);

  // Draw fit in 1D projections
  Fitter::DrawFit(fX,RooDHMap2D,ModelPdf2D,Form("xframe_%s",plottext.Data()));
  Fitter::DrawFit(fY,RooDHMap2D,ModelPdf2D,Form("yframe_%s",plottext.Data()));

  // Save in a workspace
  Fitter::ImportToWS(Workspace2D,ModelPdf2D,RooDHMap2D,text2D);
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
void Fitter::DeclareDatasets(const T & HistMap, RDHMap & RooDHMap, const RooArgList & ArgList, const TString & text)
{
  std::cout << "Setting datasets..." << std::endl;
  
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const TString name = Form("%s_RooDataHist_%s",Config::HistNameMap[sample].Data(),text.Data());
    RooDHMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),ArgList,HistPair.second);
  }
}

void Fitter::MakeSamplePdfs(const RDHMap & RooDHMap, RHPMap & RooHPdfMap, const RooArgList & ArgList, const TString & text)
{
  std::cout << "Setting Pdfs..." << std::endl;

  for (const auto & RooDHPair : RooDHMap)
  {
    const auto & sample = RooDHPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_PDF_%s",Config::HistNameMap[sample].Data(),text.Data());
    RooHPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(*fX,*fY),*RooDHPair.second);
  }
}

void Fitter::DeclareFractions(const RHPMap & RooHPdfMap, RRVMap & FracMap, const TString & text)
{
  std::cout << "Init fractions..." << std::endl;

  for (const auto & RooHPdfPair : RooHPdfMap)
  {
    const auto & sample = RooHPdfPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_frac_%s",Config::HistNameMap[sample].Data(),text.Data());
    FracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),0.5,0,1);
  }
}

void Fitter::FitModel(RooAddPdf *& ModelPdf, const RHPMap & RooHPdfMap, const RRVMap & FracMap, const RDHMap & RooDHMap, const TString & text)
{
  std::cout << "Build and fit to model..." << std::endl;

  const TString name = Form("modelpdf_%s",text.Data());
  ModelPdf = new RooAddPdf(Form("%s",name.Data()),"modelpdf",RooArgList(*RooHPdfMap.at(GJets),*RooHPdfMap.at(QCD)),RooArgList(*FracMap.at(GJets),*FracMap.at(QCD)));
  ModelPdf->fitTo(*RooDHMap.at(Data));
}

void Fitter::DrawFit(RooRealVar *& var, const RDHMap & RooDHMap, RooAddPdf *& ModelPdf, const TString & text)
{
  std::cout << "Draw fits projected into 1D..." << std::endl;

  // Get Canvas
  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogy();

  // Draw 1D stuff
  RooPlot * frame = var->frame();
  RooDHMap.at(Data)->plotOn(frame);
  ModelPdf->plotOn(frame);
  frame->Draw();
  canv->SaveAs(Form("%s.png",text.Data()));

  // delete it
  delete frame;
  delete canv;
}

void Fitter::ImportToWS(RooWorkspace *& Workspace, RooAddPdf *& ModelPdf, const RDHMap & RooDHMap, const TString & text)
{
  std::cout << "Make workspace..." << std::endl;

  Workspace = new RooWorkspace(Form("workspace_%s",text.Data()),Form("workspace_%s",text.Data()));

  Workspace->import(*ModelPdf);
  Workspace->import(*RooDHMap.at(Data));

  Workspace->writeToFile(fOutFileName.Data());
}

void Fitter::InitConfig()
{
  Config::SetupHistNames();
}

template <typename T>
void Fitter::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
  Map.clear();			  
}
