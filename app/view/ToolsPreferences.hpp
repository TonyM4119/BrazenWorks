// SPDX-License-Identifier: MIT
#pragma once

// Fork: Preferences › Tools, as SolidCraft's: Create (Hole, Extrude,
// Placed features), Command dialogs, Assembly move, and Measure and
// pointer. Extrude's Choose the operation automatically is the General
// page's Extrude starts with (set through `applyAutoOperation`); the rest
// is ToolsPrefs ("tools/").

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QSettings>

#include "PreferenceKit.hpp"

namespace mitcad {

struct ToolsPrefs {
  bool holeSnapCenters = false;
  bool holeMoveByClick = false;
  bool holeSeveralOnFace = false;
  bool holeGroupTogether = false;
  bool extrudeToTarget = true;
  bool extrudeArrowStops = true;
  bool extrudeIsometric = false;
  bool extrudeReturnView = false;
  int newExtrudeDistance = 0;       // 0 Fixed, 1 The last used, 2 From the profile's size
  double fixedExtrudeDistance = 10.0; // mm
  bool placementHandles = false;
  bool placedSnapPoints = true;
  bool rememberDialogValues = false;
  bool previousFilletDefault = false;
  bool jointMotionHandles = true;
  bool movedGhost = true;
  bool highlightUnderMove = false;
  double nearAxisStep = 10.0;       // degrees
  double farAxisStep = 0.10;        // degrees
  bool axisDistances = true;
  bool toolAtCursor = false;
  bool directInteraction = false;

  static ToolsPrefs load() {
    const QSettings store;
    ToolsPrefs s;
    const QString g = QStringLiteral("tools/");
    const auto flag = [&](const char* n, bool& v) { v = store.value(g + QLatin1String(n), v).toBool(); };
    const auto real = [&](const char* n, double& v) {
      bool ok = false;
      const double read = store.value(g + QLatin1String(n), v).toDouble(&ok);
      if (ok && read > 0) v = read;
    };
    flag("holeSnapCenters", s.holeSnapCenters);
    flag("holeMoveByClick", s.holeMoveByClick);
    flag("holeSeveralOnFace", s.holeSeveralOnFace);
    flag("holeGroupTogether", s.holeGroupTogether);
    flag("extrudeToTarget", s.extrudeToTarget);
    flag("extrudeArrowStops", s.extrudeArrowStops);
    flag("extrudeIsometric", s.extrudeIsometric);
    flag("extrudeReturnView", s.extrudeReturnView);
    s.newExtrudeDistance = std::clamp(store.value(g + QStringLiteral("newExtrudeDistance"), 0).toInt(), 0, 2);
    real("fixedExtrudeDistance", s.fixedExtrudeDistance);
    flag("placementHandles", s.placementHandles);
    flag("placedSnapPoints", s.placedSnapPoints);
    flag("rememberDialogValues", s.rememberDialogValues);
    flag("previousFilletDefault", s.previousFilletDefault);
    flag("jointMotionHandles", s.jointMotionHandles);
    flag("movedGhost", s.movedGhost);
    flag("highlightUnderMove", s.highlightUnderMove);
    real("nearAxisStep", s.nearAxisStep);
    real("farAxisStep", s.farAxisStep);
    flag("axisDistances", s.axisDistances);
    flag("toolAtCursor", s.toolAtCursor);
    flag("directInteraction", s.directInteraction);
    return s;
  }
  void save() const {
    QSettings store;
    const QString g = QStringLiteral("tools/");
    const auto put = [&](const char* n, const QVariant& v) { store.setValue(g + QLatin1String(n), v); };
    put("holeSnapCenters", holeSnapCenters);
    put("holeMoveByClick", holeMoveByClick);
    put("holeSeveralOnFace", holeSeveralOnFace);
    put("holeGroupTogether", holeGroupTogether);
    put("extrudeToTarget", extrudeToTarget);
    put("extrudeArrowStops", extrudeArrowStops);
    put("extrudeIsometric", extrudeIsometric);
    put("extrudeReturnView", extrudeReturnView);
    put("newExtrudeDistance", newExtrudeDistance);
    put("fixedExtrudeDistance", fixedExtrudeDistance);
    put("placementHandles", placementHandles);
    put("placedSnapPoints", placedSnapPoints);
    put("rememberDialogValues", rememberDialogValues);
    put("previousFilletDefault", previousFilletDefault);
    put("jointMotionHandles", jointMotionHandles);
    put("movedGhost", movedGhost);
    put("highlightUnderMove", highlightUnderMove);
    put("nearAxisStep", nearAxisStep);
    put("farAxisStep", farAxisStep);
    put("axisDistances", axisDistances);
    put("toolAtCursor", toolAtCursor);
    put("directInteraction", directInteraction);
  }
};

class ToolsPreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::ToolsPreferences)

public:
  // `icon`: a command's icon by its icon name (the ribbon's), for the
  // Create rows; `autoOperation`: Extrude starts with the operation from
  // the bodies it touches.
  ToolsPreferences(std::function<QIcon(const QString&)> icon, bool autoOperation, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("tools"), parent), m_prefs(ToolsPrefs::load()), m_icon(std::move(icon)),
        m_autoOperation(autoOperation) {
    buildCreate();
    buildDialogs();
    buildMove();
    buildMeasure();
    finish();
  }

  std::function<void(bool)> applyAutoOperation;

  void commit() {
    m_prefs.save();
    qDebug().noquote() << QStringLiteral("Preferences: tools saved");
  }

private:
  // A tool's row in the Create card: its icons and name at the left, its
  // checks at the right.
  QVBoxLayout* toolRow(QVBoxLayout* card, const QStringList& icons, const QString& title, bool line) {
    if (line) {
      auto* rule = new QFrame;
      rule->setFrameShape(QFrame::HLine);
      rule->setStyleSheet(QStringLiteral("color: %1;").arg(palette().color(QPalette::Mid).name()));
      card->addWidget(rule);
    }
    auto* row = new QHBoxLayout;
    row->setSpacing(16);
    auto* head = new QHBoxLayout;
    head->setSpacing(6);
    for (const QString& name : icons) {
      auto* tile = new QLabel;
      tile->setFixedSize(uiRoomSize(36), uiRoomSize(36));
      tile->setAlignment(Qt::AlignCenter);
      tile->setStyleSheet(QStringLiteral("QLabel { background: rgba(128,128,128,30); border-radius: 8px; }"));
      if (m_icon) tile->setPixmap(m_icon(name).pixmap(uiRoomSize(24), uiRoomSize(24)));
      head->addWidget(tile);
    }
    auto* name = new QLabel(title);
    QFont bold = name->font();
    bold.setBold(true);
    name->setFont(bold);
    head->addWidget(name);
    head->addStretch(1);
    auto* headBox = new QWidget;
    headBox->setLayout(head);
    headBox->setFixedWidth(uiRoomSize(300));
    row->addWidget(headBox, 0, Qt::AlignTop);
    auto* checks = new QVBoxLayout;
    checks->setSpacing(8);
    row->addLayout(checks, 1);
    card->addLayout(row);
    return checks;
  }

  void buildCreate() {
    QVBoxLayout* column = section(tr("Create"));
    auto* frame = new QFrame;
    frame->setObjectName(name("createCard"));
    frame->setStyleSheet(QStringLiteral("QFrame#%1 { border: 1px solid %2; border-radius: 10px; }")
                             .arg(frame->objectName(), palette().color(QPalette::Mid).name()));
    auto* card = new QVBoxLayout(frame);
    card->setContentsMargins(16, 14, 16, 14);
    card->setSpacing(12);
    QVBoxLayout* hole = toolRow(card, {QStringLiteral("hole")}, tr("Hole"), false);
    check(hole, tr("Snap to centers"), "holeSnapCenters", m_prefs.holeSnapCenters,
          [this](bool on) { m_prefs.holeSnapCenters = on; }, tr("A hole's center snaps to circles' and arcs' centers"));
    check(hole, tr("Move by click or drag"), "holeMoveByClick", m_prefs.holeMoveByClick,
          [this](bool on) { m_prefs.holeMoveByClick = on; }, tr("A click elsewhere on the face moves the hole there"));
    check(hole, tr("Several holes on one face"), "holeSeveralOnFace", m_prefs.holeSeveralOnFace,
          [this](bool on) { m_prefs.holeSeveralOnFace = on; }, tr("Each click on the face adds another hole"));
    check(hole, tr("Group holes made together"), "holeGroupTogether", m_prefs.holeGroupTogether,
          [this](bool on) { m_prefs.holeGroupTogether = on; }, tr("The holes of one command are one feature"));
    QVBoxLayout* extrude = toolRow(card, {QStringLiteral("extrude")}, tr("Extrude"), true);
    check(extrude, tr("Extrude to a picked target"), "extrudeToTarget", m_prefs.extrudeToTarget,
          [this](bool on) { m_prefs.extrudeToTarget = on; }, tr("A click on a face or vertex extrudes to it"));
    check(extrude, tr("Choose the operation automatically"), "autoOperation", m_autoOperation,
          [this](bool on) {
            m_autoOperation = on;
            if (applyAutoOperation) applyAutoOperation(on);
          },
          tr("Join where it touches a body, Cut where it goes into one, otherwise New Body"));
    check(extrude, tr("Extrude arrow stops at faces"), "extrudeArrowStops", m_prefs.extrudeArrowStops,
          [this](bool on) { m_prefs.extrudeArrowStops = on; }, tr("Dragging the arrow snaps to the faces it passes"));
    check(extrude, tr("Turn the view to isometric when Extrude opens"), "extrudeIsometric", m_prefs.extrudeIsometric,
          [this](bool on) {
            m_prefs.extrudeIsometric = on;
            if (m_returnView != nullptr) m_returnView->setEnabled(on);
          });
    m_returnView = check(extrude, tr("Go back to the earlier view when Extrude closes"), "extrudeReturnView",
                         m_prefs.extrudeReturnView, [this](bool on) { m_prefs.extrudeReturnView = on; });
    m_returnView->setEnabled(m_prefs.extrudeIsometric);
    auto* distanceHead = new QHBoxLayout;
    distanceHead->addWidget(new QLabel(tr("New extrude distance")));
    distanceHead->addWidget(info(tr("The distance a new extrusion starts at")));
    distanceHead->addStretch(1);
    extrude->addLayout(distanceHead);
    auto* distanceRow = new QHBoxLayout;
    auto* distanceKind = new QComboBox;
    distanceKind->setObjectName(name("newExtrudeDistance"));
    distanceKind->addItems({tr("Fixed (%1)").arg(m_prefs.fixedExtrudeDistance), tr("The last used"),
                            tr("From the profile's size")});
    distanceKind->setCurrentIndex(m_prefs.newExtrudeDistance);
    auto* distance = new QDoubleSpinBox;
    distance->setObjectName(name("fixedExtrudeDistance"));
    distance->setRange(0.001, 100000.0);
    distance->setDecimals(3);
    distance->setSuffix(tr(" mm"));
    distance->setValue(m_prefs.fixedExtrudeDistance);
    distance->setEnabled(m_prefs.newExtrudeDistance == 0);
    connect(distanceKind, &QComboBox::currentIndexChanged, this, [this, distance](int i) {
      m_prefs.newExtrudeDistance = i;
      distance->setEnabled(i == 0);
      changed();
    });
    connect(distance, &QDoubleSpinBox::valueChanged, this, [this, distanceKind](double v) {
      m_prefs.fixedExtrudeDistance = v;
      distanceKind->setItemText(0, tr("Fixed (%1)").arg(v));
      changed();
    });
    distanceRow->addWidget(distanceKind);
    distanceRow->addWidget(distance);
    distanceRow->addStretch(1);
    extrude->addLayout(distanceRow);
    QVBoxLayout* placed = toolRow(card, {QStringLiteral("cylinder"), QStringLiteral("box"), QStringLiteral("hole")},
                                  tr("Placed features"), true);
    check(placed, tr("Placement handles"), "placementHandles", m_prefs.placementHandles,
          [this](bool on) { m_prefs.placementHandles = on; }, tr("Arrows to move a placed feature before OK"));
    check(placed, tr("Snap to center, middle and end points when picking a place"), "placedSnapPoints",
          m_prefs.placedSnapPoints, [this](bool on) { m_prefs.placedSnapPoints = on; });
    column->addWidget(frame);
  }

  void buildDialogs() {
    QVBoxLayout* column = section(tr("Command dialogs"));
    check(column, tr("Remember command dialog values"), "rememberDialogValues", m_prefs.rememberDialogValues,
          [this](bool on) { m_prefs.rememberDialogValues = on; });
    check(column, tr("Use the previous Solid Fillet radius default"), "previousFilletDefault",
          m_prefs.previousFilletDefault, [this](bool on) { m_prefs.previousFilletDefault = on; });
  }

  void buildMove() {
    QVBoxLayout* column = section(tr("Assembly move"));
    auto* row = new QHBoxLayout;
    row->setSpacing(24);
    auto* picture = new PictureCard(QString(), QString(),
                                    [](QPainter& p, const QRectF& r, const QPalette& pal) {
                                      const QPointF c = r.center();
                                      const QColor ink = pal.color(QPalette::WindowText);
                                      p.setPen(QPen(ink, 1.4, Qt::DashLine));
                                      p.setBrush(Qt::NoBrush);
                                      p.drawEllipse(c, 30, 30);
                                      p.setPen(QPen(ink, 1.4));
                                      p.drawEllipse(c, 24, 24);
                                      p.drawRect(QRectF(c.x() - 7, c.y() - 7, 14, 14));
                                      p.drawEllipse(c + QPointF(24, 0), 4, 4);
                                    },
                                    false);
    picture->setCheckable(false);
    picture->setFixedSize(uiRoomSize(150), uiRoomSize(96));
    row->addWidget(picture, 0, Qt::AlignTop);
    auto* checks = new QVBoxLayout;
    checks->setSpacing(8);
    check(checks, tr("Use joint motion handles for Move"), "jointMotionHandles", m_prefs.jointMotionHandles,
          [this](bool on) { m_prefs.jointMotionHandles = on; },
          tr("Move shows the joint's own handles: only the motion the joint allows"));
    check(checks, tr("Show moved parts' old place as a ghost"), "movedGhost", m_prefs.movedGhost,
          [this](bool on) { m_prefs.movedGhost = on; });
    check(checks, tr("Highlight what is under the pointer while Move's arrows drag a component"), "highlightUnderMove",
          m_prefs.highlightUnderMove, [this](bool on) { m_prefs.highlightUnderMove = on; },
          tr("Faces and edges under the pointer light up as snap targets during the drag"));
    const auto step = [this, checks](const QString& title, const QString& tip, double& field, int decimals) {
      auto* line = new QHBoxLayout;
      auto* text = new QLabel(title);
      text->setMinimumWidth(uiRoomSize(150));
      line->addWidget(text);
      if (!tip.isEmpty()) line->addWidget(info(tip));
      auto* spin = new QDoubleSpinBox;
      spin->setRange(0.01, 90.0);
      spin->setDecimals(decimals);
      spin->setValue(field);
      connect(spin, &QDoubleSpinBox::valueChanged, this, [this, &field](double v) {
        field = v;
        changed();
      });
      line->addWidget(spin);
      line->addWidget(new QLabel(tr("deg")));
      line->addStretch(1);
      checks->addLayout(line);
    };
    step(tr("Near-axis step"), tr("The rotation step while the pointer is near the axis"), m_prefs.nearAxisStep, 1);
    step(tr("Far-axis step"), QString(), m_prefs.farAxisStep, 2);
    row->addLayout(checks, 1);
    column->addLayout(row);
  }

  void buildMeasure() {
    QVBoxLayout* column = section(tr("Measure and pointer"));
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    toggleCard(row, "axisDistances", tr("Axis distances in Measure"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QColor ink = pal.color(QPalette::WindowText);
                 const QRectF box(r.center().x() - 36, r.center().y() - 6, 56, 30);
                 p.setPen(QPen(ink, 1.6));
                 p.setBrush(Qt::NoBrush);
                 p.drawRect(box);
                 p.drawLine(QPointF(box.left(), box.top() - 12), QPointF(box.right(), box.top() - 12));
                 p.drawLine(QPointF(box.right() + 14, box.top()), QPointF(box.right() + 14, box.bottom()));
                 p.drawText(QRectF(box.left(), box.top() - 30, box.width(), 16), Qt::AlignCenter, QStringLiteral("X 46"));
                 p.drawText(QRectF(box.right() + 18, box.center().y() - 8, 40, 16), Qt::AlignLeft, QStringLiteral("Y 28"));
               },
               m_prefs.axisDistances, [this](bool on) { m_prefs.axisDistances = on; });
    toggleCard(row, "toolAtCursor", tr("Tool at the cursor"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QColor ink = pal.color(QPalette::WindowText);
                 const QPointF c = r.center();
                 p.setPen(QPen(ink, 1.4));
                 p.drawLine(c + QPointF(-40, 22), c + QPointF(-6, -14));
                 p.setBrush(ink);
                 p.drawPolygon(QPolygonF{c + QPointF(-6, -16), c + QPointF(6, 0), c + QPointF(0, 0), c + QPointF(4, 8),
                                         c + QPointF(1, 9), c + QPointF(-3, 1), c + QPointF(-8, 5)});
                 p.setBrush(pal.color(QPalette::Base));
                 p.drawRoundedRect(QRectF(c.x() + 6, c.y() + 2, 30, 26), 6, 6);
                 p.drawLine(c + QPointF(12, 22), c + QPointF(30, 8));
               },
               m_prefs.toolAtCursor, [this](bool on) { m_prefs.toolAtCursor = on; });
    toggleCard(row, "directInteraction", tr("Direct interaction"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QColor ink = pal.color(QPalette::WindowText);
                 const QPointF c = r.center() + QPointF(-14, 4);
                 p.setPen(QPen(ink, 1.3));
                 p.setBrush(QColor(200, 210, 225));
                 p.drawPolygon(QPolygonF{c + QPointF(0, -18), c + QPointF(16, -10), c + QPointF(0, -2), c + QPointF(-16, -10)});
                 p.setBrush(QColor(170, 182, 200));
                 p.drawPolygon(QPolygonF{c + QPointF(-16, -10), c + QPointF(0, -2), c + QPointF(0, 22), c + QPointF(-16, 14)});
                 p.drawPolygon(QPolygonF{c + QPointF(0, -2), c + QPointF(16, -10), c + QPointF(16, 14), c + QPointF(0, 22)});
                 p.setPen(QPen(pal.color(QPalette::Highlight), 2));
                 p.drawLine(c + QPointF(0, -10), c + QPointF(0, -30));
                 p.setPen(QPen(ink, 1.2));
                 p.setBrush(pal.color(QPalette::Base));
                 const QRectF tag(c.x() + 26, c.y() - 26, 40, 16);
                 p.drawRoundedRect(tag, 5, 5);
                 QFont f = p.font();
                 f.setPointSizeF(std::max(6.0, f.pointSizeF() * 0.7));
                 p.setFont(f);
                 p.drawText(tag, Qt::AlignCenter, QStringLiteral("+18 mm"));
               },
               m_prefs.directInteraction, [this](bool on) { m_prefs.directInteraction = on; });
    row->addStretch(1);
    column->addLayout(row);
  }

  ToolsPrefs m_prefs;
  std::function<QIcon(const QString&)> m_icon;
  bool m_autoOperation;
  QCheckBox* m_returnView = nullptr;
};

} // namespace mitcad
