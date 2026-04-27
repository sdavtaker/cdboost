#pragma once

#include <boost/rational.hpp>

#include <cdboost/model.hpp>
#include <limits>

namespace cdboost {

    template <> struct time_inf<boost::rational<int>> {
        static boost::rational<int> value() {
            return boost::rational<int>{std::numeric_limits<int>::max(), 1};
        }
    };

} // namespace cdboost
