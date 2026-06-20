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
     * QSS Level-Triggered Switch (2-to-1 mux).
     *
     * Routes in0 to out when sw == 0, in1 to out otherwise.
     * Fires immediately (sigma=0) on every input update.
     *
     * Input:  qss_msg{0, u0}  — first signal
     *         qss_msg{1, u1}  — second signal
     *         qss_msg{2, sw}  — selector (0 → u0, else → u1)
     * Output: qss_msg{0, selected}
     *
     * @tparam TIME  Simulation time type.
     */
    template <typename TIME>
        requires cdboost::concepts::Time<TIME>
    class qss_switch : public cdboost::pdevs::atomic<TIME, qss_msg> {
      public:
        double u0;
        double u1;
        double sw;
        TIME sigma;

        qss_switch() : u0(0.0), u1(0.0), sw(0.0), sigma(this->infinity) {}

        void internal() override {
            sigma = this->infinity;
        }

        TIME advance() const noexcept override {
            return sigma;
        }

        std::vector<qss_msg> out() const noexcept override {
            return {qss_msg{0, sw != 0.0 ? u1 : u0}};
        }

        void external(const std::vector<qss_msg> &mb, const TIME &) override {
            for (const auto &msg : mb) {
                if (msg.channel == 0)
                    u0 = msg.value;
                else if (msg.channel == 1)
                    u1 = msg.value;
                else if (msg.channel == 2)
                    sw = msg.value;
            }
            sigma = TIME{0};
        }

        void confluence(const std::vector<qss_msg> &mb, const TIME &) override {
            internal();
            external(mb, TIME{});
        }
    };

} // namespace cdboost::pdevs::qss
