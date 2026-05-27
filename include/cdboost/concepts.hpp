// SPDX-License-Identifier: BSD-2-Clause
/**
 * Copyright (c) 2013-present, Damian Vicino
 * Carleton University, Universite de Nice-Sophia Antipolis
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

#include <concepts>
#include <limits>

namespace cdboost {

    // Customization point: specialize for TIME types where
    // std::numeric_limits<TIME>::has_infinity is false (e.g. boost::rational).
    // The returned value must compare greater than any valid simulation time.
    // See include/cdboost/rational_time.hpp for an example specialization.
    //
    // The default implementation is enabled only when has_infinity is true,
    // so omitting a required specialization is caught at concept-check time.
    template <class TIME> struct time_inf {
        static TIME value()
            requires(std::numeric_limits<TIME>::has_infinity)
        {
            return std::numeric_limits<TIME>::infinity();
        }
    };

    namespace concepts {

        // Time concept: contract for any type used as virtual time in a cdboost simulation.
        //
        // Required operations (all used by the engine at runtime):
        //   - totally ordered: <, >, ==, != for FEL ordering and transition guards
        //   - regular: copyable + default-constructible (TIME _last{}, _next{} in coordinator)
        //   - operator+(T,T)->T  used as  _last + model->advance()
        //   - operator-(T,T)->T  used as  t - _last, passed to external/confluence transitions
        //   - cdboost::time_inf<T>::value(): must compile and return T; the value MUST be
        //     greater than any valid simulation time. For IEEE-754 types the default
        //     time_inf<T> delegates to std::numeric_limits<T>::infinity() automatically.
        //     For other types (e.g. boost::rational), specialize cdboost::time_inf<T>
        //     (see include/cdboost/rational_time.hpp).
        //
        // Note: cadmium::concepts::Time checks std::numeric_limits<T>::infinity() directly
        // instead of cdboost::time_inf<T>::value(). The arithmetic and ordering requirements
        // are identical; only the infinity gate differs. A type satisfying both concepts
        // requires either IEEE-754 infinity or specializations of both time_inf<T> and
        // std::numeric_limits<T> (with has_infinity=true and a meaningful sentinel).
        template <typename T>
        concept Time = std::totally_ordered<T> && std::regular<T> && requires(T a, T b) {
            { a + b } -> std::same_as<T>;
            { a - b } -> std::same_as<T>;
        } && requires {
            { cdboost::time_inf<T>::value() } -> std::convertible_to<T>;
        };

    } // namespace concepts

} // namespace cdboost
