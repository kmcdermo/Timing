#include "Limits2D.hh"

const auto sortPairs = [](const auto & obj1, const auto & obj2){return obj1.second < obj2.second;};

Limits2D::Limits2D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext)
  : fInDir(indir), fInFileName(infilename), fDoObserved(doobserved), fOutText(outtext)
{  
  // setup common first
  Limits2D::SetupCombine();

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // make new output file
  fOutFile = TFile::Open(Form("%s.root",fOutText.Data()),"UPDATE");
}

Limits2D::~Limits2D() 
{
  delete fOutFile;
  delete fTDRStyle;
}

void Limits2D::MakeLimits2D()
{
  // First, import all measured values into bins
  std::cout << "Setup all filled bins..." << std::endl;
  Limits2D::SetupFilledBins();

  // Then, interpolate between all measured vales
  std::cout << "Setup all bins via interpolation..." << std::endl;
  Limits2D::SetupAllBins();

  if (false) {
  std::sort(fAllBins.begin(),fAllBins.end(),
	    [](const auto & bin1, const auto & bin2)
	    {
	      if (bin1.xcenter == bin2.xcenter) return bin1.ycenter < bin2.ycenter;
	      return bin1.xcenter < bin2.xcenter;
	    });

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

  // get bin boundaries for histogram making...
  std::vector<Double_t> xbins;
  std::vector<Double_t> ybins;
  std::cout << "Get bin boundaries..." << std::endl;
  Limits2D::GetBinBoundaries(xbins,ybins);

  // start making histograms and filling them
  std::map<TString,TH2F*> HistMap;
  std::cout << "Make histograms..." << std::endl;
  for (const auto & RVal : Combine::RValVec)
  {
    HistMap[RVal] = new TH2F(Form("%s_hist",RVal.Data()),"",xbins.size()-1,&xbins[0],ybins.size()-1,&ybins[0]);
    
    auto & hist = HistMap[RVal];
    for (const auto & bin : fAllBins)
    {
      hist->Fill(bin.xcenter,bin.ycenter,bin.rvalmap.at(RVal));
    }

    // clone for contours
    const TString cont_name = RVal+"_cont";
    HistMap[cont_name] = (TH2F*)hist->Clone(cont_name.Data());
    
    // make contours
    auto & cont_hist = HistMap[cont_name];
    cont_hist->SetContour(1);
    cont_hist->SetContourLevel(0,1.f);
    cont_hist->SetLineColor(kBlack);
    cont_hist->SetLineWidth(3);
  }
  std::cout << "Draw and save..." << std::endl;

  // final style
  if (fDoObserved) HistMap["robs_cont"]->SetLineColor(kRed);
  HistMap["rexp_cont"]->SetLineStyle(7);

  // make hist axes
  auto & hist = (fDoObserved ? HistMap["robs"] : HistMap["rexp"]);
  hist->GetXaxis()->SetTitle("#Lambda [GeV]");  
  hist->GetYaxis()->SetTitle("c#tau [cm]");
  hist->GetZaxis()->SetTitle("#sigma_{95% CL}/#sigma_{th}");

  // canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(1);
  canv->SetName("Canvas");

  // draw histogram + contours
  hist->Draw("COLZ");
  if (fDoObserved) HistMap["robs_cont"]->Draw("CONT3 same");
  HistMap["rexp_cont"]->Draw("CONT3 same");
  HistMap["r1sigdown_cont"]->Draw("CONT3 same");
  HistMap["r1sigup_cont"]->Draw("CONT3 same");

  // legend
  auto leg = new TLegend(0.2,0.7,0.4,0.8);
  leg->SetName("Legend");
  if (fDoObserved) leg->AddEntry(HistMap["robs_cont"],"Observed 95% CL","L");
  leg->AddEntry(HistMap["rexp_cont"],"Expected 95% CL","L");
  leg->AddEntry(HistMap["r1sigup_cont"],"#pm 1 s.d. (exp)","L");
  leg->Draw("same");

  // cms style
  Common::CMSLumi(canv,0);
  
  // save it!
  canv->SaveAs(Form("%s_GMSB.png",fOutText.Data()));
  canv->SaveAs(Form("%s_GMSB.pdf",fOutText.Data()));

  // write it!
  fOutFile->cd();
  for (const auto & HistPair : HistMap)
  {
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
  leg->Write(leg->GetName(),TObject::kWriteDelete);
  canv->Write(canv->GetName(),TObject::kWriteDelete);

  // delete everything
  delete canv;
  delete leg;
  for (auto & HistPair : HistMap) delete HistPair.second;
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

void Limits2D::GetBinBoundaries(std::vector<Double_t> & xbins, std::vector<Double_t> & ybins)
{
  for (const auto & bin : fAllBins)
  {
    const auto xlow = bin.xlow;
    const auto xup  = bin.xup;
    if (std::find(xbins.begin(),xbins.end(),xlow) == xbins.end()) xbins.emplace_back(xlow);
    if (std::find(xbins.begin(),xbins.end(),xup)  == xbins.end()) xbins.emplace_back(xup);

    const auto ylow = bin.ylow;
    const auto yup  = bin.yup;
    if (std::find(ybins.begin(),ybins.end(),ylow) == ybins.end()) ybins.emplace_back(ylow);
    if (std::find(ybins.begin(),ybins.end(),yup)  == ybins.end()) ybins.emplace_back(yup);
  }
  std::sort(xbins.begin(),xbins.end());
  std::sort(ybins.begin(),ybins.end());
}

void Limits2D::SetupCombine()
{
  Combine::SetupRValVec(fDoObserved);
  Combine::SetupGMSB(fInDir,fInFileName);
  Combine::RemoveGMSBSamples();
  Combine::SetupGMSBSubGroups();
}

void Limits2D::SetupFilledBins()
{
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

  // set width 
  const auto xwidth = xcentersvec[0].second / 10.f;
  const auto ywidth = ycentersvec[0].second / 10.f;
  
  // loop over centers and fill filledbins
  fFilledBins.resize(xcentersvec.size());
  for (auto i_fill = 0U; i_fill < xcentersvec.size(); i_fill++)
  {
    // x bin info
    const auto & x_s   = xcentersvec[i_fill].first;
    const auto xcenter = xcentersvec[i_fill].second;
    const auto xlow    = xcenter - (xwidth / 2.f);
    const auto xup     = xcenter + (xwidth / 2.f);

    fFilledBins[i_fill].resize(ycentersvec.size());
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
      fFilledBins[i_fill][j_fill] = {xlow,xcenter,xup,ylow,ycenter,yup,Combine::GMSBMap[name].rvalmap};
    }
  }
}

void Limits2D::SetupAllBins()
{
  // First put measured points in all bins
  for (const auto & xbins : fFilledBins)
  {
    for (const auto & xybin : xbins)
    {
      fAllBins.emplace_back(xybin);
    }
  }

  // Set number of divisions in x and y between measured points for interpolation
  const auto nx_interp = 2;
  const auto ny_interp = 2;

  // loop over filled bins and interpolate: https://en.wikipedia.org/wiki/Bilinear_interpolation#Alternative_algorithm
  const auto nx_fill = fFilledBins.size();
  for (auto i_fill = 0U; i_fill < nx_fill; i_fill++)
  {
    const Bool_t isEdgeX = (i_fill == (nx_fill-1));

    const auto ny_fill = fFilledBins[i_fill].size();
    for (auto j_fill = 0U; j_fill < ny_fill; j_fill++)
    {
      const Bool_t isEdgeY = (j_fill == (ny_fill-1));

      // bin numbers --> if at edges, do not overflow!
      const auto i_bin = (!isEdgeX ? i_fill : i_fill-1);
      const auto j_bin = (!isEdgeY ? j_fill : j_fill-1);

      // corner bins (11 and 22)
      const auto & binlow = fFilledBins[i_bin  ][j_bin  ];
      const auto & binup  = fFilledBins[i_bin+1][j_bin+1];

      // bin boundaries
      const auto xlow   = binlow.xup;
      const auto xup    = binup .xlow;
      const auto xwidth = (xup-xlow)/nx_interp;

      const auto ylow   = binlow.yup;
      const auto yup    = binup .ylow;
      const auto ywidth = (yup-ylow)/ny_interp;

      // center points for interpolation
      const auto x1 = binlow.xcenter;
      const auto x2 = binup .xcenter;
      const auto y1 = binlow.ycenter;
      const auto y2 = binup .ycenter;

      // make stripes in x when center equals filled bin
      if (!isEdgeX)
      {
	for (auto i_interp = 0; i_interp < nx_interp; i_interp++)
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
	    const auto fQ12 = fFilledBins[i_bin  ][j_bin+1].rvalmap.at(RVal);
	    const auto fQ21 = fFilledBins[i_bin+1][j_bin  ].rvalmap.at(RVal);
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
	for (auto j_interp = 0; j_interp < ny_interp; j_interp++)
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
	    const auto fQ12 = fFilledBins[i_bin  ][j_bin+1].rvalmap.at(RVal);
	    const auto fQ21 = fFilledBins[i_bin+1][j_bin  ].rvalmap.at(RVal);
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
      for (auto i_interp = 0; i_interp < nx_interp; i_interp++)
      {
	// for this new bin
	const auto xbinlow    = xlow + xwidth*(i_interp);
	const auto xbinup     = xlow + xwidth*(i_interp+1);
	const auto xbincenter = (xbinlow+xbinup)/2.f;
	
	for (auto j_interp = 0; j_interp < ny_interp; j_interp++)
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
	    const auto fQ12 = fFilledBins[i_bin  ][j_bin+1].rvalmap.at(RVal);
	    const auto fQ21 = fFilledBins[i_bin+1][j_bin  ].rvalmap.at(RVal);
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
