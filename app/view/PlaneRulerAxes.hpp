// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <AIS_InteractiveObject.hxx>
#include <Quantity_Color.hxx>

namespace mitcad {

// Fork: the plane ruler's axes (view/PlaneRuler.hpp): the two lines through
// the origin on the XY plane of its local transformation, `extent` each way,
// in the axes' colours, over the layout grid that the ruler moves onto the
// plane. Not pickable, and left out of the view's bounding box, as the
// layout grid.
class PlaneRulerAxes : public AIS_InteractiveObject {
  DEFINE_STANDARD_RTTI_INLINE(PlaneRulerAxes, AIS_InteractiveObject)

public:
  double extent() const { return m_extent; }
  // Takes effect when the object is displayed or redisplayed; 0: nothing.
  void setExtent(double extent) { m_extent = extent; }
  // The line along the plane's u axis (v = 0) and the one along its v axis.
  void setColors(const Quantity_Color& u, const Quantity_Color& v);

  bool AcceptDisplayMode(const int mode) const override { return mode == 0; }

protected:
  void Compute(const occ::handle<PrsMgr_PresentationManager>& manager,
               const occ::handle<Prs3d_Presentation>& presentation, const int mode) override;
  void ComputeSelection(const occ::handle<SelectMgr_Selection>&, const int) override {}

private:
  double m_extent = 0.0;
  Quantity_Color m_uColor = Quantity_Color(0.85, 0.25, 0.25, Quantity_TOC_RGB);
  Quantity_Color m_vColor = Quantity_Color(0.30, 0.70, 0.30, Quantity_TOC_RGB);
};

} // namespace mitcad
