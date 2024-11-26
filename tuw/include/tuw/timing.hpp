/**
 * @author Markus Bader <markus.bader@tuwien.ac.at>
 **/
#ifndef TUW__TIMING_HPP
#define TUW__TIMING_HPP

#include <chrono>
#include <string>
#include <vector>

namespace tuw
{
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;
using NamedTimePoint = std::pair<std::string, TimePoint>;
/**
     * Debug
     */
class Timing
{
public:
  Timing();
  void start();
  void add(const std::string & name);
  std::string plot();
  std::string info();

private:
  std::vector<NamedTimePoint> measurments_;
};
}  // namespace tuw
#endif  // TUW__TIMING_HPP
