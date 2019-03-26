#include "Common.cpp+"

void setup(TStyle * tdrStyle, const TString & miscconfig, std::vector<TString> & plotSignalVec);
void readHists(TFile * file, std::map<TString,TH2F*> & HistMap2D);
void projectX(const std::map<TString,TH2F*> & HistMap2D, std::map<TString,TH1D*> & HistMapX, const TString & x_label, 
	      const TString & y_label, const TString & y_low, const TString & y_high);
void projectY(const std::map<TString,TH2F*> & HistMap2D, std::map<TString,TH1D*> & HistMapY, const TString & y_label,
	      const TString & x_label, const TString & x_low, const TString & x_high);
void writeHistMap(TFile * file, std::map<TString,TH1D*> & HistMap1D, const TString & label);
void writeSignalContamination(const std::map<TString,TH1D*> & HistMap1D, const std::vector<TString> & plotSignalVec, const TString & label);
void saveTemplates(TFile * file, std::map<TString,TH1D*> & HistMap1D, const TString & label);
void drawHist(TH1D * hist, const TString & label);
void drawHist(TH1D * hist, const TString & label, const Bool_t isLogy);

void generateTemplates(const TString & outfiletext, const TString & miscconfig,
		       const TString & x_label, const TString & x_low, const TString & x_high,
		       const TString & y_label, const TString & y_low, const TString & y_high)
{
  // setup everything
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  std::vector<TString> plotSignalVec;
  setup(tdrStyle,miscconfig,plotSignalVec);

  // inputs
  const TString filename = outfiletext+".root";
  auto file = TFile::Open(filename,"UPDATE");
  Common::CheckValidFile(file,filename);
  file->cd();

  // setup map of hists
  std::map<TString,TH2F*> HistMap2D;
  std::map<TString,TH1D*> HistMapX;
  std::map<TString,TH1D*> HistMapY;

  // read 2D hists
  readHists(file,HistMap2D);

  // project out
  projectX(HistMap2D,HistMapX,x_label,y_label,y_low,y_high);
  projectY(HistMap2D,HistMapY,y_label,x_label,x_low,x_high);

  // write projections
  writeHistMap(file,HistMapX,"_projX");
  writeHistMap(file,HistMapY,"_projY");

  // signal contamination
  writeSignalContamination(HistMapX,plotSignalVec,outfiletext+"_projX");
  writeSignalContamination(HistMapY,plotSignalVec,outfiletext+"_projY");

  // make data templates, finally
  saveTemplates(file,HistMapX,outfiletext+"_projX");
  saveTemplates(file,HistMapY,outfiletext+"_projY");

  // delete it all
  Common::DeleteMap(HistMapY);
  Common::DeleteMap(HistMapX);
  Common::DeleteMap(HistMap2D);

  delete file;
  delete tdrStyle;
}

void setup(TStyle * tdrStyle, const TString & miscconfig, std::vector<TString> & plotSignalVec)
{
  std::cout << "Setting up..." << std::endl;

  // setup
  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
  Common::SetTDRStyle(tdrStyle);

  std::ifstream miscfile(Form("%s",miscconfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(miscfile,str))
  {
    if (str.find("signals_to_plot=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"signals_to_plot=");
      Common::SetupWhichSignals(str,plotSignalVec);
    }
  }
}

void readHists(TFile * file, std::map<TString,TH2F*> & HistMap2D)
{
  std::cout << "Reading hists from file..." << std::endl;

  // load all bkgd MC, signal MC, data plotted
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample   = HistNamePair.first;
    const auto & histname = ((Common::GroupMap[sample] == SampleGroup::isData) ? HistNamePair.second+"_Plotted" : HistNamePair.second); // use only plotted data!
    auto & hist = HistMap2D[sample];

    hist = (TH2F*)file->Get(histname.Data());
    Common::CheckValidHist(hist,histname,file->GetName());
  }
}

void projectX(const std::map<TString,TH2F*> & HistMap2D, std::map<TString,TH1D*> & HistMapX, const TString & x_label, 
	      const TString & y_label, const TString & y_low, const TString & y_high)
{
  std::cout << "Projecting in X..." << std::endl;

  for (const auto & HistPair2D : HistMap2D)
  {
    const auto & sample = HistPair2D.first;
    const auto & hist2D = HistPair2D.second;

    auto & histX = HistMapX[sample];    
    histX = (TH1D*)hist2D->ProjectionX(Form("%s_%s_for_%s_%s_to_%s",hist2D->GetName(),x_label.Data(),y_label.Data(),
					    Common::ReplaceMinusWithM(y_low).Data(),Common::ReplaceMinusWithM(y_high).Data()),
				       hist2D->GetYaxis()->FindBin(y_low.Atof()),
				       hist2D->GetYaxis()->FindBin(y_high.Atof())-1);

    histX->GetXaxis()->SetTitle(Form("%s {%s #leq %s < %s}",histX->GetXaxis()->GetTitle(),y_low.Data(),hist2D->GetYaxis()->GetTitle(),y_high.Data()));
    histX->GetYaxis()->SetTitle("Events");
    histX->SetTitle(histX->GetXaxis()->GetTitle());
  }
}

void projectY(const std::map<TString,TH2F*> & HistMap2D, std::map<TString,TH1D*> & HistMapY, const TString & y_label,
	      const TString & x_label, const TString & x_low, const TString & x_high)
{
  std::cout << "Projecting in Y..." << std::endl;

  for (const auto & HistPair2D : HistMap2D)
  {
    const auto & sample = HistPair2D.first;
    const auto & hist2D = HistPair2D.second;

    auto & histY = HistMapY[sample];    
    histY = (TH1D*)hist2D->ProjectionY(Form("%s_%s_for_%s_%s_to_%s",hist2D->GetName(),y_label.Data(),x_label.Data(),
					    Common::ReplaceMinusWithM(x_low).Data(),Common::ReplaceMinusWithM(x_high).Data()),
				       hist2D->GetXaxis()->FindBin(x_low.Atof()),
				       hist2D->GetXaxis()->FindBin(x_high.Atof())-1);

    histY->GetXaxis()->SetTitle(Form("%s {%s #leq %s < %s}",histY->GetXaxis()->GetTitle(),x_low.Data(),hist2D->GetXaxis()->GetTitle(),x_high.Data()));
    histY->GetYaxis()->SetTitle("Events");
    histY->SetTitle(histY->GetXaxis()->GetTitle());
  }
}

void writeHistMap(TFile * file, std::map<TString,TH1D*> & HistMap1D, const TString & label)
{
  std::cout << "Writing Hist Map for: " << label.Data() << std::endl;
  Common::WriteMap(file,HistMap1D);
}

void writeSignalContamination(const std::map<TString,TH1D*> & HistMap1D, const std::vector<TString> & plotSignalVec, const TString & label)
{
  std::cout << "Writing Signal Contamination for: " << label.Data() << std::endl;

  // data hist as denom
  const auto & data = HistMap1D.at("Data");
  auto data_err = 0.0;
  auto data_int = data->IntegralAndError(1,data->GetXaxis()->GetNbins(),data_err);

  // make output stream
  std::ofstream output(label+"."+Common::outTextExt,std::ios::trunc);
  output << "Signal Contamination: Signal MC/Data" << std::endl;

  for (const auto & signal : plotSignalVec)
  {
    const auto & sign = HistMap1D.at(signal);
    auto sign_err = 0.0;
    auto sign_int = sign->IntegralAndError(1,sign->GetXaxis()->GetNbins(),sign_err);
    
    const auto ratio = sign_int/data_int;
    const auto err   = ratio * std::sqrt(std::pow(data_err/data_int,2)+std::pow(sign_err/sign_int,2));

    output << signal.Data() << " " << ratio << " +/- " << err << std::endl;
  }
}

void saveTemplates(TFile * file, std::map<TString,TH1D*> & HistMap1D, const TString & label)
{
  std::cout << "Saving templates: " << label.Data() << std::endl;

  auto & hist1D = HistMap1D["Data"];
  hist1D->Scale(1.f/hist1D->Integral());

  Common::Write(file,hist1D,Form("%s_Templates",hist1D->GetName()));

  drawHist(hist1D,label);
}

void drawHist(TH1D * hist, const TString & label)
{
  std::cout << "Drawing Hist: " << label.Data() << std::endl;

  drawHist(hist,label,false);
  drawHist(hist,label,true);
}

void drawHist(TH1D * hist, const TString & label, const Bool_t isLogy)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogy);

  hist->Draw("ep");
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,Form("%s_%s",label.Data(),(isLogy?"log":"lin")));

  delete canv;
}
