#include "Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TH2F.h"

#include <vector>
#include <map>
#include <iostream>
#include <iomanip>
#include <fstream>

struct BinInfo
{
  BinInfo() {}
  BinInfo(const TString & bin, const Int_t binX, const Int_t binY)
    : bin(bin), binX(binX), binY(binY) {}

  TString bin;
  Int_t binX;
  Int_t binY;
};

void readSampleInfo(const TString & scan_log, const TString & plotfiletext, const TString & nom2Ddir, const TString & unc2Ddir,
		    std::vector<TString> & samples, std::map<TString,TH2F*> & NomHistMap, std::map<TString,TH2F*> & UncHistMap);

void extractSignalDiffs(const TString & scan_log, const TString & plotfiletext, const TString & nom2Ddir, const TString & unc2Ddir, const TString & systuncname)
{
  // read sample info + hists
  std::vector<TString> samples;
  std::map<TString,TH2F*> NomHistMap, UncHistMap;
  readSampleInfo(scan_log,plotfiletext,nom2Ddir,unc2Ddir,samples,NomHistMap,UncHistMap);

  // make bin infos
  const std::vector<BinInfo> binInfos =
  {
    {"1",1,1},
    {"2",1,2},
    {"3",2,2},
    {"4",2,1},
  };

  // loop binInfos
  for (const auto & binInfo : binInfos)
  {
    // get bin info
    const auto binX = binInfo.binX;
    const auto binY = binInfo.binY;
    
    // make output file
    const auto outputfilename = systuncname+"_bin"+binInfo.bin+"."+Common::outTextExt;
    std::ofstream output(outputfilename.Data(),std::ios::trunc);
    std::cout << "Writing to: " << outputfilename.Data() << std::endl;

    // loop over samples
    for (const auto & sample : samples)
    {
      // get hists
      const auto & nom_hist = NomHistMap[sample]; 
      const auto & unc_hist = UncHistMap[sample]; 

      // temp vars : nominal
      const auto nom_int = nom_hist->GetBinContent(binX,binY);
      const auto nom_err = nom_hist->GetBinError  (binX,binY);

      // temp vars : systunc
      const auto unc_int = unc_hist->GetBinContent(binX,binY);
      const auto unc_err = unc_hist->GetBinError  (binX,binY);

      // compute percent diff
      const auto per_diff     = (1.f - (unc_int/nom_int))*100.f;
      const auto per_diff_err = per_diff*Common::hypot(nom_err/nom_int,unc_err/unc_int); 

      // write to output
      output << std::setw(19)
	     << sample.Data() 
	     << " : "
	     << std::setw(8) << per_diff
	     << " +/- "
	     << std::setw(8) << per_diff_err
	     << std::endl;
    }
  }

  // delete hist maps
  Common::DeleteMap(UncHistMap);
  Common::DeleteMap(NomHistMap);
}

void readSampleInfo(const TString & scan_log, const TString & plotfiletext, const TString & nom2Ddir, const TString & unc2Ddir,
		    std::vector<TString> & samples, std::map<TString,TH2F*> & NomHistMap, std::map<TString,TH2F*> & UncHistMap)
{
  std::ifstream input(scan_log.Data(),std::ios::in);
  TString sample, fulldir, xy;

  while (input >> sample >> fulldir >> xy)
  {
    // get input files
    const auto nom_file_name = fulldir+"/"+nom2Ddir+"/"+plotfiletext+"_"+xy+".root";
    auto nom_file = TFile::Open(nom_file_name.Data());
    Common::CheckValidFile(nom_file,nom_file_name);
    
    const auto unc_file_name = fulldir+"/"+unc2Ddir+"/"+plotfiletext+"_"+xy+".root";
    auto unc_file = TFile::Open(unc_file_name.Data());
    Common::CheckValidFile(unc_file,unc_file_name);
    
    // emplace back sample
    samples.emplace_back(sample);

    // get input hists
    const auto hist_name = sample+"_Hist";

    auto & nom_hist = NomHistMap[sample];
    nom_hist = (TH2F*)nom_file->Get(hist_name.Data());
    nom_hist->SetDirectory(0);
    Common::CheckValidHist(nom_hist,hist_name,nom_file_name);

    auto & unc_hist = UncHistMap[sample];
    unc_hist = (TH2F*)unc_file->Get(hist_name.Data());
    unc_hist->SetDirectory(0);
    Common::CheckValidHist(unc_hist,hist_name,unc_file_name);

    // delete all
    delete unc_file;
    delete nom_file;
  }
}