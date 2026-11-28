// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: the view's camera glides (Home, Fit, the view cube, Look At, the
// standard views) ease in and out instead of moving at an even speed: they
// start and settle gently and are quickest in the middle. OCCT's
// AIS_AnimationCamera interpolates the camera by the animation's progress;
// this shapes that progress (a sine ease-in-out: the gentlest of the usual
// curves, its top speed about 1.6 times the average, where a cubic's is 3)
// and leaves the rest to OCCT. OcctViewer makes it the view's animation (AIS_ViewController::
// SetViewAnimation), which the view cube shares.

#include <cmath>

#include <AIS_AnimationCamera.hxx>

namespace mitcad {

// 0 at 0, 1 at 1, slow at both ends: a sine ease-in-out.
inline double easeInOutSine(double t) {
  if (t <= 0.0) {
    return 0.0;
  }
  if (t >= 1.0) {
    return 1.0;
  }
  return 0.5 - 0.5 * std::cos(3.14159265358979323846 * t);
}

// How long a glide takes (seconds): kGlideSeconds for a turn up to a
// quarter turn, longer for larger turns, at most kGlideMaxSeconds.
inline constexpr double kGlideSeconds = 0.6;
inline constexpr double kGlideMaxSeconds = 1.0;

class EasedCameraAnimation : public AIS_AnimationCamera {
  DEFINE_STANDARD_RTTI_INLINE(EasedCameraAnimation, AIS_AnimationCamera)

public:
  explicit EasedCameraAnimation(const TCollection_AsciiString& name)
      : AIS_AnimationCamera(name, occ::handle<V3d_View>()) {}

protected:
  void update(const AIS_AnimationProgress& progress) override {
    AIS_AnimationProgress eased = progress;
    eased.LocalNormalized = easeInOutSine(progress.LocalNormalized);
    AIS_AnimationCamera::update(eased);
  }
};

} // namespace mitcad
