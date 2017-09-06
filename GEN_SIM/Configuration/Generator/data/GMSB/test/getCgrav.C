void getCgrav(const int lambda, const float ctau)
{
  std::cout << "cgrav: " << std::sqrt(ctau)*(-10.7968+3.51482*std::pow(lambda,0.3871)) << std::endl;
}
