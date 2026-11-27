// SPDX-License-Identifier: MIT
#pragma once

#include <AIS_InteractiveObject.hxx>
#include <Quantity_Color.hxx>

namespace mitcad {

// Fork: the plane ruler's grid (view/PlaneRuler.hpp) on the XY plane of its
// local transformation: lines `step` apart over the rectangle the plane
// covers, every `major`-th one darker (a numbered tick's), and the two lines
// through the origin in the axes' colours. Not pickable, and left out of the
// view's bounding box, as the layout grid.
class PlaneRulerGrid : public AIS_InteractiveObject {
  DEFINE_STANDARD_RTTI_INLINE(PlaneRulerGrid, AIS_InteractiveObject)

public:
  struct Lines {
    double uMin = 0.0; // the rectangle, in the plane's coordinates (mm)
    double uMax = 0.0;
    double vMin = 0.0;
    double vMax = 0.0;
    double step = 0.0; // 0: nothing drawn
    int major = 5;
  };

  const Lines& lines() const { return m_lines; }
  // Takes effect when the object is displayed or redisplayed.
  void setLines(const Lines& lines) { m_lines = lines; }
  void setColors(const Quantity_Color& minor, const Quantity_Color& major);
  // The line along the plane's u axis (v = 0) and the one along its v axis.
  void setAxisColors(const Quantity_Color& u, const Quantity_Color& v);

  bool AcceptDisplayMode(const int mode) const override { return mode == 0; }

protected:
  void Compute(const occ::handle<PrsMgr_PresentationManager>& manager,
               const occ::handle<Prs3d_Presentation>& presentation, const int mode) override;
  void ComputeSelection(const occ::handle<SelectMgr_Selection>&, const int) override {}

private:
  Lines m_lines;
  Quantity_Color m_minorColor = Quantity_Color(0.30, 0.31, 0.34, Quantity_TOC_RGB);
  Quantity_Color m_majorColor = Quantity_Color(0.40, 0.41, 0.44, Quantity_TOC_RGB);
  Quantity_Color m_uColor = Quantity_Color(0.85, 0.25, 0.25, Quantity_TOC_RGB);
  Quantity_Color m_vColor = Quantity_Color(0.30, 0.70, 0.30, Quantity_TOC_RGB);
};

} // namespace mitcad
