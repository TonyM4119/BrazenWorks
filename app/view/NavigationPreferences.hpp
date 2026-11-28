// SPDX-License-Identifier: MIT
#pragma once

#include "Navigation.hpp"
#include "../framework/PreferencesPageLayout.hpp"
#include "../framework/TestSync.hpp"
#include "../framework/UiSizes.hpp"
#include "../platform/MacDialogs.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <functional>
#include <tuple>
#include <utility>

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEnterEvent>
#include <QGroupBox>
#include <QScrollArea>
#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QShowEvent>
#include <QSize>
#include <QSignalBlocker>
#include <QStringList>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace mitcad {

inline QString navigationActionText(NavigationAction action) {
  switch (action) {
  case NavigationAction::Orbit: return QObject::tr("Orbit");
  case NavigationAction::Pan: return QObject::tr("Pan");
  case NavigationAction::Zoom: return QObject::tr("Zoom");
  case NavigationAction::None: return QObject::tr("None");
  }
  return {};
}

inline QString navigationInputText(const NavigationBinding& binding) {
  QStringList keys;
  if (binding.f4) keys << QStringLiteral("F4");
  if (binding.modifiers.testFlag(Qt::ShiftModifier)) keys << QStringLiteral("Shift");
#ifdef Q_OS_MACOS
  if (binding.modifiers.testFlag(Qt::ControlModifier)) keys << QStringLiteral("Command");
  if (binding.modifiers.testFlag(Qt::AltModifier)) keys << QStringLiteral("Option");
#else
  if (binding.modifiers.testFlag(Qt::ControlModifier)) keys << QStringLiteral("Ctrl");
  if (binding.modifiers.testFlag(Qt::AltModifier)) keys << QStringLiteral("Alt");
#endif
  switch (binding.input) {
  case NavigationInput::LeftButton: keys << QObject::tr("Left drag"); break;
  case NavigationInput::MiddleButton: keys << QObject::tr("Middle drag"); break;
  case NavigationInput::RightButton: keys << QObject::tr("Right drag"); break;
  case NavigationInput::TrackpadScroll: keys << QObject::tr("Two-finger scroll"); break;
  }
  return keys.join(QStringLiteral(" + "));
}

// Fork: the guide's rows as SolidCraft lists them: the keys and button of
// each binding the mapping has, then the wheel and the clicks.
struct NavigationGuideRow {
  QStringList keys;
  QString action;
};

inline QVector<NavigationGuideRow> navigationGuideRows(const NavigationSettings& settings) {
  QVector<NavigationGuideRow> rows;
  const auto bindings = effectiveNavigationBindings(settings);
  for (const auto input : {NavigationInput::LeftButton, NavigationInput::MiddleButton, NavigationInput::RightButton,
                           NavigationInput::TrackpadScroll}) {
    for (const auto& binding : bindings) {
      if (binding.input != input || binding.action == NavigationAction::None || !navigationBindingAllowed(binding)) {
        continue;
      }
      // A two-finger scroll zooms unless the mapping says otherwise: only
      // what it does otherwise is listed.
      if (input == NavigationInput::TrackpadScroll && binding.action == NavigationAction::Zoom) {
        continue;
      }
      rows.append({navigationInputText(binding).split(QStringLiteral(" + ")), navigationActionText(binding.action)});
    }
  }
  rows.append({{QObject::tr("Wheel")}, settings.zoomToCursor ? QObject::tr("Zoom, toward the cursor") : QObject::tr("Zoom")});
  rows.append({{QObject::tr("Left click")}, QObject::tr("Select")});
  rows.append({{QObject::tr("Right click")}, QObject::tr("Context menu")});
  return rows;
}

inline constexpr int kNavigationGuideRow = 30;

inline int navigationGuideHeight(const NavigationSettings& settings) {
  return std::max(156, int(navigationGuideRows(settings).size()) * kNavigationGuideRow + 8);
}

// Pure artwork shared by the Qt guide and its native AppKit image view.
// The caller supplies font, palette and logical size; no QWidget is drawn.
// Fork: SolidCraft's: the mouse in a tile at the left, its middle button
// marked, and the mapping's rows at the right, each binding's keys and
// button in key caps and what it does, a hairline between the rows.
inline void paintNavigationGuide(QPainter& p, const QSizeF& size, const NavigationSettings& settings,
                                 const QPalette& colors, const QFont& font) {
  p.save();
  p.setFont(font);
  p.setRenderHint(QPainter::Antialiasing);
  const QColor text = colors.color(QPalette::Text);
  QColor faint = text;
  faint.setAlphaF(0.06f);
  QColor line = text;
  line.setAlphaF(0.12f);
  const QColor accent = colors.color(QPalette::Highlight);
  // The mouse's tile.
  const QRectF tile(0, 4, 150, std::min(size.height() - 8, 190.0));
  p.setPen(Qt::NoPen);
  p.setBrush(faint);
  p.drawRoundedRect(tile, 10, 10);
  const QPointF c = tile.center();
  const QRectF body(c.x() - 34, c.y() - 50, 68, 100);
  p.setBrush(Qt::NoBrush);
  p.setPen(QPen(text, 2.2));
  p.drawRoundedRect(body, 32, 32);
  p.drawLine(QPointF(body.left() + 1, body.top() + 46), QPointF(body.right() - 1, body.top() + 46));
  p.drawLine(QPointF(c.x(), body.top() + 1), QPointF(c.x(), body.top() + 46));
  const QRectF wheel(c.x() - 6, body.top() + 14, 12, 24);
  p.setPen(Qt::NoPen);
  p.setBrush(accent);
  p.drawRoundedRect(wheel, 6, 6);
  QFont small = font;
  small.setPointSizeF(std::max(7.0, font.pointSizeF() * 0.78));
  small.setBold(true);
  p.setFont(small);
  p.setPen(colors.color(QPalette::HighlightedText));
  p.drawText(wheel, Qt::AlignCenter, QStringLiteral("M"));
  p.setPen(text);
  p.drawText(QRectF(body.left() + 4, body.top() + 18, 22, 20), Qt::AlignCenter, QStringLiteral("L"));
  p.drawText(QRectF(body.right() - 26, body.top() + 18, 22, 20), Qt::AlignCenter, QStringLiteral("R"));
  // The rows.
  QFont capFont = font;
  capFont.setPointSizeF(std::max(7.5, font.pointSizeF() * 0.92));
  const QFontMetricsF capMetrics(capFont);
  const double left = tile.right() + 28;
  const double actionX = left + std::max(220.0, (size.width() - left) * 0.48);
  const auto rows = navigationGuideRows(settings);
  for (int i = 0; i < rows.size(); ++i) {
    const double y = 4 + i * kNavigationGuideRow;
    double x = left;
    p.setFont(capFont);
    for (const QString& key : rows[i].keys) {
      const double w = capMetrics.horizontalAdvance(key) + 16;
      const QRectF cap(x, y + 4, w, kNavigationGuideRow - 8);
      p.setPen(QPen(line, 1));
      p.setBrush(colors.color(QPalette::Base));
      p.drawRoundedRect(cap.adjusted(0.5, 0.5, -0.5, -0.5), 5, 5);
      p.setPen(text);
      p.drawText(cap, Qt::AlignCenter, key);
      x += w + 6;
    }
    p.setFont(font);
    p.setPen(text);
    p.drawText(QRectF(actionX, y, size.width() - actionX, kNavigationGuideRow), Qt::AlignLeft | Qt::AlignVCenter,
               rows[i].action);
    p.setPen(QPen(line, 1));
    p.drawLine(QPointF(left, y + kNavigationGuideRow - 0.5), QPointF(size.width(), y + kNavigationGuideRow - 0.5));
  }
  p.restore();
}

// The Qt presenter paints the shared vector artwork at its device scale.
class NavigationGuide : public QWidget {
public:
  explicit NavigationGuide(QWidget* parent = nullptr) : QWidget(parent) {
    setObjectName(QStringLiteral("navigationVisualGuide"));
    setMinimumHeight(156);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }
  QSize sizeHint() const override { return {560, minimumHeight()}; }
  const NavigationSettings& navigation() const { return m_settings; }
  quint64 artworkRevision() const { return m_artworkRevision; }
  void setNavigation(const NavigationSettings& settings) {
    m_settings = settings;
    ++m_artworkRevision;
    QStringList labels;
    const auto bindings = effectiveNavigationBindings(settings);
    for (const auto& binding : bindings) {
      if (binding.action != NavigationAction::None && navigationBindingAllowed(binding)) {
        labels << navigationInputText(binding) + QStringLiteral(": ") + navigationActionText(binding.action);
      }
    }
    setAccessibleName(QObject::tr("Mouse and trackpad navigation"));
    setAccessibleDescription(labels.join(QStringLiteral("; ")));
    setMinimumHeight(navigationGuideHeight(settings));
    updateGeometry();
    update();
  }
protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    paintNavigationGuide(p, size(), m_settings, palette(), font());
  }
private:
  NavigationSettings m_settings;
  quint64 m_artworkRevision = 0;
};

// Fork: a choice drawn as a card (SolidCraft's Orbit around and View
// navigator): its picture over its name, the chosen one framed in the
// accent with a check in its corner.
class NavigationChoiceCard : public QAbstractButton {
public:
  using Painter = std::function<void(QPainter&, const QRectF&, const QPalette&)>;
  NavigationChoiceCard(const QString& text, Painter picture, QWidget* parent = nullptr)
      : QAbstractButton(parent), m_picture(std::move(picture)) {
    setText(text);
    setCheckable(true);
    setAutoExclusive(true);
    setCursor(Qt::PointingHandCursor);
    setAccessibleName(text);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }
  QSize sizeHint() const override { return {180, 132}; }
  QSize minimumSizeHint() const override { return {120, 132}; }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    const QColor ink = pal.color(QPalette::WindowText);
    const QColor accent = pal.color(QPalette::Highlight);
    QColor line = ink;
    line.setAlphaF(isChecked() ? 1.0f : (underMouse() ? 0.28f : 0.14f));
    QColor fill = ink;
    fill.setAlphaF(0.04f);
    if (isChecked()) {
      fill = accent;
      fill.setAlphaF(0.10f);
    }
    const QRectF card = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
    p.setPen(QPen(isChecked() ? accent : line, isChecked() ? 2.0 : 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(card, 8, 8);
    const QRectF art(card.left() + 10, card.top() + 8, card.width() - 20, card.height() - 40);
    if (m_picture) m_picture(p, art, pal);
    QFont font = this->font();
    font.setBold(isChecked());
    p.setFont(font);
    p.setPen(ink);
    p.drawText(QRectF(card.left(), card.bottom() - 30, card.width(), 26), Qt::AlignCenter, text());
    if (isChecked()) {
      const QRectF badge(card.right() - 22, card.top() + 6, 16, 16);
      p.setPen(Qt::NoPen);
      p.setBrush(accent);
      p.drawEllipse(badge);
      p.setPen(QPen(pal.color(QPalette::HighlightedText), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      p.drawPolyline(QPolygonF{QPointF(badge.left() + 4, badge.center().y()),
                               QPointF(badge.left() + 7, badge.bottom() - 4.5),
                               QPointF(badge.right() - 4, badge.top() + 4.5)});
    }
  }
  void enterEvent(QEnterEvent* event) override { QAbstractButton::enterEvent(event); update(); }
  void leaveEvent(QEvent* event) override { QAbstractButton::leaveEvent(event); update(); }

private:
  Painter m_picture;
};

// Fork: the pictures of Orbit around's cards: a cube, the orbit's ellipse
// with its arrow, and where it turns about: the origin (a dot under the
// cube), the view's middle (a dashed frame), the point under the cursor.
inline void paintOrbitChoice(QPainter& p, const QRectF& r, const QPalette& pal, OrbitCenter center) {
  p.save();
  const QColor text = pal.color(QPalette::WindowText);
  const QColor accent = pal.color(QPalette::Highlight);
  const QPointF c = r.center() + QPointF(center == OrbitCenter::Cursor ? -14 : 0, 0);
  const double s = std::min(r.width(), r.height()) / 4.2;
  QPolygonF top{c + QPointF(0, -s * 1.1), c + QPointF(s, -s * 0.55), c + QPointF(0, 0), c + QPointF(-s, -s * 0.55)};
  QColor topFill = text;
  topFill.setAlphaF(0.08f);
  QColor sideFill = text;
  sideFill.setAlphaF(0.20f);
  p.setPen(QPen(text, 1.2));
  p.setBrush(topFill);
  p.drawPolygon(top);
  p.setBrush(sideFill);
  p.drawPolygon(QPolygonF{c + QPointF(-s, -s * 0.55), c, c + QPointF(0, s * 1.0), c + QPointF(-s, s * 0.45)});
  p.setBrush(topFill);
  p.drawPolygon(QPolygonF{c, c + QPointF(s, -s * 0.55), c + QPointF(s, s * 0.45), c + QPointF(0, s * 1.0)});
  const QPointF pivot = center == OrbitCenter::Origin ? c + QPointF(0, s * 0.55)
                        : center == OrbitCenter::Model ? c + QPointF(s * 0.6, s * 0.05)
                                                       : c + QPointF(s * 2.1, -s * 0.3);
  QRectF orbit(pivot.x() - s * 2.0, pivot.y() - s * 0.55, s * 4.0, s * 1.1);
  if (center == OrbitCenter::Model) orbit.moveCenter(QPointF(pivot.x() - s * 0.3, pivot.y()));
  p.setBrush(Qt::NoBrush);
  p.setPen(QPen(accent, 2.0));
  p.drawArc(orbit, 200 * 16, 160 * 16);
  QPen dashed(accent, 1.4, Qt::DashLine);
  p.setPen(dashed);
  p.drawArc(orbit, 0, 200 * 16);
  const QPointF tip(orbit.right() - 2, orbit.center().y() - 2);
  p.setPen(QPen(accent, 2.0));
  p.drawLine(tip, tip + QPointF(-6, -4));
  p.drawLine(tip, tip + QPointF(1, -7));
  p.setPen(QPen(text, 1.6));
  p.setBrush(pal.color(QPalette::Base));
  p.drawEllipse(pivot, 4.5, 4.5);
  p.setBrush(text);
  p.drawEllipse(pivot, 1.6, 1.6);
  if (center == OrbitCenter::Model) {
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(text, 1.0, Qt::DashLine));
    p.drawRect(r.adjusted(r.width() * 0.12, 2, -r.width() * 0.12, -2));
  } else if (center == OrbitCenter::Cursor) {
    const QPointF a = pivot + QPointF(2, 2);
    p.setPen(QPen(pal.color(QPalette::Base), 1.0));
    p.setBrush(text);
    p.drawPolygon(QPolygonF{a, a + QPointF(0, 15), a + QPointF(4, 11), a + QPointF(8, 17), a + QPointF(10, 16),
                            a + QPointF(6, 10), a + QPointF(11, 10)});
  }
  p.restore();
}

// Fork: the pictures of View navigator's cards: the axes with their
// lettered ends, and the cube.
inline void paintNavigatorChoice(QPainter& p, const QRectF& r, const QPalette& pal, ViewNavigator navigator) {
  p.save();
  const QPointF c = r.center();
  const double s = std::min(r.width(), r.height()) / 2.4;
  const QColor text = pal.color(QPalette::WindowText);
  if (navigator == ViewNavigator::Axes) {
    QColor disc = text;
    disc.setAlphaF(0.06f);
    p.setPen(Qt::NoPen);
    p.setBrush(disc);
    p.drawEllipse(c, s, s);
    const struct { QPointF dir; QColor colour; const char* name; } axes[] = {
        {{0, -0.8}, QColor(0x1f4fb8), "Z"}, {{0.55, -0.35}, QColor(0x2e9e3e), "Y"}, {{0.55, 0.3}, QColor(0xc8282a), "X"}};
    for (const auto& axis : axes) {
      const QPointF end = c + axis.dir * s;
      p.setPen(QPen(axis.colour, 2.0));
      p.drawLine(c, end);
      p.setPen(QPen(axis.colour, 1.6));
      p.setBrush(pal.color(QPalette::Base));
      p.drawEllipse(c - (end - c) * 0.6, 4.5, 4.5);
      p.setPen(Qt::NoPen);
      p.setBrush(axis.colour);
      p.drawEllipse(end, 9, 9);
      QFont font = p.font();
      font.setBold(true);
      p.setFont(font);
      p.setPen(Qt::white);
      p.drawText(QRectF(end.x() - 9, end.y() - 9, 18, 18), Qt::AlignCenter, QString::fromLatin1(axis.name));
    }
  } else {
    const double h = s * 0.85;
    QColor topFill = text;
    topFill.setAlphaF(0.10f);
    QColor sideFill = text;
    sideFill.setAlphaF(0.22f);
    p.setPen(QPen(text, 1.3));
    p.setBrush(topFill);
    p.drawPolygon(QPolygonF{c + QPointF(0, -h), c + QPointF(h, -h * 0.5), c, c + QPointF(-h, -h * 0.5)});
    p.setBrush(sideFill);
    p.drawPolygon(QPolygonF{c + QPointF(-h, -h * 0.5), c, c + QPointF(0, h), c + QPointF(-h, h * 0.5)});
    p.setBrush(topFill);
    p.drawPolygon(QPolygonF{c, c + QPointF(h, -h * 0.5), c + QPointF(h, h * 0.5), c + QPointF(0, h)});
    QFont font = p.font();
    font.setPointSizeF(std::max(6.0, font.pointSizeF() * 0.7));
    p.setFont(font);
    p.setPen(text);
    p.drawText(QRectF(c.x() - h, c.y() - h, 2 * h, h * 0.9), Qt::AlignCenter, QObject::tr("TOP"));
  }
  p.restore();
}

// The bindings table: it asks for no more than its minimum height, so that
// the page's natural height stays one a low screen shows (mitcad#951), and
// takes what the page has to spare (mitcad#1394).
class NavigationBindingsTable : public QTableWidget {
public:
  static constexpr int kMinimumHeight = 138;
  using QTableWidget::QTableWidget;
  QSize sizeHint() const override { return {QTableWidget::sizeHint().width(), minimumHeight()}; }
};

class NavigationPreferences : public QWidget {
  Q_DECLARE_TR_FUNCTIONS(mitcad::NavigationPreferences)

public:
  // `buttonCommands`: the commands a 3D mouse's button can run, by id and
  // name (Fit and Home when empty).
  NavigationPreferences(const NavigationSettings& settings,
                        const QVector<QPair<NavigationScheme, QString>>& schemes, QWidget* parent = nullptr,
                        const QVector<QPair<QString, QString>>& buttonCommands = {})
      : QWidget(parent), m_settings(settings), m_buttonCommands(buttonCommands) {
    if (m_buttonCommands.isEmpty()) {
      m_buttonCommands = {{QStringLiteral("view.fit"), tr("Fit")}, {QStringLiteral("view.home"), tr("Home")}};
    }
    setObjectName(QStringLiteral("navigationPreferences"));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    // The mouse and the trackpad, and the 3D mouse (mitcad#951), on tabs
    // of their own, so that the page keeps a height a low screen shows.
    auto* tabs = new QTabWidget;
    tabs->setObjectName(QStringLiteral("navigationTabs"));
    tabs->setDocumentMode(true);
    outer->addWidget(tabs);
    // Fork: SolidCraft's page: Mouse (the mapping by program, its picture
    // and rows, the options of the buttons and their drags), Orbit around,
    // Orbit style, Wheel and trackpad, Speed and View, as sections that
    // fold (stylePreferencesPage), in a page that scrolls.
    auto* mousePage = new QWidget;
    auto* pageColumn = new QVBoxLayout(mousePage);
    pageColumn->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea;
    scroll->setObjectName(QStringLiteral("navigationScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* content = new QWidget;
    content->setAutoFillBackground(false);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 6, 8, 0);
    layout->setSpacing(4);
    scroll->setWidget(content);
    scroll->viewport()->setAutoFillBackground(false);
    pageColumn->addWidget(scroll);
    const auto section = [layout](const QString& title) {
      auto* box = new QGroupBox(title);
      auto* column = new QVBoxLayout(box);
      column->setContentsMargins(0, 4, 0, 8);
      column->setSpacing(8);
      layout->addWidget(box);
      return column;
    };
    // MOUSE: the mapping by program.
    QVBoxLayout* mouse = section(tr("Mouse"));
    auto* top = new QFormLayout;
    m_mapping = new QComboBox;
    m_mapping->setObjectName(QStringLiteral("navigationMouseMapping"));
    for (const MouseMapping& mapping : kMouseMappings) {
      m_mapping->addItem(QCoreApplication::translate("View", mapping.name), QLatin1String(mapping.id));
    }
    m_mapping->setCurrentIndex(std::max(0, m_mapping->findData(QLatin1String(mouseMappingOf(settings).id))));
    m_mapping->setToolTip(tr("The mouse buttons and keys of the program you know: pan, zoom and orbit as it does"));
    m_mapping->setMinimumWidth(uiRoomSize(260));
    // The schemes behind the mappings (the tests and older settings choose
    // a scheme): kept, hidden, following the mapping.
    m_scheme = new QComboBox;
    m_scheme->setObjectName(QStringLiteral("navigationScheme"));
    m_scheme->hide();
    m_seed = new QComboBox;
    m_seed->setObjectName(QStringLiteral("navigationStartFrom"));
    for (const auto& entry : schemes) {
      m_scheme->addItem(entry.second, int(entry.first));
      if (entry.first != NavigationScheme::Custom) m_seed->addItem(entry.second, int(entry.first));
    }
    // Every mapping's scheme in the list, even one the caller left out.
    for (const MouseMapping& mapping : kMouseMappings) {
      if (m_scheme->findData(int(mapping.scheme)) < 0) {
        m_scheme->addItem(QCoreApplication::translate("View", mapping.name), int(mapping.scheme));
        if (mapping.scheme != NavigationScheme::Custom) {
          m_seed->addItem(QCoreApplication::translate("View", mapping.name), int(mapping.scheme));
        }
      }
    }
    m_scheme->setCurrentIndex(m_scheme->findData(int(settings.scheme)));
    m_seed->setCurrentIndex(m_seed->findData(int(settings.scheme == NavigationScheme::Custom
                                                   ? NavigationScheme::MiddlePan : settings.scheme)));
    auto* mappingRow = new QHBoxLayout;
    mappingRow->addWidget(m_mapping);
    mappingRow->addWidget(m_scheme);
    mappingRow->addStretch(1);
    top->addRow(tr("Mouse mapping"), mappingRow);
    mouse->addLayout(top);
    m_guide = new NavigationGuide;
    mouse->addWidget(m_guide);
    // The bindings, each one changeable (Custom), under them.
    auto* seedRow = new QHBoxLayout;
    seedRow->addWidget(new QLabel(tr("Custom bindings, starting from")));
    seedRow->addWidget(m_seed, 1);
    auto* seed = new QPushButton(tr("Start from"));
    seed->setObjectName(QStringLiteral("navigationSeed"));
    seedRow->addWidget(seed);
    mouse->addLayout(seedRow);
    m_table = new NavigationBindingsTable;
    m_table->setObjectName(QStringLiteral("navigationBindings"));
    mac::styleListTable(m_table);
    m_table->setColumnCount(2);
    m_table->setHorizontalHeaderLabels({tr("Input"), tr("Action")});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->verticalHeader()->hide();
    m_table->setSelectionMode(QAbstractItemView::NoSelection);
    m_table->setMinimumHeight(NavigationBindingsTable::kMinimumHeight);
    m_table->setAccessibleName(tr("Navigation bindings"));
    mouse->addWidget(m_table, 1);
    auto* reserved = new QLabel(tr("Left drags used for selection and component movement are reserved. "
                                  "The orientation cube keeps its own gestures."));
    reserved->setObjectName(QStringLiteral("navigationConflicts"));
    reserved->setWordWrap(true);
    reserved->setForegroundRole(QPalette::PlaceholderText);
    mouse->addWidget(reserved);
    // The buttons' options.
    const auto check = [this](QVBoxLayout* column, const QString& title, const char* name, bool NavigationSettings::*field,
                              const QString& tip = QString()) {
      auto* box = new QCheckBox(title);
      box->setObjectName(QString::fromLatin1(name));
      if (!tip.isEmpty()) box->setToolTip(tip);
      box->setChecked(m_settings.*field);
      column->addWidget(box);
      connect(box, &QCheckBox::toggled, this, [this, field](bool value) {
        m_settings.*field = value;
        refreshGuide();
        notify();
      });
      return box;
    };
    check(mouse, tr("Touchscreen navigation"), "navigationTouchscreen", &NavigationSettings::touchscreen,
          tr("One finger orbits, two fingers pan, a pinch zooms"));
    check(mouse, tr("Right drag orbits"), "navigationRightDragOrbits", &NavigationSettings::rightDragOrbits,
          tr("A right drag orbits where the mapping gives the right button nothing to drag; a right click still opens the menu"));
    check(mouse, tr("A drag keeps the navigation it started with"), "navigationDragKeeps",
          &NavigationSettings::dragKeepsNavigation, tr("A key pressed or let go during a drag does not change it"));
    check(mouse, tr("Wrap the cursor while orbiting"), "navigationWrapCursor", &NavigationSettings::wrapCursor,
          tr("At the edge of the screen the cursor jumps to the other edge, so an orbit goes on"));
    check(mouse, tr("Orbiting stands a rolled view upright"), "navigationUprightAfterRoll",
          &NavigationSettings::uprightAfterRoll, tr("An orbit starts by turning a rolled view back upright"));
    check(mouse, tr("Mark the orbit point"), "navigationMarkOrbitPoint", &NavigationSettings::markOrbitPoint,
          tr("A dot where the orbit turns about while it turns"));
    check(mouse, tr("Radial context menu"), "navigationRadialMenu", &NavigationSettings::radialContextMenu,
          tr("The right click's menu as a ring of commands around the cursor"));
    check(mouse, tr("Quick menu after a selection"), "navigationQuickMenu", &NavigationSettings::quickMenuAfterSelection,
          tr("A few commands for what is selected beside the selection"));
    auto* buttons = new QFormLayout;
    buttons->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    auto* doubleClick = new QComboBox;
    doubleClick->setObjectName(QStringLiteral("navigationMiddleDoubleClick"));
    doubleClick->addItem(tr("Nothing"), int(MiddleDoubleClick::Nothing));
    doubleClick->addItem(tr("Home"), int(MiddleDoubleClick::Home));
    doubleClick->addItem(tr("Fit"), int(MiddleDoubleClick::Fit));
    doubleClick->setCurrentIndex(doubleClick->findData(int(m_settings.middleDoubleClick)));
    doubleClick->setMinimumWidth(uiRoomSize(200));
    connect(doubleClick, &QComboBox::currentIndexChanged, this, [this, doubleClick] {
      m_settings.middleDoubleClick = MiddleDoubleClick(doubleClick->currentData().toInt());
      notify();
    });
    buttons->addRow(tr("Middle double-click"), doubleClick);
    auto* dragsNote = new QLabel(tr("What dragging each mouse button does. A side button still goes Back when you click it."));
    dragsNote->setWordWrap(true);
    buttons->addRow(dragsNote);
    const auto drag = [this, buttons](const QString& title, const char* name, DragOverride NavigationSettings::*field,
                                      bool preset) {
      auto* box = new QComboBox;
      box->setObjectName(QString::fromLatin1(name));
      if (preset) box->addItem(tr("Preset"), int(DragOverride::Preset));
      box->addItem(tr("Nothing"), int(DragOverride::Nothing));
      box->addItem(tr("Orbit"), int(DragOverride::Orbit));
      box->addItem(tr("Pan"), int(DragOverride::Pan));
      box->addItem(tr("Zoom"), int(DragOverride::Zoom));
      box->setCurrentIndex(std::max(0, box->findData(int(m_settings.*field))));
      box->setMinimumWidth(uiRoomSize(200));
      connect(box, &QComboBox::currentIndexChanged, this, [this, box, field] {
        m_settings.*field = DragOverride(box->currentData().toInt());
        refreshGuide();
        notify();
      });
      buttons->addRow(title, box);
      m_dragBoxes.append(box);
      return box;
    };
    // The left button's drag without keys selects: Preset or Nothing only.
    auto* left = drag(tr("Left drag"), "navigationLeftDrag", &NavigationSettings::leftDrag, true);
    for (int i = left->count() - 1; i >= 0; --i) {
      const auto value = DragOverride(left->itemData(i).toInt());
      if (value != DragOverride::Preset && value != DragOverride::Nothing) left->removeItem(i);
    }
    left->setToolTip(tr("A left drag selects with a window; Nothing turns the window selection off"));
    drag(tr("Middle drag"), "navigationMiddleDrag", &NavigationSettings::middleDrag, true);
    drag(tr("Right drag"), "navigationRightDrag", &NavigationSettings::rightDrag, true);
    drag(tr("Side button (back) drag"), "navigationBackDrag", &NavigationSettings::backDrag, false);
    drag(tr("Side button (forward) drag"), "navigationForwardDrag", &NavigationSettings::forwardDrag, false);
    auto* resetDrags = new QPushButton(tr("Reset to the preset"));
    resetDrags->setObjectName(QStringLiteral("navigationResetDrags"));
    connect(resetDrags, &QPushButton::clicked, this, [this] {
      for (QComboBox* box : m_dragBoxes) {
        const int preset = box->findData(int(DragOverride::Preset));
        box->setCurrentIndex(preset >= 0 ? preset : box->findData(int(DragOverride::Nothing)));
      }
    });
    buttons->addRow(QString(), resetDrags);
    // The ring of tools (SolidWorks's mouse gestures).
    auto* ringRow = new QHBoxLayout;
    auto* ring = new QCheckBox(tr("Ring of tools on the right button"));
    ring->setObjectName(QStringLiteral("navigationGestureRing"));
    ring->setChecked(m_settings.gestureRing);
    connect(ring, &QCheckBox::toggled, this, [this](bool on) {
      m_settings.gestureRing = on;
      notify();
    });
    ringRow->addWidget(ring);
    auto* customize = new QPushButton(tr("Customize…"));
    customize->setObjectName(QStringLiteral("navigationGestureCustomize"));
    connect(customize, &QPushButton::clicked, this, [this] { customizeGestureRing(); });
    ringRow->addWidget(customize);
    ringRow->addStretch(1);
    buttons->addRow(tr("Mouse gestures"), ringRow);
    m_gestureNote = new QLabel(tr("Mouse gestures only work with the SolidWorks mouse mapping (the middle button "
                                  "orbits, Ctrl + middle pans), where a right drag opens the ring."));
    m_gestureNote->setWordWrap(true);
    m_gestureNote->setForegroundRole(QPalette::PlaceholderText);
    buttons->addRow(QString(), m_gestureNote);
    m_useSolidWorks = new QPushButton(tr("Use the SolidWorks mouse mapping"));
    m_useSolidWorks->setObjectName(QStringLiteral("navigationUseSolidWorks"));
    connect(m_useSolidWorks, &QPushButton::clicked, this, [this] {
      m_mapping->setCurrentIndex(m_mapping->findData(QStringLiteral("solidworks")));
    });
    buttons->addRow(QString(), m_useSolidWorks);
    mouse->addLayout(buttons);
    // ORBIT AROUND: the three cards.
    QVBoxLayout* around = section(tr("Orbit around"));
    auto* cards = new QHBoxLayout;
    cards->setSpacing(10);
    const struct { OrbitCenter center; QString name; const char* object; } centers[] = {
        {OrbitCenter::Origin, tr("Origin"), "navigationOrbitOrigin"},
        {OrbitCenter::Model, tr("View center"), "navigationOrbitViewCenter"},
        {OrbitCenter::Cursor, tr("Under cursor"), "navigationOrbitUnderCursor"}};
    for (const auto& entry : centers) {
      const OrbitCenter center = entry.center;
      auto* card = new NavigationChoiceCard(
          entry.name, [center](QPainter& p, const QRectF& r, const QPalette& pal) { paintOrbitChoice(p, r, pal, center); });
      card->setObjectName(QString::fromLatin1(entry.object));
      card->setChecked(m_settings.orbitCenter == center);
      connect(card, &QAbstractButton::toggled, this, [this, center](bool on) {
        if (!on) return;
        m_settings.orbitCenter = center;
        if (m_orbitCenter != nullptr) {
          const QSignalBlocker blocker(m_orbitCenter);
          m_orbitCenter->setCurrentIndex(m_orbitCenter->findData(int(center)));
        }
        notify();
      });
      cards->addWidget(card);
    }
    around->addLayout(cards);
    // The same choice as a list, hidden (the UI tests and search find it).
    m_orbitCenter = new QComboBox;
    m_orbitCenter->setObjectName(QStringLiteral("navigationOrbitCenter"));
    m_orbitCenter->addItem(tr("View center"), int(OrbitCenter::Model));
    m_orbitCenter->addItem(tr("Under cursor"), int(OrbitCenter::Cursor));
    m_orbitCenter->addItem(tr("Origin"), int(OrbitCenter::Origin));
    m_orbitCenter->setCurrentIndex(m_orbitCenter->findData(int(m_settings.orbitCenter)));
    m_orbitCenter->hide();
    around->addWidget(m_orbitCenter);
    connect(m_orbitCenter, &QComboBox::currentIndexChanged, this, [this] {
      m_settings.orbitCenter = static_cast<OrbitCenter>(m_orbitCenter->currentData().toInt());
      for (auto* card : findChildren<NavigationChoiceCard*>()) {
        if (card->objectName() == QLatin1String(m_settings.orbitCenter == OrbitCenter::Origin ? "navigationOrbitOrigin"
                                                 : m_settings.orbitCenter == OrbitCenter::Cursor ? "navigationOrbitUnderCursor"
                                                                                                 : "navigationOrbitViewCenter")) {
          const QSignalBlocker blocker(card);
          card->setChecked(true);
        }
      }
      notify();
    });
    // ORBIT STYLE: Turntable or Free.
    QVBoxLayout* style = section(tr("Orbit style"));
    auto* styleForm = new QFormLayout;
    styleForm->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    auto* segments = new QHBoxLayout;
    segments->setSpacing(0);
    auto* turntable = new QPushButton(tr("Turntable"));
    auto* free = new QPushButton(tr("Free"));
    turntable->setObjectName(QStringLiteral("navigationOrbitTurntable"));
    free->setObjectName(QStringLiteral("navigationOrbitFree"));
    const QPalette pal = palette();
    const QString segmentStyle =
        QStringLiteral("QPushButton { border: 1px solid %1; padding: 4px 12px; background: %2; } "
                       "QPushButton:checked { background: %3; font-weight: 600; }")
            .arg(pal.color(QPalette::Mid).name(), pal.color(QPalette::Base).name(),
                 pal.color(QPalette::Highlight).lighter(185).name());
    turntable->setStyleSheet(segmentStyle + QStringLiteral(" QPushButton { border-top-left-radius: 5px; "
                                                           "border-bottom-left-radius: 5px; }"));
    free->setStyleSheet(segmentStyle + QStringLiteral(" QPushButton { border-left: none; border-top-right-radius: 5px; "
                                                      "border-bottom-right-radius: 5px; }"));
    for (auto* segment : {turntable, free}) {
      segment->setCheckable(true);
      segment->setAutoExclusive(true);
      segments->addWidget(segment);
    }
    segments->addStretch(1);
    turntable->setChecked(m_settings.constrainedOrbit);
    free->setChecked(!m_settings.constrainedOrbit);
    turntable->setToolTip(tr("Turntable keeps the vertical axis upright; Free turns the design any way"));
    styleForm->addRow(tr("Orbit"), segments);
    auto* vertical = new QCheckBox(tr("Turn around the vertical axis"));
    vertical->setObjectName(QStringLiteral("navigationTurnAroundVertical"));
    vertical->setChecked(m_settings.turnAroundVertical);
    vertical->setEnabled(m_settings.constrainedOrbit);
    vertical->setToolTip(tr("A sideways turntable orbit turns about the design's vertical axis, not the screen's"));
    connect(vertical, &QCheckBox::toggled, this, [this](bool on) {
      m_settings.turnAroundVertical = on;
      notify();
    });
    styleForm->addRow(QString(), vertical);
    connect(turntable, &QPushButton::toggled, this, [this, vertical](bool on) {
      m_settings.constrainedOrbit = on;
      vertical->setEnabled(on);
      if (auto* box = findChild<QCheckBox*>(QStringLiteral("navigationConstrainedOrbit"))) {
        const QSignalBlocker blocker(box);
        box->setChecked(on);
      }
      refreshGuide();
      notify();
    });
    const auto keyBox = [this](const char* name, HeldKey NavigationSettings::*field, const QString& none) {
      auto* box = new QComboBox;
      box->setObjectName(QString::fromLatin1(name));
      box->addItem(none, int(HeldKey::None));
      box->addItem(QStringLiteral("Shift"), int(HeldKey::Shift));
#ifdef Q_OS_MACOS
      box->addItem(QStringLiteral("Command"), int(HeldKey::Ctrl));
      box->addItem(QStringLiteral("Option"), int(HeldKey::Alt));
#else
      box->addItem(QStringLiteral("Ctrl"), int(HeldKey::Ctrl));
      box->addItem(QStringLiteral("Alt"), int(HeldKey::Alt));
#endif
      box->setCurrentIndex(std::max(0, box->findData(int(m_settings.*field))));
      connect(box, &QComboBox::currentIndexChanged, this, [this, box, field] {
        m_settings.*field = HeldKey(box->currentData().toInt());
        notify();
      });
      return box;
    };
    auto* freeKey = keyBox("navigationFreeOrbitKey", &NavigationSettings::freeOrbitKey, tr("No key"));
    freeKey->setToolTip(tr("Held during a turntable orbit, the orbit turns freely"));
    styleForm->addRow(tr("Free orbit while holding"), freeKey);
    style->addLayout(styleForm);
    // The old check box, hidden, following the segments (UI tests).
    auto* constrained = new QCheckBox(tr("Constrained orbit (Z stays up)"));
    constrained->setObjectName(QStringLiteral("navigationConstrainedOrbit"));
    constrained->setChecked(m_settings.constrainedOrbit);
    constrained->hide();
    style->addWidget(constrained);
    connect(constrained, &QCheckBox::toggled, this, [turntable, free](bool on) { (on ? turntable : free)->setChecked(true); });
    // WHEEL AND TRACKPAD.
    QVBoxLayout* wheel = section(tr("Wheel and trackpad"));
    auto* flags = new QGridLayout;
    flags->setHorizontalSpacing(24);
    addFlag(flags, 0, 0, tr("Zoom to the cursor"), "navigationZoomToCursor", &NavigationSettings::zoomToCursor);
    addFlag(flags, 1, 0, tr("Reverse zoom direction"), "preferencesReverseZoom", &NavigationSettings::reverseZoom);
    addFlag(flags, 2, 0, tr("Reverse horizontal pan direction"), "navigationReversePanHorizontal",
            &NavigationSettings::reversePanHorizontal,
            tr("A two-finger trackpad scroll pans the view left and right the other way"));
    addFlag(flags, 3, 0, tr("Reverse vertical pan direction"), "navigationReversePanVertical",
            &NavigationSettings::reversePanVertical,
            tr("A two-finger trackpad scroll pans the view up and down the other way"));
    addFlag(flags, 0, 1, tr("Smooth wheel zoom"), "navigationSmoothWheelZoom", &NavigationSettings::smoothWheelZoom,
            tr("Each wheel step zooms in a short glide instead of a jump"));
    addFlag(flags, 1, 1, tr("Two-finger scroll pans"), "navigationTwoFingerPans", &NavigationSettings::twoFingerScrollPans,
            tr("A two-finger trackpad scroll pans the view; off, it zooms"));
    addFlag(flags, 2, 1, tr("Reverse trackpad orbit direction"), "navigationReverseScrollOrbit",
            &NavigationSettings::reverseScrollOrbit, tr("Shift with two-finger scrolling orbits the other way"));
    addFlag(flags, 3, 1, tr("Stop zoom at the model"), "navigationStopZoomAtModel", &NavigationSettings::stopZoomAtModel,
            tr("Zooming to the cursor stops at the surface under it instead of passing through"));
    wheel->addLayout(flags);
    auto* wheelForm = new QFormLayout;
    wheelForm->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    wheelForm->addRow(tr("Zoom with two-finger scroll while holding"),
                      keyBox("navigationScrollZoomKey", &NavigationSettings::scrollZoomKey, tr("Off")));
    wheel->addLayout(wheelForm);
    // SPEED.
    QVBoxLayout* speedColumn = section(tr("Speed"));
    auto* speeds = new QHBoxLayout;
    auto* mouseSpeeds = new QFormLayout;
    auto* scrollSpeeds = new QFormLayout;
    addSpeed(mouseSpeeds, tr("Orbit speed:"), "navigationOrbitSpeed", &NavigationSettings::orbitSpeed);
    addSpeed(mouseSpeeds, tr("Pan speed:"), "navigationPanSpeed", &NavigationSettings::panSpeed);
    addSpeed(scrollSpeeds, tr("Zoom speed:"), "navigationZoomSpeed", &NavigationSettings::zoomSpeed);
    addSpeed(scrollSpeeds, tr("Trackpad scroll speed:"), "navigationTrackpadSpeed", &NavigationSettings::trackpadSpeed);
    speeds->addLayout(mouseSpeeds);
    speeds->addLayout(scrollSpeeds);
    speedColumn->addLayout(speeds);
    // VIEW.
    QVBoxLayout* view = section(tr("View"));
    view->addWidget(new QLabel(tr("View navigator")));
    auto* navigatorCards = new QHBoxLayout;
    navigatorCards->setSpacing(10);
    for (const auto& [navigator, name, object] :
         {std::tuple{ViewNavigator::Axes, tr("Axes"), "navigationNavigatorAxes"},
          std::tuple{ViewNavigator::Cube, tr("Cube"), "navigationNavigatorCube"}}) {
      const ViewNavigator chosenNavigator = navigator;
      auto* card = new NavigationChoiceCard(name, [chosenNavigator](QPainter& p, const QRectF& r, const QPalette& colors) {
        paintNavigatorChoice(p, r, colors, chosenNavigator);
      });
      card->setObjectName(QString::fromLatin1(object));
      card->setChecked(m_settings.viewNavigator == navigator);
      card->setMaximumWidth(uiRoomSize(200));
      connect(card, &QAbstractButton::toggled, this, [this, chosenNavigator](bool on) {
        if (!on) return;
        m_settings.viewNavigator = chosenNavigator;
        notify();
      });
      navigatorCards->addWidget(card);
    }
    navigatorCards->addStretch(1);
    view->addLayout(navigatorCards);
    check(view, tr("Animate view changes"), "navigationAnimate", &NavigationSettings::animate);
    auto* viewForm = new QFormLayout;
    const auto slider = [this, viewForm](const QString& title, const char* name, int low, int high, int value,
                                         std::function<QString(int)> shown, std::function<void(int)> apply) {
      auto* row = new QHBoxLayout;
      auto* bar = new QSlider(Qt::Horizontal);
      bar->setObjectName(QString::fromLatin1(name));
      bar->setRange(low, high);
      bar->setValue(value);
      auto* label = new QLabel(shown(value));
      label->setMinimumWidth(uiRoomSize(56));
      label->setAlignment(Qt::AlignCenter);
      label->setFrameShape(QFrame::StyledPanel);
      row->addWidget(bar, 1);
      row->addWidget(label);
      connect(bar, &QSlider::valueChanged, this, [this, label, shown, apply](int v) {
        label->setText(shown(v));
        apply(v);
        notify();
      });
      viewForm->addRow(title, row);
    };
    slider(tr("Animation speed"), "navigationAnimationSpeed", 25, 400, int(std::lround(m_settings.animationSpeed * 100)),
           [](int v) { return QStringLiteral("%1×").arg(v / 100.0, 0, 'f', 2); },
           [this](int v) { m_settings.animationSpeed = v / 100.0; });
    slider(tr("Home and Fit size"), "navigationHomeFitSize", 50, 100, m_settings.homeFitSize,
           [](int v) { return QStringLiteral("%1 %").arg(v); }, [this](int v) { m_settings.homeFitSize = v; });
    // How far the arrow keys turn the view while it has the keyboard
    // (mitcad#1211).
    auto* turnStep = new QSpinBox;
    turnStep->setObjectName(QStringLiteral("navigationTurnStep"));
    turnStep->setRange(1, 90);
    turnStep->setSuffix(QStringLiteral("°"));
    turnStep->setValue(int(std::lround(sanitizeTurnStep(m_settings.turnStep))));
    turnStep->setToolTip(tr("How far an arrow key turns the 3D view"));
    turnStep->setMaximumWidth(uiRoomSize(90));
    connect(turnStep, &QSpinBox::valueChanged, this, [this](int degrees) {
      m_settings.turnStep = degrees;
      notify();
    });
    viewForm->addRow(tr("Arrow key step"), turnStep);
    view->addLayout(viewForm);
    check(view, tr("Fit the model after a navigator view change"), "navigationFitAfterNavigator",
          &NavigationSettings::fitAfterNavigatorChange);
    check(view, tr("Show the Turn Around button on the navigator"), "navigationShowTurnAround",
          &NavigationSettings::showTurnAround);
    check(view, tr("Keep the highlight while the view moves"), "navigationKeepHighlight",
          &NavigationSettings::keepHighlightWhileMoving);
    check(view, tr("Invert the navigator rotation arrows"), "navigationInvertNavigatorArrows",
          &NavigationSettings::invertNavigatorArrows);
    check(view, tr("Invert the curved roll arrows"), "navigationInvertRollArrows", &NavigationSettings::invertRollArrows);
    check(view, tr("Arrow keys move through the Browser while it has focus"), "navigationArrowKeysBrowser",
          &NavigationSettings::arrowKeysBrowser);
    m_hint = new QLabel(tr("The wheel zooms; pinch zooms, rotation rolls, and a two-finger double tap fits."));
    m_hint->setObjectName(QStringLiteral("navigationGuideExplanation"));
    m_hint->setWordWrap(true);
    m_hint->setForegroundRole(QPalette::PlaceholderText);
    layout->addWidget(m_hint);
    layout->addStretch(1);
    // The mapping chooses the scheme; a scheme chosen otherwise (tests,
    // Custom by an edit) shows its first mapping.
    connect(m_mapping, &QComboBox::currentIndexChanged, this, [this] {
      const QString id = m_mapping->currentData().toString();
      for (const MouseMapping& mapping : kMouseMappings) {
        if (id == QLatin1String(mapping.id)) {
          m_settings.mouseMapping = id;
          const int index = m_scheme->findData(int(mapping.scheme));
          if (index == m_scheme->currentIndex()) {
            refreshGuide();
            notify();
          } else {
            m_scheme->setCurrentIndex(index);
          }
        }
      }
    });
    tabs->addTab(mousePage, tr("Mouse and Trackpad"));
    auto* spacePage = new QWidget;
    auto* spaceLayout = new QVBoxLayout(spacePage);
    spaceLayout->setContentsMargins(0, 6, 0, 0);
    addSpaceMouse(spaceLayout);
    spaceLayout->addStretch(1);
    tabs->addTab(spacePage, tr("3D Mouse"));
    connect(m_scheme, &QComboBox::currentIndexChanged, this, [this] {
      m_settings.scheme = static_cast<NavigationScheme>(m_scheme->currentData().toInt());
      if (mouseMappingOf(m_settings).scheme != m_settings.scheme ||
          m_settings.mouseMapping != QLatin1String(mouseMappingOf(m_settings).id)) {
        m_settings.mouseMapping = QLatin1String(mouseMappingOf(m_settings).id);
      }
      {
        const QSignalBlocker blocker(m_mapping);
        m_mapping->setCurrentIndex(std::max(0, m_mapping->findData(QLatin1String(mouseMappingOf(m_settings).id))));
      }
      if (m_settings.scheme == NavigationScheme::Custom && m_settings.customBindings.isEmpty())
        m_settings.customBindings = navigationBindings(static_cast<NavigationScheme>(m_seed->currentData().toInt()));
      if (m_settings.scheme != NavigationScheme::Custom) {
        const QSignalBlocker blocker(m_seed);
        m_seed->setCurrentIndex(m_seed->findData(int(m_settings.scheme)));
      }
      rebuildBindings();
      notify();
    });
    connect(seed, &QPushButton::clicked, this, [this] {
      m_settings.customBindings = navigationBindings(static_cast<NavigationScheme>(m_seed->currentData().toInt()));
      chooseCustom();
      rebuildBindings();
      notify();
    });
    rebuildBindings();
  }
  NavigationSettings chosen() const { return m_settings; }
  std::function<void()> changed;
  // The 3D mouse's Lock rotation and buttons as changed outside the page
  // (mitcad#1910: Lock Rotation's command, a device's default button): the
  // check box and the button choices show them, without a change of the
  // page's own.
  void followSpaceMouse(const SpaceMouseSettings& live) {
    m_settings.spaceMouse.lockRotation = live.lockRotation;
    m_settings.spaceMouse.buttons = live.buttons;
    if (auto* lock = findChild<QCheckBox*>(QStringLiteral("navigationSpaceMouseLockRotation"))) {
      const QSignalBlocker blocker(lock);
      lock->setChecked(live.lockRotation);
    }
    for (int row = 0; row < 32; ++row) {
      auto* command = findChild<QComboBox*>(QStringLiteral("navigationSpaceMouseButton%1").arg(row));
      if (command == nullptr) {
        break;
      }
      const QSignalBlocker blocker(command);
      const QString bound = live.buttons.value(row);
      if (!bound.isEmpty() && command->findData(bound) < 0) {
        command->addItem(bound, bound); // as the page keeps a command its list does not offer
      }
      command->setCurrentIndex(std::max(0, command->findData(bound)));
    }
  }
protected:
  void showEvent(QShowEvent* event) override {
    QWidget::showEvent(event);
    // The table's real viewport geometry is available after the page is
    // shown, including when Settings switches to this previously hidden pane.
    QTimer::singleShot(0, m_table, [this] { revealFirstBinding(); });
    logGeometry();
  }
  void resizeEvent(QResizeEvent* event) override {
    QWidget::resizeEvent(event);
    logGeometry();
  }
private:
  // Once the layout has settled after a show or a resize (UI tests,
  // mitcad#1394): the bindings table's height and how far the page's last
  // line (the hint) ends above the bottom of the area its window gives the
  // page, with a picture when the tests ask for one.
  void logGeometry() {
    if (m_geometryLogPending) return;
    m_geometryLogPending = true;
    TestSync::singleShot(0, this, [this] {
      m_geometryLogPending = false;
      if (!isVisible() || !m_hint->isVisible()) return;
      logPreferencesPageLayout(this, QStringLiteral("Navigation page: bindings table"), m_table, m_hint,
                               QStringLiteral("navigation"));
    });
  }
  void revealFirstBinding() {
    int firstEditable = -1;
    for (int row = 0; row < m_bindings.size(); ++row) {
      const auto& binding = m_bindings[row];
      if (!navigationBindingAllowed(binding)) continue;
      if (firstEditable < 0) firstEditable = row;
      if (binding.action != NavigationAction::None) {
        m_table->scrollToItem(m_table->item(row, 0), QAbstractItemView::PositionAtTop);
        return;
      }
    }
    if (firstEditable >= 0)
      m_table->scrollToItem(m_table->item(firstEditable, 0), QAbstractItemView::PositionAtTop);
  }
  void notify() {
    m_guide->setNavigation(m_settings);
    if (changed) changed();
  }
  void chooseCustom() {
    m_settings.scheme = NavigationScheme::Custom;
    const QSignalBlocker blocker(m_scheme);
    m_scheme->setCurrentIndex(m_scheme->findData(int(NavigationScheme::Custom)));
  }
  void rebuildBindings() {
    m_bindings = m_settings.scheme == NavigationScheme::Custom
                     ? m_settings.customBindings : navigationBindings(m_settings.scheme);
    m_table->setRowCount(0);
    m_table->setRowCount(m_bindings.size());
    for (int row = 0; row < m_bindings.size(); ++row) {
      const auto binding = m_bindings.at(row);
      const bool allowed = navigationBindingAllowed(binding);
      auto* label = new QTableWidgetItem(navigationInputText(binding));
      label->setFlags(Qt::ItemIsEnabled);
      m_table->setItem(row, 0, label);
      auto* action = new QComboBox;
      action->setObjectName(QStringLiteral("navigationBinding%1").arg(row));
      for (const auto value : {NavigationAction::None, NavigationAction::Orbit, NavigationAction::Pan, NavigationAction::Zoom})
        action->addItem(navigationActionText(value), int(value));
      action->setCurrentIndex(action->findData(int(binding.action)));
      action->setEnabled(allowed);
      if (!allowed) {
        label->setText(label->text() + tr(" (reserved)"));
        label->setForeground(palette().color(QPalette::Disabled, QPalette::Text));
        action->setToolTip(tr("Reserved for selection and component movement"));
      }
      m_table->setCellWidget(row, 1, action);
      connect(action, &QComboBox::currentIndexChanged, this, [this, row, action] {
        if (m_settings.scheme != NavigationScheme::Custom) m_settings.customBindings = m_bindings;
        chooseCustom();
        m_settings.customBindings[row].action = static_cast<NavigationAction>(action->currentData().toInt());
        m_bindings = m_settings.customBindings;
        notify();
      });
    }
    m_table->resizeRowsToContents();
    refreshGuide();
    QTimer::singleShot(0, m_table, [this] { revealFirstBinding(); });
  }
  // Fork: the guide follows the settings; the gesture ring's note and its
  // button show while the mapping is not SolidWorks's.
  void refreshGuide() {
    m_guide->setNavigation(m_settings);
    const bool solidWorks = m_settings.scheme == NavigationScheme::MiddleOrbit;
    if (m_gestureNote != nullptr) m_gestureNote->setVisible(!solidWorks);
    if (m_useSolidWorks != nullptr) m_useSolidWorks->setVisible(!solidWorks);
  }
  // Fork: the gesture ring's eight tools, clockwise from the top.
  void customizeGestureRing() {
    QDialog dialog(this);
    dialog.setWindowTitle(tr("Ring of Tools"));
    auto* form = new QFormLayout(&dialog);
    QVector<QPair<QString, QString>> commands = m_buttonCommands;
    for (const auto& extra : {QPair<QString, QString>{QStringLiteral("sketch.create"), tr("Create Sketch")},
                              QPair<QString, QString>{QStringLiteral("inspect.measure"), tr("Measure")},
                              QPair<QString, QString>{QStringLiteral("solid.extrude"), tr("Extrude")},
                              QPair<QString, QString>{QStringLiteral("solid.fillet"), tr("Fillet")}}) {
      bool known = false;
      for (const auto& command : commands) known = known || command.first == extra.first;
      if (!known) commands.append(extra);
    }
    const QString places[8] = {tr("Top"), tr("Top right"), tr("Right"), tr("Bottom right"),
                               tr("Bottom"), tr("Bottom left"), tr("Left"), tr("Top left")};
    QVector<QComboBox*> boxes;
    QStringList current = m_settings.gestureCommands;
    if (current.size() != 8) current = defaultGestureCommands();
    for (int i = 0; i < 8; ++i) {
      auto* box = new QComboBox;
      box->setObjectName(QStringLiteral("navigationGestureTool%1").arg(i));
      box->addItem(tr("Nothing"), QString());
      for (const auto& command : commands) box->addItem(command.second, command.first);
      if (box->findData(current.at(i)) < 0 && !current.at(i).isEmpty()) box->addItem(current.at(i), current.at(i));
      box->setCurrentIndex(std::max(0, box->findData(current.at(i))));
      form->addRow(places[i], box);
      boxes.append(box);
    }
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::RestoreDefaults);
    form->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons->button(QDialogButtonBox::RestoreDefaults), &QPushButton::clicked, &dialog, [&boxes] {
      const QStringList defaults = defaultGestureCommands();
      for (int i = 0; i < boxes.size(); ++i) boxes[i]->setCurrentIndex(std::max(0, boxes[i]->findData(defaults.at(i))));
    });
    if (dialog.exec() != QDialog::Accepted) return;
    QStringList chosen;
    for (QComboBox* box : boxes) chosen << box->currentData().toString();
    m_settings.gestureCommands = chosen;
    notify();
  }
  void addSpeed(QFormLayout* form, const QString& title, const char* name, double NavigationSettings::*field) {
    auto* row = new QHBoxLayout;
    auto* slider = new QSlider(Qt::Horizontal);
    slider->setObjectName(QString::fromLatin1(name));
    slider->setMinimumWidth(70);
    slider->setRange(10, 400);
    slider->setSingleStep(10);
    slider->setPageStep(25);
    slider->setValue(int(std::lround(sanitizeNavigationSpeed(m_settings.*field) * 100)));
    auto* value = new QLabel(QStringLiteral("%1×").arg(slider->value() / 100.0, 0, 'f', 2));
    value->setMinimumWidth(48);
    auto* reset = new QPushButton(tr("Reset"));
    reset->setObjectName(QString::fromLatin1(name) + QStringLiteral("Reset"));
    reset->setToolTip(tr("Restores the default navigation speed"));
    row->addWidget(slider, 1);
    row->addWidget(value);
    row->addWidget(reset);
    form->addRow(title, row);
    connect(slider, &QSlider::valueChanged, this, [this, value, field](int position) {
      m_settings.*field = position / 100.0;
      value->setText(QStringLiteral("%1×").arg(m_settings.*field, 0, 'f', 2));
      notify();
    });
    connect(reset, &QPushButton::clicked, slider, [slider] { slider->setValue(100); });
  }
  void addFlag(QGridLayout* grid, int row, int column, const QString& title, const char* name, bool NavigationSettings::*field,
               const QString& tip = QString()) {
    auto* box = new QCheckBox(title);
    box->setObjectName(QString::fromLatin1(name));
    if (!tip.isEmpty()) box->setToolTip(tip);
    box->setChecked(m_settings.*field);
    grid->addWidget(box, row, column);
    connect(box, &QCheckBox::toggled, this, [this, field](bool value) { m_settings.*field = value; notify(); });
  }
  // The 3D mouse's tab (mitcad#951): what is connected, a hint while the
  // vendor's driver runs (mitcad#1981), on or off, its speed, its Navigation
  // mode, each axis on and reversed, the locks, the dominant axis, its
  // Lock horizon (mitcad#978), the cross-talk filter (mitcad#1720) and its
  // buttons' commands.
  void addSpaceMouse(QVBoxLayout* layout) {
    auto* group = new QWidget;
    group->setObjectName(QStringLiteral("navigationSpaceMouse"));
    auto* box = new QVBoxLayout(group);
    box->setContentsMargins(0, 0, 0, 0);
    const QString status = spaceMouseStatus().text;
    auto* found = new QLabel(status.isEmpty() ? tr("No 3D mouse found") : status);
    found->setObjectName(QStringLiteral("navigationSpaceMouseStatus"));
    found->setWordWrap(true);
    box->addWidget(found);
    // The vendor's driver can send wheel steps for the cap's Tilt, which
    // zoom the view (mitcad#1981); the view ignores the wheel during a
    // motion, and the line says how to stop it at the source.
    if (spaceMouseStatus().vendorDriver) {
      auto* driver = new QLabel(tr("The 3D mouse driver is running: in its settings for MitCad, turn off Tilt or "
                                   "set it to send nothing, so it does not zoom the view."));
      driver->setObjectName(QStringLiteral("navigationSpaceMouseDriverHint"));
      driver->setWordWrap(true);
      box->addWidget(driver);
    }
    auto* on = new QCheckBox(tr("Use a 3D mouse"));
    on->setObjectName(QStringLiteral("navigationSpaceMouseOn"));
    on->setChecked(m_settings.spaceMouse.enabled);
    box->addWidget(on);
    connect(on, &QCheckBox::toggled, this, [this](bool value) {
      m_settings.spaceMouse.enabled = value;
      notify();
    });
    auto* speedForm = new QFormLayout;
    auto* speedRow = new QHBoxLayout;
    auto* slider = new QSlider(Qt::Horizontal);
    slider->setObjectName(QStringLiteral("navigationSpaceMouseSpeed"));
    slider->setMinimumWidth(70);
    slider->setRange(10, 400);
    slider->setSingleStep(10);
    slider->setPageStep(25);
    slider->setValue(int(std::lround(sanitizeSpaceMouseSpeed(m_settings.spaceMouse.speed) * 100)));
    auto* speedValue = new QLabel(QStringLiteral("%1×").arg(slider->value() / 100.0, 0, 'f', 2));
    speedValue->setMinimumWidth(48);
    auto* speedReset = new QPushButton(tr("Reset"));
    speedReset->setObjectName(QStringLiteral("navigationSpaceMouseSpeedReset"));
    speedReset->setToolTip(tr("Restores the default 3D mouse speed"));
    speedRow->addWidget(slider, 1);
    speedRow->addWidget(speedValue);
    speedRow->addWidget(speedReset);
    speedForm->addRow(tr("3D mouse speed:"), speedRow);
    // What follows the cap (mitcad#978).
    auto* mode = new QComboBox;
    mode->setObjectName(QStringLiteral("navigationSpaceMouseMode"));
    mode->addItem(tr("Object"), int(SpaceMouseMode::Object));
    mode->addItem(tr("Camera"), int(SpaceMouseMode::Camera));
    mode->addItem(tr("Fly"), int(SpaceMouseMode::Fly));
    mode->setCurrentIndex(std::max(0, mode->findData(int(m_settings.spaceMouse.mode))));
    mode->setToolTip(tr("What follows the cap: the model (Object) or the camera (Camera, Fly)"));
    speedForm->addRow(tr("Navigation mode:"), mode);
    box->addLayout(speedForm);
    connect(mode, &QComboBox::currentIndexChanged, this, [this, mode] {
      m_settings.spaceMouse.mode = static_cast<SpaceMouseMode>(mode->currentData().toInt());
      notify();
    });
    connect(slider, &QSlider::valueChanged, this, [this, speedValue](int position) {
      m_settings.spaceMouse.speed = position / 100.0;
      speedValue->setText(QStringLiteral("%1×").arg(m_settings.spaceMouse.speed, 0, 'f', 2));
      notify();
    });
    connect(speedReset, &QPushButton::clicked, slider, [slider] { slider->setValue(100); });
    // One row per axis: whether it moves the view, and its direction.
    auto* axes = new QGridLayout;
    axes->addWidget(new QLabel(tr("Axis")), 0, 0);
    axes->addWidget(new QLabel(tr("On")), 0, 1);
    axes->addWidget(new QLabel(tr("Reverse")), 0, 2);
    const QString axisNames[6] = {tr("Pan left and right"), tr("Pan up and down"), tr("Zoom (pull and push)"),
                                  tr("Tilt forward and back"), tr("Spin"), tr("Roll sideways")};
    for (int axis = 0; axis < 6; ++axis) {
      const auto index = static_cast<std::size_t>(axis);
      axes->addWidget(new QLabel(axisNames[axis]), axis + 1, 0);
      auto* axisOn = new QCheckBox;
      axisOn->setObjectName(QStringLiteral("navigationSpaceMouseAxis%1").arg(axis));
      axisOn->setAccessibleName(tr("%1 on").arg(axisNames[axis]));
      axisOn->setChecked(m_settings.spaceMouse.axisOn[index]);
      axes->addWidget(axisOn, axis + 1, 1);
      auto* reverse = new QCheckBox;
      reverse->setObjectName(QStringLiteral("navigationSpaceMouseReverse%1").arg(axis));
      reverse->setAccessibleName(tr("%1 reversed").arg(axisNames[axis]));
      reverse->setChecked(m_settings.spaceMouse.reversed[index]);
      axes->addWidget(reverse, axis + 1, 2);
      connect(axisOn, &QCheckBox::toggled, this, [this, index](bool value) {
        m_settings.spaceMouse.axisOn[index] = value;
        notify();
      });
      connect(reverse, &QCheckBox::toggled, this, [this, index](bool value) {
        m_settings.spaceMouse.reversed[index] = value;
        notify();
      });
    }
    axes->setColumnStretch(0, 1);
    box->addLayout(axes);
    auto* locks = new QGridLayout;
    const auto lock = [this, locks](int row, int column, const QString& title, const char* name,
                                    bool SpaceMouseSettings::*field) {
      auto* check = new QCheckBox(title);
      check->setObjectName(QString::fromLatin1(name));
      check->setChecked(m_settings.spaceMouse.*field);
      locks->addWidget(check, row, column);
      connect(check, &QCheckBox::toggled, this, [this, field](bool value) {
        m_settings.spaceMouse.*field = value;
        notify();
      });
      return check;
    };
    lock(0, 0, tr("Lock rotation"), "navigationSpaceMouseLockRotation", &SpaceMouseSettings::lockRotation);
    lock(0, 1, tr("Lock translation"), "navigationSpaceMouseLockTranslation", &SpaceMouseSettings::lockTranslation);
    auto* dominant =
        lock(1, 0, tr("Dominant axis only"), "navigationSpaceMouseDominant", &SpaceMouseSettings::dominantAxis);
    // The axis is held until the cap is released (mitcad#1720).
    dominant->setToolTip(tr("Only the axis moved most moves the view, and it stays the same until the 3D mouse is "
                            "released"));
    lock(1, 1, tr("Lock horizon"), "navigationSpaceMouseLockHorizon", &SpaceMouseSettings::lockHorizon)
        ->setToolTip(tr("Keeps Z upright and the view level; the roll axis does nothing"));
    box->addLayout(locks);
    // The cross-talk filter (mitcad#1720): Off, or the share of the
    // strongest axis below which an axis does nothing. Dominant axis only
    // keeps one axis anyway, so the filter waits while it is on.
    auto* crossTalkForm = new QFormLayout;
    auto* crossTalk = new QSpinBox;
    crossTalk->setObjectName(QStringLiteral("navigationSpaceMouseCrossTalk"));
    crossTalk->setRange(0, kSpaceMouseCrossTalkMax);
    crossTalk->setSingleStep(5);
    crossTalk->setSuffix(QStringLiteral(" %"));
    crossTalk->setSpecialValueText(tr("Off"));
    crossTalk->setValue(sanitizeSpaceMouseCrossTalk(m_settings.spaceMouse.crossTalk));
    crossTalk->setToolTip(tr("An axis moved less than this share of the most moved axis does not move the view"));
    crossTalk->setEnabled(!m_settings.spaceMouse.dominantAxis);
    crossTalkForm->addRow(tr("Ignore weaker axes:"), crossTalk);
    box->addLayout(crossTalkForm);
    connect(crossTalk, &QSpinBox::valueChanged, this, [this](int percent) {
      m_settings.spaceMouse.crossTalk = percent;
      notify();
    });
    connect(dominant, &QCheckBox::toggled, crossTalk, [crossTalk](bool only) { crossTalk->setEnabled(!only); });
    // The buttons the device has, at least two, and those bound before.
    const int rows = std::clamp(std::max({2, int(m_settings.spaceMouse.buttons.size()), spaceMouseStatus().buttonsSeen}),
                                2, 32);
    auto* buttons = new QTableWidget(rows, 2);
    buttons->setObjectName(QStringLiteral("navigationSpaceMouseButtons"));
    mac::styleListTable(buttons);
    buttons->setHorizontalHeaderLabels({tr("Button"), tr("Command")});
    buttons->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    buttons->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    buttons->verticalHeader()->hide();
    buttons->setSelectionMode(QAbstractItemView::NoSelection);
    buttons->setAccessibleName(tr("3D mouse buttons"));
    for (int row = 0; row < rows; ++row) {
      auto* label = new QTableWidgetItem(tr("Button %1").arg(row + 1));
      label->setFlags(Qt::ItemIsEnabled);
      buttons->setItem(row, 0, label);
      auto* command = new QComboBox;
      command->setObjectName(QStringLiteral("navigationSpaceMouseButton%1").arg(row));
      command->addItem(tr("None"), QString());
      for (const auto& entry : m_buttonCommands) {
        command->addItem(entry.second, entry.first);
      }
      const QString bound = m_settings.spaceMouse.buttons.value(row);
      if (!bound.isEmpty() && command->findData(bound) < 0) {
        command->addItem(bound, bound); // a command this list does not offer keeps its binding
      }
      command->setCurrentIndex(std::max(0, command->findData(bound)));
      buttons->setCellWidget(row, 1, command);
      connect(command, &QComboBox::currentIndexChanged, this, [this, row, command] {
        QStringList& bindings = m_settings.spaceMouse.buttons;
        while (bindings.size() <= row) {
          bindings.append(QString());
        }
        bindings[row] = command->currentData().toString();
        notify();
      });
    }
    buttons->resizeRowsToContents();
    buttons->setFixedHeight(std::min(rows, 4) * buttons->rowHeight(0) +
                            buttons->horizontalHeader()->sizeHint().height() + 2 * buttons->frameWidth());
    box->addWidget(buttons);
    layout->addWidget(group);
  }

  NavigationSettings m_settings;
  QVector<QPair<QString, QString>> m_buttonCommands;
  QVector<NavigationBinding> m_bindings;
  QComboBox* m_scheme = nullptr;
  // Fork: the mapping by program, the orbit center's hidden list, the
  // buttons' drag lists and the gesture ring's note and button.
  QComboBox* m_mapping = nullptr;
  QComboBox* m_orbitCenter = nullptr;
  QVector<QComboBox*> m_dragBoxes;
  QLabel* m_gestureNote = nullptr;
  QPushButton* m_useSolidWorks = nullptr;
  QComboBox* m_seed = nullptr;
  QTableWidget* m_table = nullptr;
  NavigationGuide* m_guide = nullptr;
  QLabel* m_hint = nullptr;
  bool m_geometryLogPending = false;
};

} // namespace mitcad
