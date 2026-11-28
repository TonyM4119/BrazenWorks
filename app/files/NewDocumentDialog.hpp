// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// PureCAD: File › New asks what to create (file.new_choice). Picture cards
// on the left (a design, a drawing of the open design, an import into a new
// design, a project), the chosen one's words and options on the right (a
// design's units), and Create New. Double-clicking a card creates it. The
// card last created is chosen next time (newDocument/last).

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

#include "../view/PreferenceKit.hpp"

namespace mitcad {

class NewDocumentDialog : public QDialog {
  Q_DECLARE_TR_FUNCTIONS(mitcad::NewDocumentDialog)

public:
  enum class Kind { Design, Drawing, Import, Project };

  struct Choice {
    Kind kind = Kind::Design;
    QString units;            // a design's length unit (mm, cm, m, in, ft)
    bool unitsDefault = false; // the units kept for every new design
  };

  // `units`: the unit choices with the default first chosen; `drawingOf`:
  // the open design's name, empty when there is none to draw.
  NewDocumentDialog(const QStringList& units, const QString& defaultUnits, const QString& drawingOf,
                    QWidget* parent = nullptr)
      : QDialog(parent), m_drawingOf(drawingOf) {
    setWindowTitle(tr("New"));
    setObjectName(QStringLiteral("newDocumentDialog"));
    auto* column = new QVBoxLayout(this);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(0);

    auto* heading = new QLabel(tr("What do you want to create?"));
    QFont big = heading->font();
    big.setPointSizeF(big.pointSizeF() * 1.25);
    big.setBold(true);
    heading->setFont(big);
    heading->setContentsMargins(20, 16, 20, 12);
    column->addWidget(heading);

    auto* body = new QHBoxLayout;
    body->setContentsMargins(16, 4, 16, 12);
    body->setSpacing(16);
    auto* grid = new QGridLayout;
    grid->setSpacing(12);
    const struct {
      Kind kind;
      QString name;
      PictureCard::Painter paint;
    } cards[] = {
        {Kind::Design, tr("Design"), paintDesign},
        {Kind::Drawing, tr("Drawing"), paintDrawing},
        {Kind::Import, tr("Import"), paintImport},
        {Kind::Project, tr("Project"), paintProject},
    };
    int index = 0;
    for (const auto& card : cards) {
      auto* button = new PictureCard(card.name, QString(), card.paint, false);
      button->setObjectName(QStringLiteral("newDocumentCard_%1").arg(index));
      button->setAutoExclusive(true);
      button->setMinimumSize(uiRoomSize(170), uiRoomSize(150));
      if (card.kind == Kind::Drawing && drawingOf.isEmpty()) {
        button->setEnabled(false);
        button->setToolTip(tr("Open a design first: a drawing shows a design's views"));
      }
      const Kind kind = card.kind;
      connect(button, &QAbstractButton::toggled, this, [this, kind](bool on) {
        if (on) choose(kind);
      });
      button->installEventFilter(this);
      m_cards << button;
      grid->addWidget(button, index / 2, index % 2);
      ++index;
    }
    body->addLayout(grid);

    // The chosen card's words and options.
    auto* side = new QWidget;
    side->setObjectName(QStringLiteral("newDocumentSide"));
    side->setMinimumWidth(uiRoomSize(300));
    auto* sideColumn = new QVBoxLayout(side);
    sideColumn->setContentsMargins(12, 4, 4, 4);
    sideColumn->setSpacing(10);
    m_title = new QLabel;
    QFont bold = m_title->font();
    bold.setBold(true);
    m_title->setFont(bold);
    m_about = new QLabel;
    m_about->setWordWrap(true);
    m_about->setForegroundRole(QPalette::PlaceholderText);
    sideColumn->addWidget(m_title);
    sideColumn->addWidget(m_about);
    m_unitsRow = new QWidget;
    auto* unitsForm = new QGridLayout(m_unitsRow);
    unitsForm->setContentsMargins(0, 6, 0, 0);
    unitsForm->setHorizontalSpacing(10);
    unitsForm->addWidget(new QLabel(tr("Units")), 0, 0);
    m_units = new QComboBox;
    m_units->setObjectName(QStringLiteral("newDocumentUnits"));
    for (const QString& unit : units) m_units->addItem(unitName(unit), unit);
    m_units->setCurrentIndex(std::max(0, m_units->findData(defaultUnits)));
    unitsForm->addWidget(m_units, 0, 1);
    m_unitsDefault = new QCheckBox(tr("Use for every new design"));
    m_unitsDefault->setObjectName(QStringLiteral("newDocumentUnitsDefault"));
    m_unitsDefault->setToolTip(tr("New Design (Ctrl+N) and the Start page start in these units too"));
    unitsForm->addWidget(m_unitsDefault, 1, 1);
    connect(m_units, &QComboBox::currentIndexChanged, this, [this, defaultUnits] {
      m_unitsDefault->setEnabled(m_units->currentData().toString() != defaultUnits);
      if (!m_unitsDefault->isEnabled()) m_unitsDefault->setChecked(false);
    });
    m_unitsDefault->setEnabled(false);
    sideColumn->addWidget(m_unitsRow);
    sideColumn->addStretch(1);
    body->addWidget(side, 1);
    column->addLayout(body, 1);

    auto* foot = new QHBoxLayout;
    foot->setContentsMargins(20, 10, 16, 14);
    foot->addStretch(1);
    auto* cancel = new QPushButton(tr("Cancel"));
    cancel->setObjectName(QStringLiteral("newDocumentCancel"));
    m_create = new QPushButton(tr("Create New"));
    m_create->setObjectName(QStringLiteral("newDocumentCreate"));
    m_create->setDefault(true);
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_create, &QPushButton::clicked, this, &QDialog::accept);
    foot->addWidget(cancel);
    foot->addWidget(m_create);
    column->addLayout(foot);

    // The navy of Preferences' OK on light, the accent on dark.
    const bool dark = palette().color(QPalette::Window).lightness() < 128;
    const QColor fill = dark ? palette().color(QPalette::Highlight) : QColor(0x10, 0x2a, 0x5c);
    m_create->setStyleSheet(QStringLiteral("QPushButton#newDocumentCreate { background: %1; color: white; border: none; "
                                           "border-radius: 6px; padding: 6px 22px; font-weight: 700; } "
                                           "QPushButton#newDocumentCreate:hover { background: %2; }")
                                .arg(fill.name(), fill.lighter(125).name()));

    // The card last created, if it can be made now.
    int last = QSettings().value(QStringLiteral("newDocument/last"), 0).toInt();
    if (last < 0 || last >= m_cards.size() || !m_cards.at(last)->isEnabled()) last = 0;
    m_cards.at(last)->setChecked(true);
    choose(static_cast<Kind>(last));
  }

  Choice choice() const {
    Choice chosen;
    chosen.kind = m_kind;
    chosen.units = m_units->currentData().toString();
    chosen.unitsDefault = m_unitsDefault->isChecked();
    return chosen;
  }

  void accept() override {
    QSettings().setValue(QStringLiteral("newDocument/last"), static_cast<int>(m_kind));
    QDialog::accept();
  }

protected:
  // A double-click on a card creates it.
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (event->type() == QEvent::MouseButtonDblClick) {
      auto* card = qobject_cast<QAbstractButton*>(watched);
      if (card != nullptr && card->isEnabled()) {
        card->setChecked(true);
        accept();
        return true;
      }
    }
    return QDialog::eventFilter(watched, event);
  }

private:
  static QString unitName(const QString& unit) {
    if (unit == QLatin1String("mm")) return tr("Millimeter (mm)");
    if (unit == QLatin1String("cm")) return tr("Centimeter (cm)");
    if (unit == QLatin1String("m")) return tr("Meter (m)");
    if (unit == QLatin1String("in")) return tr("Inch (in)");
    if (unit == QLatin1String("ft")) return tr("Foot (ft)");
    return unit;
  }

  void choose(Kind kind) {
    m_kind = kind;
    m_unitsRow->setVisible(kind == Kind::Design || kind == Kind::Import);
    switch (kind) {
    case Kind::Design:
      m_title->setText(tr("Design"));
      m_about->setText(tr("Model bodies and components in one design, and place other designs in it as components."));
      break;
    case Kind::Drawing:
      m_title->setText(tr("Drawing"));
      m_about->setText(tr("A drawing of %1: its views, dimensions and parts list on a sheet.").arg(m_drawingOf));
      break;
    case Kind::Import:
      m_title->setText(tr("Import"));
      m_about->setText(tr("A new design from a STEP, IGES, mesh or other CAD file, chosen next."));
      break;
    case Kind::Project:
      m_title->setText(tr("Project"));
      m_about->setText(tr("A folder of designs kept together with their versions, named and placed next."));
      break;
    }
  }

  // The cards' pictures: flat, in the palette's ink with one blue, as the
  // other picture cards.
  static void cube(QPainter& p, const QPointF& c, double s, const QColor& top, const QColor& left, const QColor& right,
                   const QColor& edge) {
    const QPointF t(c.x(), c.y() - s), r(c.x() + s * 0.87, c.y() - s * 0.5), l(c.x() - s * 0.87, c.y() - s * 0.5);
    const QPointF b(c.x(), c.y() + s), br(c.x() + s * 0.87, c.y() + s * 0.5), bl(c.x() - s * 0.87, c.y() + s * 0.5);
    p.setPen(QPen(edge, 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(top);
    p.drawPolygon(QPolygonF{t, r, c, l});
    p.setBrush(left);
    p.drawPolygon(QPolygonF{l, c, b, bl});
    p.setBrush(right);
    p.drawPolygon(QPolygonF{c, r, br, b});
  }
  static QColor ink(const QPalette& pal, double alpha) {
    QColor color = pal.color(QPalette::WindowText);
    color.setAlphaF(alpha);
    return color;
  }
  static void paintDesign(QPainter& p, const QRectF& r, const QPalette& pal) {
    const QPointF c = r.center();
    const double s = std::min(r.width(), r.height()) * 0.2;
    cube(p, c + QPointF(s * 0.9, s * 0.6), s * 0.75, QColor(0xe9, 0xee, 0xf5), QColor(0xc9, 0xd3, 0xe1),
         QColor(0xb2, 0xc0, 0xd3), ink(pal, 0.6));
    cube(p, c + QPointF(-s * 0.55, -s * 0.15), s, QColor(0xcf, 0xe0, 0xf7), QColor(0x7a, 0xa6, 0xdc),
         QColor(0x4f, 0x80, 0xc4), ink(pal, 0.7));
  }
  static void paintDrawing(QPainter& p, const QRectF& r, const QPalette& pal) {
    const double h = r.height() * 0.66, w = h * 1.35;
    const QRectF sheet(r.center().x() - w / 2, r.center().y() - h / 2, w, h);
    p.setPen(QPen(ink(pal, 0.6), 1.1));
    p.setBrush(pal.color(QPalette::Base));
    p.drawRect(sheet);
    const QRectF block(sheet.right() - w * 0.38, sheet.bottom() - h * 0.2, w * 0.38, h * 0.2);
    p.drawRect(block);
    p.drawLine(QPointF(block.left(), block.center().y()), QPointF(block.right(), block.center().y()));
    p.setPen(QPen(QColor(0x4f, 0x80, 0xc4), 1.3));
    p.setBrush(Qt::NoBrush);
    const double u = h * 0.18;
    const QPointF o(sheet.left() + w * 0.14, sheet.top() + h * 0.16);
    p.drawRect(QRectF(o, QSizeF(u * 1.6, u)));                             // front
    p.drawRect(QRectF(o + QPointF(0, u * 1.4), QSizeF(u * 1.6, u * 1.2)));   // top
    p.drawRect(QRectF(o + QPointF(u * 2.0, 0), QSizeF(u * 1.2, u)));         // side
    p.setPen(QPen(ink(pal, 0.45), 0.9, Qt::DashLine));
    p.drawLine(o + QPointF(-u * 0.3, u * 0.5), o + QPointF(u * 3.5, u * 0.5));
  }
  static void paintImport(QPainter& p, const QRectF& r, const QPalette& pal) {
    const double s = std::min(r.width(), r.height()) * 0.22;
    const QPointF c = r.center();
    // A file with a folded corner, then an arrow into a cube.
    const QRectF file(c.x() - s * 2.3, c.y() - s * 1.2, s * 1.5, s * 1.9);
    QPainterPath page;
    page.moveTo(file.topLeft());
    page.lineTo(file.right() - s * 0.45, file.top());
    page.lineTo(file.right(), file.top() + s * 0.45);
    page.lineTo(file.bottomRight());
    page.lineTo(file.bottomLeft());
    page.closeSubpath();
    p.setPen(QPen(ink(pal, 0.6), 1.1));
    p.setBrush(pal.color(QPalette::Base));
    p.drawPath(page);
    p.setPen(QPen(ink(pal, 0.4), 1.0));
    for (int i = 0; i < 3; ++i) {
      const double y = file.top() + s * (0.8 + 0.35 * i);
      p.drawLine(QPointF(file.left() + s * 0.25, y), QPointF(file.right() - s * 0.25, y));
    }
    p.setPen(QPen(QColor(0x4f, 0x80, 0xc4), 1.8, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    const QPointF a(c.x() - s * 0.55, c.y() - s * 0.25), b(c.x() + s * 0.35, c.y() - s * 0.25);
    p.drawLine(a, b);
    p.drawPolyline(QPolygonF{b + QPointF(-s * 0.3, -s * 0.3), b, b + QPointF(-s * 0.3, s * 0.3)});
    cube(p, c + QPointF(s * 1.55, -s * 0.05), s * 0.9, QColor(0xcf, 0xe0, 0xf7), QColor(0x7a, 0xa6, 0xdc),
         QColor(0x4f, 0x80, 0xc4), ink(pal, 0.7));
  }
  static void paintProject(QPainter& p, const QRectF& r, const QPalette& pal) {
    const double h = r.height() * 0.56, w = h * 1.45;
    const QRectF body(r.center().x() - w / 2, r.center().y() - h / 2 + h * 0.08, w, h * 0.84);
    QPainterPath folder;
    folder.moveTo(body.left(), body.top());
    folder.lineTo(body.left(), body.top() - h * 0.16);
    folder.lineTo(body.left() + w * 0.36, body.top() - h * 0.16);
    folder.lineTo(body.left() + w * 0.44, body.top());
    folder.closeSubpath();
    p.setPen(QPen(ink(pal, 0.6), 1.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    p.setBrush(QColor(0xe6, 0xd3, 0xa3));
    p.drawPath(folder);
    p.setBrush(QColor(0xf2, 0xe2, 0xb8));
    p.drawRoundedRect(body, 2, 2);
    cube(p, body.center() + QPointF(0, h * 0.02), h * 0.2, QColor(0xcf, 0xe0, 0xf7), QColor(0x7a, 0xa6, 0xdc),
         QColor(0x4f, 0x80, 0xc4), ink(pal, 0.7));
  }

  QString m_drawingOf;
  QList<QAbstractButton*> m_cards;
  Kind m_kind = Kind::Design;
  QLabel* m_title = nullptr;
  QLabel* m_about = nullptr;
  QWidget* m_unitsRow = nullptr;
  QComboBox* m_units = nullptr;
  QCheckBox* m_unitsDefault = nullptr;
  QPushButton* m_create = nullptr;
};

} // namespace mitcad
