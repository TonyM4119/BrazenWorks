// SPDX-License-Identifier: MIT
#pragma once

// Fork: the plane ruler, as Fusion's. While Create Sketch waits for a plane,
// the layout grid moves onto the origin plane under the mouse, so that it
// shows the plane a sketch would be drawn on, with the plane's two axes in
// their colours and numbered ticks along their positive halves in the
// design's length unit (on the grid's major lines: 5, 10, 15 ... mm).
// This is the numbers' spacing and text: plain Qt Core and <cmath>, without
// OCCT, so the unit tests (tests/plane_ruler.cpp) call it as well as
// OcctViewer.

#include <QString>

namespace mitcad {

// Numbered ticks no closer than this on the screen (logical pixels).
constexpr double kPlaneRulerMinLabelPixels = 80.0;

// The spacing (mm) of the numbered ticks on a grid whose major lines are
// `majorMm` apart, drawn at `pixelsPerMm`: the major spacing times the
// smallest of 1, 2, 5, 10, 20, 50 ... that keeps the numbers
// kPlaneRulerMinLabelPixels apart, so that each number sits on a major line.
// A spacing or scale that is no number or not positive: 0 (no numbers).
double planeRulerLabelStep(double majorMm, double pixelsPerMm);

// A tick's number in `unit`, without the unit: "0.5", "1", "-2.5", "0.125".
QString planeRulerLabel(double millimetres, const QString& unit);

} // namespace mitcad
