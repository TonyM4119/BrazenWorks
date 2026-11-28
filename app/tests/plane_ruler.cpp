// SPDX-License-Identifier: AGPL-3.0-or-later
// Fork: the plane ruler (view/PlaneRuler.hpp): the numbered ticks' spacing
// on the layout grid's major lines, and the labels' text.
#include "UnitTests.hpp"
#include "view/PlaneRuler.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include <QByteArray>
#include <QString>

namespace {

int failures = 0;

void check(bool condition, const char* expression, int line) {
  if (!condition) {
    std::fprintf(stderr, "plane_ruler.cpp:%d: check failed: %s\n", line, expression);
    ++failures;
  }
}

#define CHECK(expr) check((expr), #expr, __LINE__)

bool nearly(double a, double b) { return std::abs(a - b) <= 1e-9 * std::max(1.0, std::abs(b)); }

void stepTests() {
  // On the grid's major lines: their spacing times 1, 2, 5, 10 ...
  CHECK(nearly(mitcad::planeRulerLabelStep(10.0, 10.0), 10.0));   // 100 px apart
  CHECK(nearly(mitcad::planeRulerLabelStep(10.0, 5.0), 20.0));    // 50 px: twice
  CHECK(nearly(mitcad::planeRulerLabelStep(10.0, 2.0), 50.0));    // 20 px: five times
  CHECK(nearly(mitcad::planeRulerLabelStep(10.0, 0.5), 200.0));   // 5 px: 20 times
  CHECK(nearly(mitcad::planeRulerLabelStep(25.4, 4.0), 25.4));    // 1 in major lines at 101 px
  CHECK(nearly(mitcad::planeRulerLabelStep(6.35, 4.0), 31.75));   // 1/4 in at 25 px: five times, 1.25 in
  for (const double scale : {0.01, 0.3, 1.0, 7.0, 40.0}) {
    CHECK(mitcad::planeRulerLabelStep(5.0, scale) * scale >= mitcad::kPlaneRulerMinLabelPixels - 1e-9);
  }
  // No spacing, no scale: no numbers.
  CHECK(mitcad::planeRulerLabelStep(0.0, 1.0) == 0.0);
  CHECK(mitcad::planeRulerLabelStep(5.0, 0.0) == 0.0);
  CHECK(mitcad::planeRulerLabelStep(std::nan(""), 1.0) == 0.0);
}

#define EXPECT_LABEL(mm, unit, text)                                                                       \
  CHECK(mitcad::planeRulerLabel((mm), QStringLiteral(unit)) == QStringLiteral(text))

void labelTests() {
  EXPECT_LABEL(12.7, "in", "0.5");
  EXPECT_LABEL(25.4, "in", "1");
  EXPECT_LABEL(38.1, "in", "1.5");
  EXPECT_LABEL(-63.5, "in", "-2.5");
  EXPECT_LABEL(3.175, "in", "0.125");
  EXPECT_LABEL(20.0, "mm", "20");
  EXPECT_LABEL(0.05, "mm", "0.05");
  EXPECT_LABEL(-0.0, "mm", "0");
  EXPECT_LABEL(304.8, "ft", "1");
}

} // namespace

int planeRulerTests() {
  stepTests();
  labelTests();
  return failures;
}
MITCAD_UNIT_TEST(planeRulerTests);
