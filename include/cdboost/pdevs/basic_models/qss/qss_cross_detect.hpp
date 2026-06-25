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

#include <cdboost/pdevs/atomic.hpp>
#include <cdboost/pdevs/basic_models/qss/qss_msg.hpp>
#include <limits>
#include <vector>

namespace cdboost::pdevs::qss {

    /**
     * QSS Zero-Crossing Detector.
     *
     * Predicts when signal u with derivative Du will cross a fixed threshold.
     * On crossing, emits +1.0 (upward) or -1.0 (downward).
     *
     * sigma = max(0, (threshold - u) / Du)
     *
     * Input:  qss_msg{0, u}   — signal value
     *         qss_msg{1, Du}  — signal derivative
     * Output: qss_msg{0, ±1}  — crossing direction
     *
     * @tparam TIME  Simulation time type.
     */
    template <typename TIME>
        requires cdboost::concepts::Time<TIME>
    class qss_cross_detect : public cdboost::pdevs::atomic<TIME, qss_msg> {
      public:
        double u;
        double Du;
        TIME sigma;
        const double threshold;

        explicit qss_cross_detect(double threshold_ = 0.0)
            : u(0.0), Du(0.0), sigma(this->infinity), threshold(threshold_) {}

        void internal() override {
            sigma = this->infinity;
        }

        TIME advance() const noexcept override {
            return sigma;
        }

        std::vector<qss_msg> out() const noexcept override {
            return {qss_msg{0, Du > 0.0 ? 1.0 : -1.0}};
        }

        void external(const std::vector<qss_msg> &mb, const TIME &) override {
            for (const auto &msg : mb) {
                if (msg.channel == 0)
                    u = msg.value;
                else if (msg.channel == 1)
                    Du = msg.value;
            }
            if (Du == 0.0) {
                sigma = this->infinity;
                return;
            }
            double t = (threshold - u) / Du;
            sigma    = (t <= 0.0) ? TIME{0} : static_cast<TIME>(t);
        }

        void confluence(const std::vector<qss_msg> &mb, const TIME &) override {
            internal();
            external(mb, TIME{});
        }
    };

} // namespace cdboost::pdevs::qss
