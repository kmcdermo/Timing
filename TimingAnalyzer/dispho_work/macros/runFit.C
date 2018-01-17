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
  
  // Get the hists
  std::map<SampleType,TH2F*> HistMap;
  for (const auto & HistNamePair : Config::HistNameMap)
  {
    const TString histname = HistNamePair.second;
    HistMap[HistNamePair.first] = (TH2F*)infile->Get(Form("%s",histname.Data())); 
    Config::CheckValidTH2F(HistMap[HistNamePair.first],histname,infilename);
  }
  
  // Get mins and maxes
  const Float_t xmin = HistMap[Data]->GetXaxis()->GetBinLowEdge(1);
  const Float_t xmax = HistMap[Data]->GetXaxis()->GetBinUpEdge(HistMap[Data]->GetXaxis()->GetNbins());
  const Float_t ymin = HistMap[Data]->GetYaxis()->GetBinLowEdge(1);
  const Float_t ymax = HistMap[Data]->GetYaxis()->GetBinUpEdge(HistMap[Data]->GetYaxis()->GetNbins());
  
  // declare variable of interest
  RooRealVar phoseedtime_0("phoseedtime_0","phoseedtime_0",xmin,xmax);
  RooRealVar t1pfMETpt    ("t1pfMETpt"    ,"t1pfMETpt"    ,ymin,ymax);

  // Declare datasets
  std::map<SampleType,RooDataHist> RooDHMap;
  for (const auto & HistPair : HistMap)
  {
    const TString name = Form("%s_RooDataHist",Config::HistNameMap[HistPair.first].Data());
    RooDHMap[HistPair.first] = RooDataHist(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(phoseedtime_0,t1pfMETpt),HistPair.second);
  }

  // make pdfs from histograms
  std::map<SampleType,RooHistPdf> RooHPdfMap;
  for (const auto & RooDHPair : RooDHMap)
  {
    if (RooDHPair.first == Data) continue;
    if (RooDHPair.first == GMSB) continue;
    
    const TString name = Form("%s_PDF",Config::HistNameMap[RooDHPair.first].Data());
    RooHPdfMap[RooDHPair.first] = RooHistPdf(Form("%s",name.Data()),Form("%s",name.Data()),RooArgList(phoseedtime_0,t1pfMETpt),RooDHPair.second);
  }

  // fractions
  std::map<SampleType,RooRealVar> FracMap;
  for (const auto & RooHPdfPair : RooHPdfMap)
  {
    if (RooHPdfPair.first == Data) continue;
    if (RooHPdfPair.first == GMSB) continue;
    
    const TString name = Form("%s_frac",Config::HistNameMap[RooHPdfPair.first].Data());
    FracMap[RooHPdfPair.first] = RooRealVar(Form("%s",name.Data()),Form("%s",name.Data()),0.5,0,1);
  }

  // FitModel
  RooAddPdf modelpdf("modelpdf","modelpdf",RooArgList(RooHPdfMap[GJets],RooHPdfMap[QCD]),RooArgList(FracMap[GJets],FracMap[QCD]));
  modelpdf.fitTo(RooDHMap[Data]);

  // delete it all
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete infile;
}


