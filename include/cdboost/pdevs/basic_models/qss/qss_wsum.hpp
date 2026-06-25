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

#include <array>
#include <cdboost/pdevs/atomic.hpp>
#include <cdboost/pdevs/basic_models/qss/qss_msg.hpp>
#include <limits>
#include <vector>

namespace cdboost::pdevs::qss {

    /**
     * QSS Weighted Sum (N inputs).
     *
     * Computes y = K[0]*u0 + K[1]*u1 + ... + K[N-1]*u(N-1).
     * Fires immediately (sigma=0) whenever any input is received.
     *
     * Input:  qss_msg{i, u_i}  — value on channel i  (0 ≤ i < N)
     * Output: qss_msg{0, y}    — weighted sum
     *
     * @tparam N     Number of inputs (compile-time).
     * @tparam TIME  Simulation time type.
     */
    template <std::size_t N, typename TIME>
        requires cdboost::concepts::Time<TIME>
    class qss_wsum : public cdboost::pdevs::atomic<TIME, qss_msg> {
      public:
        std::array<double, N> inputs{};
        bool pending;
        TIME sigma;
        std::array<double, N> coeffs;

        explicit qss_wsum(std::array<double, N> k) : coeffs(k) {
            inputs.fill(0.0);
            pending = false;
            sigma   = this->infinity;
        }

        void internal() override {
            pending = false;
            sigma   = this->infinity;
        }

        TIME advance() const noexcept override {
            return sigma;
        }

        std::vector<qss_msg> out() const noexcept override {
            double y = 0.0;
            for (std::size_t i = 0; i < N; ++i)
                y += coeffs[i] * inputs[i];
            return {qss_msg{0, y}};
        }

        void external(const std::vector<qss_msg> &mb, const TIME &) override {
            for (const auto &msg : mb) {
                if (msg.channel >= 0 && static_cast<std::size_t>(msg.channel) < N)
                    inputs[static_cast<std::size_t>(msg.channel)] = msg.value;
            }
            pending = true;
            sigma   = TIME{0};
        }

        void confluence(const std::vector<qss_msg> &mb, const TIME &) override {
            internal();
            external(mb, TIME{});
        }
    };

} // namespace cdboost::pdevs::qss
