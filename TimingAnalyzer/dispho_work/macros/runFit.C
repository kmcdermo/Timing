#include "common/Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TH2F.h"

#include <map>

void runFit(const TString & infilename)
{
  // Get valid input
  TFile * infile = TFile::Open(Form("%s",infilename.Data()));
  Config::CheckValidFile(infile,infilename);

  // Get hist names loaded
  Config::SetupHistNames();
  
  // Get the input 2D histograms
  std::map<SampleType,TH2F*> HistMap;
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    const auto & sample = HistNamePair.first;
    const TString histname = HistNamePair.second;
    HistMap[sample] = (TH2F*)infile->Get(Form("%s",histname.Data())); 
    Config::CheckValidTH2F(HistMap[sample],histname,infilename);
  }
  
  // Get mins and maxes
  const Float_t xmin = HistMap[Data]->GetXaxis()->GetBinLowEdge(1);
  const Float_t xmax = HistMap[Data]->GetXaxis()->GetBinUpEdge(HistMap[Data]->GetXaxis()->GetNbins());
  const Float_t ymin = HistMap[Data]->GetYaxis()->GetBinLowEdge(1);
  const Float_t ymax = HistMap[Data]->GetYaxis()->GetBinUpEdge(HistMap[Data]->GetYaxis()->GetNbins());
  
  // declare variable of interest
  RooRealVar phoseedtime_0("phoseedtime_0","phoseedtime_0",xmin,xmax);
  RooRealVar t1pfMETpt    ("t1pfMETpt"    ,"t1pfMETpt"    ,ymin,ymax);

  // Declare datasets with input histograms
  std::map<SampleType,RooDataHist*> RooDHMap;
  for (const auto & HistPair : HistMap)
  {
    const auto & sample = HistPair.first;
    const TString name = Form("%s_RooDataHist",Config::HistNameMap[sample].Data());
    RooDHMap[sample] = new RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(phoseedtime_0,t1pfMETpt),HistPair.second);
  }

  // make pdfs from histograms
  std::map<SampleType,RooHistPdf*> RooHPdfMap;
  for (auto & RooDHPair : RooDHMap)
  {
    const auto & sample = RooDHPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_PDF",Config::HistNameMap[sample].Data());
    RooHPdfMap[sample] = new RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(phoseedtime_0,t1pfMETpt),*RooDHPair.second);
  }

  // fractions
  std::map<SampleType,RooRealVar*> FracMap;
  for (auto & RooHPdfPair : RooHPdfMap)
  {
    const auto & sample = RooHPdfPair.first;
    if (sample == Data) continue;
    if (sample == GMSB) continue;
    
    const TString name = Form("%s_frac",Config::HistNameMap[sample].Data());
    FracMap[sample] = new RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),0.5,0,1);
  }

  // Fit Model
  RooAddPdf modelpdf("modelpdf","modelpdf",RooArgList(*RooHPdfMap[GJets],*RooHPdfMap[QCD]),RooArgList(*FracMap[GJets],*FracMap[QCD]));
  modelpdf.fitTo(*RooDHMap[Data]);

  // Draw 1D stuff
  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogy();

  // first 1D in x
  RooPlot * xframe = phoseedtime_0.frame();
  RooDHMap[Data]->plotOn(xframe);
  modelpdf.plotOn(xframe);
  xframe->Draw();
  canv->SaveAs("xframe.png");

  // second 1D in y
  RooPlot * yframe = t1pfMETpt.frame();
  RooDHMap[Data]->plotOn(yframe);
  modelpdf.plotOn(yframe);
  yframe->Draw();
  canv->SaveAs("yframe.png");
  
  // delete it all
  delete yframe;
  delete xframe;
  delete canv;

  for (auto & RooDHPair   : RooDHMap  ) delete RooDHPair.second;
  for (auto & RooHPdfPair : RooHPdfMap) delete RooHPdfPair.second;
  for (auto & FracPair    : FracMap   ) delete FracPair.second;
  for (auto & HistPair    : HistMap   ) delete HistPair.second;

  delete infile;
}
