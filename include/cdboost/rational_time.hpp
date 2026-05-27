// SPDX-License-Identifier: BSD-2-Clause
/**
 * Copyright (c) 2026-present, Damian Vicino
 * Carleton University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <boost/rational.hpp>

#include <cdboost/log.hpp>
#include <cdboost/model.hpp>
#include <limits>

// std::numeric_limits specialization for boost::rational<int>.
// Boost does not provide this specialization; we define it so boost::rational<int>
// satisfies cdboost::concepts::Time (and cadmium::concepts::Time).
// The infinity sentinel is {INT_MAX, 1} — greater than any representable rational time.
// All scalar members are provided to avoid missing-member errors in Boost's internal
// template metaprogramming (which reads std::numeric_limits of arbitrary types).
namespace std {
    template <> struct numeric_limits<boost::rational<int>> {
        static constexpr bool is_specialized    = true;
        static constexpr bool is_signed         = true;
        static constexpr bool is_integer        = false;
        static constexpr bool is_exact          = true;
        static constexpr bool has_infinity      = true;
        static constexpr bool has_quiet_NaN     = false;
        static constexpr bool has_signaling_NaN = false;
        static constexpr bool is_bounded        = true;
        static constexpr bool is_modulo         = false;
        static constexpr bool is_iec559         = false;
        static constexpr bool traps             = false;
        static constexpr bool tinyness_before   = false;
        static constexpr int radix              = 2;
        static constexpr int digits             = 0;
        static constexpr int digits10           = 0;
        static constexpr int max_digits10       = 0;
        static constexpr int min_exponent       = 0;
        static constexpr int min_exponent10     = 0;
        static constexpr int max_exponent       = 0;
        static constexpr int max_exponent10     = 0;

        static boost::rational<int> infinity() noexcept {
            return boost::rational<int>{numeric_limits<int>::max(), 1};
        }
        static boost::rational<int> min() noexcept {
            return boost::rational<int>{numeric_limits<int>::min(), 1};
        }
        static boost::rational<int> max() noexcept {
            return boost::rational<int>{numeric_limits<int>::max() - 1, 1};
        }
        static boost::rational<int> lowest() noexcept {
            return boost::rational<int>{numeric_limits<int>::min(), 1};
        }
        static boost::rational<int> quiet_NaN() noexcept {
            return boost::rational<int>{};
        }
        static boost::rational<int> signaling_NaN() noexcept {
            return boost::rational<int>{};
        }
        static boost::rational<int> denorm_min() noexcept {
            return boost::rational<int>{1, numeric_limits<int>::max()};
        }
        static boost::rational<int> epsilon() noexcept {
            return boost::rational<int>{1, numeric_limits<int>::max()};
        }
        static boost::rational<int> round_error() noexcept {
            return boost::rational<int>{};
        }
    };
} // namespace std

namespace cdboost {

    template <> struct time_inf<boost::rational<int>> {
        static boost::rational<int> value() {
            return std::numeric_limits<boost::rational<int>>::infinity();
        }
    };

} // namespace cdboost

namespace cdboost::log {

    template <>
    inline double to_sim_double<boost::rational<int>>(const boost::rational<int> &t) noexcept {
        return static_cast<double>(t.numerator()) / static_cast<double>(t.denominator());
    }

} // namespace cdboost::log
