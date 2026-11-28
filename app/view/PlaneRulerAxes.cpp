// SPDX-License-Identifier: AGPL-3.0-or-later
#include "view/PlaneRulerAxes.hpp"

#include <Graphic3d_ArrayOfSegments.hxx>
#include <Graphic3d_AspectLine3d.hxx>
#include <Graphic3d_Group.hxx>
#include <Prs3d_Presentation.hxx>

namespace mitcad {

void PlaneRulerAxes::setColors(const Quantity_Color& u, const Quantity_Color& v) {
  m_uColor = u;
  m_vColor = v;
}

void PlaneRulerAxes::Compute(const occ::handle<PrsMgr_PresentationManager>&,
                             const occ::handle<Prs3d_Presentation>& presentation, const int mode) {
  presentation->SetInfiniteState(true); // not in the bounding box
  if (mode != 0 || !(m_extent > 0.0)) {
    return;
  }
  const auto line = [&](double x0, double y0, double x1, double y1, const Quantity_Color& color) {
    const occ::handle<Graphic3d_ArrayOfSegments> segment = new Graphic3d_ArrayOfSegments(2);
    segment->AddVertex(x0, y0, 0.0);
    segment->AddVertex(x1, y1, 0.0);
    const occ::handle<Graphic3d_Group> group = presentation->NewGroup();
    group->SetGroupPrimitivesAspect(new Graphic3d_AspectLine3d(color, Aspect_TOL_SOLID, 1.5));
    group->AddPrimitiveArray(segment);
  };
  line(-m_extent, 0.0, m_extent, 0.0, m_uColor);
  line(0.0, -m_extent, 0.0, m_extent, m_vColor);
}

} // namespace mitcad
