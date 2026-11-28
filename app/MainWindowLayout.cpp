// SPDX-License-Identifier: MIT
// Fork: Preferences › Layout (view/LayoutSettings.hpp, SolidCraft's Layout
// page) applied to the window: kept on top of other windows, the cards'
// headers, where notices show, and the navigation bar's place, size,
// labels and how it rests when idle. Applied when the window is made and
// again whenever the page's settings are saved.
#include "MainWindow.hpp"

#include <QEvent>
#include <QLabel>
#include <QStatusBar>
#include <QPointer>
#include <QTimer>

#include "framework/FloatingLayout.hpp"
#include "framework/IconSizeStyle.hpp"
#include "framework/Ribbon.hpp"
#include "framework/GlassCard.hpp"
#include "view/LayoutSettings.hpp"
#include "view/NavigationBar.hpp"

namespace mitcad {
namespace {

// The navigation bar's card at rest (Layout › Navigation bar › When idle):
// 0 always as it is, 1 faded a moment after the pointer leaves it, 2 not
// drawn until the pointer is over it. It comes back in full while hovered.
class NavigationIdle : public QObject {
public:
  explicit NavigationIdle(GlassCard* card) : QObject(card), m_card(card) {
    setObjectName(QStringLiteral("navigationIdle"));
    m_rest.setSingleShot(true);
    m_rest.setInterval(1500);
    QObject::connect(&m_rest, &QTimer::timeout, this, [this] { rest(); });
    card->installEventFilter(this);
  }
  void setMode(int mode) {
    m_mode = mode;
    m_rest.stop();
    if (m_mode == 0 || m_card->underMouse()) {
      m_card->setOpacityLevel(1.0);
    } else {
      rest();
    }
  }

protected:
  bool eventFilter(QObject* watched, QEvent* event) override {
    if (watched == m_card && m_mode != 0) {
      if (event->type() == QEvent::Enter) {
        m_rest.stop();
        m_card->setOpacityLevel(1.0);
      } else if (event->type() == QEvent::Leave) {
        m_rest.start();
      }
    }
    return QObject::eventFilter(watched, event);
  }

private:
  void rest() {
    if (m_mode != 0 && !m_card->underMouse()) {
      m_card->setOpacityLevel(m_mode == 1 ? 0.35 : 0.0);
    }
  }
  GlassCard* m_card;
  QTimer m_rest;
  int m_mode = 0;
};

} // namespace

void MainWindow::applyLayoutSettings(const LayoutSettings& layout) {
  // Window › Keep on top of other windows. Changing a window's flags hides
  // it; it shows again where it was.
  const bool onTop = windowFlags().testFlag(Qt::WindowStaysOnTopHint);
  if (onTop != layout.keepOnTop) {
    const bool shown = isVisible();
    setWindowFlag(Qt::WindowStaysOnTopHint, layout.keepOnTop);
    if (shown) {
      show();
    }
  }
  // Window › Show window control buttons: the title bar's minimize,
  // maximize and close (the system's frame keeps its title).
  // Changed only when asked for: the window keeps the system's own flags
  // otherwise.
  const bool customized = windowFlags().testFlag(Qt::CustomizeWindowHint);
  if (layout.windowButtons == customized) {
    const bool shown = isVisible();
    Qt::WindowFlags flags = windowFlags();
    if (layout.windowButtons) {
      flags &= ~(Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint);
    } else {
      flags |= Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
      flags &= ~(Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    }
    setWindowFlags(flags);
    if (shown) {
      show();
    }
  }
  // Window › Menus: List or Large icons (menus made from now on).
  IconSizeStyle::setLargeMenuIcons(layout.largeMenuIcons);
  // Window › Ribbon.
  if (m_ribbon != nullptr) {
    m_ribbon->setGroupsShown(layout.ribbon);
  }
  // Window › Status bar: the Floating layout's status line at the bottom of
  // the window (the Docked layout always has one: it holds the navigation
  // bar).
  if (m_floating != nullptr) {
    m_layoutStatusBar = layout.statusBar;
    statusBar()->setSizeGripEnabled(false);
    statusBar()->setVisible(layout.statusBar);
    echoStatusLine();
  }
  // Window › Card headers, Notices.
  GlassCard::setHeaderMode(layout.cardHeaders);
  if (m_floating != nullptr) {
    m_floating->setBandAtRight(!layout.noticesCentered);
  }
  // Navigation bar › Position, When idle, Size, Buttons.
  if (m_navigationBar != nullptr) {
    m_navigationBar->setLook(layout.navigationCompact, layout.navigationLabels);
  }
  if (m_navigationCard != nullptr && m_floating != nullptr) {
    FloatingLayout::Sizing sizing;
    sizing.align = layout.navigationPosition % 3;
    sizing.atTop = layout.navigationPosition < 3;
    sizing.shown = m_navigationCard->isShown();
    m_floating->setCardAnchor(m_navigationCard, FloatingLayout::Anchor::BottomCenter, sizing);
    auto* idle = m_navigationCard->findChild<NavigationIdle*>(QStringLiteral("navigationIdle"),
                                                                 Qt::FindDirectChildrenOnly);
    if (idle == nullptr) {
      idle = new NavigationIdle(m_navigationCard);
    }
    idle->setMode(layout.navigationIdle);
    m_floating->requestLayout();
  }
  m_searchCentered = layout.searchCentered;
  qDebug().noquote() << QStringLiteral("Layout applied: on top %1, card headers %2, notices %3, navigation bar %4 idle %5%6%7")
                            .arg(layout.keepOnTop ? QStringLiteral("yes") : QStringLiteral("no"))
                            .arg(layout.cardHeaders)
                            .arg(layout.noticesCentered ? QStringLiteral("centre") : QStringLiteral("right"))
                            .arg(layout.navigationPosition)
                            .arg(layout.navigationIdle)
                            .arg(layout.navigationCompact ? QStringLiteral(" compact") : QString())
                            .arg(layout.navigationLabels ? QStringLiteral(" labels") : QString());
}

void MainWindow::echoStatusLine() {
  if (m_layoutStatusBar && m_floating != nullptr && m_statusLabel != nullptr) {
    statusBar()->showMessage(m_statusLabel->text());
  }
}

void MainWindow::followLayoutSettings() {
  applyLayoutSettings(LayoutSettings::load());
  QPointer<MainWindow> self(this);
  layoutSettingsListeners().push_back([self](const LayoutSettings& layout) {
    if (self != nullptr) {
      self->applyLayoutSettings(layout);
    }
  });
}

} // namespace mitcad
