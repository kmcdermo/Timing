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
  hist->SetBinErrorOption(TH1::kPoisson);

  // fill hist
  std::cout << "Filling hist..." << std::endl;
  tree->Draw("t1pfMETpt:phoweightedtimeLT120_0>>hist",selection.Data(),"goff");

  //////////
  // data //
  //////////

  const auto obsA       = hist->GetBinContent (1,1);
  const auto obsAunc    = hist->GetBinError   (1,1);
  const auto obsAuncLow = hist->GetBinErrorLow(1,1);
  const auto obsAuncUp  = hist->GetBinErrorUp (1,1);

  const auto obsB       = hist->GetBinContent (1,2);
  const auto obsBunc    = hist->GetBinError   (1,2);
  const auto obsBuncLow = hist->GetBinErrorLow(1,2);
  const auto obsBuncUp  = hist->GetBinErrorUp (1,2);

  const auto obsC       = hist->GetBinContent (2,2);
  const auto obsCunc    = hist->GetBinError   (2,2);
  const auto obsCuncLow = hist->GetBinErrorLow(2,2);
  const auto obsCuncUp  = hist->GetBinErrorUp (2,2);

  const auto obsD       = hist->GetBinContent (2,1);
  const auto obsDunc    = hist->GetBinError   (2,1);
  const auto obsDuncLow = hist->GetBinErrorLow(2,1);
  const auto obsDuncUp  = hist->GetBinErrorUp (2,1);

  ////////////
  // params //
  ////////////

  const auto BovA       = obsB/obsA;
  const auto BovAunc    = BovA*std::sqrt(std::pow(obsBunc   /obsB,2)+std::pow(obsAunc   /obsA,2));
  const auto BovAuncLow = BovA*std::sqrt(std::pow(obsBuncLow/obsB,2)+std::pow(obsAuncLow/obsA,2)); 
  const auto BovAuncUp  = BovA*std::sqrt(std::pow(obsBuncUp /obsB,2)+std::pow(obsAuncUp /obsA,2)); 

  const auto DovA       = obsD/obsA;
  const auto DovAunc    = DovA*std::sqrt(std::pow(obsDunc   /obsD,2)+std::pow(obsAunc   /obsA,2));
  const auto DovAuncLow = DovA*std::sqrt(std::pow(obsDuncLow/obsD,2)+std::pow(obsAuncLow/obsA,2)); 
  const auto DovAuncUp  = DovA*std::sqrt(std::pow(obsDuncUp /obsD,2)+std::pow(obsAuncUp /obsA,2)); 

  const auto predC       = obsB*obsD/obsA;
  const auto predCunc    = predC*std::sqrt(std::pow(obsBunc   /obsB,2)+std::pow(obsDunc   /obsD,2)+std::pow(obsAunc   /obsA,2));
  const auto predCuncLow = predC*std::sqrt(std::pow(obsBuncLow/obsB,2)+std::pow(obsDuncLow/obsD,2)+std::pow(obsAuncLow/obsA,2));
  const auto predCuncUp  = predC*std::sqrt(std::pow(obsBuncUp /obsB,2)+std::pow(obsDuncUp /obsD,2)+std::pow(obsAuncUp /obsA,2));

  ///////////
  // pulls //
  ///////////

  const auto pullObsC    = (obsC-predC)/obsCunc;
  const auto pullObsCLow = (obsC-predC)/obsCuncLow;
  const auto pullObsCUp  = (obsC-predC)/obsCuncUp;

  const auto pullPredC    = (obsC-predC)/predCunc;
  const auto pullPredCLow = (obsC-predC)/predCuncLow;
  const auto pullPredCUp  = (obsC-predC)/predCuncUp;

  const auto pullC    = (obsC-predC)/std::sqrt(std::pow(obsCunc   ,2)+std::pow(predCunc   ,2));
  const auto pullCLow = (obsC-predC)/std::sqrt(std::pow(obsCuncLow,2)+std::pow(predCuncLow,2));
  const auto pullCUp  = (obsC-predC)/std::sqrt(std::pow(obsCuncUp ,2)+std::pow(predCuncUp ,2));

  // fill output file
  std::cout << "Filling text file..." << std::endl;

  norms << label.Data() << "," << int(time_bin) << "," << int(met_bin) << ","
	<< std::setprecision(3) << obsA << " +/- " << std::setprecision(3) << obsAunc << " -" << std::setprecision(3) << obsAuncLow << " +" << std::setprecision(3) << obsAuncUp << ","
	<< std::setprecision(3) << obsB << " +/- " << std::setprecision(3) << obsBunc << " -" << std::setprecision(3) << obsBuncLow << " +" << std::setprecision(3) << obsBuncUp << ","
	<< std::setprecision(3) << obsC << " +/- " << std::setprecision(3) << obsCunc << " -" << std::setprecision(3) << obsCuncLow << " +" << std::setprecision(3) << obsCuncUp << ","
	<< std::setprecision(3) << obsD << " +/- " << std::setprecision(3) << obsDunc << " -" << std::setprecision(3) << obsDuncLow << " +" << std::setprecision(3) << obsDuncUp << ","
	<< std::setprecision(3) << BovA << " +/- " << std::setprecision(3) << BovAunc << " -" << std::setprecision(3) << BovAuncLow << " +" << std::setprecision(3) << BovAuncUp << ","
	<< std::setprecision(3) << DovA << " +/- " << std::setprecision(3) << DovAunc << " -" << std::setprecision(3) << DovAuncLow << " +" << std::setprecision(3) << DovAuncUp << ","
	<< std::setprecision(3) << predC << " +/- " << std::setprecision(3) << predCunc << " -" << std::setprecision(3) << predCuncLow << " +" << std::setprecision(3) << predCuncUp << ","
	<< std::setprecision(3) << pullObsC << "," << std::setprecision(3) << pullObsCLow << "," << std::setprecision(3) << pullObsCUp << ","
	<< std::setprecision(3) << pullPredC << "," << std::setprecision(3) << pullPredCLow << "," << std::setprecision(3) << pullPredCUp << ","
	<< std::setprecision(3) << pullC << "," << std::setprecision(3) << pullCLow << "," << std::setprecision(3) << pullCUp << std::endl;
  
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
