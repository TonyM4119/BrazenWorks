// SPDX-License-Identifier: MIT
#pragma once

// Fork: the plane ruler. While Create Sketch waits for a plane, the origin
// plane under the mouse shows a grid and numbered ticks along its two axes,
// in the design's length unit (0.5, 1, 1.5, ... in), so that its size reads
// at a glance. This is the tick spacing and the labels' text: plain Qt Core
// and <cmath>, without OCCT, so the unit tests (tests/plane_ruler.cpp) call
// it as well as OcctViewer.

#include <QString>

namespace mitcad {

// Numbered ticks no closer than this on the screen (logical pixels).
constexpr double kPlaneRulerMinLabelPixels = 36.0;
// Grid lines between the numbered ticks no closer than this; else only the
// ticks' lines are drawn.
constexpr double kPlaneRulerMinGridPixels = 10.0;
// At most this many numbered ticks from the origin to the plane's edge.
constexpr int kPlaneRulerMaxLabels = 6;

struct PlaneRulerTicks {
  double step = 0.0;     // mm between numbered ticks; 0: none
  int subdivisions = 5;  // grid lines per numbered tick
};

// The ticks of a plane reaching `halfSizeMm` from the origin, drawn at
// `pixelsPerMm`, for a design in `unit`: the smallest step of the unit's
// series that keeps the numbered ticks kPlaneRulerMinLabelPixels apart and
// no more than kPlaneRulerMaxLabels to the edge. The series is 1, 2 and 5
// times a power of ten in the unit; in and ft go through 1/8, 1/4 and 1/2
// below one. The grid divides a step of 1 or 5 into five, of 2, 1/4 or 1/8
// into four. A scale or size that is no number or not positive: no ticks.
PlaneRulerTicks planeRulerTicks(double pixelsPerMm, double halfSizeMm, const QString& unit);

// A tick's number in `unit`, without the unit: "0.5", "1", "-2.5", "0.125".
QString planeRulerLabel(double millimetres, const QString& unit);

} // namespace mitcad
