// Class include
#include "Fitter2D.hh"

// ROOT+RooFit includes
#include "TVirtualFitter.h"
#include "RooPlot.h"

Fitter2D::Fitter2D(const TString & infilename, const TString & outfilename) 
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
  Fitter2D::InitConfig();
  TVirtualFitter::SetDefaultFitter("Minuit2");

  // get input root file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Config::CheckValidFile(fInFile,fInFileName);
  
  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s",fOutFileName.Data()),"RECREATE");
}

Fitter2D::~Fitter2D()
{
  for (auto & RooDHPair   : RooDHMap  ) delete RooDHPair.second;
  for (auto & RooHPdfPair : RooHPdfMap) delete RooHPdfPair.second;
  for (auto & FracPair    : FracMap   ) delete FracPair.second;
  
  delete fY;
  delete fX;

  for (auto & HistPair    : HistMap   ) delete HistPair.second;

  delete fInFile;

  delete fWorkspace;
  delete fOutFile;
}

void Fitter2D::MakeFit()
{
  // Get the input 2D histograms
  Fitter2D::GetInputHists();

  // Get mins and maxes
  Fitter2D::GetMinMax();

  // Declare variable of interest
  Fitter2D::DeclareVars();

  // Declare datasets with input histograms
  Fitter2D::DeclareDatasets();

  // Make pdfs from histograms
  Fitter2D::MakeSamplePdfs();

  // Declare fractions for pdfs
  Fitter2D::DeclareFractions();

  // Fit Model
  Fitter2D::FitModel();

  // Draw fit in 1D projections
  Fitter2D::DrawProjectedFits();

  // Save in a workspace
  Fitter2D::ImportToWS();
}

void Fitter2D::GetInputHists()
{
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    const auto & sample = HistNamePair.first;
    const TString histname = HistNamePair.second;
    HistMap[sample] = (TH2F*)fInFile->Get(Form("%s",histname.Data())); 
    Config::CheckValidTH2F(HistMap[sample],histname,fInFileName);
  }
}  

void Fitter2D::GetMinMax()
{
  fXmin = HistMap[Data]->GetXaxis()->GetBinLowEdge(1);
  fXmax = HistMap[Data]->GetXaxis()->GetBinUpEdge(HistMap[Data]->GetXaxis()->GetNbins());
  fYmin = HistMap[Data]->GetYaxis()->GetBinLowEdge(1);
  fYmax = HistMap[Data]->GetYaxis()->GetBinUpEdge(HistMap[Data]->GetYaxis()->GetNbins());
}

void Fitter2D::DeclareVars()
{
  fX = new RooRealVar("x",HistMap[Data]->GetXaxis()->GetTitle(),fXmin,fXmax);
  fY = new RooRealVar("y",HistMap[Data]->GetYaxis()->GetTitle(),fYmin,fYmax);
}
  
void Fitter2D::DeclareDatasets()
{
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const TString name = Form("%s_RooDataHist",Config::HistNameMap[sample].Data());
    RooDHMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(*fX,*fY),HistPair.second);
  }
}

void Fitter2D::MakeSamplePdfs()
{
  for (auto & RooDHPair : RooDHMap)
  {
    const auto & sample = RooDHPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_PDF",Config::HistNameMap[sample].Data());
    RooHPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(*fX,*fY),*RooDHPair.second);
  }
}

void Fitter2D::DeclareFractions()
{
  for (auto & RooHPdfPair : RooHPdfMap)
  {
    const auto & sample = RooHPdfPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_frac",Config::HistNameMap[sample].Data());
    FracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),0.5,0,1);
  }
}

void Fitter2D::FitModel()
{
  ModelPdf = new RooAddPdf("modelpdf","modelpdf",RooArgList(*RooHPdfMap[GJets],*RooHPdfMap[QCD]),RooArgList(*FracMap[GJets],*FracMap[QCD]));
  ModelPdf->fitTo(*RooDHMap[Data]);
}

void Fitter2D::DrawProjectedFits()
{
  // Draw 1D stuff
  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogy();

  // first 1D in x
  RooPlot * xframe = fX->frame();
  RooDHMap[Data]->plotOn(xframe);
  ModelPdf->plotOn(xframe);
  xframe->Draw();
  canv->SaveAs("xframe_projfit.png");

  // second 1D in y
  RooPlot * yframe = fY->frame();
  RooDHMap[Data]->plotOn(yframe);
  ModelPdf->plotOn(yframe);
  yframe->Draw();
  canv->SaveAs("yframe_projfit.png");

  // delete it
  delete yframe;
  delete xframe;
  delete canv;
}

void Fitter2D::ImportToWS()
{
  fWorkspace = new RooWorkspace("workspace","workspace");

  fWorkspace->import(*ModelPdf);
  fWorkspace->import(*RooDHMap[Data]);

  fWorkspace->writeToFile(fOutFileName.Data());
}

void Fitter2D::InitConfig()
{
  Config::SetupHistNames();
}
