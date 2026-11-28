// SPDX-License-Identifier: MIT
#pragma once

// Fork: the parts SolidCraft's Preferences pages are made of, shared by the
// pages made after its (Graphics, Appearance, Design, Sketch, Tools,
// Shortcuts): a page that scrolls in sections that fold
// (stylePreferencesPage folds the QGroupBoxes), rows of joined segments,
// check boxes with an ⓘ that holds what they do, picture cards (a drawn
// picture, a name and a caption, a check badge when chosen; one of a row
// or each on its own), a slider with its value in a box, and a caption
// under a row's name. A page calls changed() when a choice changes; the
// dialog's Apply and OK call commit() (PreferenceKitPage::commit).

#include <algorithm>
#include <functional>
#include <tuple>
#include <utility>

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QScrollArea>
#include <QSlider>
#include <QStyle>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>

#include "../framework/UiSizes.hpp"

namespace mitcad {

// A card that shows a choice as a picture: `paint` draws it in the card's
// upper part; the name (bold when chosen) and a caption under it. Chosen,
// the card's border is the window text's colour, 2 px, and a round check
// badge sits at its upper right; one that is checked on its own (a row of
// independent cards) shows an empty square there when not chosen.
class PictureCard : public QAbstractButton {
public:
  using Painter = std::function<void(QPainter&, const QRectF&, const QPalette&)>;

  PictureCard(const QString& name, const QString& caption, Painter paint, bool squareWhenOff, QWidget* parent = nullptr)
      : QAbstractButton(parent), m_caption(caption), m_paint(std::move(paint)), m_square(squareWhenOff) {
    setText(name);
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(uiRoomSize(150), uiRoomSize(caption.isEmpty() ? 112 : 128));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  }
  QSize sizeHint() const override { return QSize(uiRoomSize(170), minimumHeight()); }
  // A smaller card (Curved surfaces, Anti-aliasing).
  void setCompact(int width, int height) {
    setMinimumSize(uiRoomSize(width), uiRoomSize(height));
    m_compactWidth = uiRoomSize(width);
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    const QColor ink = pal.color(QPalette::WindowText);
    const QRectF card = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
    QColor fill = ink;
    fill.setAlphaF(isChecked() ? 0.07f : (underMouse() ? 0.05f : 0.025f));
    QColor edge = ink;
    edge.setAlphaF(isChecked() ? 0.9f : 0.16f);
    p.setPen(QPen(edge, isChecked() ? 2.0 : 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(card, 10, 10);
    // The picture's well, a shade darker, over the name.
    const double textHeight = m_caption.isEmpty() ? 26.0 : 40.0;
    const QRectF well = card.adjusted(6, 6, -6, -textHeight);
    QColor wellFill = ink;
    wellFill.setAlphaF(0.035f);
    p.setPen(Qt::NoPen);
    p.setBrush(wellFill);
    p.drawRoundedRect(well, 7, 7);
    if (m_paint) {
      p.save();
      m_paint(p, well, pal);
      p.restore();
    }
    // The name and the caption.
    QFont name = font();
    name.setBold(isChecked());
    p.setFont(name);
    p.setPen(ink);
    const QRectF nameBox(card.left(), well.bottom() + 3, card.width(), 20);
    p.drawText(nameBox, Qt::AlignHCenter | Qt::AlignVCenter, text());
    if (!m_caption.isEmpty()) {
      QFont small = font();
      small.setPointSizeF(std::max(7.0, small.pointSizeF() * 0.88));
      p.setFont(small);
      QColor quiet = ink;
      quiet.setAlphaF(0.65f);
      p.setPen(quiet);
      p.drawText(QRectF(card.left(), nameBox.bottom() - 2, card.width(), 16), Qt::AlignHCenter | Qt::AlignTop,
                 m_caption);
    }
    // The badge: a round check when chosen, an empty square for an
    // independent card that is not.
    const QRectF badge(card.right() - 24, card.top() + 7, 17, 17);
    if (isChecked()) {
      p.setPen(Qt::NoPen);
      p.setBrush(pal.color(QPalette::Highlight));
      p.drawEllipse(badge);
      p.setPen(QPen(pal.color(QPalette::HighlightedText), 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
      p.drawPolyline(QPolygonF{badge.center() + QPointF(-4.0, 0.2), badge.center() + QPointF(-1.2, 3.0),
                               badge.center() + QPointF(4.2, -3.0)});
    } else if (m_square) {
      QColor box = ink;
      box.setAlphaF(0.35f);
      p.setPen(QPen(box, 1.2));
      p.setBrush(pal.color(QPalette::Base));
      p.drawRoundedRect(badge.adjusted(1, 1, -1, -1), 3, 3);
    }
  }
  void enterEvent(QEnterEvent* event) override {
    update();
    QAbstractButton::enterEvent(event);
  }
  void leaveEvent(QEvent* event) override {
    update();
    QAbstractButton::leaveEvent(event);
  }

private:
  QString m_caption;
  Painter m_paint;
  bool m_square = false;
  int m_compactWidth = 0;
};

// The base of the pages: a scroll area of sections, and the rows' makers.
class PreferenceKitPage : public QWidget {
  Q_DECLARE_TR_FUNCTIONS(mitcad::PreferenceKitPage)

public:
  explicit PreferenceKitPage(const QString& name, QWidget* parent = nullptr) : QWidget(parent), m_name(name) {
    setObjectName(name + QStringLiteral("Preferences"));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    m_scroll = new QScrollArea;
    m_scroll->setObjectName(name + QStringLiteral("Scroll"));
    m_scroll->setWidgetResizable(true);
    m_scroll->setFrameShape(QFrame::NoFrame);
    m_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    auto* content = new QWidget;
    m_column = new QVBoxLayout(content);
    m_column->setContentsMargins(0, 0, 10, 0);
    m_column->setSpacing(4);
    m_scroll->setWidget(content);
    m_scroll->viewport()->setAutoFillBackground(false);
    outer->addWidget(m_scroll);
  }

  // Up to 640 px of the content's height and its whole width.
  QSize sizeHint() const override {
    if (m_scroll->widget() == nullptr) return QWidget::sizeHint();
    const QSize content = m_scroll->widget()->sizeHint();
    const int bar = style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    return QSize(content.width() + bar + 4, std::min(content.height(), uiRoomSize(640)));
  }

  // Called when a choice changes (Preferences' Settings window commits at
  // once with it).
  std::function<void()> onChanged;

protected:
  void finish() { m_column->addStretch(1); }
  void changed() {
    if (onChanged) onChanged();
  }
  QString name(const char* part) const { return m_name + QLatin1Char('_') + QLatin1String(part); }

  QVBoxLayout* section(const QString& title) {
    auto* box = new QGroupBox(title);
    auto* column = new QVBoxLayout(box);
    column->setContentsMargins(0, 4, 0, 12);
    column->setSpacing(8);
    m_column->addWidget(box);
    return column;
  }
  // A caption in capitals inside a section ("WHEN A SKETCH ENDS").
  void subheading(QVBoxLayout* column, const QString& text) {
    auto* label = new QLabel(text.toUpper());
    QFont font = label->font();
    font.setBold(true);
    font.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
    label->setFont(font);
    column->addSpacing(4);
    column->addWidget(label);
  }
  QFormLayout* form(QVBoxLayout* column) {
    auto* f = new QFormLayout;
    f->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    f->setLabelAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    f->setHorizontalSpacing(28);
    f->setVerticalSpacing(10);
    column->addLayout(f);
    return f;
  }
  // A row's name with a caption under it, for a form's label column.
  QWidget* label(const QString& title, const QString& caption = QString(), const QString& tip = QString()) {
    auto* box = new QWidget;
    auto* column = new QVBoxLayout(box);
    column->setContentsMargins(0, 0, 0, 0);
    column->setSpacing(2);
    auto* top = new QHBoxLayout;
    top->setSpacing(6);
    top->addWidget(new QLabel(title));
    if (!tip.isEmpty()) top->addWidget(info(tip));
    top->addStretch(1);
    column->addLayout(top);
    if (!caption.isEmpty()) {
      auto* note = new QLabel(caption);
      note->setWordWrap(true);
      note->setForegroundRole(QPalette::PlaceholderText);
      column->addWidget(note);
      // Room for the caption to wrap in a few lines, not one word a line.
      box->setFixedWidth(uiRoomSize(250));
    } else {
      box->setMinimumWidth(box->sizeHint().width());
    }
    return box;
  }
  // The ⓘ after a row: what it does, in its tooltip.
  QLabel* info(const QString& tip) {
    auto* mark = new QLabel(QStringLiteral("ⓘ"));
    mark->setToolTip(tip);
    mark->setForegroundRole(QPalette::PlaceholderText);
    mark->setCursor(Qt::WhatsThisCursor);
    return mark;
  }
  // A check box bound to a value, with its ⓘ when `tip` says what it does.
  QCheckBox* check(QVBoxLayout* column, const QString& title, const char* part, bool value,
                   std::function<void(bool)> apply, const QString& tip = QString()) {
    auto* row = new QHBoxLayout;
    row->setSpacing(6);
    QCheckBox* box = makeCheck(title, part, value, std::move(apply));
    row->addWidget(box);
    if (!tip.isEmpty()) row->addWidget(info(tip));
    row->addStretch(1);
    column->addLayout(row);
    return box;
  }
  QCheckBox* makeCheck(const QString& title, const char* part, bool value, std::function<void(bool)> apply) {
    auto* box = new QCheckBox(title);
    box->setObjectName(name(part));
    box->setChecked(value);
    connect(box, &QCheckBox::toggled, this, [this, apply](bool on) {
      apply(on);
      changed();
    });
    return box;
  }
  // Joined segments; the buttons' indexes are the values.
  QWidget* segments(const char* part, const QStringList& names, int value, std::function<void(int)> apply,
                    bool stretch = false) {
    auto* row = new QWidget;
    row->setObjectName(name(part));
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    line->setSpacing(0);
    auto* group = new QButtonGroup(row);
    const QPalette pal = palette();
    for (int i = 0; i < names.size(); ++i) {
      auto* button = new QPushButton(names.at(i));
      button->setCheckable(true);
      button->setChecked(i == value);
      // As wide as its name in bold, which it shows when chosen.
      QFont bold = button->font();
      bold.setBold(true);
      button->setMinimumWidth(QFontMetrics(bold).horizontalAdvance(names.at(i)) + uiRoomSize(32));
      if (stretch) button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      const QString radius = i == 0 ? QStringLiteral("border-top-left-radius: 6px; border-bottom-left-radius: 6px;")
                             : i == names.size() - 1
                                 ? QStringLiteral("border-left: none; border-top-right-radius: 6px; "
                                                  "border-bottom-right-radius: 6px;")
                                 : QStringLiteral("border-left: none;");
      button->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; padding: 5px 14px; background: %2; %4 } "
                                           "QPushButton:checked { background: %3; font-weight: 600; }")
                                .arg(pal.color(QPalette::Mid).name(), pal.color(QPalette::Base).name(),
                                     pal.color(QPalette::Mid).lighter(135).name(), radius));
      group->addButton(button, i);
      line->addWidget(button);
    }
    connect(group, &QButtonGroup::idClicked, this, [this, apply](int id) {
      apply(id);
      changed();
    });
    row->setMinimumHeight(row->sizeHint().height()); // never squeezed by a row of combo boxes
    return row;
  }
  // A row of picture cards, one of which is chosen.
  QHBoxLayout* choiceCards(QVBoxLayout* column, const char* part,
                           const QVector<std::tuple<QString, QString, PictureCard::Painter>>& cards, int value,
                           std::function<void(int)> apply, int width = 0, int height = 0) {
    auto* row = new QHBoxLayout;
    row->setSpacing(12);
    auto* group = new QButtonGroup(this);
    group->setExclusive(true);
    for (int i = 0; i < cards.size(); ++i) {
      const auto& [title, caption, paint] = cards.at(i);
      auto* card = new PictureCard(title, caption, paint, false);
      card->setObjectName(name(part) + QLatin1Char('_') + QString::number(i));
      if (width > 0) card->setCompact(width, height);
      card->setChecked(i == value);
      group->addButton(card, i);
      row->addWidget(card);
    }
    row->addStretch(1);
    connect(group, &QButtonGroup::idClicked, this, [this, apply](int id) {
      apply(id);
      changed();
    });
    column->addLayout(row);
    return row;
  }
  // A picture card checked on its own (a row of independent choices).
  PictureCard* toggleCard(QHBoxLayout* row, const char* part, const QString& title, PictureCard::Painter paint,
                          bool value, std::function<void(bool)> apply) {
    auto* card = new PictureCard(title, QString(), std::move(paint), true);
    card->setObjectName(name(part));
    card->setChecked(value);
    connect(card, &QAbstractButton::toggled, this, [this, apply](bool on) {
      apply(on);
      changed();
    });
    row->addWidget(card);
    return card;
  }
  // A slider with its value in a box at the right ("100 %", "34°").
  QWidget* slider(const char* part, int low, int high, int value, std::function<QString(int)> text,
                  std::function<void(int)> apply) {
    auto* row = new QWidget;
    row->setObjectName(name(part));
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    line->setSpacing(14);
    auto* bar = new QSlider(Qt::Horizontal);
    bar->setRange(low, high);
    bar->setValue(value);
    bar->setMinimumWidth(uiRoomSize(260));
    auto* shown = new QLabel(text(value));
    shown->setAlignment(Qt::AlignCenter);
    shown->setMinimumWidth(uiRoomSize(64));
    shown->setStyleSheet(QStringLiteral("QLabel { border: 1px solid %1; border-radius: 6px; padding: 3px 8px; }")
                             .arg(palette().color(QPalette::Mid).name()));
    connect(bar, &QSlider::valueChanged, this, [this, shown, text, apply](int v) {
      shown->setText(text(v));
      apply(v);
      changed();
    });
    line->addWidget(bar, 1);
    line->addWidget(shown);
    return row;
  }
  // Small rounded chips that each turn on and off (Status chips).
  QWidget* chips(const char* part, const QStringList& names, const QVector<bool>& values,
                 std::function<void(int, bool)> apply) {
    auto* row = new QWidget;
    row->setObjectName(name(part));
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    line->setSpacing(6);
    const QPalette pal = palette();
    for (int i = 0; i < names.size(); ++i) {
      auto* chip = new QPushButton(names.at(i));
      chip->setCheckable(true);
      chip->setChecked(i < values.size() && values.at(i));
      chip->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; border-radius: 12px; padding: 4px 12px; "
                                         "background: %2; } QPushButton:checked { border: 1px solid %3; "
                                         "background: %4; }")
                              .arg(pal.color(QPalette::Mid).name(), pal.color(QPalette::Base).name(),
                                   pal.color(QPalette::WindowText).name(), pal.color(QPalette::Mid).lighter(140).name()));
      connect(chip, &QPushButton::toggled, this, [this, apply, i](bool on) {
        apply(i, on);
        changed();
      });
      line->addWidget(chip);
    }
    line->addStretch(1);
    return row;
  }
  // A small rounded tag ("Also in the view bar", "Restart needed").
  QLabel* tag(const QString& text, const QColor& tint) {
    auto* label = new QLabel(text);
    QColor back = tint;
    back.setAlphaF(0.16f);
    label->setStyleSheet(QStringLiteral("QLabel { background: %1; color: %2; border-radius: 9px; padding: 2px 9px; "
                                        "font-weight: 600; }")
                             .arg(back.name(QColor::HexArgb), tint.darker(130).name()));
    return label;
  }

  QVBoxLayout* m_column = nullptr;

private:
  QString m_name;
  QScrollArea* m_scroll = nullptr;
};

} // namespace mitcad
