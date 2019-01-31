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

    // get C sign contribution
    const auto sign_C = SignHist->GetBinContent(2,2);

    // compute bkgd in C
    auto bkgd_C = 0.0;
    if (blind_data)
    {
      // compute bkgd in ABD
      const auto bkgd_A = DataHist->GetBinContent(1,1) - SignHist->GetBinContent(1,1);
      const auto bkgd_B = DataHist->GetBinContent(1,2) - SignHist->GetBinContent(1,2);
      const auto bkgd_D = DataHist->GetBinContent(2,1) - SignHist->GetBinContent(2,1);

      // assume signal strength of 1, so subtract signal from data for bkgd
      bkgd_C = (bkgd_B*bkgd_D)/bkgd_A;
      
      // compute C unc
      const auto bkgd_C_unc = bkgd_C*std::sqrt(
					       ((std::pow(DataHist->GetBinError(1,1),2)+std::pow(SignHist->GetBinError(1,1),2))/std::pow(bkgd_A,2))+
					       ((std::pow(DataHist->GetBinError(1,2),2)+std::pow(SignHist->GetBinError(1,2),2))/std::pow(bkgd_B,2))+
					       ((std::pow(DataHist->GetBinError(2,1),2)+std::pow(SignHist->GetBinError(2,1),2))/std::pow(bkgd_D,2))
					       );

      // get signal C unc
      const auto sign_C_unc = SignHist->GetBinError(2,2);
      
      // "data" C
      const auto data_C = bkgd_C+sign_C;
      const auto data_C_unc = std::sqrt(std::pow(bkgd_C_unc,2)+std::pow(sign_C_unc,2));

      // set the C bin
      DataHist->SetBinContent(2,2,data_C);
      DataHist->SetBinError  (2,2,data_C_unc);
      
      // write it back to the file
      DataHist->Write(DataHist->GetName(),TObject::kWriteDelete);
    }
    else
    {
      // unblinded
      const auto data_C = DataHist->GetBinContent(2,2);
      bkgd_C = data_C-sign_C;
    }

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
