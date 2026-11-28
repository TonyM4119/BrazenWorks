// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Graphics' own settings (SolidCraft's Graphics page),
// under "graphics/". The page also shows settings kept elsewhere: Curved
// surfaces and Anti-aliasing (view/ViewQualityPreference.hpp), the grid,
// the camera's projection, the field of view and the visual style
// (ViewSettings, the View menu's commands). The parts of the window that
// follow these listen for changes (graphicsSettingsListeners).

#include <algorithm>
#include <functional>
#include <vector>

#include <QSettings>
#include <QString>
#include <QVariant>

namespace mitcad {

struct GraphicsSettings {
  // PRESET: 0 Performance, 1 Quality, 2 Custom (a choice of its own below).
  int preset = 0;
  // EDGES AND SHADING: supersampling 0 off, 1 at 1.5×, 2 at 2× (the view
  // drawn larger and scaled down), only while the view is still.
  int supersampling = 0;
  bool supersampleOnlyStill = true;
  bool keepDarkShapes = false;    // dark parts keep an outline of their shape
  bool studioLighting = true;     // material finish and studio lighting
  // CURRENT VIEW (with the grid, the projection and the visual style kept
  // in ViewSettings).
  bool groundShadow = true;
  bool groundShadowFollowsModel = false;
  bool ambientOcclusion = false;
  // IN THE VIEW
  bool viewAngles = false;
  bool vertexMarkers = false;
  bool quickMeasurement = false;
  bool frameRate = false;
  bool frameRateDetails = false;
  bool profileOperations = false;
  bool chipAnalysis = true;
  bool chipSection = true;
  bool chipLiveDetails = true;
  bool chipFoldItems = true;
  bool chipEscClosesAnalyses = false;
  int jointSymbols = 3;           // 0 Ring and axis, 1 Ring, 2 Axis, 3 None (PureCAD: None keeps the view as before)
  bool highlightMovingJoints = true;
  // SURFACES AND EDGES
  bool cosmeticThreads = true;   // PureCAD: on, as MitCad draws them
  bool sketchHalo = false;
  bool hatchByPart = true;        // Section hatch: One style or By part
  bool tintNewSurface = true;
  bool removedMaterialRed = true;
  bool hiddenEdgesOfSelected = false;
  bool referenceThroughBodies = false;
  bool gridFollowsView = false;
  bool singleSurfaceAnalysis = true;
  // THIS COMPUTER (applied at the next start)
  int graphicsApi = 0;            // 0 Automatic, 1 OpenGL (the view needs desktop OpenGL)
  int graphicsCard = 0;           // 0 Automatic, 1 High performance, 2 Power saving
  bool safeGraphicsMode = false;
  bool previousRenderPreviewBudget = false;
  bool previousRenderImageDefaults = false;
  bool previousRenderMemoryChecks = false;

  // The render scale supersampling asks for.
  double renderScale() const {
    return safeGraphicsMode ? 1.0 : supersampling == 2 ? 2.0 : supersampling == 1 ? 1.5 : 1.0;
  }

  static GraphicsSettings load() {
    const QSettings store;
    GraphicsSettings s;
    const QString g = QStringLiteral("graphics/");
    const auto flag = [&](const char* name, bool otherwise) {
      return store.value(g + QLatin1String(name), otherwise).toBool();
    };
    const auto number = [&](const char* name, int otherwise, int low, int high) {
      bool ok = false;
      const int value = store.value(g + QLatin1String(name), otherwise).toInt(&ok);
      return ok ? std::clamp(value, low, high) : otherwise;
    };
    s.preset = number("preset", s.preset, 0, 2);
    s.supersampling = number("supersampling", s.supersampling, 0, 2);
    s.supersampleOnlyStill = flag("supersampleOnlyStill", s.supersampleOnlyStill);
    s.keepDarkShapes = flag("keepDarkShapes", s.keepDarkShapes);
    s.studioLighting = flag("studioLighting", s.studioLighting);
    s.groundShadow = flag("groundShadow", s.groundShadow);
    s.groundShadowFollowsModel = flag("groundShadowFollowsModel", s.groundShadowFollowsModel);
    s.ambientOcclusion = flag("ambientOcclusion", s.ambientOcclusion);
    s.viewAngles = flag("viewAngles", s.viewAngles);
    s.vertexMarkers = flag("vertexMarkers", s.vertexMarkers);
    s.quickMeasurement = flag("quickMeasurement", s.quickMeasurement);
    s.frameRate = flag("frameRate", s.frameRate);
    s.frameRateDetails = flag("frameRateDetails", s.frameRateDetails);
    s.profileOperations = flag("profileOperations", s.profileOperations);
    s.chipAnalysis = flag("chipAnalysis", s.chipAnalysis);
    s.chipSection = flag("chipSection", s.chipSection);
    s.chipLiveDetails = flag("chipLiveDetails", s.chipLiveDetails);
    s.chipFoldItems = flag("chipFoldItems", s.chipFoldItems);
    s.chipEscClosesAnalyses = flag("chipEscClosesAnalyses", s.chipEscClosesAnalyses);
    s.jointSymbols = number("jointSymbols", s.jointSymbols, 0, 3);
    s.highlightMovingJoints = flag("highlightMovingJoints", s.highlightMovingJoints);
    s.cosmeticThreads = flag("cosmeticThreads", s.cosmeticThreads);
    s.sketchHalo = flag("sketchHalo", s.sketchHalo);
    s.hatchByPart = flag("hatchByPart", s.hatchByPart);
    s.tintNewSurface = flag("tintNewSurface", s.tintNewSurface);
    s.removedMaterialRed = flag("removedMaterialRed", s.removedMaterialRed);
    s.hiddenEdgesOfSelected = flag("hiddenEdgesOfSelected", s.hiddenEdgesOfSelected);
    s.referenceThroughBodies = flag("referenceThroughBodies", s.referenceThroughBodies);
    s.gridFollowsView = flag("gridFollowsView", s.gridFollowsView);
    s.singleSurfaceAnalysis = flag("singleSurfaceAnalysis", s.singleSurfaceAnalysis);
    s.graphicsApi = number("graphicsApi", s.graphicsApi, 0, 1);
    s.graphicsCard = number("graphicsCard", s.graphicsCard, 0, 2);
    s.safeGraphicsMode = flag("safeGraphicsMode", s.safeGraphicsMode);
    s.previousRenderPreviewBudget = flag("previousRenderPreviewBudget", s.previousRenderPreviewBudget);
    s.previousRenderImageDefaults = flag("previousRenderImageDefaults", s.previousRenderImageDefaults);
    s.previousRenderMemoryChecks = flag("previousRenderMemoryChecks", s.previousRenderMemoryChecks);
    return s;
  }

  void save() const {
    QSettings store;
    const QString g = QStringLiteral("graphics/");
    const auto put = [&](const char* name, const QVariant& value) { store.setValue(g + QLatin1String(name), value); };
    put("preset", preset);
    put("supersampling", supersampling);
    put("supersampleOnlyStill", supersampleOnlyStill);
    put("keepDarkShapes", keepDarkShapes);
    put("studioLighting", studioLighting);
    put("groundShadow", groundShadow);
    put("groundShadowFollowsModel", groundShadowFollowsModel);
    put("ambientOcclusion", ambientOcclusion);
    put("viewAngles", viewAngles);
    put("vertexMarkers", vertexMarkers);
    put("quickMeasurement", quickMeasurement);
    put("frameRate", frameRate);
    put("frameRateDetails", frameRateDetails);
    put("profileOperations", profileOperations);
    put("chipAnalysis", chipAnalysis);
    put("chipSection", chipSection);
    put("chipLiveDetails", chipLiveDetails);
    put("chipFoldItems", chipFoldItems);
    put("chipEscClosesAnalyses", chipEscClosesAnalyses);
    put("jointSymbols", jointSymbols);
    put("highlightMovingJoints", highlightMovingJoints);
    put("cosmeticThreads", cosmeticThreads);
    put("sketchHalo", sketchHalo);
    put("hatchByPart", hatchByPart);
    put("tintNewSurface", tintNewSurface);
    put("removedMaterialRed", removedMaterialRed);
    put("hiddenEdgesOfSelected", hiddenEdgesOfSelected);
    put("referenceThroughBodies", referenceThroughBodies);
    put("gridFollowsView", gridFollowsView);
    put("singleSurfaceAnalysis", singleSurfaceAnalysis);
    put("graphicsApi", graphicsApi);
    put("graphicsCard", graphicsCard);
    put("safeGraphicsMode", safeGraphicsMode);
    put("previousRenderPreviewBudget", previousRenderPreviewBudget);
    put("previousRenderImageDefaults", previousRenderImageDefaults);
    put("previousRenderMemoryChecks", previousRenderMemoryChecks);
  }
};

// Who follows the settings (each view): told after Apply or OK.
inline std::vector<std::function<void(const GraphicsSettings&)>>& graphicsSettingsListeners() {
  static std::vector<std::function<void(const GraphicsSettings&)>> listeners;
  return listeners;
}
inline void notifyGraphicsSettings(const GraphicsSettings& settings) {
  for (const auto& listener : graphicsSettingsListeners()) {
    listener(settings);
  }
}

} // namespace mitcad
