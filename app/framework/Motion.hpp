// SPDX-License-Identifier: MIT
#pragma once

// Fork: small, consistent motion for the interface (docs/ui-conventions.md,
// Motion and smoothness). Durations are short on purpose: the application
// should feel quick, not floaty. Under the UI tests (MITCAD_TEST_INPUT)
// nothing animates: every change is made at once, as the tests expect.

#include <functional>

namespace mitcad {

class GlassCard;

namespace motion {

inline constexpr int kFast = 120;   // a hover's highlight, a card fading out
inline constexpr int kNormal = 180; // a card fading in

// Whether the interface animates (not under the UI tests).
bool enabled();

// Shows the card and fades it in, card and shadow (GlassCard::setOpacityLevel).
void fadeIn(GlassCard* card);

// Fades the card out, then hides it and calls `done` (if any). At once when
// motion is off or the card is hidden already. Showing the card again
// (fadeIn) stops a fade out.
void fadeOut(GlassCard* card, const std::function<void()>& done = {});

} // namespace motion
} // namespace mitcad
