// SPDX-License-Identifier: BSD-2-Clause
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <cdboost/pdevs/basic_models/qss/qss1_integrator.hpp>
#include <limits>

using namespace cdboost::pdevs::qss;
using Catch::Matchers::WithinAbs;

using Q = qss1_integrator<double>;

static std::vector<qss_msg> make_u(double u) {
    return {qss_msg{0, u}};
}

TEST_CASE("qss1_integrator: initial sigma is zero", "[qss1]") {
    Q integrator(1.0, 1e-2, 0.0);
    REQUIRE(integrator.advance() == 0.0);
}

TEST_CASE("qss1_integrator: output at t=0 is initial value", "[qss1]") {
    Q integrator(2.5, 1e-2, 0.0);
    auto out = integrator.out();
    REQUIRE(out.size() == 1u);
    REQUIRE_THAT(out[0].value, WithinAbs(2.5, 1e-12));
}

TEST_CASE("qss1_integrator: internal transition with zero derivative gives infinity", "[qss1]") {
    Q integrator(1.0, 1e-2, 0.0);
    integrator.internal();
    REQUIRE(integrator.advance() == std::numeric_limits<double>::infinity());
}

TEST_CASE("qss1_integrator: constant derivative schedules correct sigma", "[qss1]") {
    Q integrator(1.0, 0.1, 0.0);
    integrator.internal();
    integrator.external(make_u(1.0), 0.0);
    REQUIRE_THAT(integrator.advance(), WithinAbs(0.1, 1e-12));
}

TEST_CASE("qss1_integrator: output value is at next crossing boundary", "[qss1]") {
    Q integrator(0.0, 0.1, 0.0);
    integrator.internal();
    integrator.external(make_u(1.0), 0.0);
    REQUIRE_THAT(integrator.advance(), WithinAbs(0.1, 1e-12));
    auto out = integrator.out();
    REQUIRE_THAT(out[0].value, WithinAbs(0.1, 1e-12));
}

TEST_CASE("qss1_integrator: negative derivative crosses lower boundary", "[qss1]") {
    Q integrator(1.0, 0.1, 0.0);
    integrator.internal();
    integrator.external(make_u(-2.0), 0.0);
    REQUIRE_THAT(integrator.advance(), WithinAbs(0.05, 1e-12));
    auto out = integrator.out();
    REQUIRE_THAT(out[0].value, WithinAbs(0.9, 1e-12));
}

TEST_CASE("qss1_integrator: state advances correctly on external transition", "[qss1]") {
    Q integrator(0.0, 0.1, 0.0);
    integrator.internal();
    integrator.external(make_u(1.0), 0.0);
    integrator.external(make_u(2.0), 0.05);
    REQUIRE_THAT(integrator.advance(), WithinAbs(0.025, 1e-12));
}

TEST_CASE("qss1_integrator: crossing already past boundary gives sigma>0 after partial advance",
          "[qss1]") {
    Q integrator(0.0, 0.1, 0.0);
    integrator.internal();
    integrator.external(make_u(1.0), 0.0);
    integrator.internal();
    integrator.external(make_u(100.0), 0.05);
    REQUIRE(integrator.advance() > 0.0);
}

TEST_CASE("qss1_integrator: relative quantization scales with state", "[qss1]") {
    Q integrator(10.0, 0.0, 0.1);
    integrator.internal();
    integrator.external(make_u(1.0), 0.0);
    REQUIRE_THAT(integrator.advance(), WithinAbs(1.0, 1e-10));
}

TEST_CASE("qss1_integrator: linear integration over multiple steps", "[qss1]") {
    Q integrator(0.0, 0.5, 0.0);
    integrator.internal();
    integrator.external(make_u(1.0), 0.0);

    double t = 0.0;
    for (int step = 1; step <= 4; ++step) {
        double sigma = integrator.advance();
        REQUIRE_THAT(sigma, WithinAbs(0.5, 1e-10));
        auto out = integrator.out();
        REQUIRE_THAT(out[0].value, WithinAbs(step * 0.5, 1e-10));
        t += sigma;
        integrator.internal();
    }
    REQUIRE_THAT(t, WithinAbs(2.0, 1e-10));
}

TEST_CASE("qss1_integrator: confluence = internal then external", "[qss1]") {
    Q integrator(0.0, 0.1, 0.0);
    // sigma=0: fire internal (q=0, sigma=inf since u=0), then external sets u=1
    integrator.confluence(make_u(1.0), 0.0);
    REQUIRE_THAT(integrator.advance(), WithinAbs(0.1, 1e-12));
}
