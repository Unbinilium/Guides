/*
 Note:
    - CPU side multi-threading cv::inRange() accelerate
    - currently no worker implementation, not stable, substantial improvement on large frames
 */

#ifndef FAST_IN_RANGE_HPP
#define FAST_IN_RANGE_HPP

#include <cstdint>
#include <future>

#include <opencv2/core.hpp>
#include <opencv2/core/mat.hpp>

namespace ubn {
    class fastInRange {
    private:
        uint      thr;
        const int b_h;
        cv::Mat   out;
        
        inline void Impl(cv::Mat& input, cv::Scalar&& l, cv::Scalar&& h, cv::Mat& output, const int h_s) {
            const int h_e { h_s + b_h };
            
            std::future<void> f;
            if (--thr) { f = std::async(std::launch::async, &fastInRange::Impl, this, std::ref(input), l, h, std::ref(output), std::ref(h_e)); }
            cv::inRange(input.rowRange(h_s, h_e), l, h, output.rowRange(h_s, h_e));
            if (thr) { f.get(); }
        }
        
    public:
        inline fastInRange(cv::Mat& input, cv::Scalar&& l, cv::Scalar&& h, cv::Mat& output) :
        thr { std::thread::hardware_concurrency() },
        b_h { static_cast<int>(std::floor(input.rows / thr)) },
        out { cv::Mat(input.size(), CV_8UC1) }
        {
            this->Impl(input, std::move(l), std::move(h), out, 0);
            out.copyTo(output);
        }
    };
}

#endif
