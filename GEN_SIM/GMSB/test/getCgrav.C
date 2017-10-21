void getCgrav(const int lambda, const float ctau)
{
  std::cout << "cgrav: " << std::sqrt(ctau)*(-5.857+1.893*std::pow(lambda,0.4652)) << std::endl; 
}
