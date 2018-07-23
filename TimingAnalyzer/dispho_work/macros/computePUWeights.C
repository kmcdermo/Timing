#include "Common.cpp+"

#include "TFile.h"
#include "TH1F.h"
#include "TString.h"

void computePUWeights(const TString & indir, const TString & files, const TString & puwgtfile)
{
  // make out file
  auto outfile = TFile::Open(Form("%s",puwgtfile.Data()),"UPDATE");
  outfile->cd();
  
  // get mc sum hists
  std::map<TString,TH1F*> mchistmap;
  mchistmap[Common::puObsHistName] = new TH1F(Form("%s_sum",Common::puObsHistName.Data()),Form("%s_sum",Common::puObsHistName.Data()),Common::nPUBins,0,Common::nPUBins);
  mchistmap[Common::puObsHistName+"_wgt"] = new TH1F(Form("%s_wgt_sum",Common::puObsHistName.Data()),Form("%s_wgt_sum",Common::puObsHistName.Data()),Common::nPUBins,0,Common::nPUBins);
  mchistmap[Common::puTrueHistName] = new TH1F(Form("%s_sum",Common::puTrueHistName.Data()),Form("%s_sum",Common::puTrueHistName.Data()),Common::nPUBins,0,Common::nPUBins);
  mchistmap[Common::puTrueHistName+"_wgt"] = new TH1F(Form("%s_wgt_sum",Common::puTrueHistName.Data()),Form("%s_wgt_sum",Common::puTrueHistName.Data()),Common::nPUBins,0,Common::nPUBins);

  // get list of files
  std::ifstream infiles(files.Data(),std::ios::in);

  // sum the weights
  TString infile = "";
  while (infiles >> infile)
  {
    // first get the file
    const TString infilename = Form("%s/%s",indir.Data(),infile.Data());
    auto file = TFile::Open(infilename.Data());
    Common::CheckValidFile(file,infilename);
    file->cd();
    
    // get input hists + add it up
    for (auto & mchistpair : mchistmap)
    {
      const auto & histname = mchistpair.first;
      auto       & hist     = mchistpair.second;

      const TString inhistname = Form("%s/%s",Common::rootdir.Data(),histname.Data());
      auto inhist = (TH1F*)file->Get(Form("%s",inhistname.Data()));
      Common::CheckValidHist(inhist,inhistname,infilename);
      
      // add hists
      hist->Add(inhist);
      
      // delete hists
      delete inhist;
    }

    // delete file
    delete file;
  }

  // save tmp output
  outfile->cd();
  for (const auto & mchistpair : mchistmap)
  {
    const auto & hist = mchistpair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }

  // input data file
  const TString indatafilename = Form("%s/%s.root",Common::eosDir.Data(),Common::dataPUFileName.Data());
  auto indatafile = TFile::Open(indatafilename.Data());
  Common::CheckValidFile(indatafile,indatafilename);
  indatafile->cd();
  
  // Get data hist
  const TString indatahistname = Form("%s",Common::dataPUHistName.Data());
  auto indatahist = (TH1D*)indatafile->Get(indatahistname.Data());
  Common::CheckValidHist(indatahist,indatahistname,indatafilename);

  // Make data hist resized
  auto datahist = new TH1F("h_pudata","h_pudata",Common::nPUBins,0,Common::nPUBins);
  datahist->Sumw2();
  for (auto ibin = 1; ibin <= datahist->GetXaxis()->GetNbins(); ibin++) 
  {
    datahist->SetBinContent(ibin,indatahist->GetBinContent(ibin));
    datahist->SetBinError  (ibin,indatahist->GetBinError  (ibin));
  }
  
  // save data
  outfile->cd();
  datahist->Write(datahist->GetName(),TObject::kWriteDelete);

  // clone data hists
  std::map<TString,TH1F*> datahistmap;
  for (const auto & mchistpair : mchistmap)
  {
    const auto & name = mchistpair.first;
    datahistmap[name] = (TH1F*)datahist->Clone(Form("%s_%s",name.Data(),Common::puwgtHistName.Data()));
  }

  // normalize MC
  for (auto & mchistpair : mchistmap)
  {
    auto & hist = mchistpair.second;
    hist->Scale(1.f/hist->Integral());
  }

  // normalize data
  for (auto & datahistpair : datahistmap)
  {
    auto & hist = datahistpair.second;
    hist->Scale(1.f/hist->Integral());
  }

  // divide and stuff
  for (auto & datahistpair : datahistmap)
  {
    auto & name = datahistpair.first;
    auto & hist = datahistpair.second;
    hist->Divide(mchistmap[name]);
  }

  // save it
  outfile->cd();
  for (const auto & datahistpair : datahistmap)
  {
    const auto & hist = datahistpair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }

  // delete the rest
  for (auto & datahistpair : datahistmap) delete datahistpair.second;
  delete datahist;
  delete indatahist;
  delete indatafile;
  for (auto & mchistpair : mchistmap) delete mchistpair.second;
  delete outfile;
}

