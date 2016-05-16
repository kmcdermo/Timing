void runTimeResolution(){
  gROOT->LoadMacro("TimeRes.cpp++g");
  
  TimeRes ResObj("input/tree.root","output",false,"png");
  ResObj.MakePlots();
}
