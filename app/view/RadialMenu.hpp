// SPDX-License-Identifier: MIT
#pragma once

// Fork: SolidCraft's navigation menus (Preferences › Navigation):
// - the radial context menu: the right click's commands in a ring around
//   the cursor, a submenu or what does not fit opening as a list;
// - the gesture ring: a right drag's ring of tools (the SolidWorks mouse
//   mapping), the tool the drag points at run as the button goes up;
// - the quick menu: a few commands for what was just selected, in a small
//   bar beside the cursor that goes after a while or a click elsewhere.

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QEventLoop>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QScreen>
#include <QTimer>
#include <QToolButton>
#include <QVector>
#include <QWidget>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <functional>
#include <optional>

namespace mitcad {

// A ring of entries around a centre, each a name (and icon) in a pill; one
// highlighted. Shared by the radial menu and the gesture ring.
class RingOfEntries : public QWidget {
public:
  struct Entry {
    QString text;
    QIcon icon;
    bool enabled = true;
  };

  explicit RingOfEntries(QWidget* parent = nullptr) : QWidget(parent, Qt::Popup | Qt::FramelessWindowHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose, false);
    setMouseTracking(true);
    resize(kSize, kSize);
  }

  void setEntries(const QVector<Entry>& entries) {
    m_entries = entries;
    update();
  }
  // Centred on `center` (global), kept on its screen.
  void showAt(const QPoint& center) {
    QPoint topLeft = center - QPoint(kSize / 2, kSize / 2);
    if (const QScreen* screen = QGuiApplication::screenAt(center)) {
      const QRect room = screen->availableGeometry();
      topLeft.setX(std::clamp(topLeft.x(), room.left(), room.right() - kSize));
      topLeft.setY(std::clamp(topLeft.y(), room.top(), room.bottom() - kSize));
    }
    move(topLeft);
    show();
    raise();
  }
  // The entry the direction from the middle to `global` points at, beyond
  // the dead zone in the middle; none inside it.
  std::optional<int> entryAt(const QPoint& global) const {
    if (m_entries.isEmpty()) return std::nullopt;
    const QPointF from = QPointF(mapFromGlobal(global)) - QPointF(width() / 2.0, height() / 2.0);
    if (std::hypot(from.x(), from.y()) < kDeadZone) return std::nullopt;
    // Clockwise from the top.
    double angle = std::atan2(from.x(), -from.y());
    if (angle < 0) angle += 2 * M_PI;
    const double slice = 2 * M_PI / m_entries.size();
    const int index = int(std::floor((angle + slice / 2) / slice)) % int(m_entries.size());
    return m_entries.at(index).enabled ? std::optional<int>(index) : std::nullopt;
  }
  void setHighlighted(std::optional<int> index) {
    if (index != m_highlighted) {
      m_highlighted = index;
      update();
    }
  }
  std::optional<int> highlighted() const { return m_highlighted; }

  static constexpr int kSize = 420;
  static constexpr int kDeadZone = 26;

protected:
  QRectF pillRect(int index) const {
    const double angle = 2 * M_PI * index / std::max<qsizetype>(1, m_entries.size());
    const QPointF c(width() / 2.0 + 118 * std::sin(angle), height() / 2.0 - 118 * std::cos(angle));
    const QFontMetrics metrics(font());
    const double w = std::min(150.0, double(metrics.horizontalAdvance(m_entries.at(index).text)) + 44);
    return QRectF(c.x() - w / 2, c.y() - 15, w, 30);
  }
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    const QPointF c(width() / 2.0, height() / 2.0);
    // The middle: where the drag or click started.
    QColor hub = pal.color(QPalette::Window);
    hub.setAlphaF(0.92f);
    p.setPen(QPen(pal.color(QPalette::Mid), 1));
    p.setBrush(hub);
    p.drawEllipse(c, kDeadZone, kDeadZone);
    if (m_highlighted) {
      const double angle = 2 * M_PI * *m_highlighted / m_entries.size();
      p.setPen(QPen(pal.color(QPalette::Highlight), 3, Qt::SolidLine, Qt::RoundCap));
      p.drawLine(c, c + QPointF(kDeadZone * std::sin(angle), -kDeadZone * std::cos(angle)));
    }
    for (int i = 0; i < m_entries.size(); ++i) {
      const QRectF pill = pillRect(i);
      const bool lit = m_highlighted == i;
      QColor fill = lit ? pal.color(QPalette::Highlight) : pal.color(QPalette::Window);
      fill.setAlphaF(lit ? 1.0f : 0.95f);
      p.setPen(QPen(lit ? fill : pal.color(QPalette::Mid), 1));
      p.setBrush(fill);
      p.drawRoundedRect(pill, 15, 15);
      QColor ink = lit ? pal.color(QPalette::HighlightedText) : pal.color(QPalette::WindowText);
      if (!m_entries.at(i).enabled) ink.setAlphaF(0.4f);
      QRectF text = pill.adjusted(10, 0, -10, 0);
      if (!m_entries.at(i).icon.isNull()) {
        const QRectF icon(pill.left() + 8, pill.center().y() - 9, 18, 18);
        p.drawPixmap(icon.toRect(), m_entries.at(i).icon.pixmap(18, 18, m_entries.at(i).enabled ? QIcon::Normal : QIcon::Disabled));
        text.setLeft(icon.right() + 5);
      }
      p.setPen(ink);
      p.drawText(text, Qt::AlignCenter, QFontMetrics(font()).elidedText(m_entries.at(i).text, Qt::ElideRight, int(text.width())));
    }
  }

  QVector<Entry> m_entries;
  std::optional<int> m_highlighted;
};

// The radial context menu: the menu's commands (up to eight, the rest under
// More…) in a ring; a click on one runs it, a submenu opens as a list, a
// click in the middle, outside or Esc closes it. Returns the action run.
class RadialMenuPopup : public RingOfEntries {
public:
  explicit RadialMenuPopup(QMenu* menu) : m_menu(menu) {
    QList<QAction*> actions;
    for (QAction* action : menu->actions()) {
      if (!action->isSeparator() && action->isVisible()) actions.append(action);
    }
    QVector<Entry> entries;
    const int shown = actions.size() > 8 ? 7 : int(actions.size());
    for (int i = 0; i < shown; ++i) {
      QString text = actions.at(i)->text();
      text.remove(QLatin1Char('&'));
      if (actions.at(i)->menu() != nullptr) text += QStringLiteral(" ▸");
      entries.append({text, actions.at(i)->icon(), actions.at(i)->isEnabled()});
      m_actions.append(actions.at(i));
    }
    if (actions.size() > 8) {
      entries.append({QObject::tr("More…"), QIcon(), true});
      m_actions.append(nullptr); // the whole menu as a list
    }
    setEntries(entries);
  }

  QAction* exec(const QPoint& center) {
    m_center = center;
    showAt(center);
    QEventLoop loop;
    m_loop = &loop;
    loop.exec();
    m_loop = nullptr;
    hide();
    return m_chosen;
  }

protected:
  void mouseMoveEvent(QMouseEvent* event) override { setHighlighted(entryAt(event->globalPosition().toPoint())); }
  void mouseReleaseEvent(QMouseEvent* event) override {
    const std::optional<int> index = entryAt(event->globalPosition().toPoint());
    if (!rect().contains(event->position().toPoint()) || !index) {
      finish(nullptr);
      return;
    }
    QAction* action = m_actions.at(*index);
    hide();
    if (action == nullptr) {
      m_chosen = m_menu->exec(event->globalPosition().toPoint());
    } else if (action->menu() != nullptr) {
      m_chosen = action->menu()->exec(event->globalPosition().toPoint());
    } else {
      m_chosen = action;
      action->trigger();
    }
    finish(m_chosen);
  }
  void keyPressEvent(QKeyEvent* event) override {
    if (event->key() == Qt::Key_Escape) finish(nullptr);
  }
  void hideEvent(QHideEvent* event) override {
    RingOfEntries::hideEvent(event);
    if (m_loop != nullptr && !m_choosing) QTimer::singleShot(0, m_loop, [loop = m_loop] { loop->quit(); });
  }

private:
  void finish(QAction* chosen) {
    m_chosen = chosen;
    if (m_loop != nullptr) m_loop->quit();
  }
  QMenu* m_menu;
  QVector<QAction*> m_actions;
  QPoint m_center;
  QAction* m_chosen = nullptr;
  QEventLoop* m_loop = nullptr;
  bool m_choosing = false;
};

inline QAction* execRadialMenu(QMenu* menu, const QPoint& center) {
  RadialMenuPopup popup(menu);
  qDebug().noquote() << QStringLiteral("Radial context menu at %1,%2").arg(center.x()).arg(center.y());
  return popup.exec(center);
}

// The gesture ring: shown as a right drag starts, the view passes it the
// drag's moves and its release; it runs the tool pointed at.
class GestureRing : public RingOfEntries {
public:
  // `commands`: id and action of each place, clockwise from the top (none
  // where a place is empty).
  explicit GestureRing(const QVector<QPair<QString, QAction*>>& commands) : m_commands(commands) {
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::WindowTransparentForInput);
    setAttribute(Qt::WA_ShowWithoutActivating);
    QVector<Entry> entries;
    for (const auto& [id, action] : commands) {
      if (action == nullptr) {
        entries.append({QString(), QIcon(), false});
        continue;
      }
      QString text = action->text();
      text.remove(QLatin1Char('&'));
      text.remove(QChar(0x2026));
      entries.append({text, action->icon(), action->isEnabled()});
    }
    setEntries(entries);
  }
  void follow(const QPoint& global) { setHighlighted(entryAt(global)); }
  // The tool pointed at as the button goes up, run; the ring goes.
  void release(const QPoint& global) {
    const std::optional<int> index = entryAt(global);
    hide();
    if (index && m_commands.at(*index).second != nullptr) {
      qDebug().noquote() << QStringLiteral("Gesture ring: %1").arg(m_commands.at(*index).first);
      m_commands.at(*index).second->trigger();
    } else {
      qDebug().noquote() << "Gesture ring: nothing";
    }
  }

private:
  QVector<QPair<QString, QAction*>> m_commands;
};

// The quick menu: a small bar of buttons beside the cursor; it goes after a
// few seconds without the pointer on it, at a click elsewhere, or when one
// of its buttons runs.
class QuickMenuBar : public QWidget {
public:
  explicit QuickMenuBar(const QList<QAction*>& actions)
      : QWidget(nullptr, Qt::ToolTip | Qt::FramelessWindowHint) {
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_DeleteOnClose);
    setObjectName(QStringLiteral("quickMenu"));
    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(8, 6, 8, 6);
    row->setSpacing(2);
    for (QAction* action : actions) {
      auto* button = new QToolButton;
      button->setDefaultAction(action);
      button->setAutoRaise(true);
      button->setIconSize(QSize(22, 22));
      button->setFocusPolicy(Qt::NoFocus);
      connect(action, &QAction::triggered, this, &QWidget::close);
      row->addWidget(button);
    }
    m_timer.setSingleShot(true);
    m_timer.setInterval(4000);
    connect(&m_timer, &QTimer::timeout, this, [this] {
      if (!underMouse()) close();
    });
    qApp->installEventFilter(this);
  }
  ~QuickMenuBar() override { qApp->removeEventFilter(this); }
  void showNear(const QPoint& global) {
    adjustSize();
    QPoint at = global + QPoint(18, 18);
    if (const QScreen* screen = QGuiApplication::screenAt(global)) {
      const QRect room = screen->availableGeometry();
      at.setX(std::clamp(at.x(), room.left(), room.right() - width()));
      at.setY(std::clamp(at.y(), room.top(), room.bottom() - height()));
    }
    move(at);
    show();
    m_timer.start();
  }

protected:
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (event->type() == QEvent::MouseButtonPress) {
      auto* widget = qobject_cast<QWidget*>(watched);
      if (widget == nullptr || (widget != this && !isAncestorOf(widget))) {
        QTimer::singleShot(0, this, &QWidget::close);
      }
    }
    return QWidget::eventFilter(watched, event);
  }
  void leaveEvent(QEvent* event) override {
    QWidget::leaveEvent(event);
    m_timer.start(1500);
  }
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QColor fill = palette().color(QPalette::Window);
    fill.setAlphaF(0.97f);
    p.setPen(QPen(palette().color(QPalette::Mid), 1));
    p.setBrush(fill);
    p.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 10, 10);
  }

private:
  QTimer m_timer;
};

} // namespace mitcad
