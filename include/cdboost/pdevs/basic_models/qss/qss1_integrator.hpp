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

#include <algorithm>
#include <cdboost/pdevs/atomic.hpp>
#include <cdboost/pdevs/basic_models/qss/qss_msg.hpp>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace cdboost::pdevs::qss {

    /**
     * QSS1 Quantized Integrator (Kofman & Junco 2001, Section 4).
     *
     * Integrates dx/dt = u(t) using a piecewise-constant quantized approximation.
     * State variable x fires an output event whenever x crosses a quantization
     * boundary q ± dq, where dq = max(dqmin, dqrel * |x|).
     *
     * Input/output use qss_msg (channel 0 for both).
     *   in:  qss_msg{0, u}  — derivative u = dx/dt
     *   out: qss_msg{0, q}  — quantized state at the next crossing boundary
     *
     * At t=0 the integrator fires with sigma=0 to announce its initial value.
     *
     * @tparam TIME  Simulation time type satisfying cdboost::concepts::Time.
     */
    template <typename TIME>
        requires cdboost::concepts::Time<TIME>
    class qss1_integrator : public cdboost::pdevs::atomic<TIME, qss_msg> {
      public:
        double x;   // continuous state
        double u;   // current derivative
        double q;   // last quantized output
        double dq;  // current quantum
        TIME sigma; // time to next internal transition

        const double dqmin;
        const double dqrel;

        qss1_integrator(double x0, double dqmin_, double dqrel_) : dqmin(dqmin_), dqrel(dqrel_) {
            u  = 0.0;
            x  = x0;
            q  = x0;
            dq = std::max(dqmin, std::abs(x0) * dqrel);
            if (dq == 0.0)
                dq = dqmin;
            sigma = TIME{0};
        }

        void internal() override {
            x += u * static_cast<double>(sigma);
            q  = x;
            dq = std::max(dqmin, dqrel * std::abs(x));
            if (dq == 0.0)
                dq = dqmin;
            sigma = (u == 0.0) ? this->infinity : static_cast<TIME>(dq / std::abs(u));
        }

        TIME advance() const noexcept override {
            return sigma;
        }

        std::vector<qss_msg> out() const noexcept override {
            double val = x + u * static_cast<double>(sigma);
            return {qss_msg{0, val}};
        }

        void external(const std::vector<qss_msg> &mb, const TIME &e) override {
            double new_u = u;
            for (const auto &msg : mb) {
                if (msg.channel == 0)
                    new_u = msg.value;
            }
            x += u * static_cast<double>(e);
            u  = new_u;
            dq = std::max(dqmin, dqrel * std::abs(x));
            if (dq == 0.0)
                dq = dqmin;

            if (u == 0.0) {
                sigma = this->infinity;
                return;
            }

            double t    = std::numeric_limits<double>::infinity();
            double t_up = (q + dq - x) / u;
            double t_dn = (q - dq - x) / u;
            if (t_up > 0.0)
                t = std::min(t, t_up);
            if (t_dn > 0.0)
                t = std::min(t, t_dn);
            if (std::abs(x - q) >= dq)
                t = 0.0;

            sigma = static_cast<TIME>(t);
        }

        void confluence(const std::vector<qss_msg> &mb, const TIME &) override {
            internal();
            external(mb, TIME{});
        }
    };

} // namespace cdboost::pdevs::qss
