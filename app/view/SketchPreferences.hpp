// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Sketch, as SolidCraft's: Snapping and guides,
// Drawing, Opening and closing, and Selecting. Its own settings are
// SketchPrefs ("sketchPrefs/"); the rows PureCAD had (the first dimension
// scales the sketch, a click adds to the selection, Esc leaves an
// unchanged sketch, profiles shown when a sketch opens, the face's edges
// projected, a used sketch hidden) keep their own settings and saving and
// are moved into their places (adopt); grid snap is the view's.

#include <QColorDialog>
#include <QHash>
#include <QSettings>
#include <QComboBox>
#include <QLineEdit>
#include <QPointer>
#include <QSpinBox>

#include "PreferenceKit.hpp"

namespace mitcad {

struct SketchPrefs {
  // SNAPPING AND GUIDES
  bool centerGuides = true;
  bool pointGuides = true;
  bool guideCrossings = false;
  bool lineUpPoints = false;
  bool shiftSquare = false;
  int stopSnappingKey = 0;        // 0 Ctrl/Cmd or Alt, 1 Ctrl/Cmd, 2 Alt, 3 Shift
  // DRAWING
  bool dragDrawsLine = false;
  bool restForTangentArc = false;
  bool centerArcFollowsCursor = true;
  bool extendedConstructionLines = true;
  bool classicConstructionLines = false;
  bool mirrorUsesPreselectedLine = true;
  bool clickInsideProfilePicksShapes = false;
  bool ellipsesWithAxes = true;
  bool dimensionNewShapes = false;
  bool placeFromOrigin = false;
  bool scaleAboutOrigin = false;
  bool shortcutAgainCloses = true;
  bool shortcutAgainSwitchesMode = false;
  bool buttonEndsRunningTool = true;
  bool projectAsConstruction = false;
  bool askProjectOptions = true;
  bool olderMoveHandles = false;
  bool cursorCoordinates = true;
  bool dimensionInWindow = false;   // Dimension values: In the view or In a window
  double windowLengthStep = 1.0;    // mm
  double windowAngleStep = 1.0;     // degrees
  int dimensionTextSize = 100;      // percent
  bool dimensionColorCustom = false;
  QString dimensionColor = QStringLiteral("#822a6e");
  bool dimensionRim = true;
  int largestSketch = 20000;        // points or curves
  bool stateIcons = true;
  bool conflictColor = true;
  bool hideGlyphsZoomedOut = true;
  int glyphSize = 100;              // percent
  // OPENING AND CLOSING
  bool faceActivatesPart = false;
  bool turnToFace = true;
  bool doubleEscFinishes = false;   // off: UI tests and habits press Esc twice
  int whenSketchEnds = 0;           // 0 Return to the view before (as before), 1 Keep, 2 Unless turned
  // SELECTING
  bool clickSelectsBodies = false;  // Click selects: Faces and edges or Whole bodies
  bool shiftAddsOnly = false;       // Shift adds only; Ctrl / Cmd toggles
  bool preferMovablePoints = true;
  bool edgesThroughFaces = false;
  bool sketch3dCurvePoints = false;
  bool selectOtherHidden = true;
  bool altFreesArrows = true;

  static SketchPrefs load() {
    const QSettings store;
    SketchPrefs s;
    const QString g = QStringLiteral("sketchPrefs/");
    const auto flag = [&](const char* name, bool& value) { value = store.value(g + QLatin1String(name), value).toBool(); };
    const auto number = [&](const char* name, int& value, int low, int high) {
      bool ok = false;
      const int read = store.value(g + QLatin1String(name), value).toInt(&ok);
      if (ok) value = std::clamp(read, low, high);
    };
    const auto real = [&](const char* name, double& value) {
      bool ok = false;
      const double read = store.value(g + QLatin1String(name), value).toDouble(&ok);
      if (ok && read > 0) value = read;
    };
    flag("centerGuides", s.centerGuides);
    flag("pointGuides", s.pointGuides);
    flag("guideCrossings", s.guideCrossings);
    flag("lineUpPoints", s.lineUpPoints);
    flag("shiftSquare", s.shiftSquare);
    number("stopSnappingKey", s.stopSnappingKey, 0, 3);
    flag("dragDrawsLine", s.dragDrawsLine);
    flag("restForTangentArc", s.restForTangentArc);
    flag("centerArcFollowsCursor", s.centerArcFollowsCursor);
    flag("extendedConstructionLines", s.extendedConstructionLines);
    flag("classicConstructionLines", s.classicConstructionLines);
    flag("mirrorUsesPreselectedLine", s.mirrorUsesPreselectedLine);
    flag("clickInsideProfilePicksShapes", s.clickInsideProfilePicksShapes);
    flag("ellipsesWithAxes", s.ellipsesWithAxes);
    flag("dimensionNewShapes", s.dimensionNewShapes);
    flag("placeFromOrigin", s.placeFromOrigin);
    flag("scaleAboutOrigin", s.scaleAboutOrigin);
    flag("shortcutAgainCloses", s.shortcutAgainCloses);
    flag("shortcutAgainSwitchesMode", s.shortcutAgainSwitchesMode);
    flag("buttonEndsRunningTool", s.buttonEndsRunningTool);
    flag("projectAsConstruction", s.projectAsConstruction);
    flag("askProjectOptions", s.askProjectOptions);
    flag("olderMoveHandles", s.olderMoveHandles);
    flag("cursorCoordinates", s.cursorCoordinates);
    flag("dimensionInWindow", s.dimensionInWindow);
    real("windowLengthStep", s.windowLengthStep);
    real("windowAngleStep", s.windowAngleStep);
    number("dimensionTextSize", s.dimensionTextSize, 50, 300);
    flag("dimensionColorCustom", s.dimensionColorCustom);
    s.dimensionColor = store.value(g + QStringLiteral("dimensionColor"), s.dimensionColor).toString();
    flag("dimensionRim", s.dimensionRim);
    number("largestSketch", s.largestSketch, 1000, 200000);
    flag("stateIcons", s.stateIcons);
    flag("conflictColor", s.conflictColor);
    flag("hideGlyphsZoomedOut", s.hideGlyphsZoomedOut);
    number("glyphSize", s.glyphSize, 100, 300);
    flag("faceActivatesPart", s.faceActivatesPart);
    flag("turnToFace", s.turnToFace);
    flag("doubleEscFinishes", s.doubleEscFinishes);
    number("whenSketchEnds", s.whenSketchEnds, 0, 2);
    flag("clickSelectsBodies", s.clickSelectsBodies);
    flag("shiftAddsOnly", s.shiftAddsOnly);
    flag("preferMovablePoints", s.preferMovablePoints);
    flag("edgesThroughFaces", s.edgesThroughFaces);
    flag("sketch3dCurvePoints", s.sketch3dCurvePoints);
    flag("selectOtherHidden", s.selectOtherHidden);
    flag("altFreesArrows", s.altFreesArrows);
    return s;
  }

  void save() const {
    QSettings store;
    const QString g = QStringLiteral("sketchPrefs/");
    const auto put = [&](const char* name, const QVariant& value) { store.setValue(g + QLatin1String(name), value); };
    put("centerGuides", centerGuides);
    put("pointGuides", pointGuides);
    put("guideCrossings", guideCrossings);
    put("lineUpPoints", lineUpPoints);
    put("shiftSquare", shiftSquare);
    put("stopSnappingKey", stopSnappingKey);
    put("dragDrawsLine", dragDrawsLine);
    put("restForTangentArc", restForTangentArc);
    put("centerArcFollowsCursor", centerArcFollowsCursor);
    put("extendedConstructionLines", extendedConstructionLines);
    put("classicConstructionLines", classicConstructionLines);
    put("mirrorUsesPreselectedLine", mirrorUsesPreselectedLine);
    put("clickInsideProfilePicksShapes", clickInsideProfilePicksShapes);
    put("ellipsesWithAxes", ellipsesWithAxes);
    put("dimensionNewShapes", dimensionNewShapes);
    put("placeFromOrigin", placeFromOrigin);
    put("scaleAboutOrigin", scaleAboutOrigin);
    put("shortcutAgainCloses", shortcutAgainCloses);
    put("shortcutAgainSwitchesMode", shortcutAgainSwitchesMode);
    put("buttonEndsRunningTool", buttonEndsRunningTool);
    put("projectAsConstruction", projectAsConstruction);
    put("askProjectOptions", askProjectOptions);
    put("olderMoveHandles", olderMoveHandles);
    put("cursorCoordinates", cursorCoordinates);
    put("dimensionInWindow", dimensionInWindow);
    put("windowLengthStep", windowLengthStep);
    put("windowAngleStep", windowAngleStep);
    put("dimensionTextSize", dimensionTextSize);
    put("dimensionColorCustom", dimensionColorCustom);
    put("dimensionColor", dimensionColor);
    put("dimensionRim", dimensionRim);
    put("largestSketch", largestSketch);
    put("stateIcons", stateIcons);
    put("conflictColor", conflictColor);
    put("hideGlyphsZoomedOut", hideGlyphsZoomedOut);
    put("glyphSize", glyphSize);
    put("faceActivatesPart", faceActivatesPart);
    put("turnToFace", turnToFace);
    put("doubleEscFinishes", doubleEscFinishes);
    put("whenSketchEnds", whenSketchEnds);
    put("clickSelectsBodies", clickSelectsBodies);
    put("shiftAddsOnly", shiftAddsOnly);
    put("preferMovablePoints", preferMovablePoints);
    put("edgesThroughFaces", edgesThroughFaces);
    put("sketch3dCurvePoints", sketch3dCurvePoints);
    put("selectOtherHidden", selectOtherHidden);
    put("altFreesArrows", altFreesArrows);
  }
};

class SketchPreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::SketchPreferences)

public:
  // `snapToGrid`: the view's grid snap, which "Snap to the grid's visible
  // crossings" shows; applySnapToGrid sets it.
  explicit SketchPreferences(bool snapToGrid, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("sketch"), parent), m_prefs(SketchPrefs::load()), m_snapToGrid(snapToGrid) {
    buildSnapping();
    buildDrawing();
    buildOpening();
    buildSelecting();
    finish();
  }

  std::function<void(bool)> applySnapToGrid;

  // A row PureCAD had, put in its place: its own check box (which keeps
  // its saving) with SolidCraft's words for it. Places: "scale" (the first
  // dimension scales the drawing), "project" (face edges into new
  // sketches), "click" (a click adds to the selection), "esc" (Esc leaves
  // an unchanged sketch), "profiles" (profiles shown when a sketch opens),
  // "hideUsed" (a used sketch hidden).
  void adopt(const QString& place, QCheckBox* box, const QString& text, const QString& tip) {
    QLayout* slot = m_slots.value(place);
    if (slot == nullptr || box == nullptr) return;
    box->setText(text);
    auto* row = new QHBoxLayout;
    row->setSpacing(6);
    row->addWidget(box);
    if (!tip.isEmpty()) row->addWidget(info(tip));
    row->addStretch(1);
    if (auto* column = qobject_cast<QVBoxLayout*>(slot)) column->addLayout(row);
    connect(box, &QCheckBox::toggled, this, [this] { changed(); });
  }

  void commit() {
    m_prefs.save();
    if (applySnapToGrid) applySnapToGrid(m_snapToGrid);
    qDebug().noquote() << QStringLiteral("Preferences: sketch saved");
  }

private:
  // A check in a grid cell, with its ⓘ.
  void gridCheck(QGridLayout* g, int row, int column, const QString& title, const char* part, bool& field,
                 const QString& tip = QString()) {
    auto* cell = new QHBoxLayout;
    cell->setSpacing(6);
    QCheckBox* box = makeCheck(title, part, field, [&field](bool on) { field = on; });
    cell->addWidget(box);
    if (!tip.isEmpty()) cell->addWidget(info(tip));
    cell->addStretch(1);
    g->addLayout(cell, row, column);
  }
  QVBoxLayout* slot(const QString& place, QVBoxLayout* into) {
    auto* column = new QVBoxLayout;
    column->setSpacing(8);
    into->addLayout(column);
    m_slots.insert(place, column);
    return column;
  }

  void buildSnapping() {
    QVBoxLayout* column = section(tr("Snapping and guides"));
    auto* row = new QHBoxLayout;
    row->setSpacing(24);
    auto* picture = new PictureCard(QString(), QString(),
                                    [](QPainter& p, const QRectF& r, const QPalette& pal) {
                                      const QPointF c = r.center() + QPointF(-14, 0);
                                      QColor ink = pal.color(QPalette::WindowText);
                                      p.setPen(QPen(ink, 1.6));
                                      p.setBrush(Qt::NoBrush);
                                      p.drawRect(QRectF(c.x() - 24, c.y() - 16, 48, 32));
                                      QColor guide = ink;
                                      guide.setAlphaF(0.5f);
                                      p.setPen(QPen(guide, 1, Qt::DotLine));
                                      p.drawLine(QPointF(c.x(), r.top() + 6), QPointF(c.x(), r.bottom() - 6));
                                      p.drawLine(QPointF(r.left() + 6, c.y()), QPointF(r.right() - 6, c.y()));
                                      p.setPen(Qt::NoPen);
                                      p.setBrush(pal.color(QPalette::Highlight));
                                      p.drawEllipse(c, 3, 3);
                                      const QPointF d(c.x() + 52, c.y());
                                      p.setPen(QPen(ink, 1.2));
                                      p.setBrush(pal.color(QPalette::Highlight).lighter(150));
                                      p.drawPolygon(QPolygonF{d + QPointF(0, -7), d + QPointF(7, 0), d + QPointF(0, 7),
                                                              d + QPointF(-7, 0)});
                                    },
                                    false);
    picture->setCheckable(false);
    picture->setEnabled(true);
    picture->setCursor(Qt::ArrowCursor);
    picture->setCompact(170, 96);
    picture->setFixedSize(uiRoomSize(170), uiRoomSize(96));
    row->addWidget(picture, 0, Qt::AlignTop);
    auto* checks = new QVBoxLayout;
    checks->setSpacing(8);
    check(checks, tr("Center and midpoint guides"), "centerGuides", m_prefs.centerGuides,
          [this](bool on) { m_prefs.centerGuides = on; },
          tr("Dotted guides through lines' centers and midpoints while you draw"));
    check(checks, tr("Guides from sketch points too"), "pointGuides", m_prefs.pointGuides,
          [this](bool on) { m_prefs.pointGuides = on; }, tr("The sketch's points cast guides as well"));
    check(checks, tr("Snap to where two guides cross"), "guideCrossings", m_prefs.guideCrossings,
          [this](bool on) { m_prefs.guideCrossings = on; }, tr("The pointer snaps where two guides meet"));
    check(checks, tr("Line up with other sketch points"), "lineUpPoints", m_prefs.lineUpPoints,
          [this](bool on) { m_prefs.lineUpPoints = on; },
          tr("A point being drawn lines up horizontally or vertically with others"));
    check(checks, tr("Snap to the grid's visible crossings"), "snapToGrid", m_snapToGrid,
          [this](bool on) { m_snapToGrid = on; }, tr("The pointer snaps to the grid lines you see"));
    check(checks, tr("Shift makes a rectangle a square while drawing it"), "shiftSquare", m_prefs.shiftSquare,
          [this](bool on) { m_prefs.shiftSquare = on; });
    row->addLayout(checks, 1);
    column->addLayout(row);
    QFormLayout* f = form(column);
    auto* key = new QComboBox;
    key->setObjectName(name("stopSnappingKey"));
    key->addItems({tr("Ctrl/Cmd or Alt"), tr("Ctrl/Cmd"), tr("Alt"), tr("Shift")});
    key->setCurrentIndex(m_prefs.stopSnappingKey);
    key->setMinimumWidth(uiRoomSize(220));
    connect(key, &QComboBox::currentIndexChanged, this, [this](int i) {
      m_prefs.stopSnappingKey = i;
      changed();
    });
    f->addRow(tr("Hold to stop snapping"), key);
  }

  void buildDrawing() {
    QVBoxLayout* column = section(tr("Drawing"));
    auto* g = new QGridLayout;
    g->setHorizontalSpacing(28);
    g->setVerticalSpacing(10);
    column->addLayout(g);
    int l = 0;
    gridCheck(g, l++, 0, tr("Press and drag draws one line"), "dragDrawsLine", m_prefs.dragDrawsLine,
              tr("Line draws one segment from the press to the release"));
    gridCheck(g, l++, 0, tr("Rest on a line's last point to switch to a tangent arc"), "restForTangentArc",
              m_prefs.restForTangentArc, tr("Hold the pointer still on the end of a line to continue with an arc"));
    gridCheck(g, l++, 0, tr("Center-point arc follows the cursor"), "centerArcFollowsCursor",
              m_prefs.centerArcFollowsCursor, tr("The arc's sweep follows the pointer around its center"));
    gridCheck(g, l++, 0, tr("Extended construction line tools"), "extendedConstructionLines",
              m_prefs.extendedConstructionLines);
    gridCheck(g, l++, 0, tr("Classic construction line tools"), "classicConstructionLines",
              m_prefs.classicConstructionLines, tr("The construction line tools as they were before"));
    gridCheck(g, l++, 0, tr("Use the preselected construction line for Mirror"), "mirrorUsesPreselectedLine",
              m_prefs.mirrorUsesPreselectedLine);
    gridCheck(g, l++, 0, tr("A click inside a profile also picks the shapes inside it"),
              "clickInsideProfilePicksShapes", m_prefs.clickInsideProfilePicksShapes,
              tr("Picking a closed profile also picks the curves it encloses"));
    gridCheck(g, l++, 0, tr("Draw ellipses with their axes"), "ellipsesWithAxes", m_prefs.ellipsesWithAxes,
              tr("An ellipse comes with its major and minor axes as construction lines"));
    gridCheck(g, l++, 0, tr("Dimension new shapes fully"), "dimensionNewShapes", m_prefs.dimensionNewShapes,
              tr("A new rectangle, circle or polygon gets the dimensions that fix its size"));
    gridCheck(g, l++, 0, tr("Place from the sketch origin"), "placeFromOrigin", m_prefs.placeFromOrigin,
              tr("New dimensions measure from the sketch's origin"));
    // The first dimension scales the drawing: PureCAD's own row (adopt).
    auto* scaleSlot = new QVBoxLayout;
    g->addLayout(scaleSlot, l++, 0);
    m_slots.insert(QStringLiteral("scale"), scaleSlot);
    gridCheck(g, l++, 0, tr("Scale about the sketch origin"), "scaleAboutOrigin", m_prefs.scaleAboutOrigin,
              tr("The first dimension's scaling keeps the origin in place"));
    int r = 0;
    gridCheck(g, r++, 1, tr("Pressing an active tool's shortcut again closes it"), "shortcutAgainCloses",
              m_prefs.shortcutAgainCloses, tr("L, then L again, ends Line"));
    gridCheck(g, r++, 1, tr("Shortcut again switches mode"), "shortcutAgainSwitchesMode",
              m_prefs.shortcutAgainSwitchesMode, tr("A tool's shortcut again goes to its next mode"));
    gridCheck(g, r++, 1, tr("Clicking the running tool's button ends it"), "buttonEndsRunningTool",
              m_prefs.buttonEndsRunningTool, tr("Its toolbar button ends the tool as Esc does"));
    auto* projectSlot = new QVBoxLayout;
    g->addLayout(projectSlot, r++, 1);
    m_slots.insert(QStringLiteral("project"), projectSlot);
    gridCheck(g, r++, 1, tr("Project references as construction geometry"), "projectAsConstruction",
              m_prefs.projectAsConstruction, tr("Projected edges come in as construction lines"));
    gridCheck(g, r++, 1, tr("Ask for Project options first"), "askProjectOptions", m_prefs.askProjectOptions,
              tr("Project opens its options before you pick"));
    gridCheck(g, r++, 1, tr("Older sketch move handles"), "olderMoveHandles", m_prefs.olderMoveHandles,
              tr("Move's handles as they were before"));
    gridCheck(g, r++, 1, tr("Show the cursor's coordinates in a sketch"), "cursorCoordinates",
              m_prefs.cursorCoordinates, tr("The pointer's x and y beside it while you draw"));
    g->setColumnStretch(0, 1);
    g->setColumnStretch(1, 1);
    column->addSpacing(8);
    QFormLayout* f = form(column);
    QWidget* steps = new QWidget;
    auto* stepRow = new QHBoxLayout(steps);
    stepRow->setContentsMargins(0, 0, 0, 0);
    auto* lengthStep = new QLineEdit(QString::number(m_prefs.windowLengthStep, 'g', 6) + QStringLiteral(" mm"));
    auto* angleStep = new QLineEdit(QString::number(m_prefs.windowAngleStep, 'f', 1) + QStringLiteral("°"));
    for (QLineEdit* edit : {lengthStep, angleStep}) edit->setMaximumWidth(uiRoomSize(110));
    connect(lengthStep, &QLineEdit::editingFinished, this, [this, lengthStep] {
      bool ok = false;
      const double v = lengthStep->text().remove(QStringLiteral("mm")).trimmed().toDouble(&ok);
      if (ok && v > 0) m_prefs.windowLengthStep = v;
      changed();
    });
    connect(angleStep, &QLineEdit::editingFinished, this, [this, angleStep] {
      bool ok = false;
      const double v = angleStep->text().remove(QStringLiteral("°")).trimmed().toDouble(&ok);
      if (ok && v > 0) m_prefs.windowAngleStep = v;
      changed();
    });
    stepRow->addWidget(lengthStep);
    stepRow->addWidget(angleStep);
    stepRow->addStretch(1);
    steps->setEnabled(m_prefs.dimensionInWindow);
    f->addRow(label(tr("Dimension values"),
                    tr("Where you type a dimension's value. A window has arrows that step it; the Dimension tool "
                       "places the text first.")),
              segments("dimensionValues", {tr("In the view"), tr("In a window")}, m_prefs.dimensionInWindow ? 1 : 0,
                       [this, steps](int i) {
                         m_prefs.dimensionInWindow = i == 1;
                         steps->setEnabled(i == 1);
                       }));
    f->addRow(tr("Window steps"), steps);
    QWidget* textSize = slider("dimensionTextSize", 50, 300, m_prefs.dimensionTextSize,
                               [](int v) { return QStringLiteral("%1 %").arg(v); },
                               [this](int v) { m_prefs.dimensionTextSize = v; });
    textSize->setMinimumWidth(uiRoomSize(400));
    f->addRow(label(tr("Dimension text size"), tr("Scales dimension values in sketches and in shown dimensions.")),
              textSize);
    // Dimension color: Theme or Custom, the colour, the rim.
    auto* colorBox = new QWidget;
    auto* colorColumn = new QVBoxLayout(colorBox);
    colorColumn->setContentsMargins(0, 0, 0, 0);
    colorColumn->setSpacing(8);
    auto* swatch = new QPushButton;
    swatch->setObjectName(name("dimensionColorSwatch"));
    swatch->setFixedSize(uiRoomSize(44), uiRoomSize(32));
    auto* hex = new QLineEdit(m_prefs.dimensionColor);
    hex->setMaximumWidth(uiRoomSize(140));
    const auto paintSwatch = [swatch](const QString& color) {
      swatch->setStyleSheet(QStringLiteral("QPushButton { background: %1; border: 1px solid rgba(0,0,0,60); "
                                           "border-radius: 6px; }")
                                .arg(color));
    };
    paintSwatch(m_prefs.dimensionColor);
    connect(swatch, &QPushButton::clicked, this, [this, hex, paintSwatch] {
      const QColor chosen = QColorDialog::getColor(QColor(m_prefs.dimensionColor), this, tr("Dimension color"));
      if (!chosen.isValid()) return;
      m_prefs.dimensionColor = chosen.name();
      hex->setText(m_prefs.dimensionColor);
      paintSwatch(m_prefs.dimensionColor);
      changed();
    });
    connect(hex, &QLineEdit::editingFinished, this, [this, hex, paintSwatch] {
      const QColor typed(hex->text().trimmed());
      if (!typed.isValid()) return;
      m_prefs.dimensionColor = typed.name();
      paintSwatch(m_prefs.dimensionColor);
      changed();
    });
    auto* swatchRow = new QHBoxLayout;
    swatchRow->addWidget(swatch);
    swatchRow->addWidget(hex);
    swatchRow->addStretch(1);
    QWidget* colorChoice = segments("dimensionColor", {tr("Theme"), tr("Custom")}, m_prefs.dimensionColorCustom ? 1 : 0,
                                    [this, swatch, hex](int i) {
                                      m_prefs.dimensionColorCustom = i == 1;
                                      swatch->setEnabled(i == 1);
                                      hex->setEnabled(i == 1);
                                    });
    swatch->setEnabled(m_prefs.dimensionColorCustom);
    hex->setEnabled(m_prefs.dimensionColorCustom);
    auto* choiceRow = new QHBoxLayout;
    choiceRow->addWidget(colorChoice);
    choiceRow->addStretch(1);
    colorColumn->addLayout(choiceRow);
    colorColumn->addLayout(swatchRow);
    colorColumn->addWidget(makeCheck(tr("Rim under dimensions and fixed sketch lines"), "dimensionRim",
                                     m_prefs.dimensionRim, [this](bool on) { m_prefs.dimensionRim = on; }));
    f->addRow(label(tr("Dimension color"), tr("The theme's colors, or a color of your own for the lines, arrows and "
                                              "values. Selection, hover and conflicts keep their colors.")),
              colorBox);
    auto* largest = new QSpinBox;
    largest->setObjectName(name("largestSketch"));
    largest->setRange(1000, 200000);
    largest->setSingleStep(1000);
    largest->setValue(m_prefs.largestSketch);
    largest->setSuffix(tr(" points"));
    connect(largest, &QSpinBox::valueChanged, this, [this](int v) {
      m_prefs.largestSketch = v;
      changed();
    });
    f->addRow(label(tr("Largest sketch"), tr("The most points or curves one sketch holds: 20000 by default, at most "
                                             "200000. Insert SVG and Insert DXF refuse a larger drawing. A large "
                                             "sketch solves and draws more slowly.")),
              largest);
    // The constraints' glyphs.
    auto* cards = new QHBoxLayout;
    cards->setSpacing(12);
    toggleCard(cards, "stateIcons", tr("State icons"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QColor colors[3] = {pal.color(QPalette::WindowText), QColor(196, 120, 20), QColor(190, 40, 40)};
                 for (int i = 0; i < 3; ++i) {
                   const QRectF box(r.center().x() - 54 + i * 38, r.center().y() - 15, 30, 30);
                   p.setPen(QPen(colors[i], 2));
                   p.setBrush(Qt::NoBrush);
                   p.drawRoundedRect(box, 6, 6);
                   QPainterPath arc(box.bottomLeft() + QPointF(8, -7));
                   arc.quadTo(box.center() + QPointF(-2, -12), box.topRight() + QPointF(-7, 9));
                   p.drawPath(arc);
                 }
               },
               m_prefs.stateIcons, [this](bool on) { m_prefs.stateIcons = on; });
    toggleCard(cards, "conflictColor", tr("Conflict color"),
               [](QPainter& p, const QRectF& r, const QPalette&) {
                 const QColor conflict(110, 30, 20);
                 p.setPen(QPen(conflict, 2.2));
                 p.setBrush(Qt::NoBrush);
                 QPainterPath curve(QPointF(r.left() + 16, r.bottom() - 18));
                 curve.cubicTo(QPointF(r.center().x() - 20, r.top() + 2), QPointF(r.center().x() + 30, r.top() + 4),
                               QPointF(r.right() - 16, r.bottom() - 22));
                 p.drawPath(curve);
                 p.drawLine(QPointF(r.left() + 16, r.top() + 18), QPointF(r.right() - 16, r.bottom() - 22));
                 p.setBrush(QColor(255, 255, 255));
                 p.drawEllipse(r.center() + QPointF(0, -6), 6, 6);
                 p.setPen(QPen(conflict, 1.4, Qt::DashLine));
                 p.drawLine(QPointF(r.left() + 16, r.bottom() - 8), QPointF(r.right() - 16, r.bottom() - 8));
               },
               m_prefs.conflictColor, [this](bool on) { m_prefs.conflictColor = on; });
    toggleCard(cards, "hideGlyphsZoomedOut", tr("Hide glyphs zoomed out"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 p.setPen(QPen(pal.color(QPalette::WindowText), 1.8));
                 p.setBrush(Qt::NoBrush);
                 for (int i = 0; i < 4; ++i) {
                   const double y = r.center().y() - 14 + (i % 2 == 0 ? 6 : -4);
                   p.drawRoundedRect(QRectF(r.center().x() - 62 + i * 32, y, 20, 20), 4, 4);
                 }
                 p.drawLine(QPointF(r.left() + 16, r.bottom() - 14), QPointF(r.right() - 16, r.bottom() - 14));
               },
               m_prefs.hideGlyphsZoomedOut, [this](bool on) { m_prefs.hideGlyphsZoomedOut = on; });
    cards->addStretch(1);
    column->addLayout(cards);
    QFormLayout* glyphs = form(column);
    QWidget* glyphSize = slider("glyphSize", 100, 300, m_prefs.glyphSize, [](int v) { return QStringLiteral("%1%").arg(v); },
                                [this](int v) { m_prefs.glyphSize = v; });
    glyphSize->setMinimumWidth(uiRoomSize(440));
    glyphs->addRow(tr("Constraint glyph size"), glyphSize);
  }

  void buildOpening() {
    QVBoxLayout* column = section(tr("Opening and closing"));
    check(column, tr("Sketching on a part's face makes that part active"), "faceActivatesPart",
          m_prefs.faceActivatesPart, [this](bool on) { m_prefs.faceActivatesPart = on; },
          tr("A sketch made on a component's face activates that component"));
    check(column, tr("Turn to face a sketch when it opens"), "turnToFace", m_prefs.turnToFace,
          [this](bool on) { m_prefs.turnToFace = on; }, tr("The view looks straight at the sketch's plane"));
    check(column, tr("Double Esc finishes the sketch"), "doubleEscFinishes", m_prefs.doubleEscFinishes,
          [this](bool on) { m_prefs.doubleEscFinishes = on; }, tr("Esc twice with no tool running ends the sketch"));
    slot(QStringLiteral("esc"), column);
    slot(QStringLiteral("profiles"), column);
    slot(QStringLiteral("hideUsed"), column);
    subheading(column, tr("When a sketch ends"));
    // before › sketch › after, the "after" framed as the view it leaves.
    const auto steps = [](int kind) {
      return [kind](QPainter& p, const QRectF& r, const QPalette& pal) {
        const QColor ink = pal.color(QPalette::WindowText);
        const double w = std::min(40.0, (r.width() - 40) / 3.0);
        const QStringList words{QStringLiteral("before"), QStringLiteral("sketch"), QStringLiteral("after")};
        for (int i = 0; i < 3; ++i) {
          const QRectF box(r.left() + 12 + i * (w + 14), r.center().y() - w / 2 - 6, w, w);
          p.setPen(QPen(ink, i == 2 ? 2.2 : 1.4));
          p.setBrush(pal.color(QPalette::Base));
          p.drawRoundedRect(box, 6, 6);
          const bool cubeView = i == 0 || (i == 2 && kind == 0);
          if (cubeView) {
            const QPointF c = box.center();
            const double s = w * 0.25;
            p.setPen(QPen(ink, 1.1));
            p.setBrush(QColor(220, 224, 230));
            p.drawPolygon(QPolygonF{c + QPointF(0, -s), c + QPointF(s, -s / 2), c + QPointF(0, 0), c + QPointF(-s, -s / 2)});
            p.setBrush(QColor(180, 186, 196));
            p.drawPolygon(QPolygonF{c + QPointF(-s, -s / 2), c, c + QPointF(0, s), c + QPointF(-s, s / 2)});
            p.drawPolygon(QPolygonF{c, c + QPointF(s, -s / 2), c + QPointF(s, s / 2), c + QPointF(0, s)});
          } else {
            const QRectF plane = box.adjusted(w * 0.25, w * 0.25, -w * 0.25, -w * 0.25);
            p.setPen(QPen(ink, 1.2));
            p.setBrush(Qt::NoBrush);
            p.drawRect(plane);
            p.setPen(QPen(ink, 0.8, Qt::DotLine));
            p.drawLine(QPointF(plane.center().x(), box.top() + 3), QPointF(plane.center().x(), box.bottom() - 3));
            p.drawLine(QPointF(box.left() + 3, plane.center().y()), QPointF(box.right() - 3, plane.center().y()));
            if (i == 2 && kind == 2) {
              p.setPen(QPen(ink, 1.0));
              p.drawLine(box.bottomLeft() + QPointF(5, -5), box.topRight() + QPointF(-5, 5));
            }
          }
          if (i < 2) {
            p.setPen(Qt::NoPen);
            p.setBrush(ink);
            const QPointF a(box.right() + 4, box.center().y());
            p.drawPolygon(QPolygonF{a + QPointF(0, -4), a + QPointF(5, 0), a + QPointF(0, 4)});
          }
          QFont f = p.font();
          f.setFamily(QStringLiteral("monospace"));
          f.setPointSizeF(std::max(6.0, f.pointSizeF() * 0.75));
          p.setFont(f);
          p.setPen(ink);
          p.drawText(QRectF(box.left() - 6, box.bottom() + 2, box.width() + 12, 14), Qt::AlignCenter, words.at(i));
        }
      };
    };
    choiceCards(column, "whenSketchEnds",
                {{tr("Return to the view before"), QString(), steps(0)},
                 {tr("Keep"), tr("default"), steps(1)},
                 {tr("Unless turned"), tr("return if not rotated"), steps(2)}},
                m_prefs.whenSketchEnds, [this](int i) { m_prefs.whenSketchEnds = i; }, 220, 132);
  }

  void buildSelecting() {
    QVBoxLayout* column = section(tr("Selecting"));
    subheading(column, tr("Click selects"));
    const auto body = [](bool whole) {
      return [whole](QPainter& p, const QRectF& r, const QPalette& pal) {
        const QPointF c = r.center();
        const double s = 18;
        const QColor ink = pal.color(QPalette::WindowText);
        p.setPen(QPen(ink, 1.2));
        p.setBrush(QColor(110, 150, 200));
        p.drawPolygon(QPolygonF{c + QPointF(0, -s), c + QPointF(s, -s / 2), c, c + QPointF(-s, -s / 2)});
        p.setBrush(whole ? QColor(80, 120, 180) : QColor(225, 228, 232));
        p.drawPolygon(QPolygonF{c + QPointF(-s, -s / 2), c, c + QPointF(0, s), c + QPointF(-s, s / 2)});
        p.setBrush(whole ? QColor(60, 100, 160) : QColor(190, 194, 200));
        p.drawPolygon(QPolygonF{c, c + QPointF(s, -s / 2), c + QPointF(s, s / 2), c + QPointF(0, s)});
      };
    };
    choiceCards(column, "clickSelects", {{tr("Faces and edges"), QString(), body(false)}, {tr("Whole bodies"), QString(), body(true)}},
                m_prefs.clickSelectsBodies ? 1 : 0, [this](int i) { m_prefs.clickSelectsBodies = i == 1; }, 180, 112);
    // The key table follows Shift adds only.
    auto* table = new QGridLayout;
    table->setHorizontalSpacing(40);
    table->setVerticalSpacing(10);
    const auto keys = [this](const QStringList& names) {
      auto* row = new QWidget;
      auto* line = new QHBoxLayout(row);
      line->setContentsMargins(0, 0, 0, 0);
      line->setSpacing(4);
      for (const QString& key : names) {
        auto* chip = new QLabel(key);
        chip->setStyleSheet(QStringLiteral("QLabel { border: 1px solid %1; border-radius: 4px; padding: 1px 7px; }")
                                .arg(palette().color(QPalette::Mid).name()));
        line->addWidget(chip);
      }
      line->addStretch(1);
      return row;
    };
    auto* shiftMeaning = new QLabel;
    const auto meanings = [this, shiftMeaning] {
      shiftMeaning->setText(m_prefs.shiftAddsOnly ? tr("Add the item") : tr("Add the item, or take it out again"));
    };
    check(column, tr("Shift adds only; Ctrl / Cmd toggles"), "shiftAddsOnly", m_prefs.shiftAddsOnly,
          [this, meanings](bool on) {
            m_prefs.shiftAddsOnly = on;
            meanings();
          },
          tr("Shift-click only adds; Ctrl- or Cmd-click adds or takes out"));
    meanings();
    table->addWidget(keys({tr("Click")}), 0, 0);
    table->addWidget(new QLabel(tr("Select one item")), 0, 1);
    table->addWidget(keys({tr("Shift"), tr("Click")}), 1, 0);
    table->addWidget(shiftMeaning, 1, 1);
    table->addWidget(keys({tr("Ctrl/Cmd"), tr("Click")}), 2, 0);
    table->addWidget(new QLabel(tr("Add the item, or take it out again")), 2, 1);
    table->setColumnStretch(1, 1);
    column->addLayout(table);
    slot(QStringLiteral("click"), column);
    check(column, tr("Prefer movable points where they overlap"), "preferMovablePoints", m_prefs.preferMovablePoints,
          [this](bool on) { m_prefs.preferMovablePoints = on; });
    check(column, tr("Select edges through faces"), "edgesThroughFaces", m_prefs.edgesThroughFaces,
          [this](bool on) { m_prefs.edgesThroughFaces = on; }, tr("An edge behind a face can be picked through it"));
    check(column, tr("Select 3D sketch curve points"), "sketch3dCurvePoints", m_prefs.sketch3dCurvePoints,
          [this](bool on) { m_prefs.sketch3dCurvePoints = on; }, tr("The points on a 3D sketch's curves can be picked"));
    check(column, tr("Select Other reaches hidden geometry"), "selectOtherHidden", m_prefs.selectOtherHidden,
          [this](bool on) { m_prefs.selectOtherHidden = on; }, tr("Select Other lists what is hidden under the pointer too"));
    check(column, tr("Alt / Option also frees manipulator arrows"), "altFreesArrows", m_prefs.altFreesArrows,
          [this](bool on) { m_prefs.altFreesArrows = on; },
          tr("Holding Alt while dragging an arrow drags it without snapping"));
  }

  SketchPrefs m_prefs;
  bool m_snapToGrid;
  QHash<QString, QLayout*> m_slots;
};

} // namespace mitcad
