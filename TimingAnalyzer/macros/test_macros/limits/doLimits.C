struct Coord
{
  Coord() {}
  Coord(const Double_t x, const Double_t y) 
    : x(x), y(y) {}

  Double_t x;
  Double_t y;
};

typedef std::vector<Coord> CoordVec;

void setupCoordVecMap(std::map<TString,CoordVec> & coordVecMap, const TString & infilename)
{
  std::ifstream infile(infilename.Data(),std::ios::in);
  TString label; Double_t x,y;
  
  while (infile >> label >> x >> y)
  {
    coordVecMap[label].emplace_back(x,y);
  }
}

struct Opts
{
  Opts() {}
  Opts(const Int_t style, const Int_t width, const Color_t color)
    : style(style), width(width), color(color) 
  {
    if (width != 0) draw = "L";
    else            draw = "F";
  }

  Int_t style;
  Int_t width;
  Color_t color;
  TString draw;
};

void setupOptsMap(std::map<TString,Opts> & optsMap)
{
  std::ifstream infile("input/gmsb_config.txt",std::ios::in);
  TString label; short style, width, color;
  
  while (infile >> label >> style >> width >> color)
  {
    optsMap[label] = {style,width,color};
  }
}

void doLimits()
{
  std::map<TString,CoordVec> coordVecMap;
  setupCoordVecMap(coordVecMap,"input/old_gmsb.txt");
  setupCoordVecMap(coordVecMap,"input/abcd_2x2_2016.txt");
  setupCoordVecMap(coordVecMap,"input/abcd_2x2_2017.txt");

  std::map<TString,Opts> optsMap;
  setupOptsMap(optsMap);
  
  std::map<TString,TGraph*> graphMap;
  for (const auto & coordVecPair : coordVecMap)
  {
    const auto & label = coordVecPair.first;
    const auto & coordVec = coordVecPair.second;
    const auto & opts = optsMap[label];
    const auto npoints = coordVec.size();

    graphMap[label] = new TGraph(npoints);
    auto & graph = graphMap[label];
    graph->SetName(label.Data());
    graph->SetTitle("GMSB Limit Comparison");

    // set opts
    if (opts.width > 0)
    {
      graph->SetLineStyle(opts.style);
      graph->SetLineWidth(opts.width);
      graph->SetLineColor(opts.color);
    }
    else
    {
      graph->SetFillColorAlpha(opts.color,0.4);
    }

    for (auto ipoint = 0U; ipoint < npoints; ipoint++)
    {
      const auto coord = coordVec[ipoint];
      graph->SetPoint(ipoint,coord.x,coord.y);
    }
  }

  // draw it
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();

  auto leg = new TLegend(0.7,0.7,1.0,1.0);
  auto i = 0;
  for (auto & graphPair : graphMap)
  {
    auto label = graphPair.first;
    auto & graph = graphPair.second;
    const auto & draw = optsMap[label].draw;

    label.ReplaceAll("_"," ");
    leg->AddEntry(graph,label.Data(),draw.Data());
    
    graph->Draw(i>0?Form("%s SAME",draw.Data()):Form("A%s",draw.Data()));
    graph->GetXaxis()->SetTitle("#Lambda [TeV]");
    graph->GetYaxis()->SetTitle("c#tau [cm]");

    graph->GetXaxis()->SetLimits(75,375);
    graph->GetHistogram()->SetMinimum(1e-2);
    graph->GetHistogram()->SetMaximum(1e5);

    i++;
  }

  leg->Draw("same");

  // draw mass axis
  auto axis = new TGaxis(100,0.1,350,0.1,139.7,503.4,505,"");
  axis->SetTitle("M_{#tilde{#chi}^{0}_{1}} [GeV]");
  axis->SetTitleSize(0.035);
  axis->SetLabelSize(0.035);
  axis->Draw();

  canv->SaveAs("limits.pdf");
  canv->SaveAs("limits.png");
}
