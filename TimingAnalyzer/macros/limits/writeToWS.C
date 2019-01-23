#include "../Common.cpp+"

// ROOT includes
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"

// c++ includes
#include <iostream>

void fillConstants(const TH2F * hist, const TString & label, std::ofstream & output);
void makeWS(const TH2F * hist, TFile * iofile, const TString & ws_name);

void writeToWS(const TString & inlimitdir, const TString & ws_filename, const TString & ws_name,
	       const TString & tmplog_filename, const TString & sample)
{
  // setup config
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
  Common::RemoveGroup(SampleGroup::isBkgd);

  // get input file
  const auto iofilename = inlimitdir+"/"+ws_filename;
  auto iofile = TFile::Open(iofilename.Data(),"UPDATE");
  Common::CheckValidFile(iofile,iofilename);
  
  // get data hist
  const auto datahistname = sample+"_"+Common::HistNameMap["Data"]+"_Plotted";
  auto DataHist = (TH2F*)iofile->Get(datahistname.Data());
  Common::CheckValidHist(DataHist,datahistname,iofilename);

  // get signal hist
  const auto signhistname = Common::HistNameMap[sample];
  auto SignHist = (TH2F*)iofile->Get(signhistname.Data());
  Common::CheckValidHist(SignHist,signhistname,iofilename);

  // open output file
  std::ofstream output(tmplog_filename.Data(),std::ios::trunc);
  
  // fill data + Make WS
  fillConstants(DataHist,"BIN",output);
  makeWS(DataHist,iofile,ws_name);

  // fill sig
  fillConstants(SignHist,"SIG",output);

  // delete it all
  delete SignHist;
  delete DataHist;
  delete iofile;
}

void fillConstants(const TH2F * hist, const TString & label, std::ofstream & output)
{
  // get constants
  const auto A = hist->GetBinContent(1,1);
  const auto B = hist->GetBinContent(1,2);
  const auto C = hist->GetBinContent(2,2);
  const auto D = hist->GetBinContent(2,1);

  const auto Aunc = hist->GetBinError(1,1);
  const auto Bunc = hist->GetBinError(1,2);
  const auto Cunc = hist->GetBinError(2,2);
  const auto Dunc = hist->GetBinError(2,1);

  const auto BovA     = B/A;
  const auto BovAunc  = BovA*std::sqrt(std::pow(Bunc/B,2)+std::pow(Aunc/A,2));

  const auto DovA     = D/A;
  const auto DovAunc  = DovA*std::sqrt(std::pow(Dunc/D,2)+std::pow(Aunc/A,2));

  const auto predC    = B*D/A;
  const auto predCunc = predC*std::sqrt(std::pow(Bunc/B,2)+std::pow(Dunc/D,2)+std::pow(Aunc/A,2));

  // write to output file
  output << Form("%s_A ",label.Data()) << A << " +/- " << Aunc << std::endl;
  output << Form("%s_B ",label.Data()) << B << " +/- " << Bunc << std::endl;
  output << Form("%s_C ",label.Data()) << C << " +/- " << Cunc << std::endl;
  output << Form("%s_D ",label.Data()) << D << " +/- " << Dunc << std::endl;
  output << "----------------------------" << std::endl;
  output << "BovA " << BovA << " +/- " << BovAunc << std::endl;
  output << "DovA " << DovA << " +/- " << DovAunc << std::endl;
  output << "predC " << predC << " +/- " << predCunc << std::endl;
  output << std::endl;
}

void makeWS(const TH2F * hist, TFile * iofile, const TString & ws_name)
{
  // make WS
  iofile->cd();
  auto workspace = new RooWorkspace(ws_name.Data(),ws_name.Data());
  
  // get constants
  auto in_bkgA = hist->GetBinContent(1,1);
  auto in_BovA = hist->GetBinContent(1,2) / in_bkgA;
  auto in_DovA = hist->GetBinContent(2,1) / in_bkgA;

  // fill WS
  RooRealVar bkgA("bkgA", "bkgA", in_bkgA, 0.0, 2.*in_bkgA);
  RooRealVar BovA("BovA", "BovA", in_BovA, 0.0, 1.);
  RooRealVar DovA("DovA", "DovA", in_DovA, 0.0, 1.);
  
  // import vars into WS
  workspace->import(bkgA);
  workspace->import(BovA);
  workspace->import(DovA);
  
  // save file
  iofile->cd();
  workspace->Write(workspace->GetName(),TObject::kWriteDelete);

  // delete it all
  delete workspace;
}
