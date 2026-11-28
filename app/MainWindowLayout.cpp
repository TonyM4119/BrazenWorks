// SPDX-License-Identifier: AGPL-3.0-or-later
// Fork: Preferences › Layout (view/LayoutSettings.hpp, SolidCraft's Layout
// page) applied to the window: kept on top of other windows, the cards'
// headers, where notices show, and the navigation bar's place, size,
// labels and how it rests when idle. Applied when the window is made and
// again whenever the page's settings are saved.
#include "MainWindow.hpp"

#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QLineEdit>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QRegularExpression>
#include <QInputDialog>
#include <QEvent>
#include <QLabel>
#include <QStatusBar>
#include <QPointer>
#include <QTimer>
#include <QToolButton>

#include "framework/AppTheme.hpp"
#include "framework/CommandRegistry.hpp"
#include "framework/FloatingLayout.hpp"
#include "framework/IconSizeStyle.hpp"
#include "framework/Ribbon.hpp"
#include "framework/GlassCard.hpp"
#include "browser/BrowserController.hpp"
#include "browser/BrowserPanel.hpp"
#include "browser/TimelineWidget.hpp"
#include "view/LayoutPreferences.hpp"
#include "view/GraphicsSettings.hpp"
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

// A card's idle look (above), made the first time. No Q_OBJECT here, so not
// found by findChild<NavigationIdle*>: by its name among the card's own
// children.
NavigationIdle* idleOf(GlassCard* card) {
  for (QObject* child : card->children()) {
    if (child->objectName() == QLatin1String("navigationIdle")) {
      return static_cast<NavigationIdle*>(child);
    }
  }
  return new NavigationIdle(card);
}

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
  // Window › Ribbon; Toolbar tabs › Keep the Sketch tab shown.
  if (m_ribbon != nullptr) {
    m_ribbon->setGroupsShown(layout.ribbon);
    m_ribbon->setKeepSketchTab(layout.keepSketchTab);
  }
  // Window › Ribbon › Only when hovered: View › Toolbar Panel hidden, shown
  // at the view's top edge (MainWindowToolbarPanel.cpp).
  if (layout.ribbonOnHover != m_toolbarPanelHidden) {
    setToolbarPanelHiddenEverywhere(layout.ribbonOnHover);
  }
  // Window › Other panels: the Browser's and the History's cards hidden
  // until the pointer is over them, as the navigation bar's can be.
  for (QWidget* panel : {findChild<QWidget*>(QStringLiteral("browserDock")), static_cast<QWidget*>(m_timelineCard.data())}) {
    if (auto* card = qobject_cast<GlassCard*>(panel)) {
      idleOf(card)->setMode(layout.otherPanelsOnHover ? 2 : 0);
    }
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
    // Navigation bar › Items.
    m_navigationBar->setItems(layout.navigationItems.isEmpty() ? LayoutPreferences::defaultNavigationItems()
                                                               : layout.navigationItems);
  }
  if (m_navigationCard != nullptr && m_floating != nullptr) {
    FloatingLayout::Sizing sizing;
    sizing.align = layout.navigationPosition % 3;
    sizing.atTop = layout.navigationPosition < 3;
    sizing.shown = m_navigationCard->isShown();
    m_floating->setCardAnchor(m_navigationCard, FloatingLayout::Anchor::BottomCenter, sizing);
    idleOf(m_navigationCard)->setMode(layout.navigationIdle);
    m_floating->requestLayout();
  }
  m_searchCentered = layout.searchCentered;
  // Panels › Browser tree: the Browser's card or dock.
  if (auto* browser = findChild<QWidget*>(QStringLiteral("browserDock"))) {
    if (auto* card = qobject_cast<GlassCard*>(browser)) {
      card->setSuppressed(!layout.browserTree);
    } else {
      browser->setVisible(layout.browserTree);
    }
  }
  // Panels › History: the Play button.
  if (m_controller != nullptr && m_controller->timeline() != nullptr) {
    m_controller->timeline()->setPlayButtonShown(layout.playHistoryButton);
    // Fold the History bar into groups: turned on, the groups there fold.
    if (layout.foldHistoryGroups && !timelineFoldsGroups()) {
      for (const TimelineWidget::ShownGroup& group : m_controller->timeline()->shownGroups()) {
        m_controller->timeline()->setGroupCollapsed(group.name, true);
      }
    }
    timelineFoldsGroups() = layout.foldHistoryGroups;
  }
  // Panels › Browser: how rows start, for the rows made from now on.
  browserOpenDefaults().componentsOpen = layout.componentsStartOpen;
  browserOpenDefaults().rowsClosed = layout.rowsStartClosed;
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
  // PureCAD: what Preferences › Graphics draws through the window's own
  // highlights (joint symbols) drawn again once it is saved.
  graphicsSettingsListeners().push_back([self](const GraphicsSettings&) {
    if (self != nullptr && !self->modelBusy()) {
      self->updateIdleView();
    }
  });
}

void MainWindow::markActiveCommand(const QString& id) {
  // The buttons tinted before go back to their own look.
  const QList<QToolButton*> buttons = findChildren<QToolButton*>();
  for (QToolButton* button : buttons) {
    if (button->property("purecadActiveCommand").toBool()) {
      button->setStyleSheet(button->property("purecadOwnStyle").toString());
      button->setProperty("purecadActiveCommand", false);
    }
  }
  if (id.isEmpty() || !LayoutSettings::load().highlightActiveCommands) {
    return;
  }
  const QAction* action = m_registry->action(id);
  if (action == nullptr) {
    return;
  }
  QColor tint = accentColor(palette());
  tint.setAlpha(60);
  for (QToolButton* button : buttons) {
    if (button->defaultAction() == action && button->isVisible()) {
      button->setProperty("purecadOwnStyle", button->styleSheet());
      button->setProperty("purecadActiveCommand", true);
      button->setStyleSheet(button->styleSheet() +
                            QStringLiteral(" QToolButton { background: %1; border-radius: 6px; }")
                                .arg(tint.name(QColor::HexArgb)));
    }
  }
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef Q_OS_WIN
  if (eventType == "windows_generic_MSG" && message != nullptr && m_viewer != nullptr) {
    // A MSG starts with its window and its message id (no <windows.h> here).
    struct MessageHead {
      void* window;
      unsigned int id;
    };
    constexpr unsigned int kEnterSizeMove = 0x0231; // WM_ENTERSIZEMOVE
    constexpr unsigned int kExitSizeMove = 0x0232;  // WM_EXITSIZEMOVE
    const unsigned int id = static_cast<const MessageHead*>(message)->id;
    if (id == kEnterSizeMove && LayoutSettings::load().pauseDrawingWhileDragged) {
      m_dragPaused = true;
      m_viewer->setUpdatesEnabled(false);
    } else if (id == kExitSizeMove && m_dragPaused) {
      m_dragPaused = false;
      m_viewer->setUpdatesEnabled(true);
      m_viewer->update();
    }
  }
#endif
  return QMainWindow::nativeEvent(eventType, message, result);
}

// PureCAD: Preferences › Layout › Double-click renames tab.
void MainWindow::renameFromTab() {
  const QString current = documentName();
  bool ok = false;
  const QString typed =
      QInputDialog::getText(this, tr("Rename Design"), tr("Name:"), QLineEdit::Normal, current, &ok).trimmed();
  if (!ok || typed.isEmpty() || typed == current) return;
  static const QRegularExpression bad(QStringLiteral(R"([\\/:*?"<>|])"));
  if (typed.contains(bad)) {
    showError(tr("A name cannot hold any of \\ / : * ? \" < > |"));
    return;
  }
  if (m_filePath.isEmpty()) {
    m_documentName = typed;
    updateWindowTitle();
    qInfo().noquote() << QStringLiteral("Design renamed from its tab: %1 -> %2").arg(current, typed);
    return;
  }
  const QFileInfo was(m_filePath);
  const QString moved = was.dir().filePath(typed + QLatin1Char('.') + was.suffix());
  if (QFileInfo::exists(moved)) {
    showError(tr("%1 already exists in this folder").arg(QFileInfo(moved).fileName()));
    return;
  }
  if (!QFile::rename(m_filePath, moved)) {
    showError(tr("Could not rename %1").arg(was.fileName()));
    return;
  }
  openDesignMoved(moved);
}

namespace {
// PureCAD: the component copied last, in whichever design.
struct CrossCopy {
  QPointer<MainWindow> source;
  QString path; // the occurrence, uids from the root
  QString name;
};
CrossCopy& crossCopy() {
  static CrossCopy copy;
  return copy;
}
} // namespace

void MainWindow::copiedOccurrence(const QString& path, const QString& name) {
  crossCopy() = CrossCopy{this, path, name};
}

QString MainWindow::otherDesignCopy() const {
  const CrossCopy& copy = crossCopy();
  return copy.source != nullptr && copy.source != this ? copy.name : QString();
}

void MainWindow::pasteFromOtherDesign() {
  const CrossCopy copy = crossCopy();
  if (copy.source == nullptr || copy.source == this || !canChangeModel()) return;
  const QString file = copy.source->documentFilePath();
  if (file.isEmpty() || copy.source->isWindowModified()) {
    showError(tr("Save %1 first: the component is pasted from its saved file.").arg(copy.source->documentName()));
    return;
  }
  QJsonObject command{{QStringLiteral("cmd"), QStringLiteral("insert_component")},
                      {QStringLiteral("path"), QFileInfo(file).absoluteFilePath()},
                      {QStringLiteral("link"), true},
                      {QStringLiteral("parts"), QJsonObject{{QStringLiteral("occurrences"), QJsonArray{copy.path}}}}};
  if (!QSettings().value(QStringLiteral("design/pasteAtOrigin"), false).toBool()) {
    // Where it was in its design: its placement there.
    const gp_Trsf where = copy.source->m_placements.value(copy.path);
    QJsonArray rows;
    for (int r = 1; r <= 3; ++r) {
      rows.append(QJsonArray{where.Value(r, 1), where.Value(r, 2), where.Value(r, 3), where.Value(r, 4)});
    }
    command.insert(QStringLiteral("transform"), rows);
  }
  if (runModelCommand(command)) {
    qInfo().noquote() << QStringLiteral("Pasted %1 from %2").arg(copy.name, file);
  }
}

QString MainWindow::toolbarTab() const { return m_ribbon != nullptr ? m_ribbon->currentTab() : QString(); }

void MainWindow::showToolbarTab(const QString& tab) {
  if (m_ribbon != nullptr && !tab.isEmpty() && m_mode != Mode::Sketch) m_ribbon->showTab(tab);
}

void MainWindow::cancelOnLeave() {
  if (m_session && !modelBusy()) {
    qInfo().noquote() << QStringLiteral("Command closed on leaving the design (Preferences)");
    m_session->cancel();
  }
}

} // namespace mitcad
