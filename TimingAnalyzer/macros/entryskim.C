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
  Float_t time;

  std::ifstream textfile;
  textfile.open(input.c_str(),std::ios::in);
  
  while (textfile >> ientry >> run >> lumi >> event >> goodpho >> time)
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

  // output dir
  std::string dir = "IsTrackCheck_SP_2017C/cuts_jetIdL_jetER_DispEff";

  // get input entry skim info (good)
  std::string ginput = dir+"/"+"goodinfo.txt";
  ibmap gentrymap;
  getEntryMap(ginput,gentrymap);

  // get input entry skim info (bad)
  std::string binput = dir+"/"+"badinfo.txt";
  ibmap bentrymap;
  getEntryMap(binput,bentrymap);

  // declare output skim files (good)
  std::string goutname = dir+"/"+"goodinfo.root";
  TFile * goutfile = new TFile(goutname.c_str(),"RECREATE");
  TDirectory * gsubdir = goutfile->mkdir("tree"); gsubdir->cd();
  TTree * gouttree = (TTree*)infile->Get("tree/tree");
  gouttree = intree->CloneTree(0); // clone structure of input tree, but storing no events

  // declare output skim files (bad)
  std::string boutname = dir+"/"+"badinfo.root";
  TFile * boutfile = new TFile(boutname.c_str(),"RECREATE");
  TDirectory * bsubdir = boutfile->mkdir("tree"); bsubdir->cd();
  TTree * bouttree = (TTree*)infile->Get("tree/tree");
  bouttree = intree->CloneTree(0); // clone structure of input tree, but storing no events
  
  // loop over input tree
  for (UInt_t ientry = 0; ientry < intree->GetEntries(); ientry++)
  {
    intree->GetEntry(ientry);

    if (gentrymap.count(ientry)) 
    {
      std::cout << "GOOD: " << ientry << std::endl; 
      gsubdir->cd();
      gouttree->Fill(); // fill output tree
    }

    if (bentrymap.count(ientry)) 
    {
      std::cout << "BAD : " << ientry << std::endl; 
      bsubdir->cd();
      bouttree->Fill(); // fill output tree
    }
  } // end loop over events

  // write the output skim tree (good)
  goutfile->cd();
  gsubdir->cd();
  gouttree->Write();

  // write the output skim tree (bad)
  boutfile->cd();
  bsubdir->cd();
  bouttree->Write();

  // now delete (close) everything
  delete intree;
  delete infile;
  delete gouttree;
  delete goutfile;
  delete bouttree;
  delete boutfile;
}
