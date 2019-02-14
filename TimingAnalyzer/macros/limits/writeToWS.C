#include "../Common.cpp+"

// ROOT includes
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"

// c++ includes
#include <iostream>

void predictC(TH2F * DataHist);
void predictI(TH2F * DataHist);
void makeWS2x2(const TH2F * DataHist, const TH2F * SignHist, const TString & outfile_name, const TString & ws_name);
void makeWS3x3(const TH2F * DataHist, const TH2F * SignHist, const TString & outfile_name, const TString & ws_name);
void fillLog2x2(const TH2F * hist, const TString & label, std::ofstream & outfile);
void fillLog3x3(const TH2F * hist, const TString & label, std::ofstream & outfile);

void writeToWS(const TString & inlimitdir, const TString & ws_filename, const TString & ws_name, const TString & sample, const TString & logfilename, const TString & outfile_name)
{
  // setup config
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
  Common::RemoveGroup(SampleGroup::isBkgd);

  // get input file
  const auto infilename = inlimitdir+"/"+ws_filename;
  auto infile = TFile::Open(infilename.Data());
  Common::CheckValidFile(infile,infilename);
  
  // get data hist
  const auto datahistname = Common::HistNameMap["Data"]+"_Plotted";
  auto DataHist = (TH2F*)infile->Get(datahistname.Data());
  Common::CheckValidHist(DataHist,datahistname,infilename);

  // get signal hist
  const auto signhistname = Common::HistNameMap[sample];
  auto SignHist = (TH2F*)infile->Get(signhistname.Data());
  Common::CheckValidHist(SignHist,signhistname,infilename);

  // fill data + Make WS
  predictC(DataHist);
  makeWS2x2(DataHist,SignHist,outfile_name,ws_name);
  //  predictI(DataHist);
  //  makeWS3x3(DataHist,SignHist,outfile_name,ws_name);

  // write to tmp log file
  ofstream outfile(logfilename.Data(),std::ios::trunc);
  fillLog2x2(DataHist,"BIN",outfile);
  fillLog2x2(SignHist,"SIG",outfile);
  //  fillLog3x3(DataHist,"BIN",outfile);
  //  fillLog3x3(SignHist,"SIG",outfile);

  // delete it all
  delete SignHist;
  delete DataHist;
  delete infile;
}

void setNaN(Double_t & c)
{
  if (std::isnan(c)) c = 0.0;
}

void predictC(TH2F * DataHist)
{
  // C = B*D / A
  const auto data_A = DataHist->GetBinContent(1,1);
  const auto data_B = DataHist->GetBinContent(1,2);
  const auto data_D = DataHist->GetBinContent(2,1);

  // predictions
  auto data_C = (data_B*data_D)/data_A;
  auto data_C_unc = data_C*std::sqrt(std::pow(DataHist->GetBinError(1,1)/data_A,2)+
				     std::pow(DataHist->GetBinError(1,2)/data_B,2)+
				     std::pow(DataHist->GetBinError(2,1)/data_D,2)
				     );
  
  // ensure no nans
  setNaN(data_C);
  setNaN(data_C_unc);

  // set the I bin
  DataHist->SetBinContent(2,2,data_C);
  DataHist->SetBinError  (2,2,data_C_unc);
}

void predictI(TH2F * DataHist)
{
  // I = E*G / A
  const auto data_A = DataHist->GetBinContent(1,1);
  const auto data_E = DataHist->GetBinContent(1,3);
  const auto data_G = DataHist->GetBinContent(3,1);

  // predictions
  auto data_I = (data_E*data_G)/data_A;
  auto data_I_unc = data_I*std::sqrt(std::pow(DataHist->GetBinError(1,1)/data_A,2)+
				     std::pow(DataHist->GetBinError(1,3)/data_E,2)+
				     std::pow(DataHist->GetBinError(3,1)/data_G,2)
				     );
  
  // ensure no nans
  setNaN(data_I);
  setNaN(data_I_unc);

  // set the I bin
  DataHist->SetBinContent(3,3,data_I);
  DataHist->SetBinError  (3,3,data_I_unc);
}

void makeWS2x2(const TH2F * DataHist, const TH2F * SignHist, const TString & outfile_name, const TString & ws_name)
{
  // make file + WS
  auto outfile = TFile::Open(outfile_name.Data(),"UPDATE");
  outfile->cd();

  auto workspace = new RooWorkspace(ws_name.Data(),ws_name.Data());
  
  // get constants
  const auto bkgd_A = std::max(DataHist->GetBinContent(1,1) - SignHist->GetBinContent(1,1),0.0);

  const auto bkgd_B = std::max(DataHist->GetBinContent(1,2) - SignHist->GetBinContent(1,2),0.0);
  const auto bkgd_D = std::max(DataHist->GetBinContent(2,1) - SignHist->GetBinContent(2,1),0.0);

  // set params
  auto in_bkgA = bkgd_A;
  auto in_c1 = bkgd_B / bkgd_A;
  auto in_c2 = bkgd_D / bkgd_A;

  setNaN(in_c1);
  setNaN(in_c2);
  
  // fill WS
  RooRealVar bkgA("bkgA", "bkgA", in_bkgA, 0.0, 10.0*in_bkgA);
  RooRealVar c1("c1", "c1", in_c1, 0.0, 10.0);
  RooRealVar c2("c2", "c2", in_c2, 0.0, 10.0);
  
  // import vars into WS
  workspace->import(bkgA);
  workspace->import(c1);
  workspace->import(c2);
  
  // save file
  outfile->cd();
  workspace->Write(workspace->GetName(),TObject::kWriteDelete);

  // delete it all
  delete workspace;
  delete outfile;
}

void makeWS3x3(const TH2F * DataHist, const TH2F * SignHist, const TString & outfile_name, const TString & ws_name)
{
  // make file + WS
  auto outfile = TFile::Open(outfile_name.Data(),"UPDATE");
  outfile->cd();

  auto workspace = new RooWorkspace(ws_name.Data(),ws_name.Data());
  
  // get constants
  const auto bkgd_A = std::max(DataHist->GetBinContent(1,1) - SignHist->GetBinContent(1,1),0.0);

  const auto bkgd_B = std::max(DataHist->GetBinContent(1,2) - SignHist->GetBinContent(1,2),0.0);
  const auto bkgd_E = std::max(DataHist->GetBinContent(1,3) - SignHist->GetBinContent(1,3),0.0);

  const auto bkgd_D = std::max(DataHist->GetBinContent(2,1) - SignHist->GetBinContent(2,1),0.0);
  const auto bkgd_G = std::max(DataHist->GetBinContent(3,1) - SignHist->GetBinContent(3,1),0.0);

  // set params
  auto in_bkgA = bkgd_A;
  auto in_c1 = bkgd_B / bkgd_A;
  auto in_c2 = bkgd_D / bkgd_A;
  auto in_c3 = bkgd_E / bkgd_B;
  auto in_c4 = bkgd_G / bkgd_D;

  setNaN(in_c1);
  setNaN(in_c2);
  setNaN(in_c3);
  setNaN(in_c4);
  
  // fill WS
  RooRealVar bkgA("bkgA", "bkgA", in_bkgA, 0.0, 10.0*in_bkgA);
  RooRealVar c1("c1", "c1", in_c1, 0.0, 10.0);
  RooRealVar c2("c2", "c2", in_c2, 0.0, 10.0);
  RooRealVar c3("c3", "c3", in_c3, 0.0, 10.0);
  RooRealVar c4("c4", "c4", in_c4, 0.0, 10.0);
  
  // import vars into WS
  workspace->import(bkgA);
  workspace->import(c1);
  workspace->import(c2);
  workspace->import(c3);
  workspace->import(c4);
  
  // save file
  outfile->cd();
  workspace->Write(workspace->GetName(),TObject::kWriteDelete);

  // delete it all
  delete workspace;
  delete outfile;
}

void fillLog2x2(const TH2F * hist, const TString & label, std::ofstream & outfile)
{
  outfile << Form("%s_A",label.Data()) << " " << hist->GetBinContent(1,1) << std::endl;
  outfile << Form("%s_B",label.Data()) << " " << hist->GetBinContent(1,2) << std::endl;
  outfile << Form("%s_C",label.Data()) << " " << hist->GetBinContent(2,2) << std::endl;
  outfile << Form("%s_D",label.Data()) << " " << hist->GetBinContent(2,1) << std::endl;
  outfile << std::endl;
}

void fillLog3x3(const TH2F * hist, const TString & label, std::ofstream & outfile)
{
  outfile << Form("%s_A",label.Data()) << " " << hist->GetBinContent(1,1) << std::endl;
  outfile << Form("%s_B",label.Data()) << " " << hist->GetBinContent(1,2) << std::endl;
  outfile << Form("%s_C",label.Data()) << " " << hist->GetBinContent(2,2) << std::endl;
  outfile << Form("%s_D",label.Data()) << " " << hist->GetBinContent(2,1) << std::endl;
  outfile << Form("%s_E",label.Data()) << " " << hist->GetBinContent(1,3) << std::endl;
  outfile << Form("%s_F",label.Data()) << " " << hist->GetBinContent(2,3) << std::endl;
  outfile << Form("%s_G",label.Data()) << " " << hist->GetBinContent(3,1) << std::endl;
  outfile << Form("%s_H",label.Data()) << " " << hist->GetBinContent(3,2) << std::endl;
  outfile << Form("%s_I",label.Data()) << " " << hist->GetBinContent(3,3) << std::endl;
  outfile << std::endl;
}

