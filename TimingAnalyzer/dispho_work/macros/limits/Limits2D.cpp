#include "Limits2D.hh"

Limits2D::Limits2D(const TString & indir, const TString & infilename, const TString & limitconfig, const TString & era, const TString & outtext)
  : fInDir(indir), fInFileName(infilename), fLimitConfig(limitconfig), fEra(era), fOutText(outtext)
{  
  // setup common first
  Limits2D::SetupDefaults();
  Limits2D::SetupLimitConfig();
  Limits2D::SetupCommon();
  Limits2D::SetupCombine();

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // make new output file
  fOutFile = TFile::Open(Form("%s.root",fOutText.Data()),"UPDATE");
}

Limits2D::~Limits2D() 
{
  delete fConfigPave;
  for (auto & HistPair : fHistMap) delete HistPair.second;
  delete fOutFile;
  delete fTDRStyle;
}

void Limits2D::MakeLimits2D()
{
  // First, import all measured values into bins
  Limits2D::FillKnownBins();

  // Then, interpolate between all measured vales
  Limits2D::InterpolateKnownBins();

  // Maybe dump all bins
  if (fDumpBins) Limits2D::DumpAllBins();

  // get bin boundaries for histogram making...
  Limits2D::GetHistBinBoundaries();

  // fill histograms from full bin info
  Limits2D::FillRValHists();
  
  // draw 2D limits!
  Limits2D::DrawLimits();
  
  // save config pave
  Limits2D::MakeConfigPave();
}
 
void Limits2D::FillKnownBins()
{
  std::cout << "Fill known bins..." << std::endl;

  // get x-centers (tmp map)
  std::map<TString,Float_t> xcentersmap;
  for (const auto & GMSBPair : Combine::GMSBMap)
  {
    const auto & s_lambda = GMSBPair.second.s_lambda;
    const auto lambda     = GMSBPair.second.lambda;
    xcentersmap[s_lambda] = lambda;
  }

  // put into vector, sort, and use
  std::vector<std::pair<TString,Float_t> > xcentersvec;
  for (const auto & xcenterspair : xcentersmap) xcentersvec.emplace_back(xcenterspair.first,xcenterspair.second);
  std::sort(xcentersvec.begin(),xcentersvec.end(),sortPairs);

  // get y-centers (tmp map)
  std::map<TString,Float_t> ycentersmap;
  for (const auto & GMSBPair : Combine::GMSBMap)
  {
    const auto & s_ctau = GMSBPair.second.s_ctau;
    const auto ctau     = GMSBPair.second.ctau;
    ycentersmap[s_ctau] = ctau;
  }

  // put into vector, sort, and use
  std::vector<std::pair<TString,Float_t> > ycentersvec;
  for (const auto & ycenterspair : ycentersmap) ycentersvec.emplace_back(ycenterspair.first,ycenterspair.second);
  std::sort(ycentersvec.begin(),ycentersvec.end(),sortPairs);

  // set width for known bins
  const auto xwidth = xcentersvec[0].second / fXMinWidthDiv;
  const auto ywidth = ycentersvec[0].second / fYMinWidthDiv;
  
  // loop over centers and fill filledbins
  fKnownBins.resize(xcentersvec.size());
  for (auto i_fill = 0U; i_fill < xcentersvec.size(); i_fill++)
  {
    // x bin info
    const auto & x_s   = xcentersvec[i_fill].first;
    const auto xcenter = xcentersvec[i_fill].second;
    const auto xlow    = xcenter - (xwidth / 2.f);
    const auto xup     = xcenter + (xwidth / 2.f);

    fKnownBins[i_fill].resize(ycentersvec.size());
    for (auto j_fill = 0U; j_fill < ycentersvec.size(); j_fill++)
    {
      // y bin info
      const auto & y_s   = ycentersvec[j_fill].first;
      const auto ycenter = ycentersvec[j_fill].second;
      const auto ylow    = ycenter - ywidth;
      const auto yup     = ycenter + ywidth;
      
      // get name of gmsb sample
      const TString name = "GMSB_L"+x_s+"TeV_CTau"+y_s+"cm";

      // set basic info for bins
      fKnownBins[i_fill][j_fill] = {xlow,xcenter,xup,ylow,ycenter,yup,Combine::GMSBMap[name].rvalmap};
    } // end loop over ycenters [j_fill]
  } // end loop over xcenters [i_fill]

  // Make sure to emplace the known bins into all bins
  for (const auto & xBins : fKnownBins)
  {
    for (const auto & xyBin : xBins)
    {
      fAllBins.emplace_back(xyBin);
    }
  }
}

// loop over known bins and interpolate: https://en.wikipedia.org/wiki/Bilinear_interpolation#Alternative_algorithm
void Limits2D::InterpolateKnownBins()
{
  std::cout << "Setup all bins from filled bins..." << std::endl;

  const auto nx_fill = fKnownBins.size();
  for (auto i_fill = 0U; i_fill < nx_fill; i_fill++)
  {
    const Bool_t isEdgeX = (i_fill == (nx_fill-1));

    const auto ny_fill = fKnownBins[i_fill].size();
    for (auto j_fill = 0U; j_fill < ny_fill; j_fill++)
    {
      const Bool_t isEdgeY = (j_fill == (ny_fill-1));

      // bin numbers --> if at edges, do not overflow!
      const auto i_bin = (!isEdgeX ? i_fill : i_fill-1);
      const auto j_bin = (!isEdgeY ? j_fill : j_fill-1);

      // corner bins (11 and 22)
      const auto & binlow = fKnownBins[i_bin  ][j_bin  ];
      const auto & binup  = fKnownBins[i_bin+1][j_bin+1];

      // bin boundaries
      const auto xlow   = binlow.xup;
      const auto xup    = binup .xlow;
      const auto xwidth = (xup-xlow)/fNX_Interp;

      const auto ylow   = binlow.yup;
      const auto yup    = binup .ylow;
      const auto ywidth = (yup-ylow)/fNY_Interp;

      // center points for interpolation
      const auto x1 = binlow.xcenter;
      const auto x2 = binup .xcenter;
      const auto y1 = binlow.ycenter;
      const auto y2 = binup .ycenter;

      // make stripes in x when center equals filled bin
      if (!isEdgeX)
      {
	for (auto i_interp = 0; i_interp < fNX_Interp; i_interp++)
        {
	  // bin boundaries
	  const auto xbinlow    = xlow + xwidth*(i_interp);
	  const auto xbinup     = xlow + xwidth*(i_interp+1);
	  const auto xbincenter = (xbinlow+xbinup)/2.f;
	  
	  const auto ybinlow    = (!isEdgeY ? binlow : binup).ylow;
	  const auto ybinup     = (!isEdgeY ? binlow : binup).yup;
	  const auto ybincenter = (!isEdgeY ? binlow : binup).ycenter;

	  // loop over values to interpolate!
	  std::map<TString,Float_t> tmpmap; // rval map for tmp zs
	  for (const auto & RVal : Combine::RValVec)
	  {
	    // values
	    const auto fQ11 = binlow.rvalmap.at(RVal);
	    const auto fQ12 = fKnownBins[i_bin  ][j_bin+1].rvalmap.at(RVal);
	    const auto fQ21 = fKnownBins[i_bin+1][j_bin  ].rvalmap.at(RVal);
	    const auto fQ22 = binup.rvalmap.at(RVal);

	    tmpmap[RVal] = Limits2D::ZValue(xbincenter,x1,x2,ybincenter,y1,y2,fQ11,fQ12,fQ21,fQ22);
	  }
	  
	  // place bin into all bins
	  fAllBins.emplace_back(xbinlow,xbincenter,xbinup,ybinlow,ybincenter,ybinup,tmpmap);
	}
      }

      // make stripes in y when center equals filled bin
      if (!isEdgeY)
      {
	for (auto j_interp = 0; j_interp < fNY_Interp; j_interp++)
        {
	  // bin boundaries
	  const auto xbinlow    = (!isEdgeX ? binlow : binup).xlow;
	  const auto xbinup     = (!isEdgeX ? binlow : binup).xup;
	  const auto xbincenter = (!isEdgeX ? binlow : binup).xcenter;

	  const auto ybinlow    = ylow + ywidth*(j_interp);
	  const auto ybinup     = ylow + ywidth*(j_interp+1);
	  const auto ybincenter = (ybinlow+ybinup)/2.f;

	  // loop over values to interpolate!
	  std::map<TString,Float_t> tmpmap; // rval map for tmp zs
	  for (const auto & RVal : Combine::RValVec)
	  {
	    // values
	    const auto fQ11 = binlow.rvalmap.at(RVal);
	    const auto fQ12 = fKnownBins[i_bin  ][j_bin+1].rvalmap.at(RVal);
	    const auto fQ21 = fKnownBins[i_bin+1][j_bin  ].rvalmap.at(RVal);
	    const auto fQ22 = binup.rvalmap.at(RVal);

	    tmpmap[RVal] = Limits2D::ZValue(xbincenter,x1,x2,ybincenter,y1,y2,fQ11,fQ12,fQ21,fQ22);	    
	  }
	  
	  // place bin into all bins
	  fAllBins.emplace_back(xbinlow,xbincenter,xbinup,ybinlow,ybincenter,ybinup,tmpmap);
	}
      }

      // skip interpolation of inbetween boxes if either condition is met
      if (isEdgeX || isEdgeY) continue;

      // interpolate inner boxes
      for (auto i_interp = 0; i_interp < fNX_Interp; i_interp++)
      {
	// for this new bin
	const auto xbinlow    = xlow + xwidth*(i_interp);
	const auto xbinup     = xlow + xwidth*(i_interp+1);
	const auto xbincenter = (xbinlow+xbinup)/2.f;
	
	for (auto j_interp = 0; j_interp < fNY_Interp; j_interp++)
        {
	  // for this new bin
	  const auto ybinlow    = ylow + ywidth*(j_interp);
	  const auto ybinup     = ylow + ywidth*(j_interp+1);
	  const auto ybincenter = (ybinlow+ybinup)/2.f;

	  // loop over values to interpolate!
	  std::map<TString,Float_t> tmpmap; // rval map for tmp zs
	  for (const auto & RVal : Combine::RValVec)
	  {
	    // values
	    const auto fQ11 = binlow.rvalmap.at(RVal);
	    const auto fQ12 = fKnownBins[i_bin  ][j_bin+1].rvalmap.at(RVal);
	    const auto fQ21 = fKnownBins[i_bin+1][j_bin  ].rvalmap.at(RVal);
	    const auto fQ22 = binup.rvalmap.at(RVal);

	    tmpmap[RVal] = Limits2D::ZValue(xbincenter,x1,x2,ybincenter,y1,y2,fQ11,fQ12,fQ21,fQ22);	    
	  }
	
	  // place bin into all bins
	  fAllBins.emplace_back(xbinlow,xbincenter,xbinup,ybinlow,ybincenter,ybinup,tmpmap);
	} // end loop over nbins in y direction to interpolate
      } // end loop over nbins in x direction to interpolate
    } // end loop over measured y vals
  } // end loop over measured x vals
}

void Limits2D::DumpAllBins()
{
  std::cout << "Dump all bin information..." << std::endl;
  
  // sort bins first (first in x, then y)
  std::sort(fAllBins.begin(),fAllBins.end(),
	    [](const auto & bin1, const auto & bin2)
	    {
	      if (bin1.xcenter == bin2.xcenter) return bin1.ycenter < bin2.ycenter;
	      return bin1.xcenter < bin2.xcenter;
	    });

  // then dump info
  for (const auto & bin : fAllBins)
  {
    std::cout << bin.xlow << " " << bin.xcenter << " " << bin.xup << " " 
  	      << bin.ylow << " " << bin.ycenter << " " << bin.yup << std::endl;
    for (const auto & rvalpair : bin.rvalmap)
    {
      std::cout << "      " << rvalpair.first.Data() << ": " << rvalpair.second << std::endl;
    }
  }
}

void Limits2D::GetHistBinBoundaries()
{
  std::cout << "Setup bin boundaries for histograms..." << std::endl;

  // use all the bins for the histogram boundaries!
  for (const auto & bin : fAllBins)
  {
    // do x bin boundaries first
    const auto xlow = bin.xlow;
    const auto xup  = bin.xup;
    if (std::find(fXBins.begin(),fXBins.end(),xlow) == fXBins.end()) fXBins.emplace_back(xlow);
    if (std::find(fXBins.begin(),fXBins.end(),xup)  == fXBins.end()) fXBins.emplace_back(xup);

    // do y bin boundaries next
    const auto ylow = bin.ylow;
    const auto yup  = bin.yup;
    if (std::find(fYBins.begin(),fYBins.end(),ylow) == fYBins.end()) fYBins.emplace_back(ylow);
    if (std::find(fYBins.begin(),fYBins.end(),yup)  == fYBins.end()) fYBins.emplace_back(yup);
  }

  // sort the bins in decreasing order for histograms
  std::sort(fXBins.begin(),fXBins.end());
  std::sort(fYBins.begin(),fYBins.end());
}

void Limits2D::FillRValHists()
{
  std::cout << "Make histograms..." << std::endl;
 
  // loop over all r-vals, and make a histogram + contour histogram for each 
  for (const auto & RVal : Combine::RValVec)
  {
    // new the hist
    fHistMap[RVal] = new TH2F(Form("%s_hist",RVal.Data()),"",fXBins.size()-1,&fXBins[0],fYBins.size()-1,&fYBins[0]);
    
    // fill the newed hist
    auto & hist = fHistMap[RVal];
    for (const auto & bin : fAllBins)
    {
      hist->Fill(bin.xcenter,bin.ycenter,bin.rvalmap.at(RVal));
    }

    // clone for contours
    const TString cont_name = RVal+"_cont";
    fHistMap[cont_name] = (TH2F*)hist->Clone(cont_name.Data());
    
    // make contours
    auto & cont_hist = fHistMap[cont_name];
    cont_hist->SetContour(1);
    cont_hist->SetContourLevel(0,1.f);
    cont_hist->SetLineColor(kBlack);
    cont_hist->SetLineWidth(3);
  }
}

void Limits2D::DrawLimits()
{
  std::cout << "Draw 2D limits and save..." << std::endl;

  // final style
  if (fDoObserved) fHistMap["robs_cont"]->SetLineColor(kRed);
  fHistMap["rexp_cont"]->SetLineStyle(7);

  // make hist axes
  auto & hist = (fDoObserved ? fHistMap["robs"] : fHistMap["rexp"]);
  hist->GetXaxis()->SetTitle("#Lambda [GeV]");  
  hist->GetYaxis()->SetTitle("c#tau [cm]");
  hist->GetZaxis()->SetTitle("#sigma_{95% CL}/#sigma_{th}");

  // make canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(1);
  canv->SetLogz(1);
  canv->SetName("Canvas");

  // Set range user for limits in z!
  hist->GetZaxis()->SetRangeUser(1e-3,1e2);

  // draw histogram + contours
  hist->Draw("COLZ");
  if (fDoObserved) fHistMap["robs_cont"]->Draw("CONT3 same");
  fHistMap["rexp_cont"]->Draw("CONT3 same");
  fHistMap["r1sigdown_cont"]->Draw("CONT3 same");
  fHistMap["r1sigup_cont"]->Draw("CONT3 same");

  // make legend and draw it
  auto leg = new TLegend(0.2,0.7,0.4,0.8);
  leg->SetName("Legend");
  if (fDoObserved) leg->AddEntry(fHistMap["robs_cont"],"Observed 95% CL","L");
  leg->AddEntry(fHistMap["rexp_cont"],"Expected 95% CL","L");
  leg->AddEntry(fHistMap["r1sigup_cont"],"#pm 1 s.d. (exp)","L");
  leg->Draw("same");

  // cms style
  Common::CMSLumi(canv,0,fEra);
  
  // save it!
  Common::SaveAs(canv,Form("%s_GMSB",fOutText.Data()));

  // write it all out!
  fOutFile->cd();
  for (const auto & HistPair : fHistMap)
  {
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
  leg->Write(leg->GetName(),TObject::kWriteDelete);
  canv->Write(canv->GetName(),TObject::kWriteDelete);

  // delete everything
  delete canv;
  delete leg;
}

void Limits2D::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));
  std::string str; // tmp string

  // give grand title
  fConfigPave->AddText("***** Limit Config *****");

  fConfigPave->AddText(Form("Limits2D Config: %s",fLimitConfig.Data()));
  std::ifstream limitfile(Form("%s",fLimitConfig.Data()),std::ios::in);
  while (std::getline(limitfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // store input names
  fConfigPave->AddText(Form("Input directory: %s",fInDir.Data()));
  fConfigPave->AddText(Form("Input filenames: %s",fInFileName.Data()));

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

Float_t Limits2D::ZValue(const Float_t x, const Float_t x1, const Float_t x2, 
			 const Float_t y, const Float_t y1, const Float_t y2, 
			 const Float_t fQ11, const Float_t fQ12, const Float_t fQ21, const Float_t fQ22)
{
  // interpolation constants
  const auto denom = (x1-x2)*(y1-y2);
  const auto a0 = (fQ11*x2*y2-fQ12*x2*y1-fQ21*x1*y2+fQ22*x1*y1)/denom;
  const auto a1 = (-fQ11*y2+fQ12*y1+fQ21*y2-fQ22*y1)/denom;
  const auto a2 = (-fQ11*x2+fQ12*x2+fQ21*x1-fQ22*x1)/denom;
  const auto a3 = (fQ11-fQ12-fQ21+fQ22)/denom;
  
  // interpolated value
  return a0 + a1*x + a2*y + a3*x*y;
}

void Limits2D::SetupDefaults()
{
  std::cout << "Setup default values..." << std::endl;
  
  fDoObserved = false;
  fDumpBins = false;
  fXMinWidthDiv = 10.f;
  fYMinWidthDiv = 10.f;
  fNX_Interp = 10;
  fNY_Interp = 10;
}

void Limits2D::SetupLimitConfig()
{
  std::cout << "Reading limit config..." << std::endl;

  std::ifstream infile(Form("%s",fLimitConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("do_observed=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"do_observed=");
      Common::SetupBool(str,fDoObserved);
    }
    else if (str.find("dump_bins=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"dump_bins=");
      Common::SetupBool(str,fDumpBins);
    }
    else if (str.find("x_min_width_div=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"x_min_width_div=");
      fXMinWidthDiv = std::atof(str.c_str());
    }
    else if (str.find("y_min_width_div=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"y_min_width_div=");
      fYMinWidthDiv = std::atof(str.c_str());
    }
    else if (str.find("nx_interp=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"nx_interp=");
      fNX_Interp = std::atoi(str.c_str());
    }
    else if (str.find("ny_interp=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"ny_interp=");
      fNY_Interp = std::atoi(str.c_str());
    }
    else 
    {
      std::cerr << "Aye... your limit config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void Limits2D::SetupCommon()
{
  std::cout << "Setup Common Config..." << std::endl;

  Common::SetupEras();
}

void Limits2D::SetupCombine()
{
  std::cout << "Setup Combine Config..." << std::endl;

  Combine::SetupRValVec(fDoObserved);
  Combine::SetupGMSB(fInDir,fInFileName);
  Combine::RemoveGMSBSamples();
  Combine::SetupGMSBSubGroups();
}
