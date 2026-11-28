// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Graphics, as SolidCraft's: Preset, Curved surfaces,
// Edges and shading, Current view, In the view, Surfaces and edges and This
// computer. Its own settings are GraphicsSettings ("graphics/"); Curved
// surfaces and Anti-aliasing are view/ViewQualityPreference.hpp's; the grid,
// the projection, the field of view and the visual style are the view's
// (GraphicsPreferences::ViewState, which ViewController reads and applies).

#include <cmath>
#include <functional>
#include <tuple>

#include <QComboBox>
#include <QPushButton>

#include "GraphicsSettings.hpp"
#include "PreferenceKit.hpp"
#include "ViewQualityPreference.hpp"

namespace mitcad {

namespace graphics_pictures {

inline QColor ink(const QPalette& pal, float alpha = 1.0f) {
  QColor c = pal.color(QPalette::WindowText);
  c.setAlphaF(alpha);
  return c;
}
inline QColor accent(const QPalette& pal) { return pal.color(QPalette::Highlight); }

// A small isometric cube centred at `c`, `s` its half width.
inline void cube(QPainter& p, const QPointF& c, double s, const QPalette& pal, bool shaded = true) {
  const QPointF top(c.x(), c.y() - s), right(c.x() + s, c.y() - s / 2), left(c.x() - s, c.y() - s / 2);
  const QPointF mid(c.x(), c.y()), rightLow(c.x() + s, c.y() + s / 2), leftLow(c.x() - s, c.y() + s / 2),
      bottom(c.x(), c.y() + s);
  p.setPen(QPen(ink(pal, 0.8f), 1.3));
  p.setBrush(shaded ? QColor(236, 238, 242) : Qt::transparent);
  p.drawPolygon(QPolygonF{top, right, mid, left});
  p.setBrush(shaded ? QColor(190, 196, 206) : Qt::transparent);
  p.drawPolygon(QPolygonF{left, mid, bottom, leftLow});
  p.setBrush(shaded ? QColor(160, 168, 180) : Qt::transparent);
  p.drawPolygon(QPolygonF{mid, right, rightLow, bottom});
}

inline void presetIcon(QPainter& p, const QRectF& box, int which, const QPalette& pal, bool chosen) {
  p.setPen(Qt::NoPen);
  p.setBrush(chosen ? accent(pal) : ink(pal, 0.08f));
  p.drawRoundedRect(box, 8, 8);
  const QColor mark = chosen ? pal.color(QPalette::HighlightedText) : ink(pal, 0.85f);
  p.setPen(QPen(mark, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  p.setBrush(Qt::NoBrush);
  const QPointF c = box.center();
  if (which == 0) { // a lightning bolt
    p.drawPolyline(QPolygonF{c + QPointF(3, -11), c + QPointF(-5, 1), c + QPointF(1, 1), c + QPointF(-3, 11),
                             c + QPointF(6, -2), c + QPointF(0, -2), c + QPointF(3, -11)});
  } else if (which == 1) { // a gem
    p.drawPolygon(QPolygonF{c + QPointF(-9, -4), c + QPointF(-5, -9), c + QPointF(5, -9), c + QPointF(9, -4),
                            c + QPointF(0, 9)});
    p.drawLine(c + QPointF(-9, -4), c + QPointF(9, -4));
    p.drawLine(c + QPointF(-3, -4), c + QPointF(0, 9));
    p.drawLine(c + QPointF(3, -4), c + QPointF(0, 9));
  } else { // sliders
    for (int i = 0; i < 3; ++i) {
      const double y = c.y() - 7 + i * 7;
      p.drawLine(QPointF(c.x() - 9, y), QPointF(c.x() + 9, y));
      p.setBrush(mark);
      p.drawEllipse(QPointF(c.x() + (i == 1 ? 4 : -4), y), 2.2, 2.2);
      p.setBrush(Qt::NoBrush);
    }
  }
}

} // namespace graphics_pictures

// A preset's wide card: an icon tile, the name and its caption, the check
// badge when chosen.
class PresetCard : public QAbstractButton {
public:
  PresetCard(int which, const QString& name, const QString& caption, QWidget* parent = nullptr)
      : QAbstractButton(parent), m_which(which), m_caption(caption) {
    setText(name);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(uiRoomSize(200), uiRoomSize(64));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    const QRectF card = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
    p.setPen(QPen(graphics_pictures::ink(pal, isChecked() ? 0.9f : 0.16f), isChecked() ? 2.0 : 1.0));
    p.setBrush(graphics_pictures::ink(pal, isChecked() ? 0.07f : 0.025f));
    p.drawRoundedRect(card, 10, 10);
    const QRectF icon(card.left() + 12, card.center().y() - 18, 36, 36);
    graphics_pictures::presetIcon(p, icon, m_which, pal, isChecked());
    QFont bold = font();
    bold.setBold(true);
    bold.setPointSizeF(bold.pointSizeF() * 1.05);
    p.setFont(bold);
    p.setPen(graphics_pictures::ink(pal));
    p.drawText(QRectF(icon.right() + 12, card.top() + 10, card.width(), 22), Qt::AlignLeft | Qt::AlignVCenter, text());
    p.setFont(font());
    p.setPen(graphics_pictures::ink(pal, 0.65f));
    p.drawText(QRectF(icon.right() + 12, card.top() + 30, card.width(), 20), Qt::AlignLeft | Qt::AlignVCenter,
               m_caption);
    if (isChecked()) {
      const QRectF badge(card.right() - 30, card.center().y() - 9, 18, 18);
      p.setPen(Qt::NoPen);
      p.setBrush(pal.color(QPalette::Highlight));
      p.drawEllipse(badge);
      p.setPen(QPen(pal.color(QPalette::HighlightedText), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      p.drawPolyline(QPolygonF{badge.center() + QPointF(-4.0, 0.2), badge.center() + QPointF(-1.2, 3.0),
                               badge.center() + QPointF(4.2, -3.0)});
    }
  }

private:
  int m_which;
  QString m_caption;
};

class GraphicsPreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::GraphicsPreferences)

public:
  // What the page shows of the view, read from and applied to it by
  // ViewController.
  struct ViewState {
    bool grid = true;
    bool perspective = false;
    int focalLength = 29;          // mm on a 35 mm frame (ViewSettings)
    int style = 0;                 // index into styles
    QStringList styles;            // the View menu's visual styles' names
    QString gpu;                   // the graphics card's name ("" unknown)
    QString renderer;              // "OpenGL 4.6 · GPU renderer"
  };

  GraphicsPreferences(const ViewState& view, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("graphics"), parent), m_settings(GraphicsSettings::load()), m_view(view),
        m_smoothness(savedSmoothness()), m_antialiasing(savedAntialiasing()),
        m_startApi(m_settings.graphicsApi), m_startCard(m_settings.graphicsCard) {
    buildPreset();
    buildCurves();
    buildEdges();
    buildCurrentView();
    buildInTheView();
    buildSurfaces();
    buildComputer();
    finish();
  }

  const GraphicsSettings& chosen() const { return m_settings; }
  const ViewState& view() const { return m_view; }
  Smoothness smoothness() const { return m_smoothness; }
  int antialiasing() const { return m_antialiasing; }

  // Restart Now (This computer): ViewController restarts MitCad.
  std::function<void()> restartRequested;
  // The view's part applied (ViewController sets it).
  std::function<void(const ViewState&)> applyView;

  // Saves the settings and tells who follows them (Apply and OK).
  void commit() {
    m_settings.save();
    if (saveSmoothness(m_smoothness)) {
      qDebug().noquote() << QStringLiteral("Preferences: smoothness %1").arg(smoothnessId(m_smoothness));
    }
    if (saveAntialiasing(m_antialiasing)) {
      qDebug().noquote() << QStringLiteral("Preferences: anti-aliasing %1").arg(m_antialiasing);
    }
    if (applyView) applyView(m_view);
    notifyGraphicsSettings(m_settings);
    qDebug().noquote() << QStringLiteral("Preferences: graphics saved");
  }

  // Degrees of the vertical field of view for a focal length on a 35 mm
  // frame (24 mm high), and back.
  static int fieldOfView(int focalLength) {
    return int(std::lround(2.0 * std::atan(12.0 / std::max(1, focalLength)) * 180.0 / 3.14159265358979323846));
  }
  static int focalLengthFor(int degrees) {
    const double half = std::clamp(degrees, 10, 120) * 3.14159265358979323846 / 360.0;
    return int(std::lround(12.0 / std::tan(half)));
  }

private:
  // Performance and Quality set the choices they stand for; a choice of
  // its own makes the preset Custom.
  void choosePreset(int preset) {
    m_settings.preset = preset;
    if (preset == 0) {
      m_smoothness = Smoothness::Standard;
      m_antialiasing = 4;
      m_settings.supersampling = 0;
      m_settings.ambientOcclusion = false;
    } else if (preset == 1) {
      m_smoothness = Smoothness::Smooth;
      m_antialiasing = 8;
      m_settings.supersampling = 2;
      m_settings.supersampleOnlyStill = true;
    }
    syncQuality();
  }
  void custom() {
    if (m_settings.preset != 2) {
      m_settings.preset = 2;
      if (m_presets[2] != nullptr) m_presets[2]->setChecked(true);
    }
  }
  // The cards and segments follow the choices a preset made.
  void syncQuality() {
    m_syncing = true;
    const int curve = curveIndex(m_smoothness);
    for (int i = 0; i < 4; ++i) {
      if (m_curveCards[i] != nullptr) m_curveCards[i]->setChecked(i == curve);
    }
    const int aa = m_antialiasing == 0 ? 0 : m_antialiasing == 2 ? 1 : m_antialiasing == 8 ? 3 : 2;
    for (int i = 0; i < 4; ++i) {
      if (m_aaCards[i] != nullptr) m_aaCards[i]->setChecked(i == aa);
    }
    if (m_supersampling != nullptr) {
      if (auto* b = m_supersampling->findChildren<QPushButton*>().value(m_settings.supersampling)) b->setChecked(true);
    }
    m_syncing = false;
  }
  static int curveIndex(Smoothness s) {
    return s == Smoothness::Coarse ? 0 : s == Smoothness::Standard ? 1 : s == Smoothness::Smooth ? 2 : 3;
  }

  void buildPreset() {
    QVBoxLayout* column = section(tr("Preset"));
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    auto* group = new QButtonGroup(this);
    const QStringList names{tr("Performance"), tr("Quality"), tr("Custom")};
    const QStringList captions{tr("Fastest"), tr("Sharpest"), tr("Your own mix")};
    for (int i = 0; i < 3; ++i) {
      auto* card = new PresetCard(i, names.at(i), captions.at(i));
      card->setObjectName(name("preset") + QLatin1Char('_') + QString::number(i));
      card->setChecked(m_settings.preset == i);
      group->addButton(card, i);
      row->addWidget(card);
      m_presets[i] = card;
    }
    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
      choosePreset(id);
      changed();
    });
    column->addLayout(row);
  }

  void buildCurves() {
    QVBoxLayout* column = section(tr("Curved surfaces"));
    const int segmentsOf[4] = {8, 16, 32, 64};
    const QStringList names{tr("Coarse"), tr("Normal"), tr("Fine"), tr("Ultra")};
    // PureCAD's default is Fine (smoother than the upstream's Normal).
    const QStringList captions{tr("fastest"), tr("standard"), tr("default"), tr("slowest")};
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    auto* group = new QButtonGroup(this);
    for (int i = 0; i < 4; ++i) {
      const int n = segmentsOf[i];
      auto* card = new PictureCard(names.at(i), captions.at(i),
                                   [n](QPainter& p, const QRectF& r, const QPalette& pal) {
                                     const QPointF c = r.center();
                                     const double radius = std::min(r.width(), r.height()) * 0.36;
                                     QPolygonF ring;
                                     for (int k = 0; k <= n; ++k) {
                                       const double a = 2 * 3.14159265358979323846 * k / n;
                                       ring << c + QPointF(radius * std::cos(a), radius * std::sin(a));
                                     }
                                     p.setPen(QPen(graphics_pictures::ink(pal), 1.6));
                                     p.setBrush(Qt::NoBrush);
                                     p.drawPolyline(ring);
                                     if (n <= 16) {
                                       p.setBrush(graphics_pictures::ink(pal));
                                       for (int k = 0; k < n; ++k) p.drawEllipse(ring.at(k), 1.6, 1.6);
                                     }
                                     QFont f = p.font();
                                     f.setPointSizeF(std::max(6.5, f.pointSizeF() * 0.8));
                                     p.setFont(f);
                                     p.setPen(graphics_pictures::ink(pal, 0.7f));
                                     p.drawText(r.adjusted(0, 0, -8, -4), Qt::AlignRight | Qt::AlignBottom,
                                                QString::number(n));
                                   },
                                   false);
      card->setObjectName(name("curves") + QLatin1Char('_') + QString::number(i));
      card->setCompact(140, 128);
      card->setChecked(i == curveIndex(m_smoothness));
      group->addButton(card, i);
      row->addWidget(card);
      m_curveCards[i] = card;
    }
    row->addStretch(1);
    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
      if (m_syncing) return;
      m_smoothness = id == 0 ? Smoothness::Coarse : id == 1 ? Smoothness::Standard : id == 2 ? Smoothness::Smooth
                                                                                           : Smoothness::Ultra;
      custom();
      changed();
    });
    column->addLayout(row);
  }

  void buildEdges() {
    QVBoxLayout* column = section(tr("Edges and shading"));
    auto* head = new QHBoxLayout;
    auto* title = new QLabel(tr("Anti-aliasing"));
    QFont bold = title->font();
    bold.setBold(true);
    title->setFont(bold);
    head->addWidget(title);
    head->addWidget(info(tr("Smooths the jagged edges of faces, lines and the grid; higher asks more of the graphics "
                            "card. Applied at once.")));
    head->addStretch(1);
    column->addLayout(head);
    const int samples[4] = {0, 2, 4, 8};
    const QStringList names{tr("Off"), tr("2×"), tr("4×"), tr("8×")};
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    auto* group = new QButtonGroup(this);
    for (int i = 0; i < 4; ++i) {
      const int s = samples[i];
      auto* card = new PictureCard(names.at(i), QString(),
                                   [s](QPainter& p, const QRectF& r, const QPalette& pal) {
                                     // A staircase edge, softened by the samples.
                                     const QColor fill = graphics_pictures::accent(pal).darker(160);
                                     const double step = r.width() / 8.0;
                                     for (int k = 0; k < 8; ++k) {
                                       const double h = r.height() * (0.25 + 0.07 * k);
                                       const QRectF bar(r.left() + k * step, r.bottom() - h, step + 0.5, h);
                                       p.fillRect(bar, fill);
                                       if (s > 0) {
                                         QColor soft = fill;
                                         soft.setAlphaF(std::min(0.85, 0.2 + s * 0.08));
                                         p.fillRect(QRectF(bar.left(), bar.top() - 0.07 * r.height(), step,
                                                           0.07 * r.height()),
                                                    soft);
                                       }
                                     }
                                     p.setPen(QPen(graphics_pictures::ink(pal, 0.4f), 1));
                                     p.setBrush(pal.color(QPalette::Base));
                                     const QRectF dots(r.left() + 6, r.top() + 6, 18, 18);
                                     p.drawRect(dots);
                                     p.setPen(Qt::NoPen);
                                     p.setBrush(graphics_pictures::ink(pal));
                                     const int count = std::max(1, s);
                                     for (int k = 0; k < count; ++k) {
                                       const double a = 2 * 3.14159265358979323846 * k / count + 0.5;
                                       p.drawEllipse(dots.center() + QPointF(count > 1 ? 5 * std::cos(a) : 0,
                                                                             count > 1 ? 5 * std::sin(a) : 0),
                                                     1.3, 1.3);
                                     }
                                   },
                                   false);
      card->setObjectName(name("antialiasing") + QLatin1Char('_') + QString::number(i));
      card->setCompact(140, 112);
      card->setChecked(s == m_antialiasing);
      group->addButton(card, i);
      row->addWidget(card);
      m_aaCards[i] = card;
    }
    row->addStretch(1);
    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
      if (m_syncing) return;
      const int samplesOf[4] = {0, 2, 4, 8};
      m_antialiasing = samplesOf[std::clamp(id, 0, 3)];
      custom();
      changed();
    });
    column->addLayout(row);
    QFormLayout* f = form(column);
    m_supersampling = segments("supersampling", {tr("Off"), tr("1.5×"), tr("2×")}, m_settings.supersampling,
                               [this](int i) {
                                 if (m_syncing) return;
                                 m_settings.supersampling = i;
                                 custom();
                               },
                               true);
    m_supersampling->setMinimumWidth(uiRoomSize(420));
    f->addRow(label(tr("Supersampling"), QString(),
                    tr("Draws the view larger and scales it down: smoother still, but slower")),
              m_supersampling);
    check(column, tr("Only while the view is still"), "supersampleOnlyStill", m_settings.supersampleOnlyStill,
          [this](bool on) { m_settings.supersampleOnlyStill = on; });
    check(column, tr("Keep the shape of dark parts"), "keepDarkShapes", m_settings.keepDarkShapes,
          [this](bool on) { m_settings.keepDarkShapes = on; });
    check(column, tr("Material finish and studio lighting"), "studioLighting", m_settings.studioLighting,
          [this](bool on) { m_settings.studioLighting = on; });
  }

  void buildCurrentView() {
    QVBoxLayout* column = section(tr("Current view"));
    auto* tagRow = new QHBoxLayout;
    tagRow->addWidget(tag(tr("Also in the view bar"), QColor(46, 150, 52)));
    tagRow->addStretch(1);
    column->addLayout(tagRow);
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    using graphics_pictures::cube;
    toggleCard(row, "grid", tr("Grid"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF c = r.center() + QPointF(0, 8);
                 p.setPen(QPen(graphics_pictures::ink(pal, 0.45f), 0.9));
                 for (int k = -3; k <= 3; ++k) {
                   p.drawLine(c + QPointF(-48 + k * 8, k * 4 - 0), c + QPointF(k * 8 + 0, 24 + k * 4 - 24));
                 }
                 const QPolygonF diamond{c + QPointF(0, -26), c + QPointF(52, 0), c + QPointF(0, 26),
                                         c + QPointF(-52, 0)};
                 p.setBrush(Qt::NoBrush);
                 for (int k = 1; k < 6; ++k) {
                   const double t = k / 6.0;
                   p.drawLine(diamond[0] + (diamond[3] - diamond[0]) * t, diamond[1] + (diamond[2] - diamond[1]) * t);
                   p.drawLine(diamond[0] + (diamond[1] - diamond[0]) * t, diamond[3] + (diamond[2] - diamond[3]) * t);
                 }
                 p.drawPolygon(diamond);
                 cube(p, c + QPointF(0, -14), 14, pal);
               },
               m_view.grid, [this](bool on) { m_view.grid = on; });
    toggleCard(row, "groundShadow", tr("Ground shadow"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF c = r.center();
                 QRadialGradient shade(c + QPointF(0, 24), 34);
                 shade.setColorAt(0, QColor(0, 0, 0, 110));
                 shade.setColorAt(1, QColor(0, 0, 0, 0));
                 p.setPen(Qt::NoPen);
                 p.setBrush(shade);
                 p.drawEllipse(c + QPointF(0, 24), 40, 9);
                 cube(p, c + QPointF(0, -2), 16, pal);
               },
               m_settings.groundShadow, [this](bool on) { m_settings.groundShadow = on; });
    toggleCard(row, "perspective", tr("Perspective"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF c = r.center();
                 p.setPen(QPen(graphics_pictures::ink(pal), 1.6));
                 p.setBrush(Qt::NoBrush);
                 p.drawPolygon(QPolygonF{c + QPointF(-14, -8), c + QPointF(14, -8), c + QPointF(28, 24),
                                         c + QPointF(-28, 24)});
                 p.setPen(QPen(graphics_pictures::ink(pal), 1.2, Qt::DashLine));
                 p.drawLine(c + QPointF(0, -28), c + QPointF(-28, 24));
                 p.drawLine(c + QPointF(0, -28), c + QPointF(28, 24));
                 p.setPen(QPen(graphics_pictures::ink(pal), 1.6));
                 p.drawEllipse(c + QPointF(0, -28), 5, 5);
                 p.setBrush(graphics_pictures::ink(pal));
                 p.drawEllipse(c + QPointF(0, -28), 1.6, 1.6);
               },
               m_view.perspective, [this](bool on) {
                 m_view.perspective = on;
                 if (m_fov != nullptr) m_fov->setEnabled(on);
               });
    toggleCard(row, "ambientOcclusion", tr("Ambient occlusion"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF c = r.center();
                 QRadialGradient shade(c + QPointF(0, 16), 50);
                 shade.setColorAt(0, QColor(0, 0, 0, 120));
                 shade.setColorAt(1, QColor(0, 0, 0, 0));
                 p.setPen(Qt::NoPen);
                 p.setBrush(shade);
                 p.drawEllipse(c + QPointF(0, 16), 52, 12);
                 cube(p, c + QPointF(-18, 0), 14, pal);
                 cube(p, c + QPointF(18, 0), 14, pal);
               },
               m_settings.ambientOcclusion, [this](bool on) {
                 m_settings.ambientOcclusion = on;
                 custom();
               });
    row->addStretch(1);
    column->addLayout(row);
    check(column, tr("Ground shadow follows the model"), "groundShadowFollowsModel",
          m_settings.groundShadowFollowsModel, [this](bool on) { m_settings.groundShadowFollowsModel = on; });
    QFormLayout* f = form(column);
    auto* style = new QComboBox;
    style->setObjectName(name("visualStyle"));
    style->addItems(m_view.styles);
    style->setCurrentIndex(std::clamp(m_view.style, 0, std::max(0, int(m_view.styles.size()) - 1)));
    style->setMinimumWidth(uiRoomSize(300));
    connect(style, &QComboBox::currentIndexChanged, this, [this](int i) {
      m_view.style = i;
      changed();
    });
    f->addRow(tr("Visual style"), style);
    m_fov = slider("fieldOfView", 20, 90, fieldOfView(m_view.focalLength),
                   [](int v) { return QStringLiteral("%1°").arg(v); },
                   [this](int v) { m_view.focalLength = std::clamp(focalLengthFor(v), 10, 120); });
    m_fov->setEnabled(m_view.perspective);
    m_fov->setMinimumWidth(uiRoomSize(420));
    f->addRow(tr("Field of view"), m_fov);
  }

  void buildInTheView() {
    QVBoxLayout* column = section(tr("In the view"));
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    toggleCard(row, "viewAngles", tr("View angles"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF o = r.center() + QPointF(-22, 14);
                 p.setPen(QPen(QColor(30, 70, 210), 1.8));
                 p.drawLine(o, o + QPointF(0, -32));
                 p.setPen(QPen(QColor(214, 48, 40), 1.8));
                 p.drawLine(o, o + QPointF(26, 12));
                 p.setPen(QPen(QColor(46, 150, 52), 1.8));
                 p.drawLine(o, o + QPointF(-24, 12));
                 QFont f = p.font();
                 f.setPointSizeF(std::max(6.5, f.pointSizeF() * 0.8));
                 f.setFamily(QStringLiteral("monospace"));
                 p.setFont(f);
                 p.setPen(graphics_pictures::ink(pal));
                 p.drawText(QPointF(r.right() - 38, r.top() + 22), QStringLiteral("45°"));
                 p.drawText(QPointF(r.right() - 38, r.top() + 36), QStringLiteral("35°"));
                 p.drawText(QPointF(r.right() - 38, r.top() + 50), QStringLiteral(" 0°"));
               },
               m_settings.viewAngles, [this](bool on) { m_settings.viewAngles = on; });
    toggleCard(row, "vertexMarkers", tr("Vertex markers"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF c = r.center();
                 graphics_pictures::cube(p, c, 20, pal, false);
                 p.setPen(Qt::NoPen);
                 p.setBrush(graphics_pictures::ink(pal));
                 for (const QPointF& v : {QPointF(0, -20), QPointF(20, -10), QPointF(0, 0), QPointF(-20, -10),
                                          QPointF(20, 10), QPointF(-20, 10), QPointF(0, 20)}) {
                   p.drawEllipse(c + v, 3, 3);
                 }
               },
               m_settings.vertexMarkers, [this](bool on) { m_settings.vertexMarkers = on; });
    toggleCard(row, "quickMeasurement", tr("Quick measurement"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QPointF c = r.center();
                 graphics_pictures::cube(p, c + QPointF(-28, 10), 14, pal, false);
                 p.setPen(QPen(graphics_pictures::ink(pal), 2));
                 p.drawLine(c + QPointF(-8, -4), c + QPointF(12, -16));
                 p.setBrush(graphics_pictures::ink(pal));
                 p.drawEllipse(c + QPointF(-8, -4), 2.5, 2.5);
                 p.drawEllipse(c + QPointF(12, -16), 2.5, 2.5);
                 const QRectF tagBox(c.x() + 16, c.y() - 4, 40, 16);
                 p.setBrush(pal.color(QPalette::Base));
                 p.setPen(QPen(graphics_pictures::ink(pal), 1.2));
                 p.drawRoundedRect(tagBox, 5, 5);
                 QFont f = p.font();
                 f.setPointSizeF(std::max(6.0, f.pointSizeF() * 0.7));
                 p.setFont(f);
                 p.drawText(tagBox, Qt::AlignCenter, QStringLiteral("12.5 mm"));
               },
               m_settings.quickMeasurement, [this](bool on) { m_settings.quickMeasurement = on; });
    toggleCard(row, "frameRate", tr("Frame rate"),
               [](QPainter& p, const QRectF& r, const QPalette& pal) {
                 const QRectF box(r.center().x() - 34, r.center().y() - 20, 68, 40);
                 p.setPen(QPen(graphics_pictures::ink(pal), 1.6));
                 p.setBrush(pal.color(QPalette::Base));
                 p.drawRoundedRect(box, 6, 6);
                 QFont f = p.font();
                 f.setFamily(QStringLiteral("monospace"));
                 p.setFont(f);
                 p.drawText(box.adjusted(0, 3, 0, -14), Qt::AlignCenter, QStringLiteral("60 fps"));
                 QPainterPath wave(QPointF(box.left() + 8, box.bottom() - 9));
                 for (int k = 1; k <= 8; ++k) {
                   wave.lineTo(box.left() + 8 + k * 6.5, box.bottom() - (k % 2 ? 6 : 10));
                 }
                 p.setBrush(Qt::NoBrush);
                 p.drawPath(wave);
               },
               m_settings.frameRate, [this](bool on) {
                 m_settings.frameRate = on;
                 if (m_frameDetails != nullptr) m_frameDetails->setEnabled(on);
               });
    row->addStretch(1);
    column->addLayout(row);
    m_frameDetails = check(column, tr("Frame rate details"), "frameRateDetails", m_settings.frameRateDetails,
                           [this](bool on) { m_settings.frameRateDetails = on; });
    m_frameDetails->setEnabled(m_settings.frameRate);
    check(column, tr("Profile operations"), "profileOperations", m_settings.profileOperations,
          [this](bool on) { m_settings.profileOperations = on; });
    QFormLayout* f = form(column);
    f->addRow(tr("Status chips"),
              chips("statusChips", {tr("Analysis"), tr("Section"), tr("Live details"), tr("Fold items"),
                                    tr("Esc closes analyses")},
                    {m_settings.chipAnalysis, m_settings.chipSection, m_settings.chipLiveDetails,
                     m_settings.chipFoldItems, m_settings.chipEscClosesAnalyses},
                    [this](int i, bool on) {
                      bool* fields[5] = {&m_settings.chipAnalysis, &m_settings.chipSection, &m_settings.chipLiveDetails,
                                         &m_settings.chipFoldItems, &m_settings.chipEscClosesAnalyses};
                      *fields[std::clamp(i, 0, 4)] = on;
                    }));
    auto* joints = new QComboBox;
    joints->setObjectName(name("jointSymbols"));
    joints->addItems({tr("Ring and axis"), tr("Ring"), tr("Axis"), tr("None")});
    joints->setCurrentIndex(m_settings.jointSymbols);
    joints->setMinimumWidth(uiRoomSize(420));
    connect(joints, &QComboBox::currentIndexChanged, this, [this](int i) {
      m_settings.jointSymbols = i;
      changed();
    });
    f->addRow(tr("Joint symbols"), joints);
    check(column, tr("Highlight moving joints"), "highlightMovingJoints", m_settings.highlightMovingJoints,
          [this](bool on) { m_settings.highlightMovingJoints = on; });
  }

  void buildSurfaces() {
    QVBoxLayout* column = section(tr("Surfaces and edges"));
    check(column, tr("Cosmetic thread markings"), "cosmeticThreads", m_settings.cosmeticThreads,
          [this](bool on) { m_settings.cosmeticThreads = on; });
    check(column, tr("Halo under sketch curves"), "sketchHalo", m_settings.sketchHalo,
          [this](bool on) { m_settings.sketchHalo = on; });
    QFormLayout* f = form(column);
    QWidget* hatch = segments("hatch", {tr("One style"), tr("By part")}, m_settings.hatchByPart ? 1 : 0,
                              [this](int i) { m_settings.hatchByPart = i == 1; }, true);
    hatch->setMinimumWidth(uiRoomSize(420));
    f->addRow(tr("Section hatch"), hatch);
    check(column, tr("Tint new surface in a preview"), "tintNewSurface", m_settings.tintNewSurface,
          [this](bool on) { m_settings.tintNewSurface = on; });
    check(column, tr("Show removed material in red"), "removedMaterialRed", m_settings.removedMaterialRed,
          [this](bool on) { m_settings.removedMaterialRed = on; });
    check(column, tr("Show hidden edges of selected bodies"), "hiddenEdgesOfSelected",
          m_settings.hiddenEdgesOfSelected, [this](bool on) { m_settings.hiddenEdgesOfSelected = on; });
    check(column, tr("Reference geometry through bodies"), "referenceThroughBodies",
          m_settings.referenceThroughBodies, [this](bool on) { m_settings.referenceThroughBodies = on; });
    check(column, tr("Grid follows the view"), "gridFollowsView", m_settings.gridFollowsView,
          [this](bool on) { m_settings.gridFollowsView = on; });
    check(column, tr("Single surface analysis"), "singleSurfaceAnalysis", m_settings.singleSurfaceAnalysis,
          [this](bool on) { m_settings.singleSurfaceAnalysis = on; });
  }

  void buildComputer() {
    QVBoxLayout* outer = section(tr("This computer"));
    auto* card = new QFrame;
    card->setObjectName(name("computerCard"));
    card->setStyleSheet(QStringLiteral("QFrame#%1 { border: 1px solid %2; border-radius: 10px; background: %3; }")
                            .arg(card->objectName(), palette().color(QPalette::Mid).name(),
                                 palette().color(QPalette::Window).name()));
    auto* column = new QVBoxLayout(card);
    column->setContentsMargins(16, 14, 16, 14);
    column->setSpacing(8);
    auto* head = new QHBoxLayout;
    auto* names = new QVBoxLayout;
    names->setSpacing(3);
    auto* gpu = new QLabel(m_view.gpu.isEmpty() ? tr("Graphics card") : m_view.gpu);
    QFont bold = gpu->font();
    bold.setBold(true);
    gpu->setFont(bold);
    names->addWidget(gpu);
    auto* renderer = new QLabel(m_view.renderer);
    QFont mono = renderer->font();
    mono.setFamily(QStringLiteral("monospace"));
    renderer->setFont(mono);
    renderer->setForegroundRole(QPalette::PlaceholderText);
    names->addWidget(renderer);
    head->addLayout(names, 1);
    auto* kind = new QLabel(tr("GPU"));
    kind->setFont(bold);
    head->addWidget(kind, 0, Qt::AlignTop);
    column->addLayout(head);
    column->addSpacing(6);
    QFormLayout* f = new QFormLayout;
    f->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    f->setHorizontalSpacing(20);
    column->addLayout(f);
    auto* api = new QComboBox;
    api->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    api->setObjectName(name("graphicsApi"));
    api->addItems({tr("Automatic"), tr("OpenGL")});
    api->setCurrentIndex(m_settings.graphicsApi);
    f->addRow(label(tr("Graphics API"), QString(),
                    tr("How the 3D view reaches the graphics card. PureCAD's view draws with desktop OpenGL; "
                       "Automatic chooses it")),
              api);
    const QStringList apiNames{tr("Automatic"), tr("OpenGL")};
    auto* status = new QLabel(tr("Current: %1. Selected for next start: %2.")
                                  .arg(apiNames.value(m_startApi), apiNames.value(m_settings.graphicsApi)));
    status->setWordWrap(true);
    column->addWidget(status);
    column->addWidget(new QLabel(tr("Applies after a restart. Unsaved designs are handled before closing.")));
    auto* restart = new QPushButton(tr("Restart now"));
    restart->setObjectName(name("restart"));
    connect(restart, &QPushButton::clicked, this, [this] {
      commit();
      if (restartRequested) restartRequested();
    });
    auto* restartRow = new QHBoxLayout;
    restartRow->addWidget(restart);
    restartRow->addStretch(1);
    column->addLayout(restartRow);
    auto* cardForm = new QFormLayout;
    cardForm->setHorizontalSpacing(20);
    column->addLayout(cardForm);
    auto* cardChoice = new QComboBox;
    cardChoice->setMinimumWidth(uiRoomSize(240));
    cardChoice->setObjectName(name("graphicsCard"));
    cardChoice->addItems({tr("Automatic"), tr("High performance"), tr("Power saving")});
    cardChoice->setCurrentIndex(m_settings.graphicsCard);
    QLabel* restartTag = tag(tr("Restart needed"), QColor(200, 90, 20));
    restartTag->setVisible(false);
    auto* cardRow = new QHBoxLayout;
    cardRow->addWidget(cardChoice);
    cardRow->addWidget(restartTag);
    cardRow->addStretch(1);
    cardForm->addRow(label(tr("Graphics card"), QString(),
                    tr("Which graphics card Windows gives PureCAD on a computer with two")),
              cardRow);
    const auto updateStatus = [this, status, restartTag, apiNames] {
      status->setText(tr("Current: %1. Selected for next start: %2.")
                          .arg(apiNames.value(m_startApi), apiNames.value(m_settings.graphicsApi)));
      restartTag->setVisible(m_settings.graphicsApi != m_startApi || m_settings.graphicsCard != m_startCard);
    };
    connect(api, &QComboBox::currentIndexChanged, this, [this, updateStatus](int i) {
      m_settings.graphicsApi = i;
      updateStatus();
      changed();
    });
    connect(cardChoice, &QComboBox::currentIndexChanged, this, [this, updateStatus](int i) {
      m_settings.graphicsCard = i;
      updateStatus();
      changed();
    });
    check(column, tr("Start in safe graphics mode"), "safeGraphicsMode", m_settings.safeGraphicsMode,
          [this](bool on) { m_settings.safeGraphicsMode = on; },
          tr("For a graphics driver that draws the view wrongly: no anti-aliasing and no supersampling"));
    check(column, tr("Use previous Render preview geometry budget"), "previousRenderPreviewBudget",
          m_settings.previousRenderPreviewBudget, [this](bool on) { m_settings.previousRenderPreviewBudget = on; });
    check(column, tr("Use previous Render image defaults"), "previousRenderImageDefaults",
          m_settings.previousRenderImageDefaults, [this](bool on) { m_settings.previousRenderImageDefaults = on; });
    check(column, tr("Use previous render memory checks"), "previousRenderMemoryChecks",
          m_settings.previousRenderMemoryChecks, [this](bool on) { m_settings.previousRenderMemoryChecks = on; });
    outer->addWidget(card);
  }

  GraphicsSettings m_settings;
  ViewState m_view;
  Smoothness m_smoothness;
  int m_antialiasing;
  int m_startApi;
  int m_startCard;
  bool m_syncing = false;
  QAbstractButton* m_presets[3] = {nullptr, nullptr, nullptr};
  QAbstractButton* m_curveCards[4] = {nullptr, nullptr, nullptr, nullptr};
  QAbstractButton* m_aaCards[4] = {nullptr, nullptr, nullptr, nullptr};
  QWidget* m_supersampling = nullptr;
  QWidget* m_fov = nullptr;
  QCheckBox* m_frameDetails = nullptr;
};

} // namespace mitcad
