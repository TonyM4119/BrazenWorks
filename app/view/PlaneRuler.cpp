// SPDX-License-Identifier: MIT
#include "view/PlaneRuler.hpp"

#include <cmath>

#include "view/GridSpacing.hpp"

namespace mitcad {

double planeRulerLabelStep(double majorMm, double pixelsPerMm) {
  if (!(majorMm > 0.0) || !std::isfinite(majorMm) || !(pixelsPerMm > 0.0) || !std::isfinite(pixelsPerMm)) {
    return 0.0;
  }
  static constexpr double kFactors[] = {1.0, 2.0, 5.0};
  for (double power = 1.0; power < 1e9; power *= 10.0) {
    for (const double factor : kFactors) {
      const double step = majorMm * factor * power;
      if (step * pixelsPerMm >= kPlaneRulerMinLabelPixels * (1.0 - 1e-9)) {
        return step;
      }
    }
  }
  return 0.0;
}

QString planeRulerLabel(double millimetres, const QString& unit) {
  double value = gridSpacingInUnit(millimetres, unit);
  // Round off the conversion's last digits (0.49999999 in is 0.5).
  value = std::round(value * 1e6) / 1e6;
  if (value == 0.0) {
    value = 0.0; // no "-0"
  }
  return QString::number(value, 'g', 6);
}

} // namespace mitcad
