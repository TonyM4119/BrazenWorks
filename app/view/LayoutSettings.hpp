// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Layout's settings (SolidCraft's Layout page) that
// PureCAD's Display page did not have: how the window, the design tabs, the
// panels, the dialogs, the toolbar tabs and the navigation bar are laid out.
// Kept under "layout/" in the settings; the parts of the window that follow
// one listen for changes (layoutSettingsListeners).

#include <algorithm>
#include <functional>
#include <vector>

#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariant>

namespace mitcad {

struct LayoutSettings {
  // PANEL LAYOUT: the preset of the panels' places per workspace, by name
  // ("custom", "classic" or a saved one), and the workspace shown.
  QString panelPreset = QStringLiteral("custom");
  QString panelWorkspace = QStringLiteral("modeling");
  QStringList savedPresets;
  // WINDOW
  bool legacySurfaceToolbar = false;
  bool largeMenuIcons = false;          // Menus: List or Large icons
  bool groupConstructByEntity = true;   // Construct's menu in Planes, Axes, Points
  bool noticesCentered = true;          // Notices: Bottom right or Bottom center (centred as before)
  bool statusBar = false;
  bool operationsInView = true;         // a running operation's progress over the view
  bool waitForFiles = false;            // a file opens before the window takes input
  bool keepToolsOpenAcrossDesigns = false;
  bool rememberTabPerDesign = false;
  bool doubleClickRenamesTab = true;
  bool pauseDrawingWhileDragged = false;
  bool windowButtons = true;            // minimize, maximize and close in the title bar
  bool ribbon = true;                   // the toolbar's panel of groups
  bool ribbonOnHover = false;           // ... only when hovered (View › Toolbar Panel hidden, its own setting)
  bool otherPanelsOnHover = false;      // ... and the other panels too
  bool topBarAtEdge = false;            // Top bar: Always or At top edge
  int cardHeaders = 0;                  // 0 Always, 1 On hover, 2 Hidden
  bool keepOnTop = false;
  // DESIGN TABS
  bool viewSwitcherTab = true;          // one tab with a view switcher, else views as tabs
  bool showViewSwitcher = false;
  bool focusViewsOnWorkspace = true;
  bool viewStatesInTabs = true;
  bool sheetsInSubTab = true;
  bool renderInSubTab = true;
  bool reuseViews = true;
  bool reopenViews = false;
  // PANELS
  bool previewPanelPlacement = true;
  bool historyInBrowser = false;
  bool partsLibrary = false;
  bool browserTree = true;
  bool sketchesUnderFeature = false;    // Sketches in History: In order or Under their feature
  int bodiesFolders = 1;                // 0 Always, 1 Several, 2 Never
  bool playHistoryButton = true;
  bool foldHistoryGroups = false;
  bool folderIconFromFeatures = false;
  bool goToEndOfComponent = false;
  bool groupJointsInHistory = false;
  bool originAndBodiesOnTop = false;
  bool historyByComponent = true;
  bool featureMenusWholeSelection = true;
  bool componentsStartOpen = true;   // as the Browser always started (PureCAD)
  bool rowsStartClosed = false;
  bool eyeBesideArrow = false;
  bool featuresUnderBodies = false;
  // DIALOGS
  bool okCancelTop = false;
  bool okCancelInCorner = false;
  // TOOLBAR TABS
  QStringList hiddenTabs;               // the tabs the chips turned off
  bool keepSketchTab = false;
  bool searchResultsInRibbon = false;
  bool pinnedOutOfMenus = false;
  bool pinnedConstraintsInGrid = true;
  bool highlightActiveCommands = true;
  bool onlyRibbonTabs = false;
  bool panelMenusOnHover = false;
  bool searchCentered = true;           // in the view's middle as before, else below its top
  // NAVIGATION BAR
  int navigationIdle = 1;               // 0 Always visible, 1 Fade, 2 Hide until hovered
  int navigationPosition = 4;           // 0..2 top left, centre, right; 3..5 bottom
  bool navigationCompact = false;
  bool navigationLabels = false;
  QStringList navigationItems;          // in order; "|" a separator; empty: the default

  static LayoutSettings load() {
    const QSettings store;
    LayoutSettings s;
    const QString g = QStringLiteral("layout/");
    const auto flag = [&](const char* name, bool otherwise) {
      return store.value(g + QLatin1String(name), otherwise).toBool();
    };
    const auto number = [&](const char* name, int otherwise, int low, int high) {
      bool ok = false;
      const int value = store.value(g + QLatin1String(name), otherwise).toInt(&ok);
      return ok ? std::clamp(value, low, high) : otherwise;
    };
    s.panelPreset = store.value(g + QStringLiteral("panelPreset"), s.panelPreset).toString();
    s.panelWorkspace = store.value(g + QStringLiteral("panelWorkspace"), s.panelWorkspace).toString();
    s.savedPresets = store.value(g + QStringLiteral("savedPresets")).toStringList();
    s.legacySurfaceToolbar = flag("legacySurfaceToolbar", s.legacySurfaceToolbar);
    s.largeMenuIcons = flag("largeMenuIcons", s.largeMenuIcons);
    s.groupConstructByEntity = flag("groupConstructByEntity", s.groupConstructByEntity);
    s.noticesCentered = flag("noticesCentered", s.noticesCentered);
    s.statusBar = flag("statusBar", s.statusBar);
    s.operationsInView = flag("operationsInView", s.operationsInView);
    s.waitForFiles = flag("waitForFiles", s.waitForFiles);
    s.keepToolsOpenAcrossDesigns = flag("keepToolsOpenAcrossDesigns", s.keepToolsOpenAcrossDesigns);
    s.rememberTabPerDesign = flag("rememberTabPerDesign", s.rememberTabPerDesign);
    s.doubleClickRenamesTab = flag("doubleClickRenamesTab", s.doubleClickRenamesTab);
    s.pauseDrawingWhileDragged = flag("pauseDrawingWhileDragged", s.pauseDrawingWhileDragged);
    s.windowButtons = flag("windowButtons", s.windowButtons);
    s.ribbon = flag("ribbon", s.ribbon);
    // PureCAD: the toolbar panel hidden, shown at the view's top edge, is
    // View › Toolbar Panel's own setting (Ctrl+F1 changes it too).
    s.ribbonOnHover = store.value(QStringLiteral("view/toolbarPanelHidden"), false).toBool();
    s.otherPanelsOnHover = flag("otherPanelsOnHover", s.otherPanelsOnHover);
    s.topBarAtEdge = flag("topBarAtEdge", s.topBarAtEdge);
    s.cardHeaders = number("cardHeaders", s.cardHeaders, 0, 2);
    s.keepOnTop = flag("keepOnTop", s.keepOnTop);
    s.viewSwitcherTab = flag("viewSwitcherTab", s.viewSwitcherTab);
    s.showViewSwitcher = flag("showViewSwitcher", s.showViewSwitcher);
    s.focusViewsOnWorkspace = flag("focusViewsOnWorkspace", s.focusViewsOnWorkspace);
    s.viewStatesInTabs = flag("viewStatesInTabs", s.viewStatesInTabs);
    s.sheetsInSubTab = flag("sheetsInSubTab", s.sheetsInSubTab);
    s.renderInSubTab = flag("renderInSubTab", s.renderInSubTab);
    s.reuseViews = flag("reuseViews", s.reuseViews);
    s.reopenViews = flag("reopenViews", s.reopenViews);
    s.previewPanelPlacement = flag("previewPanelPlacement", s.previewPanelPlacement);
    s.historyInBrowser = flag("historyInBrowser", s.historyInBrowser);
    s.partsLibrary = flag("partsLibrary", s.partsLibrary);
    s.browserTree = flag("browserTree", s.browserTree);
    s.sketchesUnderFeature = flag("sketchesUnderFeature", s.sketchesUnderFeature);
    s.bodiesFolders = number("bodiesFolders", s.bodiesFolders, 0, 2);
    s.playHistoryButton = flag("playHistoryButton", s.playHistoryButton);
    s.foldHistoryGroups = flag("foldHistoryGroups", s.foldHistoryGroups);
    s.folderIconFromFeatures = flag("folderIconFromFeatures", s.folderIconFromFeatures);
    s.goToEndOfComponent = flag("goToEndOfComponent", s.goToEndOfComponent);
    s.groupJointsInHistory = flag("groupJointsInHistory", s.groupJointsInHistory);
    s.originAndBodiesOnTop = flag("originAndBodiesOnTop", s.originAndBodiesOnTop);
    s.historyByComponent = flag("historyByComponent", s.historyByComponent);
    s.featureMenusWholeSelection = flag("featureMenusWholeSelection", s.featureMenusWholeSelection);
    s.componentsStartOpen = flag("componentsStartOpen", s.componentsStartOpen);
    s.rowsStartClosed = flag("rowsStartClosed", s.rowsStartClosed);
    s.eyeBesideArrow = flag("eyeBesideArrow", s.eyeBesideArrow);
    s.featuresUnderBodies = flag("featuresUnderBodies", s.featuresUnderBodies);
    s.okCancelTop = flag("okCancelTop", s.okCancelTop);
    s.okCancelInCorner = flag("okCancelInCorner", s.okCancelInCorner);
    s.hiddenTabs = store.value(g + QStringLiteral("hiddenTabs")).toStringList();
    s.keepSketchTab = flag("keepSketchTab", s.keepSketchTab);
    s.searchResultsInRibbon = flag("searchResultsInRibbon", s.searchResultsInRibbon);
    s.pinnedOutOfMenus = flag("pinnedOutOfMenus", s.pinnedOutOfMenus);
    s.pinnedConstraintsInGrid = flag("pinnedConstraintsInGrid", s.pinnedConstraintsInGrid);
    s.highlightActiveCommands = flag("highlightActiveCommands", s.highlightActiveCommands);
    s.onlyRibbonTabs = flag("onlyRibbonTabs", s.onlyRibbonTabs);
    s.panelMenusOnHover = flag("panelMenusOnHover", s.panelMenusOnHover);
    s.searchCentered = flag("searchCentered", s.searchCentered);
    s.navigationIdle = number("navigationIdle", s.navigationIdle, 0, 2);
    s.navigationPosition = number("navigationPosition", s.navigationPosition, 0, 5);
    s.navigationCompact = flag("navigationCompact", s.navigationCompact);
    s.navigationLabels = flag("navigationLabels", s.navigationLabels);
    s.navigationItems = store.value(g + QStringLiteral("navigationItems")).toStringList();
    return s;
  }

  void save() const {
    QSettings store;
    const QString g = QStringLiteral("layout/");
    const auto put = [&](const char* name, const QVariant& value) { store.setValue(g + QLatin1String(name), value); };
    put("panelPreset", panelPreset);
    put("panelWorkspace", panelWorkspace);
    put("savedPresets", savedPresets);
    put("legacySurfaceToolbar", legacySurfaceToolbar);
    put("largeMenuIcons", largeMenuIcons);
    put("groupConstructByEntity", groupConstructByEntity);
    put("noticesCentered", noticesCentered);
    put("statusBar", statusBar);
    put("operationsInView", operationsInView);
    put("waitForFiles", waitForFiles);
    put("keepToolsOpenAcrossDesigns", keepToolsOpenAcrossDesigns);
    put("rememberTabPerDesign", rememberTabPerDesign);
    put("doubleClickRenamesTab", doubleClickRenamesTab);
    put("pauseDrawingWhileDragged", pauseDrawingWhileDragged);
    put("windowButtons", windowButtons);
    put("ribbon", ribbon);
    put("otherPanelsOnHover", otherPanelsOnHover);
    put("topBarAtEdge", topBarAtEdge);
    put("cardHeaders", cardHeaders);
    put("keepOnTop", keepOnTop);
    put("viewSwitcherTab", viewSwitcherTab);
    put("showViewSwitcher", showViewSwitcher);
    put("focusViewsOnWorkspace", focusViewsOnWorkspace);
    put("viewStatesInTabs", viewStatesInTabs);
    put("sheetsInSubTab", sheetsInSubTab);
    put("renderInSubTab", renderInSubTab);
    put("reuseViews", reuseViews);
    put("reopenViews", reopenViews);
    put("previewPanelPlacement", previewPanelPlacement);
    put("historyInBrowser", historyInBrowser);
    put("partsLibrary", partsLibrary);
    put("browserTree", browserTree);
    put("sketchesUnderFeature", sketchesUnderFeature);
    put("bodiesFolders", bodiesFolders);
    put("playHistoryButton", playHistoryButton);
    put("foldHistoryGroups", foldHistoryGroups);
    put("folderIconFromFeatures", folderIconFromFeatures);
    put("goToEndOfComponent", goToEndOfComponent);
    put("groupJointsInHistory", groupJointsInHistory);
    put("originAndBodiesOnTop", originAndBodiesOnTop);
    put("historyByComponent", historyByComponent);
    put("featureMenusWholeSelection", featureMenusWholeSelection);
    put("componentsStartOpen", componentsStartOpen);
    put("rowsStartClosed", rowsStartClosed);
    put("eyeBesideArrow", eyeBesideArrow);
    put("featuresUnderBodies", featuresUnderBodies);
    put("okCancelTop", okCancelTop);
    put("okCancelInCorner", okCancelInCorner);
    put("hiddenTabs", hiddenTabs);
    put("keepSketchTab", keepSketchTab);
    put("searchResultsInRibbon", searchResultsInRibbon);
    put("pinnedOutOfMenus", pinnedOutOfMenus);
    put("pinnedConstraintsInGrid", pinnedConstraintsInGrid);
    put("highlightActiveCommands", highlightActiveCommands);
    put("onlyRibbonTabs", onlyRibbonTabs);
    put("panelMenusOnHover", panelMenusOnHover);
    put("searchCentered", searchCentered);
    put("navigationIdle", navigationIdle);
    put("navigationPosition", navigationPosition);
    put("navigationCompact", navigationCompact);
    put("navigationLabels", navigationLabels);
    put("navigationItems", navigationItems);
  }
};

// Fork: who follows the Layout settings: each is called after they change
// (Preferences' Apply or OK), with the settings as saved.
inline std::vector<std::function<void(const LayoutSettings&)>>& layoutSettingsListeners() {
  static std::vector<std::function<void(const LayoutSettings&)>> listeners;
  return listeners;
}

inline void notifyLayoutSettings(const LayoutSettings& settings) {
  for (const auto& listener : layoutSettingsListeners()) {
    listener(settings);
  }
}

} // namespace mitcad
