/**
 * @author Markus Bader <markus.bader@tuwien.ac.at>
 **/

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <tuw/timing.hpp>

using namespace tuw;

Timing::Timing() {}

std::string Timing::info()
{
  std::stringstream ss;
  for (size_t i = 1; i < measurments_.size(); i++) {
    ss << measurments_[i].first << " "
       << (measurments_[i].second - measurments_[i - 1].second).count() << std::endl;
  }
  return ss.str();
}

std::string Timing::plot()
{
  if (measurments_.size() < 2) {
    return "no timings";
  }
  std::stringstream ss;
  double d_total = (measurments_.back().second - measurments_[0].second).count() / 1000. / 1000.;
  for (size_t i = 1; i < measurments_.size(); i++) {
    double d_m = (measurments_[i].second - measurments_[i - 1].second).count() / 1000. / 1000.;
    ss << std::setw(20) << measurments_[i].first << " " << std::setw(20) << std::fixed
       << std::setprecision(6) << d_m << " ms: ";
    double ratio = d_m / d_total;
    for (int p = 0; p < 100. * ratio; p++) {
      ss << "#";
    }
    ss << std::endl;
  }
  return ss.str();
}

void Timing::start()
{
  measurments_.clear();
  measurments_.push_back(NamedTimePoint("start", std::chrono::high_resolution_clock::now()));
}

void Timing::add(const std::string & name)
{
  measurments_.push_back(NamedTimePoint(name, std::chrono::high_resolution_clock::now()));
}
