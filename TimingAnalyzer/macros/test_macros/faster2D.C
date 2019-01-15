#include "Common.cpp+"

void fast2D_with_correlation(const TString & filename, const TString & treename, TString selection, const TString & label, 
			     const Double_t time_bin, const Double_t met_bin, const TString & textfile, const TString & outdir)
{
  // get text file
  std::ofstream norms(textfile.Data(),std::ios_base::app);

  if      (selection.EqualTo("NONE",TString::kExact))
  {
    selection = "";
  }
  else if (selection.Contains("MC",TString::kExact))
  {
    selection.ReplaceAll("MC","");
    selection += " * (evtwgt * puwgt)";
  }

  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);
  
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // setup hist --> adjust by hand for each version
  Double_t xbins[3] = {-2,time_bin,25};
  Double_t ybins[3] = {0,met_bin,3000};
  auto hist = new TH2F("hist","MET vs Time;Photon Weighted Time [ns];MET [GeV]",2,xbins,2,ybins);
  hist->SetMarkerSize(2);
  hist->Sumw2();

  // fill hist
  std::cout << "Filling hist..." << std::endl;
  tree->Draw("t1pfMETpt:phoweightedtimeLT120_0>>hist",selection.Data(),"goff");

  // fill output file
  std::cout << "Filling text file..." << std::endl;
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

  norms << label.Data() << ","
	<< A << "+/-" << Aunc << ","
	<< B << "+/-" << Bunc << ","
	<< C << "+/-" << Cunc << ","
	<< D << "+/-" << Dunc << ","
	<< BovA << "+/-" << BovAunc << ","
	<< DovA << "+/-" << DovAunc << ","
	<< predC << "+/-" << predCunc << std::endl;
  
  // draw it
  std::cout << "Drawing and saving..." << std::endl;
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogz(1);

  hist->Draw("colz text");

  // print correlation
  auto text = new TPaveText(0.68,0.7,0.81,0.93,"NDC");
  text->SetTextAlign(11);
  text->SetFillColorAlpha(text->GetFillColor(),0);
  text->AddText(Form("CF: %6.3f",hist->GetCorrelationFactor()));
  text->Draw("same");

  // save it
  const TString outname = Form("%s_MET%i_Time%i",label.Data(),met_bin,time_bin);

  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,outname);

  // move it to www
  gSystem->Exec("mv "+outname+".png "+outdir);
  gSystem->Exec("mv "+outname+".pdf "+outdir);

  // delete it all
  std::cout << "Deleting..." << std::endl;
  delete text;
  delete canv;
  delete hist;
  delete tree;
  delete file;
  delete tdrStyle;
}
