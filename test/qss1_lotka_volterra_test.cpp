// SPDX-License-Identifier: BSD-2-Clause
/**
 * Lotka-Volterra QSS1 integration test for cdboost.
 *
 * Mirrors the cadmium qss1_lotka_volterra_test but uses cdboost's qss_msg API.
 * Runs the same manual PDEVS simulation for t in [0, 50] and checks:
 *   1. Both populations remain strictly positive.
 *   2. The conserved quantity V = x1 - ln(x1) + x2 - ln(x2) stays within
 *      tolerance of the initial value (QSS1 is O(dq) accurate).
 *   3. Event count is in a reasonable range (not Zeno, not stalled).
 *   4. Both populations oscillate beyond their initial value.
 */

#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <cdboost/pdevs/basic_models/qss/qss1_integrator.hpp>
#include <cdboost/pdevs/basic_models/qss/qss_multiplier.hpp>
#include <cdboost/pdevs/basic_models/qss/qss_wsum.hpp>
#include <cmath>
#include <limits>
#include <optional>

using namespace cdboost::pdevs::qss;

using TIME       = double;
using Integrator = qss1_integrator<TIME>;
using WSum2      = qss_wsum<2, TIME>;
using Mult       = qss_multiplier<TIME>;

static double conserved(double x1, double x2) {
    return x1 - std::log(x1) + x2 - std::log(x2);
}

struct LVResult {
    long long events;
    double min_x1, max_x1;
    double min_x2, max_x2;
    double max_V_drift;
};

static LVResult run_lv(double t_end, double dqmin, double dqrel) {
    constexpr TIME INF = std::numeric_limits<TIME>::infinity();

    Integrator int0(0.5, dqmin, dqrel);
    Integrator int1(0.5, dqmin, dqrel);
    Mult mult;
    WSum2 ws0(std::array<double, 2>{0.1, -0.1});
    WSum2 ws1(std::array<double, 2>{0.1, -0.1});

    TIME t_int0 = int0.advance(), t_int1 = int1.advance();
    TIME t_mult = INF, t_ws0 = INF, t_ws1 = INF;
    TIME last_int0 = 0, last_int1 = 0, last_mult = 0, last_ws0 = 0, last_ws1 = 0;

    const double V0 = conserved(0.5, 0.5);
    LVResult r{};
    r.min_x1 = r.min_x2 = 0.5;
    r.max_x1 = r.max_x2 = 0.5;
    r.max_V_drift       = 0.0;

    while (true) {
        TIME t_next = std::min({t_int0, t_int1, t_mult, t_ws0, t_ws1});
        if (t_next >= t_end)
            break;
        ++r.events;

        std::optional<double> out_int0, out_int1, out_mult, out_ws0, out_ws1;
        if (t_int0 == t_next)
            out_int0 = int0.out()[0].value;
        if (t_int1 == t_next)
            out_int1 = int1.out()[0].value;
        if (t_mult == t_next)
            out_mult = mult.out()[0].value;
        if (t_ws0 == t_next)
            out_ws0 = ws0.out()[0].value;
        if (t_ws1 == t_next)
            out_ws1 = ws1.out()[0].value;

        if (t_int0 == t_next) {
            int0.internal();
            t_int0 = t_next + int0.advance();
        }
        if (t_int1 == t_next) {
            int1.internal();
            t_int1 = t_next + int1.advance();
        }
        if (t_mult == t_next) {
            mult.internal();
            t_mult = t_next + mult.advance();
        }
        if (t_ws0 == t_next) {
            ws0.internal();
            t_ws0 = t_next + ws0.advance();
        }
        if (t_ws1 == t_next) {
            ws1.internal();
            t_ws1 = t_next + ws1.advance();
        }

        // Route: int0 → mult{ch0}, ws0{ch0}
        //        int1 → mult{ch1}, ws1{ch1}
        //        mult → ws0{ch1}, ws1{ch0}
        //        ws0  → int0{ch0}
        //        ws1  → int1{ch0}
        std::vector<qss_msg> to_mult, to_ws0, to_ws1, to_int0, to_int1;
        if (out_int0) {
            to_mult.push_back({0, *out_int0});
            to_ws0.push_back({0, *out_int0});
        }
        if (out_int1) {
            to_mult.push_back({1, *out_int1});
            to_ws1.push_back({1, *out_int1});
        }
        if (out_mult) {
            to_ws0.push_back({1, *out_mult});
            to_ws1.push_back({0, *out_mult});
        }
        if (out_ws0)
            to_int0.push_back({0, *out_ws0});
        if (out_ws1)
            to_int1.push_back({0, *out_ws1});

        if (!to_mult.empty()) {
            mult.external(to_mult, t_next - last_mult);
            last_mult = t_next;
            t_mult    = t_next + mult.advance();
        }
        if (!to_ws0.empty()) {
            ws0.external(to_ws0, t_next - last_ws0);
            last_ws0 = t_next;
            t_ws0    = t_next + ws0.advance();
        }
        if (!to_ws1.empty()) {
            ws1.external(to_ws1, t_next - last_ws1);
            last_ws1 = t_next;
            t_ws1    = t_next + ws1.advance();
        }
        if (!to_int0.empty()) {
            int0.external(to_int0, t_next - last_int0);
            last_int0 = t_next;
            t_int0    = t_next + int0.advance();
        }
        if (!to_int1.empty()) {
            int1.external(to_int1, t_next - last_int1);
            last_int1 = t_next;
            t_int1    = t_next + int1.advance();
        }

        if (out_int0 || out_int1) {
            double x1 = int0.q, x2 = int1.q;
            r.min_x1 = std::min(r.min_x1, x1);
            r.max_x1 = std::max(r.max_x1, x1);
            r.min_x2 = std::min(r.min_x2, x2);
            r.max_x2 = std::max(r.max_x2, x2);
            if (x1 > 0.0 && x2 > 0.0)
                r.max_V_drift = std::max(r.max_V_drift, std::abs(conserved(x1, x2) - V0));
        }
    }
    return r;
}

TEST_CASE("lotka-volterra cdboost: both populations remain positive", "[qss1][lv]") {
    auto r = run_lv(50.0, 1e-6, 1e-3);
    REQUIRE(r.min_x1 > 0.0);
    REQUIRE(r.min_x2 > 0.0);
}

TEST_CASE("lotka-volterra cdboost: populations oscillate beyond initial value", "[qss1][lv]") {
    auto r = run_lv(50.0, 1e-6, 1e-3);
    REQUIRE(r.max_x1 > 0.5);
    REQUIRE(r.max_x2 > 0.5);
}

TEST_CASE("lotka-volterra cdboost: conserved quantity drift is small over t=50", "[qss1][lv]") {
    auto r = run_lv(50.0, 1e-6, 1e-3);
    REQUIRE(r.max_V_drift < 0.2);
}

TEST_CASE("lotka-volterra cdboost: event count is in reasonable range", "[qss1][lv]") {
    auto r = run_lv(50.0, 1e-6, 1e-3);
    REQUIRE(r.events > 100);
    REQUIRE(r.events < 5'000'000);
}

TEST_CASE("lotka-volterra cdboost: tighter quantum gives smaller conserved drift", "[qss1][lv]") {
    auto r_coarse = run_lv(20.0, 1e-3, 0.0);
    auto r_fine   = run_lv(20.0, 1e-4, 0.0);
    REQUIRE(r_fine.max_V_drift < r_coarse.max_V_drift);
}
