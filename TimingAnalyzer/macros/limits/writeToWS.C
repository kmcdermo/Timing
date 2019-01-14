#include "../Common.hh"

void fillConstants(TH1F * hist, const TString & label, std::ofstream & output);
void makeWS(TH1F * hist, const TString & tmpwsfile);

void writeToWS(const TString & tmplimitdir, const TString & inplotsfile, 
	       const TString & name, const TString & tmpwsfile)
{
  // get input file
  const TString infilename = tmplimitdir+"/"+inplotsfile;
  auto infile = TFile::Open(infilename.Data());
  Common::CheckValidFile(infile,infilename);
  
  // get data hist
  const TString datahistname = "Data_Hist";
  auto datahist = (TH2F*)infile->Get(datahistname.Data());
  Common::CheckValidHist(datahist,datahistname,infilename);

  // get signal hist
  const TString signhistname = name+"_Hist";
  auto signhist = (TH2F*)infile->Get(signhistname.Data());
  Common::CheckValidHist(signhist,signhistname,infilename);

  // open output file
  std::ofstream output(name+".txt",std::ios::trunc);
  
  // fill data + Make WS
  fillConstants(datahist,"BIN",output);
  makeWS(datahist,tmpwsfile);

  // fill sig
  fillConstants(signhist,"SIG",output);

  // delete it all
  delete signhist;
  delete datahist;
  delete infile;
}

void fillConstants(TH1F * hist, const TString & label, std::ofstream & output)
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

void makeWS(TH1F * hist, const TString & tmpwsfile)
{
  // get output file
  auto outfile = TFile::Open(tmpwsfile.Data(),"RECREATE");
  outfile->cd();

  // make WS
  RooWorkspace workspace("workspace", "workspace");
  
  // get constants
  auto in_bkgA = hist->GetBinContent(1,1);
  auto in_BovA = hist->GetBinContent(1,2) / in_bkgA;
  auto in_DovA = hist->GetBinContent(2,1) / in_bkgA;

  // fill WS
  RooRealVar bkgA("bkgA", "bkgA", in_bkgA, 0.0, 2.*in_bkgA);
  RooRealVar BovA("BovA", "BovA", in_BovA, 0.0, 1.);
  RooRealVar DovA("DovA", "DovA", in_DovA, 0.0, 1.);
  
  // import vars into WS
  workspace.import(bkgA);
  workspace.import(BovA);
  workspace.import(DovA);
  
  // save file
  outfile->cd();
  workspace.Write(workspace.GetName(),TObject::kWriteDelete);

  // delete it all
  delete outfile;
}
