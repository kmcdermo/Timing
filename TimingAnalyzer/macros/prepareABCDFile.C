#include "Common.cpp+"

// ROOT includes
#include "TString.h"
#include "TFile.h"
#include "TH2F.h"
#include "TPaveText.h"

// c++ includes
#include <iostream>
#include <vector>
#include <map>

// tmp struct
struct SigStruct
{
  SigStruct() {}
  SigStruct(const Double_t significance, const Double_t xbin_boundary, const Double_t ybin_boundary, const TString & infilename) 
    : significance(significance), xbin_boundary(xbin_boundary), ybin_boundary(ybin_boundary), infilename(infilename) {}
  
  Double_t significance;
  Double_t xbin_boundary;
  Double_t ybin_boundary;
  TString infilename;
};

void prepareABCDFile(const TString & signif_dump, const TString & signif_list, const TString & ws_filename)
{
  // setup config
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
  Common::RemoveGroup(SampleGroup::isBkgd);

  // dump sig info into map
  std::map<TString,std::vector<SigStruct> > SigMap;

  // read dump into map
  std::ifstream infile(signif_dump.Data(),std::ios::in);
  TString sample, significance, xbin_boundary, ybin_boundary, infilename;

  while (infile >> sample >> significance >> xbin_boundary >> ybin_boundary >> infilename)
  {
    // skip all NaN entries
    if (significance.EqualTo("nan")) continue;

    // pack into map
    SigMap[sample].emplace_back(significance.Atof(),xbin_boundary.Atof(),ybin_boundary.Atof(),infilename);
  }

  // write list into map
  std::ofstream outfile(signif_list.Data(),std::ios::trunc);
    
  // get output file
  auto ws_file = TFile::Open(ws_filename.Data(),"RECREATE");
  Common::CheckValidFile(ws_file,ws_filename);

  // save best histograms per sample 
  for (auto & SigPair : SigMap)
  {
    // get inputs
    const auto & sample = SigPair.first;
    auto & SigStructVec = SigPair.second;

    // sort samples by significance
    std::sort(SigStructVec.begin(),SigStructVec.end(),
	      [](const auto & SigStruct1, const auto & SigStruct2)
	      {
		if (SigStruct1.significance != SigStruct2.significance) return SigStruct1.significance > SigStruct2.significance; 
		else 
		{
		  if (SigStruct1.xbin_boundary == SigStruct2.xbin_boundary) return SigStruct1.ybin_boundary < SigStruct2.ybin_boundary;
		  else return SigStruct1.xbin_boundary < SigStruct2.xbin_boundary;
		}
	      });

    // get best one : write to signif_list
    const auto & BestSig = SigStructVec.front();
    const auto & tmpfilename = BestSig.infilename;
    
    outfile << sample.Data() << " " << std::setprecision(15) << BestSig.significance << " "
	    << BestSig.xbin_boundary << " " << BestSig.ybin_boundary << " "
	    << tmpfilename.Data() << std::endl;

    // open best file and write hists + config to final file
    auto tmpfile = TFile::Open(tmpfilename.Data());
    Common::CheckValidFile(tmpfile,tmpfilename);
    tmpfile->cd();

    // get data hist + rename
    const auto datahistname = Common::HistNameMap["Data"]+"_Plotted";
    auto DataHist = (TH2F*)tmpfile->Get(datahistname.Data());
    Common::CheckValidHist(DataHist,datahistname,tmpfilename);
    DataHist->SetName(sample+"_"+datahistname);

    // get signal hist + rename
    const auto signhistname = Common::HistNameMap[sample];
    auto SignHist = (TH2F*)tmpfile->Get(signhistname.Data());
    Common::CheckValidHist(SignHist,signhistname,tmpfilename);
    
    // get pave text
    auto ConfigPave = (TPaveText*)tmpfile->Get(Form("%s",Common::pavename.Data()));
    ConfigPave->SetName(sample+"_"+Common::pavename);

    // now save it all to output file
    ws_file->cd();
    DataHist->Write(DataHist->GetName(),TObject::kWriteDelete);
    SignHist->Write(SignHist->GetName(),TObject::kWriteDelete);
    ConfigPave->Write(ConfigPave->GetName(),TObject::kWriteDelete);
    
    // delete it all
    delete ConfigPave;
    delete SignHist;
    delete DataHist;
    delete tmpfile;
  }

  // delete the rest
  delete ws_file;
}
