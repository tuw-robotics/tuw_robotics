/**
 * @author Markus Bader <markus.bader@tuwien.ac.at>
 **/
#ifndef TUW__DEBUG_HPP
#define TUW__DEBUG_HPP

#include <string>
#include <vector>
#include <chrono>

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
        void add(const std::string &name);
        std::string plot();
        std::string info();
        

    private:
        std::vector<NamedTimePoint> measurments_;
    };
};
#endif // TUW_DEBUG__TUW_DEBUG_HPP
