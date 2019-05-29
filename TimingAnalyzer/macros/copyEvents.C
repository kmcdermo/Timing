#include "Common.cpp+"

void copyEvents(const TString & infilename, const TString & intreename,
		const TString & selection, const TString & outfilename)
{
  // get inputs
  std::cout << "Getting input file and tree..." << std::endl;

  auto infile = TFile::Open(infilename.Data());
  Common::CheckValidFile(infile,infilename);

  auto intree = (TTree*)infile->Get(intreename.Data());
  Common::CheckValidTree(intree,intreename,infilename);

  // make output file
  std::cout << "Make output file..." << std::endl;
  auto outfile = TFile::Open(outfilename.Data(),"RECREATE");
  outfile->cd();
  
  // copy tree
  std::cout << "Copy tree based on selection..." << std::endl;
  auto outtree = intree->CopyTree(selection.Data());
  
  // write it out
  std::cout << "Writing to output file..." << std::endl;
  Common::Write(outfile,outtree);

  // delete it all
  std::cout << "Deleting it all..." << std::endl;
  delete outtree;
  delete outfile;
  delete intree;
  delete infile;
}
