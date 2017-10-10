#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TDirectory.h"

#include <map>
#include <string>
#include <fstream>

typedef std::map<UInt_t,Bool_t> ibmap;

void getEntryMap(const std::string & input, ibmap & entrymap)
{
  UInt_t ientry, run, lumi;
  ULong64_t event;
  Int_t goodpho;

  std::ifstream textfile;
  textfile.open(input.c_str(),std::ios::in);
  
  while (textfile >> ientry >> run >> lumi >> event >> goodpho)
  {
    entrymap[ientry] = true;
  }

  textfile.close();
}

void entryskim()
{
  // get input full trees
  TFile * infile = TFile::Open("input/DATA/2017/HLT_CHECK/SP/hltdump_2017C.root");
  TTree * intree = (TTree*)infile->Get("tree/tree");

  // get input entry skim info
  std::string input = "goodinfo.txt";
  ibmap entrymap;
  getEntryMap(input,entrymap);

  // declare output skim files
  TFile * outfile = new TFile("goodinfo.root","RECREATE");
  TDirectory * subdir = outfile->mkdir("tree");
  subdir->cd();
  TTree * outtree = (TTree*)infile->Get("tree/tree");
  // clone structure of input tree, but storing no events
  outtree = intree->CloneTree(0);
  
  // loop over input tree
  for (UInt_t ientry = 0; ientry < intree->GetEntries(); ientry++)
  {
    intree->GetEntry(ientry);
    if (entrymap[ientry]) 
    {
      std::cout << ientry << std::endl; 
      outtree->Fill(); // fill output tree
    }
  }

  // write the output skim tree
  outfile->cd();
  subdir->cd();
  outtree->Write();

  // now delete (close) everything
  delete intree;
  delete infile;
  delete outtree;
  delete outfile;
}
