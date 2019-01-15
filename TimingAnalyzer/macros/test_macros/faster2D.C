#include "Common.cpp+"

void faster2D(const TString & filename, const TString & treename, TString selection, const TString & label, 
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
  const auto obsA = hist->GetBinContent(1,1);
  const auto obsB = hist->GetBinContent(1,2);
  const auto obsC = hist->GetBinContent(2,2);
  const auto obsD = hist->GetBinContent(2,1);

  const auto obsAunc = hist->GetBinError(1,1);
  const auto obsBunc = hist->GetBinError(1,2);
  const auto obsCunc = hist->GetBinError(2,2);
  const auto obsDunc = hist->GetBinError(2,1);

  const auto BovA     = obsB/obsA;
  const auto BovAunc  = BovA*std::sqrt(std::pow(obsBunc/obsB,2)+std::pow(obsAunc/obsA,2)); 

  const auto DovA     = obsD/obsA;
  const auto DovAunc  = DovA*std::sqrt(std::pow(obsDunc/obsD,2)+std::pow(obsAunc/obsA,2)); 

  const auto predC    = obsB*obsD/obsA;
  const auto predCunc = predC*std::sqrt(std::pow(obsBunc/obsB,2)+std::pow(obsDunc/obsD,2)+std::pow(obsAunc/obsA,2));

  const auto pullObsC  = (obsC-predC)/obsCunc;
  const auto pullPredC = (obsC-predC)/predCunc;
  const auto pullC     = (obsC-predC)/std::sqrt(std::pow(obsCunc,2)+std::pow(predCunc,2));

  norms << label.Data() << "," << int(time_bin) << "," << int(met_bin) << ","
	<< obsA << "+/-" << obsAunc << ","
	<< obsB << "+/-" << obsBunc << ","
	<< obsC << "+/-" << obsCunc << ","
	<< obsD << "+/-" << obsDunc << ","
	<< BovA << "+/-" << BovAunc << ","
	<< DovA << "+/-" << DovAunc << ","
	<< predC << "+/-" << predCunc << ","
	<< pullObsC << "," << pullPredC << "," << pullC << std::endl;
  
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
  const TString outname = Form("%s_MET%i_Time%i",label.Data(),int(met_bin),int(time_bin));

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
