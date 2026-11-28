// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Appearance, as SolidCraft's: Theme, Viewport and logo,
// Scale and text, and Color vision. Its choices of PureCAD's own settings
// (the theme, the view's background, the interface size, the font) go to
// the Display page's controls, which keep their saving (apply, called at
// each change); its own settings are AppearancePrefs ("appearance/").

#include <initializer_list>

#include <QColorDialog>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontComboBox>
#include <QLineEdit>
#include <QSettings>

#include "PreferenceKit.hpp"

namespace mitcad {

struct AppearancePrefs {
  QString picture;               // the view's background picture ("" none)
  bool logoAccent = false;       // Logo: Theme's colors or Accent
  int accent = -1;               // -1 the theme's own, else one of the swatches
  bool followThemeFile = false;
  QString themeFile;
  QString fontFile;              // a font file of one's own ("" none)
  int colorVision = 0;           // 0 Off, 1 Protan, 2 Deutan, 3 Tritan
  bool themeGrid = false;        // the theme cards in a grid, not a row

  static AppearancePrefs load() {
    const QSettings store;
    AppearancePrefs s;
    const QString g = QStringLiteral("appearance/");
    s.picture = store.value(g + QStringLiteral("picture")).toString();
    s.logoAccent = store.value(g + QStringLiteral("logoAccent"), s.logoAccent).toBool();
    s.accent = std::clamp(store.value(g + QStringLiteral("accent"), s.accent).toInt(), -1, 7);
    s.followThemeFile = store.value(g + QStringLiteral("followThemeFile"), s.followThemeFile).toBool();
    s.themeFile = store.value(g + QStringLiteral("themeFile")).toString();
    s.fontFile = store.value(g + QStringLiteral("fontFile")).toString();
    s.colorVision = std::clamp(store.value(g + QStringLiteral("colorVision"), 0).toInt(), 0, 3);
    s.themeGrid = store.value(g + QStringLiteral("themeGrid"), s.themeGrid).toBool();
    return s;
  }
  void save() const {
    QSettings store;
    const QString g = QStringLiteral("appearance/");
    store.setValue(g + QStringLiteral("picture"), picture);
    store.setValue(g + QStringLiteral("logoAccent"), logoAccent);
    store.setValue(g + QStringLiteral("accent"), accent);
    store.setValue(g + QStringLiteral("followThemeFile"), followThemeFile);
    store.setValue(g + QStringLiteral("themeFile"), themeFile);
    store.setValue(g + QStringLiteral("fontFile"), fontFile);
    store.setValue(g + QStringLiteral("colorVision"), colorVision);
    store.setValue(g + QStringLiteral("themeGrid"), themeGrid);
  }

  // The swatches' accents.
  static QColor accentColor(int index) {
    static const QColor colors[8] = {QColor(0x1f, 0x4f, 0xb8), QColor(0x2e, 0x96, 0x34), QColor(0x1f, 0x8c, 0x7a),
                                     QColor(0x6a, 0x35, 0xc8), QColor(0xc8, 0x3a, 0x8a), QColor(0xc8, 0x3a, 0x2e),
                                     QColor(0xe0, 0x6c, 0x1a), QColor(0xd8, 0xa8, 0x22)};
    return colors[std::clamp(index, 0, 7)];
  }
  // A colour as a colour-vision deficiency sees it (an approximation of
  // Machado et al.'s matrices at full severity), for the page's preview.
  static QColor simulated(const QColor& c, int mode) {
    if (mode <= 0) return c;
    static const double m[3][9] = {
        {0.152, 1.053, -0.205, 0.115, 0.786, 0.099, -0.004, -0.048, 1.052},   // protan
        {0.367, 0.861, -0.228, 0.280, 0.673, 0.047, -0.012, 0.043, 0.969},    // deutan
        {1.256, -0.077, -0.179, -0.078, 0.931, 0.148, 0.005, 0.691, 0.304}};  // tritan
    const double* k = m[std::clamp(mode, 1, 3) - 1];
    const double r = c.redF(), g = c.greenF(), b = c.blueF();
    const auto clamp01 = [](double v) { return std::clamp(v, 0.0, 1.0); };
    return QColor::fromRgbF(float(clamp01(k[0] * r + k[1] * g + k[2] * b)), float(clamp01(k[3] * r + k[4] * g + k[5] * b)),
                            float(clamp01(k[6] * r + k[7] * g + k[8] * b)), c.alphaF());
  }
};

// A theme's card: a small picture of the window in its colours.
class ThemeCard : public QAbstractButton {
public:
  ThemeCard(const QString& name, const QString& caption, bool dark, QWidget* parent = nullptr)
      : QAbstractButton(parent), m_caption(caption), m_dark(dark) {
    setText(name);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(uiRoomSize(170), uiRoomSize(164));
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    QColor ink = pal.color(QPalette::WindowText);
    const QRectF card = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
    QColor edge = ink;
    edge.setAlphaF(isChecked() ? 0.9f : 0.16f);
    QColor fill = ink;
    fill.setAlphaF(isChecked() ? 0.07f : 0.025f);
    p.setPen(QPen(edge, isChecked() ? 2.0 : 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(card, 10, 10);
    // The window: a dark or light frame, its toolbar dots, the Browser's
    // lines and a cube in the view.
    const QRectF frame = card.adjusted(8, 8, -8, -54);
    const QColor back = m_dark ? QColor(26, 28, 34) : QColor(246, 247, 249);
    const QColor panel = m_dark ? QColor(40, 43, 52) : QColor(228, 231, 236);
    const QColor line = m_dark ? QColor(210, 214, 222) : QColor(60, 64, 72);
    p.setPen(QPen(QColor(0, 0, 0, 40), 1));
    p.setBrush(back);
    p.drawRoundedRect(frame, 6, 6);
    p.setPen(Qt::NoPen);
    p.setBrush(pal.color(QPalette::Highlight));
    p.drawRoundedRect(QRectF(frame.left() + 34, frame.top() + 6, 18, 5), 2, 2);
    p.setBrush(line);
    p.drawRoundedRect(QRectF(frame.left() + 56, frame.top() + 6, 12, 5), 2, 2);
    p.drawRoundedRect(QRectF(frame.left() + 72, frame.top() + 6, 12, 5), 2, 2);
    const QRectF view(frame.left() + 34, frame.top() + 16, frame.width() - 40, frame.height() - 22);
    p.setBrush(panel);
    p.drawRoundedRect(view, 4, 4);
    p.setPen(QPen(line, 2, Qt::SolidLine, Qt::RoundCap));
    for (int i = 0; i < 4; ++i) {
      const double y = frame.top() + 26 + i * 9;
      p.drawLine(QPointF(frame.left() + 10, y), QPointF(frame.left() + (i % 2 ? 22 : 26), y));
    }
    const QPointF c = view.center() + QPointF(0, -4);
    const double s = 14;
    p.setPen(QPen(line, 1.2));
    p.setBrush(m_dark ? QColor(150, 154, 162) : QColor(120, 124, 132));
    p.drawPolygon(QPolygonF{c + QPointF(0, -s), c + QPointF(s, -s / 2), c, c + QPointF(-s, -s / 2)});
    p.setBrush(m_dark ? QColor(110, 114, 122) : QColor(90, 94, 102));
    p.drawPolygon(QPolygonF{c + QPointF(-s, -s / 2), c, c + QPointF(0, s), c + QPointF(-s, s / 2)});
    p.setBrush(m_dark ? QColor(80, 84, 92) : QColor(70, 74, 82));
    p.drawPolygon(QPolygonF{c, c + QPointF(s, -s / 2), c + QPointF(s, s / 2), c + QPointF(0, s)});
    p.setPen(Qt::NoPen);
    p.setBrush(line);
    for (int i = 0; i < 5; ++i) p.drawEllipse(QPointF(view.center().x() - 12 + i * 6, view.bottom() - 7), 1.3, 1.3);
    // The mode's badge at the upper left.
    const QRectF mode(frame.left() + 6, frame.top() + 6, 18, 18);
    p.setBrush(m_dark ? QColor(255, 255, 255, 40) : QColor(0, 0, 0, 40));
    p.drawEllipse(mode);
    p.setPen(QPen(line, 1.4));
    p.setBrush(Qt::NoBrush);
    if (m_dark) {
      QPainterPath moon;
      moon.addEllipse(mode.adjusted(4, 4, -4, -4));
      QPainterPath bite;
      bite.addEllipse(mode.adjusted(7, 2, -1, -6));
      p.drawPath(moon.subtracted(bite));
    } else {
      p.drawEllipse(mode.center(), 3, 3);
      for (int k = 0; k < 8; ++k) {
        const double a = k * 3.14159265358979323846 / 4;
        p.drawLine(mode.center() + QPointF(5 * std::cos(a), 5 * std::sin(a)),
                   mode.center() + QPointF(7 * std::cos(a), 7 * std::sin(a)));
      }
    }
    // The check badge, the name and its caption.
    if (isChecked()) {
      const QRectF badge(card.right() - 26, card.top() + 9, 18, 18);
      p.setPen(Qt::NoPen);
      p.setBrush(pal.color(QPalette::Highlight));
      p.drawEllipse(badge);
      p.setPen(QPen(pal.color(QPalette::HighlightedText), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      p.drawPolyline(QPolygonF{badge.center() + QPointF(-4.0, 0.2), badge.center() + QPointF(-1.2, 3.0),
                               badge.center() + QPointF(4.2, -3.0)});
    }
    QFont name = font();
    name.setBold(isChecked());
    name.setPointSizeF(name.pointSizeF() * 1.05);
    p.setFont(name);
    p.setPen(ink);
    p.drawText(QRectF(card.left(), frame.bottom() + 6, card.width(), 20), Qt::AlignCenter, text());
    p.setFont(font());
    ink.setAlphaF(0.65f);
    p.setPen(ink);
    p.drawText(QRectF(card.left(), frame.bottom() + 26, card.width(), 16), Qt::AlignCenter, m_caption);
  }

private:
  QString m_caption;
  bool m_dark;
};

class AppearancePreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::AppearancePreferences)

public:
  // What the page shows of PureCAD's own settings; `apply` sets them on
  // the Display page's controls at each change.
  struct Shown {
    int theme = 0;           // 0 System, 1 Light, 2 Dark
    int background = 5;      // ViewSettings::Background: 0 Gradient, 1 Light, 2 White, 3 Dark, 4 Custom, 5 Automatic
    QColor color = QColor(255, 255, 255);
    int scale = 100;         // percent, one of 100, 125, 150, 175, 200
    bool systemFont = true;
    QString family;
  };

  explicit AppearancePreferences(const Shown& shown, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("appearance"), parent), m_prefs(AppearancePrefs::load()), m_shown(shown) {
    buildTheme();
    buildViewport();
    buildScale();
    buildColorVision();
    finish();
  }

  std::function<void(const Shown&)> apply;

  void commit() {
    m_prefs.save();
    qDebug().noquote() << QStringLiteral("Preferences: appearance saved");
  }

private:
  void shownChanged() {
    if (apply) apply(m_shown);
  }

  void buildTheme() {
    QVBoxLayout* column = section(tr("Theme"));
    auto* top = new QHBoxLayout;
    top->addWidget(segments("themeMode", {tr("One theme"), tr("Follow system")}, m_shown.theme == 0 ? 1 : 0,
                            [this](int i) {
                              if (i == 1) {
                                m_shown.theme = 0;
                              } else if (m_shown.theme == 0) {
                                m_shown.theme = m_themeCards[1] != nullptr && m_themeCards[1]->isChecked() ? 2 : 1;
                              }
                              shownChanged();
                            }));
    top->addStretch(1);
    auto* import = new QPushButton(tr("⤓  Import theme file…"));
    import->setObjectName(name("importTheme"));
    connect(import, &QPushButton::clicked, this, [this] {
      const QString path = QFileDialog::getOpenFileName(this, tr("Import theme file"), QString(),
                                                        tr("Theme files (*.json *.qss);;All files (*)"));
      if (path.isEmpty()) return;
      m_prefs.themeFile = path;
      m_prefs.followThemeFile = true;
      if (m_followFile != nullptr) m_followFile->setChecked(true);
      changed();
    });
    top->addWidget(import);
    column->addLayout(top);
    auto* filters = new QHBoxLayout;
    filters->addWidget(segments("themeFilterMode", {tr("☀  Light 1"), tr("☾  Dark 1")}, -1, [](int) {}));
    filters->addWidget(segments("themeFilterKind", {tr("Standard 2"), tr("◑  High contrast 0")}, -1, [](int) {}));
    filters->addWidget(segments("themeFilterImported",
                                {tr("Imported %1").arg(m_prefs.themeFile.isEmpty() ? 0 : 1)}, -1, [](int) {}));
    filters->addStretch(1);
    column->addLayout(filters);
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    auto* group = new QButtonGroup(this);
    const bool darkChosen = m_shown.theme == 2;
    m_themeCards[0] = new ThemeCard(tr("Light"), tr("Built in"), false);
    m_themeCards[1] = new ThemeCard(tr("Dark"), tr("Built in"), true);
    for (int i = 0; i < 2; ++i) {
      m_themeCards[i]->setObjectName(name("theme") + QLatin1Char('_') + QString::number(i));
      m_themeCards[i]->setChecked(i == (darkChosen ? 1 : 0));
      group->addButton(m_themeCards[i], i);
      row->addWidget(m_themeCards[i]);
    }
    connect(group, &QButtonGroup::idClicked, this, [this](int id) {
      // A card chosen while following the system is the one theme now.
      m_shown.theme = id == 1 ? 2 : 1;
      if (auto* mode = findChild<QWidget*>(name("themeMode"))) {
        if (auto* one = mode->findChildren<QPushButton*>().value(0)) one->setChecked(true);
      }
      shownChanged();
      changed();
    });
    row->addStretch(1);
    column->addLayout(row);
  }

  void buildViewport() {
    QVBoxLayout* column = section(tr("Viewport and logo"));
    QFormLayout* f = form(column);
    // Background: the theme's gradient (Follow system, or 3D view colors'
    // choice) or one colour (Custom with both ends of it).
    auto* backgroundRow = new QHBoxLayout;
    m_swatch = new QPushButton;
    m_swatch->setObjectName(name("backgroundSwatch"));
    m_swatch->setFixedSize(uiRoomSize(40), uiRoomSize(30));
    m_hex = new QLineEdit(m_shown.color.name());
    m_hex->setMaximumWidth(uiRoomSize(150));
    const bool custom = m_shown.background == 4;
    backgroundRow->addWidget(segments("background", {tr("Theme's gradient"), tr("Color")}, custom ? 1 : 0,
                                      [this](int i) {
                                        m_colorBackground = i == 1;
                                        m_swatch->setEnabled(i == 1);
                                        m_hex->setEnabled(i == 1);
                                        updateBackground();
                                      }));
    m_colorBackground = custom;
    backgroundRow->addWidget(m_swatch);
    backgroundRow->addWidget(m_hex);
    backgroundRow->addStretch(1);
    paintSwatch();
    m_swatch->setEnabled(custom);
    m_hex->setEnabled(custom);
    connect(m_swatch, &QPushButton::clicked, this, [this] {
      const QColor chosen = QColorDialog::getColor(m_shown.color, this, tr("Background color"));
      if (!chosen.isValid()) return;
      m_shown.color = chosen;
      m_hex->setText(chosen.name());
      paintSwatch();
      updateBackground();
      changed();
    });
    connect(m_hex, &QLineEdit::editingFinished, this, [this] {
      const QColor typed(m_hex->text().trimmed());
      if (!typed.isValid()) return;
      m_shown.color = typed;
      paintSwatch();
      updateBackground();
      changed();
    });
    f->addRow(tr("Background"), backgroundRow);
    m_viewColors = m_shown.background == 3 ? 2 : (m_shown.background == 0 || m_shown.background == 1) ? 1 : 0;
    f->addRow(tr("3D view colors"), segments("viewColors", {tr("Theme's own"), tr("Light"), tr("Dark")}, m_viewColors,
                                             [this](int i) {
                                               m_viewColors = i;
                                               updateBackground();
                                             }));
    auto* picture = new QPushButton(m_prefs.picture.isEmpty() ? tr("Choose picture…")
                                                              : QFileInfo(m_prefs.picture).fileName());
    picture->setObjectName(name("picture"));
    connect(picture, &QPushButton::clicked, this, [this, picture] {
      const QString path = QFileDialog::getOpenFileName(this, tr("Background picture"), QString(),
                                                        tr("Pictures (*.png *.jpg *.jpeg *.bmp)"));
      if (path.isEmpty()) return;
      m_prefs.picture = path;
      picture->setText(QFileInfo(path).fileName());
      changed();
    });
    auto* pictureRow = new QHBoxLayout;
    pictureRow->addWidget(picture);
    pictureRow->addStretch(1);
    f->addRow(tr("Picture"), pictureRow);
    f->addRow(tr("Logo"), segments("logo", {tr("Theme's colors"), tr("Accent")}, m_prefs.logoAccent ? 1 : 0,
                                   [this](int i) { m_prefs.logoAccent = i == 1; }));
    // Accent: the theme's own, or one of eight.
    auto* accentBox = new QWidget;
    auto* accentColumn = new QVBoxLayout(accentBox);
    accentColumn->setContentsMargins(0, 0, 0, 0);
    auto* own = new QPushButton(tr("Theme's own"));
    own->setObjectName(name("accentOwn"));
    own->setCheckable(true);
    own->setChecked(m_prefs.accent < 0);
    {
      QFont bold = own->font();
      bold.setBold(true);
      own->setMinimumWidth(QFontMetrics(bold).horizontalAdvance(own->text()) + uiRoomSize(34));
    }
    own->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; border-radius: 12px; padding: 4px 12px; } "
                                      "QPushButton:checked { border: 2px solid %2; font-weight: 600; }")
                           .arg(palette().color(QPalette::Mid).name(), palette().color(QPalette::WindowText).name()));
    auto* ownRow = new QHBoxLayout;
    ownRow->addWidget(own);
    m_accentShown = new QLabel;
    m_accentShown->setFixedSize(uiRoomSize(40), uiRoomSize(28));
    ownRow->addWidget(m_accentShown);
    ownRow->addStretch(1);
    accentColumn->addLayout(ownRow);
    auto* swatches = new QHBoxLayout;
    swatches->setSpacing(10);
    auto* accents = new QButtonGroup(this);
    for (int i = 0; i < 8; ++i) {
      auto* swatch = new QPushButton;
      swatch->setObjectName(name("accent") + QString::number(i));
      swatch->setCheckable(true);
      swatch->setChecked(m_prefs.accent == i);
      swatch->setFixedSize(uiRoomSize(32), uiRoomSize(32));
      swatch->setStyleSheet(QStringLiteral("QPushButton { background: %1; border: 2px solid transparent; "
                                           "border-radius: 7px; } QPushButton:checked { border: 2px solid %2; }")
                                .arg(AppearancePrefs::accentColor(i).name(), palette().color(QPalette::WindowText).name()));
      accents->addButton(swatch, i);
      swatches->addWidget(swatch);
    }
    swatches->addStretch(1);
    accentColumn->addLayout(swatches);
    const auto showAccent = [this] {
      const QColor shown = m_prefs.accent < 0 ? palette().color(QPalette::Highlight)
                                              : AppearancePrefs::accentColor(m_prefs.accent);
      m_accentShown->setStyleSheet(QStringLiteral("QLabel { background: %1; border-radius: 6px; }").arg(shown.name()));
    };
    showAccent();
    connect(accents, &QButtonGroup::idClicked, this, [this, own, showAccent](int id) {
      m_prefs.accent = id;
      own->setChecked(false);
      showAccent();
      changed();
    });
    connect(own, &QPushButton::clicked, this, [this, own, accents, showAccent] {
      m_prefs.accent = -1;
      own->setChecked(true);
      accents->setExclusive(false);
      for (QAbstractButton* b : accents->buttons()) b->setChecked(false);
      accents->setExclusive(true);
      showAccent();
      changed();
    });
    f->addRow(tr("Accent"), accentBox);
    auto* fileRow = new QHBoxLayout;
    m_followFile = makeCheck(tr("Follow a theme file"), "followThemeFile", m_prefs.followThemeFile,
                             [this](bool on) { m_prefs.followThemeFile = on; });
    fileRow->addWidget(m_followFile);
    fileRow->addWidget(info(tr("Colors from a theme file you keep; PureCAD reads it again when it changes")));
    auto* choose = new QPushButton(tr("Choose…"));
    connect(choose, &QPushButton::clicked, this, [this] {
      const QString path = QFileDialog::getOpenFileName(this, tr("Theme file"), QString(),
                                                        tr("Theme files (*.json *.qss);;All files (*)"));
      if (path.isEmpty()) return;
      m_prefs.themeFile = path;
      m_followFile->setChecked(true);
      changed();
    });
    fileRow->addWidget(choose);
    fileRow->addWidget(tag(tr("Desktop only"), palette().color(QPalette::WindowText)));
    fileRow->addStretch(1);
    column->addLayout(fileRow);
  }

  void paintSwatch() {
    m_swatch->setStyleSheet(QStringLiteral("QPushButton { background: %1; border: 1px solid rgba(0,0,0,60); "
                                           "border-radius: 6px; }")
                                .arg(m_shown.color.name()));
  }
  // The view's background from Background and 3D view colors.
  void updateBackground() {
    m_shown.background = m_colorBackground ? 4 : m_viewColors == 1 ? 0 : m_viewColors == 2 ? 3 : 5;
    shownChanged();
  }

  void buildScale() {
    QVBoxLayout* column = section(tr("Scale and text"));
    auto* row = new QHBoxLayout;
    row->setSpacing(20);
    auto* left = new QVBoxLayout;
    const int sizes[5] = {100, 125, 150, 175, 200};
    int index = 0;
    for (int i = 0; i < 5; ++i) {
      if (sizes[i] == m_shown.scale) index = i;
    }
    auto* scale = slider("scale", 0, 4, index, [sizes](int v) { return QStringLiteral("%1 %").arg(sizes[v]); },
                         [this, sizes](int v) {
                           m_shown.scale = sizes[std::clamp(v, 0, 4)];
                           updatePreview();
                           shownChanged();
                         });
    left->addWidget(scale);
    auto* chipsRow = new QHBoxLayout;
    chipsRow->setSpacing(8);
    auto* chipGroup = new QButtonGroup(this);
    for (int i = 0; i < 5; ++i) {
      auto* chip = new QPushButton(QStringLiteral("%1%").arg(sizes[i]));
      chip->setCheckable(true);
      chip->setChecked(i == index);
      chip->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; border-radius: 10px; padding: 2px 10px; } "
                                         "QPushButton:checked { border: 2px solid %2; }")
                              .arg(palette().color(QPalette::Mid).name(), palette().color(QPalette::WindowText).name()));
      chipGroup->addButton(chip, i);
      chipsRow->addWidget(chip);
    }
    chipsRow->addStretch(1);
    connect(chipGroup, &QButtonGroup::idClicked, this, [scale](int id) {
      if (auto* bar = scale->findChild<QSlider*>()) bar->setValue(id);
    });
    if (auto* bar = scale->findChild<QSlider*>()) {
      connect(bar, &QSlider::valueChanged, this, [chipGroup](int v) {
        if (QAbstractButton* chip = chipGroup->button(v)) chip->setChecked(true);
      });
    }
    left->addLayout(chipsRow);
    left->addWidget(new QLabel(tr("Takes effect when PureCAD starts again.")));
    left->addStretch(1);
    row->addLayout(left, 1);
    // The live preview: a value field with OK and a Browser row, at the
    // chosen size.
    m_preview = new QFrame;
    m_preview->setObjectName(name("preview"));
    m_preview->setStyleSheet(QStringLiteral("QFrame#%1 { border: 1px solid %2; border-radius: 10px; }")
                                 .arg(m_preview->objectName(), palette().color(QPalette::Mid).name()));
    m_preview->setMinimumWidth(uiRoomSize(300));
    auto* previewColumn = new QVBoxLayout(m_preview);
    auto* caption = new QLabel(tr("LIVE PREVIEW"));
    caption->setAlignment(Qt::AlignRight);
    caption->setForegroundRole(QPalette::PlaceholderText);
    previewColumn->addWidget(caption);
    auto* distanceRow = new QHBoxLayout;
    m_previewLabel = new QLabel(tr("Distance"));
    m_previewField = new QLineEdit(QStringLiteral("12.50"));
    m_previewField->setAlignment(Qt::AlignCenter);
    m_previewOk = new QPushButton(tr("OK"));
    m_previewOk->setDefault(true);
    distanceRow->addWidget(m_previewLabel);
    distanceRow->addWidget(m_previewField);
    distanceRow->addWidget(m_previewOk);
    previewColumn->addLayout(distanceRow);
    m_previewRow = new QLabel(tr("◆  Extrude1                      12.5 mm"));
    m_previewRow->setStyleSheet(QStringLiteral("QLabel { background: rgba(128,128,128,40); border-radius: 6px; "
                                               "padding: 6px 10px; font-weight: 600; }"));
    previewColumn->addWidget(m_previewRow);
    row->addWidget(m_preview);
    column->addLayout(row);
    updatePreview();
    // Font: the system's or a family, or a font file of one's own.
    QFormLayout* f = form(column);
    auto* fontRow = new QHBoxLayout;
    auto* family = new QFontComboBox;
    family->setObjectName(name("font"));
    family->setMinimumWidth(uiRoomSize(300));
    if (!m_shown.systemFont && !m_shown.family.isEmpty()) family->setCurrentFont(QFont(m_shown.family));
    auto* fileButton = new QPushButton(m_prefs.fontFile.isEmpty() ? tr("Choose font file…")
                                                                  : QFileInfo(m_prefs.fontFile).fileName());
    auto* reset = new QPushButton(tr("Reset"));
    auto* sample = new QLabel(tr("The quick brown fox · Ø12.50 mm · 45.0° · 0123456789"));
    sample->setObjectName(name("fontSample"));
    sample->setStyleSheet(QStringLiteral("QLabel { border: 1px solid %1; border-radius: 6px; padding: 8px 10px; }")
                              .arg(palette().color(QPalette::Mid).name()));
    const auto showSample = [sample, family, this] {
      QFont f = sample->font();
      if (!m_shown.systemFont) f.setFamily(family->currentFont().family());
      sample->setFont(f);
    };
    connect(family, &QFontComboBox::currentFontChanged, this, [this, showSample](const QFont& chosen) {
      m_shown.systemFont = false;
      m_shown.family = chosen.family();
      showSample();
      shownChanged();
      changed();
    });
    connect(fileButton, &QPushButton::clicked, this, [this, fileButton] {
      const QString path = QFileDialog::getOpenFileName(this, tr("Font file"), QString(),
                                                        tr("Fonts (*.ttf *.otf)"));
      if (path.isEmpty()) return;
      m_prefs.fontFile = path;
      fileButton->setText(QFileInfo(path).fileName());
      changed();
    });
    connect(reset, &QPushButton::clicked, this, [this, fileButton, sample] {
      m_shown.systemFont = true;
      m_shown.family.clear();
      m_prefs.fontFile.clear();
      fileButton->setText(tr("Choose font file…"));
      sample->setFont(font());
      shownChanged();
      changed();
    });
    fontRow->addWidget(family, 1);
    fontRow->addWidget(fileButton);
    fontRow->addWidget(reset);
    f->addRow(tr("Font"), fontRow);
    column->addWidget(sample);
    showSample();
  }
  void updatePreview() {
    if (m_preview == nullptr) return;
    QFont f = font();
    f.setPointSizeF(font().pointSizeF() * m_shown.scale / 100.0);
    for (QWidget* w : std::initializer_list<QWidget*>{m_previewLabel, m_previewField, m_previewOk, m_previewRow}) {
      w->setFont(f);
    }
  }

  void buildColorVision() {
    QVBoxLayout* column = section(tr("Color vision"));
    QFormLayout* f = form(column);
    auto* modeRow = new QHBoxLayout;
    modeRow->addWidget(segments("colorVision", {tr("Off"), tr("Protan"), tr("Deutan"), tr("Tritan")},
                                m_prefs.colorVision, [this](int i) {
                                  m_prefs.colorVision = i;
                                  m_samples->update();
                                }));
    modeRow->addStretch(1);
    f->addRow(label(tr("Mode"), QString(),
                    tr("Colors that stay apart for red-weak (Protan), green-weak (Deutan) or blue-weak (Tritan) "
                       "vision; the samples below show how they look")),
              modeRow);
    m_samples = new ColorVisionSamples(&m_prefs.colorVision);
    column->addWidget(m_samples);
  }

  // The states' chips and the view's colour cues, as the chosen mode
  // sees them.
  class ColorVisionSamples : public QWidget {
  public:
    explicit ColorVisionSamples(const int* mode) : m_mode(mode) { setMinimumHeight(uiRoomSize(150)); }

  protected:
    void paintEvent(QPaintEvent*) override {
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing);
      const auto sim = [this](const QColor& c) { return AppearancePrefs::simulated(c, *m_mode); };
      const struct { const char* text; QColor color; } chips[] = {
          {"✓  Selected", QColor(0x1f, 0x3f, 0x9c)}, {"▲  Warning", QColor(0xe0, 0x9a, 0x2a)},
          {"✕  Error", QColor(0xc8, 0x2e, 0x2e)}, {"✓  Done", QColor(0x1f, 0x6a, 0x2e)}};
      double x = 0;
      QFont bold = font();
      bold.setBold(true);
      p.setFont(bold);
      for (const auto& chip : chips) {
        const QString text = QString::fromUtf8(chip.text);
        const double w = QFontMetrics(bold).horizontalAdvance(text) + 28;
        const QRectF box(x, 4, w, 28);
        p.setPen(Qt::NoPen);
        p.setBrush(sim(chip.color));
        p.drawRoundedRect(box, 14, 14);
        p.setPen(Qt::white);
        p.drawText(box, Qt::AlignCenter, text);
        x += w + 10;
      }
      // Fixed, Projected, Axes, Bends, Draft, Failed.
      const QStringList names{QStringLiteral("Fixed"), QStringLiteral("Projected"), QStringLiteral("Axes"),
                              QStringLiteral("Bends"), QStringLiteral("Draft"), QStringLiteral("Failed")};
      p.setFont(font());
      for (int i = 0; i < 6; ++i) {
        const QRectF tile(i * 108, 50, 96, 70);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(128, 128, 128, 22));
        p.drawRoundedRect(tile, 8, 8);
        const QPointF c = tile.center() + QPointF(0, -8);
        switch (i) {
        case 0:
          p.setPen(QPen(sim(QColor(0x1f, 0x6a, 0x2e)), 3));
          p.drawLine(c + QPointF(-28, 0), c + QPointF(28, 0));
          break;
        case 1:
          p.setPen(QPen(sim(QColor(0x6a, 0x35, 0xc8)), 3));
          p.drawLine(c + QPointF(-28, 0), c + QPointF(28, 0));
          break;
        case 2:
          p.setPen(QPen(sim(QColor(0x2e, 0x96, 0x34)), 3));
          p.drawLine(c + QPointF(-24, -14), c + QPointF(-24, 12));
          p.setPen(QPen(sim(QColor(0xc8, 0x2e, 0x2e)), 3));
          p.drawLine(c + QPointF(-24, 12), c + QPointF(26, 12));
          break;
        case 3:
          p.setPen(QPen(sim(QColor(0xc8, 0x2e, 0x2e)), 2, Qt::DashLine));
          p.drawLine(c + QPointF(-28, -6), c + QPointF(28, -6));
          p.drawLine(c + QPointF(-28, 6), c + QPointF(28, 6));
          break;
        case 4:
          p.setPen(Qt::NoPen);
          p.setBrush(sim(QColor(0x2e, 0x96, 0x34)));
          p.drawRect(QRectF(c.x() - 30, c.y() - 14, 20, 28));
          p.setBrush(sim(QColor(0xe0, 0xc8, 0x2a)));
          p.drawRect(QRectF(c.x() - 10, c.y() - 14, 20, 28));
          p.setBrush(QBrush(sim(QColor(0xc8, 0x2e, 0x2e)), Qt::BDiagPattern));
          p.drawRect(QRectF(c.x() + 10, c.y() - 14, 20, 28));
          p.setBrush(sim(QColor(0xc8, 0x2e, 0x2e)));
          p.setOpacity(0.6);
          p.drawRect(QRectF(c.x() + 10, c.y() - 14, 20, 28));
          p.setOpacity(1.0);
          break;
        default:
          p.setPen(QPen(sim(QColor(0xc8, 0x2e, 0x2e)), 2));
          p.setBrush(sim(QColor(0xf4, 0xc8, 0xc8)));
          p.drawRoundedRect(QRectF(c.x() - 26, c.y() - 12, 52, 24), 5, 5);
          p.drawLine(c + QPointF(-7, -7), c + QPointF(7, 7));
          p.drawLine(c + QPointF(-7, 7), c + QPointF(7, -7));
          break;
        }
        QColor ink = palette().color(QPalette::WindowText);
        ink.setAlphaF(0.7f);
        p.setPen(ink);
        p.drawText(QRectF(tile.left(), tile.bottom() + 2, tile.width(), 20), Qt::AlignCenter, names.at(i));
      }
    }

  private:
    const int* m_mode;
  };

  AppearancePrefs m_prefs;
  Shown m_shown;
  ThemeCard* m_themeCards[2] = {nullptr, nullptr};
  QPushButton* m_swatch = nullptr;
  QLineEdit* m_hex = nullptr;
  bool m_colorBackground = false;
  int m_viewColors = 0;
  QLabel* m_accentShown = nullptr;
  QCheckBox* m_followFile = nullptr;
  QFrame* m_preview = nullptr;
  QLabel* m_previewLabel = nullptr;
  QLineEdit* m_previewField = nullptr;
  QPushButton* m_previewOk = nullptr;
  QLabel* m_previewRow = nullptr;
  ColorVisionSamples* m_samples = nullptr;
};

} // namespace mitcad
