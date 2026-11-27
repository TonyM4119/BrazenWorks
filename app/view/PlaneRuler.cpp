// SPDX-License-Identifier: MIT
#include "view/PlaneRuler.hpp"

#include <cmath>

#include "view/GridSpacing.hpp"

namespace mitcad {

PlaneRulerTicks planeRulerTicks(double pixelsPerMm, double halfSizeMm, const QString& unit) {
  if (!(pixelsPerMm > 0.0) || !std::isfinite(pixelsPerMm) || !(halfSizeMm > 0.0) || !std::isfinite(halfSizeMm)) {
    return {};
  }
  const double unitMm = gridSpacingMillimetres(1.0, unit);
  const bool imperial = imperialGrid(unit);
  // The fewest unit lengths between ticks: far enough apart on the screen,
  // and few enough to the plane's edge.
  const double least = std::max(kPlaneRulerMinLabelPixels / pixelsPerMm, halfSizeMm / kPlaneRulerMaxLabels) / unitMm;
  static constexpr double kMantissas[] = {1.0, 2.5, 5.0};
  static constexpr double kFractions[] = {0.125, 0.25, 0.5};
  if (imperial && least <= 0.5 * (1.0 + 1e-9)) {
    for (const double fraction : kFractions) {
      if (fraction >= least * (1.0 - 1e-9)) {
        return {fraction * unitMm, fraction == 0.5 ? 5 : 4};
      }
    }
  }
  int exponent = static_cast<int>(std::floor(std::log10(least)));
  for (int round = 0; round < 2; ++round, ++exponent) {
    const double power = std::pow(10.0, exponent);
    for (const double mantissa : kMantissas) {
      // An inch design below one stays on its fractions (above).
      if (imperial && mantissa * power < 1.0) {
        continue;
      }
      if (mantissa * power >= least * (1.0 - 1e-9)) {
        return {mantissa * power * unitMm, 5};
      }
    }
  }
  return {10.0 * std::pow(10.0, exponent) * unitMm, 5};
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
