#include "../Common.cpp+"

// ROOT includes
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"

// c++ includes
#include <iostream>

void setDFromTemplate(TH2F * DataHist);
void predictC(TH2F * DataHist);
void predictG(TH2F * DataHist);
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
  setDFromTemplate(DataHist);
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

void setDFromTemplate(TH2F * DataHist)
{
  // get input file
  const TString tmplfilename = Form("%s/%s/%s/met_vs_time_templates.root",Common::eosPreFix.Data(),Common::eosDir.Data(),Common::calibDir.Data());
  auto tmplfile = TFile::Open(tmplfilename.Data());
  Common::CheckValidFile(tmplfile,tmplfilename);
  
  // get template hist
  const TString tmplhistname = "Data_Hist_Plotted_Time_for_MET_0_to_200";
  auto TmplHist = (TH2F*)tmplfile->Get(tmplhistname.Data());
  Common::CheckValidHist(TmplHist,tmplhistname,tmplfilename);

  // integral of A : include left edge, do not include right edge
  const auto frac_A = TmplHist->Integral(TmplHist->GetXaxis()->FindBin(DataHist->GetXaxis()->GetBinLowEdge(1)),
					 TmplHist->GetXaxis()->FindBin(DataHist->GetXaxis()->GetBinUpEdge (1))-1);

  // set datahist
  const auto data_D = DataHist->GetBinContent(1,1)*((1.0-frac_A)/frac_A);
  DataHist->SetBinContent(2,1,data_D);
  DataHist->SetBinError  (2,1,std::sqrt(data_D));
}

void predictCorner(TH2F * DataHist, const Int_t i)
{
  // TR = TL * BR / BL (T = Top, B = Bottom, L = Left, R = Right)
  const auto data_BL = DataHist->GetBinContent(1,1);
  const auto data_TL = DataHist->GetBinContent(1,i);
  const auto data_BR = DataHist->GetBinContent(i,1);

  // predictions
  auto data_TR = (data_TL*data_BR)/data_BL;
  auto data_TR_unc = data_TR*std::sqrt(std::pow(DataHist->GetBinError(1,1)/data_BL,2)+
				       std::pow(DataHist->GetBinError(1,i)/data_TL,2)+
				       std::pow(DataHist->GetBinError(i,1)/data_BR,2)
				       );
  
  // ensure no nans
  setNaN(data_TR);
  setNaN(data_TR_unc);

  // set the I bin
  DataHist->SetBinContent(i,i,data_TR);
  DataHist->SetBinError  (i,i,data_TR_unc);
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

void predictG(TH2F * DataHist)
{
  // G = E*I / A
  const auto data_A = DataHist->GetBinContent(1,1);
  const auto data_E = DataHist->GetBinContent(1,3);
  const auto data_I = DataHist->GetBinContent(3,1);

  // predictions
  auto data_G = (data_E*data_I)/data_A;
  auto data_G_unc = data_G*std::sqrt(std::pow(DataHist->GetBinError(1,1)/data_A,2)+
				     std::pow(DataHist->GetBinError(1,3)/data_E,2)+
				     std::pow(DataHist->GetBinError(3,1)/data_I,2)
				     );
  
  // ensure no nans
  setNaN(data_G);
  setNaN(data_G_unc);

  // set the I bin
  DataHist->SetBinContent(3,3,data_G);
  DataHist->SetBinError  (3,3,data_G_unc);
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
  const auto bkgd_I = std::max(DataHist->GetBinContent(3,1) - SignHist->GetBinContent(3,1),0.0);

  // set params
  auto in_bkgA = bkgd_A;
  auto in_c1 = bkgd_B / bkgd_A;
  auto in_c2 = bkgd_D / bkgd_A;
  auto in_c3 = bkgd_E / bkgd_B;
  auto in_c4 = bkgd_I / bkgd_D;

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
  outfile << Form("%s_G",label.Data()) << " " << hist->GetBinContent(3,3) << std::endl;
  outfile << Form("%s_H",label.Data()) << " " << hist->GetBinContent(3,2) << std::endl;
  outfile << Form("%s_I",label.Data()) << " " << hist->GetBinContent(3,1) << std::endl;
  outfile << std::endl;
}

