#include "Limits2D.hh"

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
  Limits2D::SetupFilledBins();

  // Then, interpolate between all measured vales
  Limist2D::SetupAllBins();

  // get bin boundaries for histogram making...
  std::vector<Double_t> xbins;
  std::vector<Double_t> ybins;
  Limits2D::GetBinBoundaries(xbins,ybins);

  // start making histograms and filling them
  std::map<TString,TH2F*> HistMap;
  for (const auto & RVal : RValVec)
  {
    HistMap[RVal] = new TH2F(Form("%s_hist",RVal.Data()),"",xbins.size()-1,&xbins[0],ybins.size()-1,&ybins[0]);
    
    auto & hist = HistMap[RVal];
    for (const auto & bin : fAllBins)
    {
      hist->Fill(bin.xcenter,bin.ycenter,bin.rvalmap[RVal]);
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

  // final style
  if (fDoObserved) HistMap["robs_cont"]->SetLineColor(kRed);
  HistMap["rexp_cont"]->SetLineStyle(7);

  // make hist axes
  auto & hist = (fDoObserved ? HistMap["robs"] : HistMap["rexp"]);
  hist->GetXaxis()->SetTitle("#Lambda [GeV]");  
  hist->GetYaxis()->SetTitle("c#tau [cm]");
  hist->GetZaxis()->SetTitle("#sigma_{95% CL}/#sigma_{th}");

  // legened
  auto leg = new TLegend(0.7,0.7,0.85,0.85);
  leg->SetName("Legend");
  if (fDoObserved) leg->AddEntry(HistMap["robs_cont"],"Observed 95% CL","L");
  leg->AddEntry(HistMap["rexp_cont"],"Expected 95% CL","L");
  leg->AddEntry(HistMap["r1sigup_cont"],"#pm 1 s.d. (exp)","L");

  // canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetName("Canvas");

  // draw histogram + contours
  hist->Draw("COLZ");
  if (fDoObserved) HistMap["robs_cont"]->Draw("CONT3 same");
  HistMap["rexp_cont"]->Draw("CONT3 same");
  HistMap["rs1sigdown_cont"]->Draw("CONT3 same");
  HistMap["rs1sigup_cont"]->Draw("CONT3 same");

  // cms style
  Common::CMSLumi(canv,0);
  
  // save it!
  canv->SaveAs(Form("%s_%s.png",fOutText.Data(),groupname.Data()));
  canv->SaveAs(Form("%s_%s.pdf",fOutText.Data(),groupname.Data()));
  
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
  Combine::SetupGMSB();
  Combine::RemoveGMSBSamples();
  Combine::SetupGMSBSubGroups();
}

void Limits2D::SetupFilledBins()
{
  // get x-centers
  std::vector<Float_t> xcenters;
  for (const auto & GMSBPair : Combine::GMSBMap)
  {
    const auto xcenter = GMSBPair.second.lambda;
    if (std::find(xcenters.begin(),xcenters.end(),xcenter) == xcenters.end()) xcenters.emplace_back(xcenter);
  }
  std::sort(xcenters.begin(),xcenters.end());

  // get y-centers
  std::vector<Float_t> ycenters;
  for (const auto & GMSBPair : Combine::GMSBMap)
  {
    const auto ycenter = GMSBPair.second.ctau;
    if (std::find(ycenters.begin(),ycenters.end(),ycenter) == ycenters.end()) ycenters.emplace_back(ycenter);
  }
  std::sort(ycenters.begin(),ycenters.end());

  // set width 
  const auto xwidth = xcenters[0] / 10.f;
  const auto ywidth = ycenters[0] / 10.f;
  
  // loop over centers and fill filledbins
  fFilledBins.resize(xcenters.size());
  for (auto i_fill = 0; i_fill < xcenters.size(); i_fill++)
  {
    // x bin info
    const auto xcenter = xcenters[i_fill];
    const auto xlow    = xcenter - (xwidth / 2.f);
    const auto xup     = xcenter + (xwidth / 2.f);
    const TString x_s  = Form("%f",xcenter);

    fFilledBins[i_fill].resize(ycenters.size());
    for (auto j_fill = 0; j_fill < ycenters.size(); j_fill++)
    {
      // y bin info
      const auto ycenter = ycenters[j_fill];
      const auto ylow    = ycenter - ywidth;
      const auto yup     = ycenter + ywidth;
      const TString y_s  = (ycenter == 0.1 ? "0p1" : Form("%f",ycenter));      
      
      // get name of gmsb sample
      const TString name = "GMSB_L"+x_s+"TeV_CTau"+y_s+"cm";

      // set basic info for bins
      fFilledBins[i_fill][j_fill] = {xlow,xcenter,xup,ylow,ycenter,yup,fGMSBMap[name].rvalmap};
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
  const auto nx_interp = 10;
  const auto ny_interp = 10;

  // loop over filled bins and interpolate: https://en.wikipedia.org/wiki/Bilinear_interpolation#Alternative_algorithm
  const auto nx_fill = fFilledBins.size();
  for (auto i_fill = 0; i_fill < nx_fill; i_fill++)
  {
    const Bool_t isEdgeX = (i_fill == (nx_fill-1));

    const auto ny_fill = filled_bins[i_fill].size();
    for (auto j_fill = 0; j_fill < n_fill; j++)
    {
      const Bool_t isEdgeY = (j_fill == (ny_fill-1));

      // bin numbers --> if at edges, do not overflow!
      const auto i_bin = (isEdgeX ? i_fill - 1 : i_fill);
      const auto j_bin = (isEdgeY ? j_fill - 1 : j_fill);

      // corner bins (11 and 22)
      const auto & binlow = fFilledBins[i_bin][j_bin];
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
	  const auto xbinlow    = xlow + xwidth*i_interp;
	  const auto xbinup     = xlow + xwidth*(i_interp+1);
	  const auto xbincenter = (xbinlow+xbinup)/2.f;
	  
	  const auto ybinlow    = (!isEdgeY ? binlow : binup).ylow;
	  const auto ybinup     = (!isEdgeY ? binlow : binup).yup;
	  const auto ybincenter = (!isEdgeY ? binlow : binup).ycenter;

	  // loop over values to interpolate!
	  std::map<TString,Float_t> tmpmap; // rval map for tmp zs
	  for (const auto & RVal : RValVec)
	  {
	    // values
	    const auto fQ11 = binlow.rvalmap[RVal];
	    const auto fQ12 = fFilledBins[i_bin][j_bin+1].rvalmap[RVal];
	    const auto fQ21 = fFilledBins[i_bin+1][j_bin].rvalmap[RVal];
	    const auto fQ22 = binup.rvalmap[RVal];

	    tmpmap[RVal] = Limits2D::ZValue(xbincenter,x1,x2,ybincenter,y1,y2,fQ11,fQ12,fQ21,fQ22);	    
	  }
	  
	  // place bin into all bins
	  fAllBins.emplace_back(xbinlow,xbincenter,xbinup,ybinlow,ybincenter,ybinup,tmpmap};
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

	  const auto ybinlow    = ylow + ywidth*j_interp;
	  const auto ybinup     = ylow + ywidth*(j_interp+1);
	  const auto ybincenter = (ybinlow+ybinup)/2.f;

	  // loop over values to interpolate!
	  std::map<TString,Float_t> tmpmap; // rval map for tmp zs
	  for (const auto & RVal : RValVec)
	  {
	    // values
	    const auto fQ11 = binlow.rvalmap[RVal];
	    const auto fQ12 = fFilledBins[i_bin][j_bin+1].rvalmap[RVal];
	    const auto fQ21 = fFilledBins[i_bin+1][j_bin].rvalmap[RVal];
	    const auto fQ22 = binup.rvalmap[RVal];

	    tmpmap[RVal] = Limits2D::ZValue(xbincenter,x1,x2,ybincenter,y1,y2,fQ11,fQ12,fQ21,fQ22);	    
	  }
	  
	  // place bin into all bins
	  fAllBins.emplace_back(xbinlow,xbincenter,xbinup,ybinlow,ybincenter,ybinup,tmpmap};
	}
      }

      // skip interpolation of inbetween boxes if either condition is met
      if (isEdgeX || isEdgeY) continue;

      // interpolate inner boxes
      for (auto i_interp = 0; i_interp < nx_interp; i_interp++)
      {
	// for this new bin
	const auto xbinlow    = xlow + xwidth*i_interp;
	const auto xbinup     = xlow + xwidth*(i_interp+1);
	const auto xbincenter = (xbinlow+xbinup)/2.f;
	
	for (auto j_interp = 0; j_interp < ny_interp; j_interp++)
        {
	  // for this new bin
	  const auto ybinlow    = ylow + ywidth*i_interp;
	  const auto ybinup     = ylow + ywidth*(i_interp+1);
	  const auto ybincenter = (ybinlow+ybinup)/2.f;

	  // loop over values to interpolate!
	  std::map<TString,Float_t> tmpmap; // rval map for tmp zs
	  for (const auto & RVal : RValVec)
	  {
	    // values
	    const auto fQ11 = binlow.rvalmap[RVal];
	    const auto fQ12 = fFilledBins[i_bin][j_bin+1].rvalmap[RVal];
	    const auto fQ21 = fFilledBins[i_bin+1][j_bin].rvalmap[RVal];
	    const auto fQ22 = binup.rvalmap[RVal];

	    tmpmap[RVal] = Limits2D::ZValue(xbincenter,x1,x2,ybincenter,y1,y2,fQ11,fQ12,fQ21,fQ22);	    
	  }
	
	  // place bin into all bins
	  fAllBins.emplace_back(xbinlow,xbincenter,xbinup,ybinlow,ybincenter,ybinup,tmpmap};
	} // end loop over nbins in y direction to interpolate
      } // end loop over nbins in x direction to interpolate
    } // end loop over measured y vals
  } // end loop over measured x vals
}
