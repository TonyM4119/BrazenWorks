// SPDX-License-Identifier: MIT
#pragma once

// Fork: Preferences › Display › Smoothness and Anti-aliasing, how smooth
// the 3D view looks.
//
// Smoothness ("view/smoothness": "smooth", the default, or "standard"):
// how finely the view meshes curved edges and faces (view/
// ViewTessellation.hpp, Smoothness). Changing it re-meshes the open
// designs' shown bodies (each window on ViewQualityNotifier::
// smoothnessChanged, MainWindow::remeshDisplay); the display meshes kept in
// a design's result store are keyed by the angles it sets, so a design
// meshed with the other choice is meshed again. Exported meshes do not
// follow it.
//
// Anti-aliasing ("view/antialiasing": 0, 2, 4 or 8 samples; 4, the
// upstream's, by default): the view's multisampling, which smooths the
// edges of faces, lines and the grid. 8 is smoother where the graphics card
// offers it (OCCT takes the most it offers below the asked count). Each
// view applies it at once (ViewQualityNotifier::antialiasingChanged);
// MITCAD_VIEWPORT_AA still overrides it for diagnostics.

#include <optional>

#include <QObject>
#include <QString>

#include "view/ViewTessellation.hpp"

namespace mitcad {

QString smoothnessId(Smoothness choice);
std::optional<Smoothness> smoothnessFromId(const QString& id);
// Smooth, unless the UI tests run the application (MITCAD_TEST_INPUT):
// their expectations were written for the upstream's meshes.
Smoothness defaultSmoothness();
Smoothness savedSmoothness();
// At the start, before any body is meshed: the view's meshes follow it.
void applySavedSmoothness();
// Saves it, makes the view's meshes from now on follow it and tells
// ViewQualityNotifier; false when it was already so.
bool saveSmoothness(Smoothness choice);

// The anti-aliasing samples offered: 0 (off), 2, 4 and 8.
int savedAntialiasing();
// Saves it (another count is taken as 4) and tells ViewQualityNotifier;
// false when it was already so.
bool saveAntialiasing(int samples);

class ViewQualityNotifier : public QObject {
  Q_OBJECT

public:
  static ViewQualityNotifier& instance();

signals:
  void smoothnessChanged();
  void antialiasingChanged(int samples);
};

} // namespace mitcad
