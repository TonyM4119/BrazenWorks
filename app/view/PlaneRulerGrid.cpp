// SPDX-License-Identifier: MIT
#include "view/PlaneRulerGrid.hpp"

#include <cmath>
#include <vector>

#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_Group.hxx>
#include <Prs3d_Presentation.hxx>

namespace mitcad {
namespace {

// More lines than this across are not drawn.
constexpr long kMaxLines = 400;

} // namespace

void PlaneRulerGrid::setColors(const Quantity_Color& minor, const Quantity_Color& major) {
  m_minorColor = minor;
  m_majorColor = major;
}

void PlaneRulerGrid::setAxisColors(const Quantity_Color& u, const Quantity_Color& v) {
  m_uColor = u;
  m_vColor = v;
}

void PlaneRulerGrid::Compute(const occ::handle<PrsMgr_PresentationManager>&,
                             const occ::handle<Prs3d_Presentation>& presentation, const int mode) {
  presentation->SetInfiniteState(true); // not in the bounding box
  const Lines& lines = m_lines;
  const double step = lines.step;
  if (mode != 0 || !(step > 0.0) || !(lines.uMax > lines.uMin) || !(lines.vMax > lines.vMin)) {
    return;
  }
  const long uFirst = static_cast<long>(std::ceil(lines.uMin / step - 1e-9));
  const long uLast = static_cast<long>(std::floor(lines.uMax / step + 1e-9));
  const long vFirst = static_cast<long>(std::ceil(lines.vMin / step - 1e-9));
  const long vLast = static_cast<long>(std::floor(lines.vMax / step + 1e-9));
  if (uLast - uFirst > kMaxLines || vLast - vFirst > kMaxLines) {
    return;
  }
  const long major = std::max(1, lines.major);
  // Segments by kind: minor, major, and the two axes.
  std::vector<double> minor;
  std::vector<double> majorLines;
  const auto add = [](std::vector<double>& to, double x0, double y0, double x1, double y1) {
    to.insert(to.end(), {x0, y0, x1, y1});
  };
  for (long i = uFirst; i <= uLast; ++i) {
    if (i == 0) {
      continue; // the v axis, below
    }
    const double u = static_cast<double>(i) * step;
    add(i % major == 0 ? majorLines : minor, u, lines.vMin, u, lines.vMax);
  }
  for (long j = vFirst; j <= vLast; ++j) {
    if (j == 0) {
      continue; // the u axis, below
    }
    const double v = static_cast<double>(j) * step;
    add(j % major == 0 ? majorLines : minor, lines.uMin, v, lines.uMax, v);
  }
  const auto draw = [&](const std::vector<double>& xy, const Quantity_Color& color, double width) {
    if (xy.empty()) {
      return;
    }
    const occ::handle<Graphic3d_ArrayOfSegments> segments =
        new Graphic3d_ArrayOfSegments(static_cast<int>(xy.size() / 2));
    for (std::size_t k = 0; k + 1 < xy.size(); k += 2) {
      segments->AddVertex(xy[k], xy[k + 1], 0.0);
    }
    const occ::handle<Graphic3d_Group> group = presentation->NewGroup();
    group->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, width));
    group->AddPrimitiveArray(segments);
  };
  draw(minor, m_minorColor, 1.0);
  draw(majorLines, m_majorColor, 1.0);
  // The axes over the grid, where the plane holds them.
  if (lines.vMin <= 0.0 && lines.vMax >= 0.0) {
    draw({lines.uMin, 0.0, lines.uMax, 0.0}, m_uColor, 2.0);
  }
  if (lines.uMin <= 0.0 && lines.uMax >= 0.0) {
    draw({0.0, lines.vMin, 0.0, lines.vMax}, m_vColor, 2.0);
  }
}

} // namespace mitcad
