#pragma once

#include <boost/rational.hpp>
#include <limits>

#include <cdboost/model.hpp>

namespace cdboost {

template<>
struct time_inf<boost::rational<int>> {
    static boost::rational<int> value() {
        return boost::rational<int>{std::numeric_limits<int>::max(), 1};
    }
};

}  // namespace cdboost
