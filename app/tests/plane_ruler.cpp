// SPDX-License-Identifier: MIT
// Fork: the plane ruler (view/PlaneRuler.hpp): the numbered ticks' step per
// unit, the grid lines per tick, and the labels' text.
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

// The step in units of `size` mm for a grid `half` units from the origin
// to its edge, where one unit is `pixels` logical pixels on the screen.
mitcad::PlaneRulerTicks ticksAt(double half, double pixels, const char* unit, double size) {
  return mitcad::planeRulerTicks(pixels / size, half * size, QString::fromLatin1(unit));
}

void stepTests() {
  // Fusion's look: 2.5, 5, 7.5 ... with the grid in fifths.
  mitcad::PlaneRulerTicks ticks = ticksAt(10.0, 40.0, "in", 25.4);
  CHECK(nearly(ticks.step, 2.5 * 25.4));
  CHECK(ticks.subdivisions == 5);
  ticks = ticksAt(40.0, 10.0, "in", 25.4);
  CHECK(nearly(ticks.step, 10.0 * 25.4));
  // Below an inch: halves, quarters and eighths, the grid in quarters.
  ticks = ticksAt(2.0, 200.0, "in", 25.4);
  CHECK(nearly(ticks.step, 0.5 * 25.4));
  CHECK(ticks.subdivisions == 5);
  ticks = ticksAt(0.8, 400.0, "in", 25.4);
  CHECK(nearly(ticks.step, 0.25 * 25.4));
  CHECK(ticks.subdivisions == 4);
  ticks = ticksAt(0.4, 800.0, "in", 25.4);
  CHECK(nearly(ticks.step, 0.125 * 25.4));
  // Metric: 1, 2.5, 5 times a power of ten mm.
  ticks = ticksAt(100.0, 4.0, "mm", 1.0);
  CHECK(nearly(ticks.step, 25.0));
  ticks = ticksAt(400.0, 1.0, "mm", 1.0);
  CHECK(nearly(ticks.step, 100.0));
  ticks = ticksAt(1.0, 200.0, "mm", 1.0);
  CHECK(nearly(ticks.step, 0.5));
  // Ticks far enough apart on the screen.
  ticks = ticksAt(50.0, 0.5, "mm", 1.0);
  CHECK(ticks.step * 0.5 >= mitcad::kPlaneRulerMinLabelPixels);
  // No scale, no size: no ticks.
  CHECK(mitcad::planeRulerTicks(0.0, 10.0, QStringLiteral("mm")).step == 0.0);
  CHECK(mitcad::planeRulerTicks(1.0, 0.0, QStringLiteral("mm")).step == 0.0);
  CHECK(mitcad::planeRulerTicks(std::nan(""), 10.0, QStringLiteral("mm")).step == 0.0);
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
