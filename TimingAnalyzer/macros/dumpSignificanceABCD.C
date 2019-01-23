#include "Common.cpp+"

// ROOT includes
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"

// c++ includes
#include <iostream>

inline Double_t ComputeSignificance(const Double_t nbkgd, const Double_t nsign)
{
  if (nbkgd > 0.0) return nsign / std::sqrt(std::pow(nsign,2)+std::pow(nbkgd,2));
  else return std::nan("");
}

void dumpSignificanceABCD(const TString & iofilename, const TString & xbin_boundary, const TString & ybin_boundary,
			  const Bool_t blind_data, const TString & signif_dump)
{
  // setup config
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
  Common::RemoveGroup(SampleGroup::isBkgd);

  // open input file
  auto iofile = TFile::Open(iofilename.Data(),(blind_data?"UPDATE":"READ"));
  Common::CheckValidFile(iofile,iofilename);
  iofile->cd();

  // grab universal data hist
  const auto datahistname = Common::HistNameMap["Data"]+"_Plotted";
  auto DataHist = (TH2F*)iofile->Get(datahistname.Data());
  Common::CheckValidHist(DataHist,datahistname,iofilename);
  
  // get ABD data contributions
  const auto data_A = DataHist->GetBinContent(1,1);
  const auto data_B = DataHist->GetBinContent(1,2);
  const auto data_D = DataHist->GetBinContent(2,1);

  // get data in C (either prediction or observed)
  const auto data_C = (blind_data ? ((data_B*data_D)/data_A) : (DataHist->GetBinContent(2,2)));
  
  // set C bin if blinded
  if (blind_data)
  {
    // get data unc
    const auto data_A_unc = DataHist->GetBinError(1,1);
    const auto data_B_unc = DataHist->GetBinError(1,2);
    const auto data_D_unc = DataHist->GetBinError(2,1);

    // compute C unc
    const auto data_C_unc = data_C*std::sqrt(std::pow(data_B_unc/data_B,2)+std::pow(data_D_unc/data_D,2)+std::pow(data_A_unc/data_A,2));

    // set the C bin
    DataHist->SetBinContent(2,2,data_C);
    DataHist->SetBinError  (2,2,data_C_unc);

    // write it back to the file
    DataHist->Write(DataHist->GetName(),TObject::kWriteDelete);
  }

  // open output file: append it!
  std::ofstream outfile(signif_dump.Data(),std::ios::app);
  
  // loop over hist names (skip data and no bkgd mc), compute significance and save it
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    // get pair info
    const auto & sample       = HistNamePair.first;
    const auto & signhistname = HistNamePair.second;

    // skip data
    if (Common::GroupMap[sample] != SampleGroup::isSignal) continue;

    // get histogram
    auto SignHist = (TH2F*)iofile->Get(signhistname.Data());
    Common::CheckValidHist(SignHist,signhistname,iofilename);

    // get ABCD sign contributions
    const auto sign_A = SignHist->GetBinContent(1,1);
    const auto sign_B = SignHist->GetBinContent(1,2);
    const auto sign_C = SignHist->GetBinContent(2,2);
    const auto sign_D = SignHist->GetBinContent(2,1);
    
    // assume signal strength of 1, so subtract signal from data for bkgd
    const auto bkgd_A = data_A-sign_A;
    const auto bkgd_B = data_B-sign_B;
    const auto bkgd_C = data_C-sign_C;
    const auto bkgd_D = data_D-sign_D;

    // compute significance in C
    const auto significance = ComputeSignificance(bkgd_C,sign_C);

    // write to output file
    outfile << sample.Data() << " " << std::setprecision(15) << significance << " "
	    << xbin_boundary.Data() << " " << ybin_boundary.Data() << " "
	    << iofilename.Data() << std::endl;

    // delete it all
    delete SignHist;
  }

  // for readability sake, add space
  outfile << std::endl;
  
  // delete it all
  delete DataHist;
  delete iofile;
}
