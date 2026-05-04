// SPDX-License-Identifier: BSD-2-Clause
#pragma once

#include <boost/rational.hpp>

#include <cdboost/log.hpp>
#include <cdboost/model.hpp>
#include <limits>

namespace cdboost {

    template <> struct time_inf<boost::rational<int>> {
        static boost::rational<int> value() {
            return boost::rational<int>{std::numeric_limits<int>::max(), 1};
        }
    };

} // namespace cdboost

namespace cdboost::log {

    template <>
    inline double to_sim_double<boost::rational<int>>(const boost::rational<int> &t) noexcept {
        return static_cast<double>(t.numerator()) / static_cast<double>(t.denominator());
    }

} // namespace cdboost::log
