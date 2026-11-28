// SPDX-License-Identifier: MIT
#pragma once

// Fork: Preferences › Layout as SolidCraft's: Panel layout (the panels'
// presets per workspace), Window, Design tabs, Panels, Dialogs, Toolbar
// tabs and Navigation bar, in sections that fold (stylePreferencesPage). The
// rows PureCAD's Display page had for these move here (addExisting).

#include "LayoutSettings.hpp"
#include "../framework/UiSizes.hpp"

#include <functional>
#include <utility>

#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

namespace mitcad {

// The schematic of the window beside Window's options: the toolbar's
// buttons, the Browser, the dialogs' card and History, as they are laid out.
class LayoutSchematic : public QWidget {
public:
  explicit LayoutSchematic(QWidget* parent = nullptr) : QWidget(parent) {
    setObjectName(QStringLiteral("layoutSchematic"));
    setFixedSize(uiRoomSize(300), uiRoomSize(200));
  }
  void setSettings(const LayoutSettings& settings) {
    m_settings = settings;
    update();
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QPalette pal = palette();
    const QColor ink = pal.color(QPalette::WindowText);
    QColor line = ink;
    line.setAlphaF(0.25f);
    QColor fill = ink;
    fill.setAlphaF(0.05f);
    const QRectF frame = QRectF(rect()).adjusted(6, 6, -6, -6);
    p.setPen(QPen(line, 1.5));
    p.setBrush(fill);
    p.drawRoundedRect(frame, 10, 10);
    QFont small = font();
    small.setPointSizeF(std::max(7.0, small.pointSizeF() * 0.85));
    p.setFont(small);
    // The toolbar.
    const double top = frame.top() + 8;
    if (m_settings.ribbon) {
      const QRectF bar(frame.left() + 8, top, frame.width() - 16, 26);
      p.setBrush(pal.color(QPalette::Base));
      p.drawRoundedRect(bar, 5, 5);
      p.setPen(Qt::NoPen);
      p.setBrush(ink);
      for (int i = 0; i < 10; ++i) {
        const double x = bar.left() + 14 + i * (bar.width() - 28) / 9.0;
        p.drawEllipse(QPointF(x, bar.center().y() - 3), 2.6, 2.6);
        if (!m_settings.ribbonOnHover) p.drawRect(QRectF(x - 5, bar.center().y() + 5, 10, 1.5));
      }
    }
    const double below = top + (m_settings.ribbon ? 34 : 4);
    p.setPen(QPen(line, 1.2));
    p.setBrush(pal.color(QPalette::Base));
    // The Browser at the left.
    const QRectF browser(frame.left() + 8, below, 62, frame.bottom() - below - 34);
    if (m_settings.browserTree) {
      p.drawRoundedRect(browser, 5, 5);
      p.setPen(ink);
      p.drawText(browser.adjusted(0, 4, 0, 0), Qt::AlignHCenter | Qt::AlignTop, QObject::tr("Browser"));
      p.setPen(QPen(line, 1.4));
      for (int i = 0; i < 3; ++i) {
        p.drawLine(QPointF(browser.left() + 10, browser.top() + 28 + i * 9),
                   QPointF(browser.right() - 10, browser.top() + 28 + i * 9));
      }
    }
    // The dialogs' card at the right.
    p.setPen(QPen(line, 1.2));
    const QRectF dialogs(frame.right() - 70, below, 62, 48);
    p.drawRoundedRect(dialogs, 5, 5);
    p.setPen(ink);
    p.drawText(dialogs.adjusted(0, 4, 0, 0), Qt::AlignHCenter | Qt::AlignTop, QObject::tr("Dialogs"));
    // History along the bottom.
    p.setPen(QPen(line, 1.2));
    const QRectF history(frame.left() + 8, frame.bottom() - 28, frame.width() - 16, 20);
    p.drawRoundedRect(history, 5, 5);
    p.setPen(ink);
    p.drawText(history, Qt::AlignCenter, QObject::tr("History"));
    // The model between them.
    const QPointF c((browser.right() + dialogs.left()) / 2.0, (below + history.top()) / 2.0 + 6);
    QColor model = ink;
    model.setAlphaF(0.7f);
    p.setPen(Qt::NoPen);
    p.setBrush(model);
    p.drawPolygon(QPolygonF{c + QPointF(0, -16), c + QPointF(14, -8), c + QPointF(14, 8), c + QPointF(0, 16),
                            c + QPointF(-14, 8), c + QPointF(-14, -8)});
  }

private:
  LayoutSettings m_settings;
};

class LayoutPreferences : public QWidget {
  Q_DECLARE_TR_FUNCTIONS(mitcad::LayoutPreferences)

public:
  // The sections the Display page's rows move into (addExisting).
  enum class Section { Window, Panels, ToolbarTabs, NavigationBar };

  // `tabs`: the toolbar's tabs by id and name, for Toolbar tabs' chips.
  explicit LayoutPreferences(const QVector<QPair<QString, QString>>& tabs, QWidget* parent = nullptr)
      : QWidget(parent), m_settings(LayoutSettings::load()) {
    setObjectName(QStringLiteral("layoutPreferences"));
    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea;
    scroll->setObjectName(QStringLiteral("layoutScroll"));
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    auto* content = new QWidget;
    m_column = new QVBoxLayout(content);
    m_column->setContentsMargins(0, 6, 8, 0);
    m_column->setSpacing(4);
    scroll->setWidget(content);
    scroll->viewport()->setAutoFillBackground(false);
    outer->addWidget(scroll);

    buildPanelLayout();
    buildWindow();
    buildDesignTabs();
    buildPanels();
    buildDialogs();
    buildToolbarTabs(tabs);
    buildNavigationBar();
    m_column->addStretch(1);
  }

  // A row of the Display page moved into one of these sections.
  void addExisting(Section section, QWidget* label, QLayoutItem* field) {
    QFormLayout* form = section == Section::Window          ? m_windowExisting
                        : section == Section::Panels        ? m_panelsExisting
                        : section == Section::ToolbarTabs   ? m_tabsExisting
                                                            : m_navigationExisting;
    if (field == nullptr) return;
    if (QWidget* widget = field->widget()) {
      label != nullptr ? form->addRow(label, widget) : form->addRow(widget);
    } else if (QLayout* layout = field->layout()) {
      label != nullptr ? form->addRow(label, layout) : form->addRow(layout);
    }
  }

  const LayoutSettings& chosen() const { return m_settings; }
  // Saves the settings and tells who follows them (Apply and OK).
  void commit() {
    m_settings.save();
    notifyLayoutSettings(m_settings);
    qDebug().noquote() << QStringLiteral("Preferences: layout saved");
  }

private:
  QVBoxLayout* section(const QString& title) {
    auto* box = new QGroupBox(title);
    auto* column = new QVBoxLayout(box);
    column->setContentsMargins(0, 4, 0, 8);
    column->setSpacing(8);
    m_column->addWidget(box);
    return column;
  }
  QFormLayout* form(QVBoxLayout* column) {
    auto* f = new QFormLayout;
    f->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    f->setHorizontalSpacing(24);
    column->addLayout(f);
    return f;
  }
  // A check box bound to a setting, with what it does in its tooltip.
  QCheckBox* check(QGridLayout* grid, int row, int column, const QString& title, const char* name,
                   bool LayoutSettings::*field, const QString& tip) {
    auto* box = new QCheckBox(title);
    box->setObjectName(QStringLiteral("layout_") + QLatin1String(name));
    box->setToolTip(tip);
    box->setChecked(m_settings.*field);
    connect(box, &QCheckBox::toggled, this, [this, field](bool on) {
      m_settings.*field = on;
      changed();
    });
    grid->addWidget(box, row, column);
    return box;
  }
  QGridLayout* grid(QVBoxLayout* column) {
    auto* g = new QGridLayout;
    g->setHorizontalSpacing(28);
    g->setVerticalSpacing(8);
    column->addLayout(g);
    return g;
  }
  // Segments bound to a choice: the buttons' indexes are the values.
  QWidget* segments(const char* name, const QStringList& names, int value, std::function<void(int)> apply) {
    auto* row = new QWidget;
    row->setObjectName(QStringLiteral("layout_") + QLatin1String(name));
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    line->setSpacing(0);
    auto* group = new QButtonGroup(row);
    const QPalette pal = palette();
    for (int i = 0; i < names.size(); ++i) {
      auto* button = new QPushButton(names.at(i));
      button->setCheckable(true);
      button->setChecked(i == value);
      const QString radius = i == 0 ? QStringLiteral("border-top-left-radius: 5px; border-bottom-left-radius: 5px;")
                             : i == names.size() - 1
                                 ? QStringLiteral("border-left: none; border-top-right-radius: 5px; border-bottom-right-radius: 5px;")
                                 : QStringLiteral("border-left: none;");
      button->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; padding: 4px 12px; background: %2; %4 } "
                                           "QPushButton:checked { background: %3; font-weight: 600; }")
                                .arg(pal.color(QPalette::Mid).name(), pal.color(QPalette::Base).name(),
                                     pal.color(QPalette::Highlight).lighter(185).name(), radius));
      group->addButton(button, i);
      line->addWidget(button);
    }
    connect(group, &QButtonGroup::idClicked, this, [this, apply](int id) {
      apply(id);
      changed();
    });
    return row;
  }
  void changed() {
    if (m_schematic != nullptr) m_schematic->setSettings(m_settings);
    if (onChanged) onChanged();
  }

  void buildPanelLayout() {
    QVBoxLayout* column = section(tr("Panel layout"));
    auto* top = new QHBoxLayout;
    const QStringList workspaces{QStringLiteral("modeling"), QStringLiteral("sketch"), QStringLiteral("render"),
                                 QStringLiteral("review"), QStringLiteral("classic")};
    top->addWidget(segments("panelWorkspace", {tr("Modeling"), tr("Sketch"), tr("Render"), tr("Review"), tr("Classic")},
                            std::max<int>(0, int(workspaces.indexOf(m_settings.panelWorkspace))),
                            [this, workspaces](int i) { m_settings.panelWorkspace = workspaces.at(i); }));
    top->addStretch(1);
    auto* reset = new QPushButton(tr("↶ Reset"));
    reset->setObjectName(QStringLiteral("layout_panelReset"));
    reset->setFlat(true);
    reset->setToolTip(tr("Puts the panels of this workspace back where its preset has them"));
    connect(reset, &QPushButton::clicked, this, [this] {
      m_settings.panelPreset = QStringLiteral("classic");
      rebuildPresets();
      changed();
    });
    top->addWidget(reset);
    column->addLayout(top);
    m_presets = new QHBoxLayout;
    m_presets->setSpacing(14);
    column->addLayout(m_presets);
    rebuildPresets();
  }
  void rebuildPresets() {
    while (QLayoutItem* item = m_presets->takeAt(0)) {
      delete item->widget();
      delete item;
    }
    QStringList names{QStringLiteral("custom"), QStringLiteral("classic")};
    names += m_settings.savedPresets;
    for (const QString& id : names) {
      auto* card = new QToolButton;
      card->setObjectName(QStringLiteral("layout_preset_") + id);
      card->setCheckable(true);
      card->setChecked(m_settings.panelPreset == id);
      card->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
      card->setText(id == QLatin1String("custom")    ? tr("Custom")
                    : id == QLatin1String("classic") ? tr("Classic")
                                                     : id);
      card->setIcon(presetIcon(id == QLatin1String("classic")));
      card->setIconSize(QSize(uiRoomSize(110), uiRoomSize(64)));
      card->setAutoRaise(true);
      card->setStyleSheet(QStringLiteral("QToolButton { border: 1px solid transparent; border-radius: 8px; padding: 4px; } "
                                         "QToolButton:checked { border: 2px dashed %1; }")
                              .arg(palette().color(QPalette::Highlight).name()));
      connect(card, &QToolButton::clicked, this, [this, id] {
        m_settings.panelPreset = id;
        rebuildPresets();
        changed();
      });
      m_presets->addWidget(card);
    }
    auto* save = new QToolButton;
    save->setObjectName(QStringLiteral("layout_preset_save"));
    save->setText(tr("Save as…"));
    save->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    save->setIconSize(QSize(uiRoomSize(110), uiRoomSize(64)));
    save->setIcon(plusIcon());
    save->setAutoRaise(true);
    connect(save, &QToolButton::clicked, this, [this] {
      bool ok = false;
      const QString name = QInputDialog::getText(this, tr("Save Panel Layout"), tr("Name:"), QLineEdit::Normal,
                                                 QString(), &ok)
                               .trimmed();
      if (!ok || name.isEmpty() || name == QLatin1String("custom") || name == QLatin1String("classic")) return;
      if (!m_settings.savedPresets.contains(name)) m_settings.savedPresets.append(name);
      m_settings.panelPreset = name;
      rebuildPresets();
      changed();
    });
    m_presets->addWidget(save);
    m_presets->addStretch(1);
  }
  QIcon presetIcon(bool classic) const {
    QPixmap pixmap(uiRoomSize(110) * 2, uiRoomSize(64) * 2);
    pixmap.setDevicePixelRatio(2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r(2, 2, uiRoomSize(110) - 4, uiRoomSize(64) - 4);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(classic ? 0x56606c : 0x4a4f56));
    p.drawRoundedRect(r, 6, 6);
    p.setBrush(QColor(classic ? 0x8fa3b8 : 0x8a8f96));
    const double w = r.width();
    p.drawRoundedRect(QRectF(r.left() + 6, r.top() + 6, w * 0.24, r.height() - 12), 3, 3);
    p.drawRoundedRect(QRectF(r.left() + w * 0.32, r.top() + 6, w * 0.06, r.height() * 0.45), 1, 1);
    p.drawRoundedRect(QRectF(r.left() + w * 0.32, r.top() + r.height() * 0.58, w * 0.30, r.height() * 0.3), 3, 3);
    p.drawRoundedRect(QRectF(r.right() - w * 0.30, r.top() + 6, w * 0.26, r.height() - 12), 3, 3);
    return QIcon(pixmap);
  }
  QIcon plusIcon() const {
    QPixmap pixmap(uiRoomSize(110) * 2, uiRoomSize(64) * 2);
    pixmap.setDevicePixelRatio(2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF r(2, 2, uiRoomSize(110) - 4, uiRoomSize(64) - 4);
    p.setPen(QPen(palette().color(QPalette::Mid), 1.2, Qt::DashLine));
    p.setBrush(Qt::NoBrush);
    p.drawRoundedRect(r, 6, 6);
    p.setPen(QPen(palette().color(QPalette::Highlight), 2.2));
    p.drawLine(r.center() + QPointF(-9, 0), r.center() + QPointF(9, 0));
    p.drawLine(r.center() + QPointF(0, -9), r.center() + QPointF(0, 9));
    return QIcon(pixmap);
  }

  void buildWindow() {
    QVBoxLayout* column = section(tr("Window"));
    auto* row = new QHBoxLayout;
    auto* left = new QVBoxLayout;
    m_windowExisting = new QFormLayout;
    m_windowExisting->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    left->addLayout(m_windowExisting);
    auto* choices = new QFormLayout;
    choices->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    choices->addRow(tr("Menus"), segments("largeMenuIcons", {tr("List"), tr("Large icons")}, m_settings.largeMenuIcons ? 1 : 0,
                                          [this](int i) { m_settings.largeMenuIcons = i == 1; }));
    choices->addRow(tr("Notices"), segments("noticesCentered", {tr("Bottom right"), tr("Bottom center")},
                                            m_settings.noticesCentered ? 1 : 0,
                                            [this](int i) { m_settings.noticesCentered = i == 1; }));
    choices->addRow(tr("Top bar"), segments("topBarAtEdge", {tr("Always"), tr("At top edge")}, m_settings.topBarAtEdge ? 1 : 0,
                                            [this](int i) { m_settings.topBarAtEdge = i == 1; }));
    choices->addRow(tr("Card headers"), segments("cardHeaders", {tr("Always"), tr("On hover"), tr("Hidden")},
                                                 m_settings.cardHeaders, [this](int i) { m_settings.cardHeaders = i; }));
    left->addLayout(choices);
    QGridLayout* g = grid(nullptr == nullptr ? left : left);
    int r = 0;
    check(g, r++, 0, tr("Use legacy Surface toolbar"), "legacySurfaceToolbar", &LayoutSettings::legacySurfaceToolbar,
          tr("The Surface tab as MitCad had it, before the Fusion layout"));
    check(g, r++, 0, tr("Group Construct tools by entity type"), "groupConstructByEntity",
          &LayoutSettings::groupConstructByEntity, tr("Construct's menu in Planes, Axes and Points"));
    check(g, r++, 0, tr("Status bar"), "statusBar", &LayoutSettings::statusBar,
          tr("A bar along the window's bottom with the status and the cursor's position"));
    check(g, r++, 0, tr("Show operations in the view"), "operationsInView", &LayoutSettings::operationsInView,
          tr("A running operation's progress over the view, not only in the status"));
    check(g, r++, 0, tr("Wait for files to open"), "waitForFiles", &LayoutSettings::waitForFiles,
          tr("A file opening holds the window until it is open, instead of opening in the background"));
    check(g, r++, 0, tr("Keep dialogs and tools open when switching designs"), "keepToolsOpenAcrossDesigns",
          &LayoutSettings::keepToolsOpenAcrossDesigns, tr("A command's dialog stays when another design's tab is chosen"));
    check(g, r++, 0, tr("Remember each design's toolbar tab"), "rememberTabPerDesign", &LayoutSettings::rememberTabPerDesign,
          tr("Each design's tab comes back with the toolbar tab it showed"));
    check(g, r++, 0, tr("Double-click a design tab to rename it"), "doubleClickRenamesTab",
          &LayoutSettings::doubleClickRenamesTab, tr("A double-click on a design's tab edits its name"));
    check(g, r++, 0, tr("Pause drawing while the window is dragged, so it moves smoothly"), "pauseDrawingWhileDragged",
          &LayoutSettings::pauseDrawingWhileDragged, tr("The 3D view is not redrawn while the window moves or resizes"));
    check(g, r++, 0, tr("Show window control buttons"), "windowButtons", &LayoutSettings::windowButtons,
          tr("Minimize, maximize and close in the title bar"));
    check(g, r++, 0, tr("Ribbon"), "ribbon", &LayoutSettings::ribbon, tr("The toolbar's panel of groups under the tabs"));
    check(g, r++, 0, tr("Only when hovered or focused"), "ribbonOnHover", &LayoutSettings::ribbonOnHover,
          tr("The ribbon shows its labels only while the pointer is over it or it has the keyboard"));
    check(g, r++, 0, tr("Other panels"), "otherPanelsOnHover", &LayoutSettings::otherPanelsOnHover,
          tr("The other panels too show their full size only while hovered or focused"));
    check(g, r++, 0, tr("Keep on top of other windows"), "keepOnTop", &LayoutSettings::keepOnTop,
          tr("PureCAD's window stays above other programs' windows"));
    row->addLayout(left, 1);
    m_schematic = new LayoutSchematic;
    m_schematic->setSettings(m_settings);
    row->addWidget(m_schematic, 0, Qt::AlignTop);
    column->addLayout(row);
  }

  void buildDesignTabs() {
    QVBoxLayout* column = section(tr("Design tabs"));
    QFormLayout* f = form(column);
    f->addRow(tr("Views of a design"), segments("viewSwitcherTab", {tr("Show views as tabs"), tr("Show one tab with a view switcher")},
                                                 m_settings.viewSwitcherTab ? 1 : 0,
                                                 [this](int i) { m_settings.viewSwitcherTab = i == 1; }));
    QGridLayout* g = grid(column);
    int r = 0;
    check(g, r++, 0, tr("Show the design view switcher"), "showViewSwitcher", &LayoutSettings::showViewSwitcher,
          tr("A switcher of the design's views on its tab"));
    check(g, r++, 0, tr("Focus views when switching workspaces"), "focusViewsOnWorkspace", &LayoutSettings::focusViewsOnWorkspace,
          tr("Choosing a workspace shows its view of the design"));
    check(g, r++, 0, tr("Show view states in tabs"), "viewStatesInTabs", &LayoutSettings::viewStatesInTabs,
          tr("A view's tab says when it has unsaved changes or is computing"));
    check(g, r++, 0, tr("Open drawing sheets in a sub-tab"), "sheetsInSubTab", &LayoutSettings::sheetsInSubTab,
          tr("A drawing's sheet opens as a view under its design's tab"));
    check(g, r++, 0, tr("Open Render and Animate in a sub-tab"), "renderInSubTab", &LayoutSettings::renderInSubTab,
          tr("Render and Animate open as views under the design's tab"));
    check(g, r++, 0, tr("Reuse a view of the same sheet, sketch or workspace"), "reuseViews", &LayoutSettings::reuseViews,
          tr("Opening what a view already shows goes to that view instead of a new one"));
    check(g, r++, 0, tr("Reopen a design's views next time it opens"), "reopenViews", &LayoutSettings::reopenViews,
          tr("A design opens with the views it had when it was closed"));
  }

  void buildPanels() {
    QVBoxLayout* column = section(tr("Panels"));
    QGridLayout* g = grid(column);
    int r = 0;
    check(g, r++, 0, tr("Preview panel placement while dragging"), "previewPanelPlacement", &LayoutSettings::previewPanelPlacement,
          tr("Dragging a panel shows where it will dock before it is let go"));
    check(g, r++, 0, tr("Show History in the Browser"), "historyInBrowser", &LayoutSettings::historyInBrowser,
          tr("The History's steps as a folder of the Browser"));
    check(g, r++, 0, tr("Show parts library"), "partsLibrary", &LayoutSettings::partsLibrary,
          tr("The library of standard parts as a panel"));
    check(g, r++, 0, tr("Browser tree"), "browserTree", &LayoutSettings::browserTree, tr("The Browser's panel"));
    QFormLayout* f = form(column);
    f->addRow(tr("Sketches in History"), segments("sketchesUnderFeature", {tr("In order"), tr("Under their feature")},
                                                   m_settings.sketchesUnderFeature ? 1 : 0,
                                                   [this](int i) { m_settings.sketchesUnderFeature = i == 1; }));
    f->addRow(tr("Bodies folders"), segments("bodiesFolders", {tr("Always"), tr("Several"), tr("Never")}, m_settings.bodiesFolders,
                                              [this](int i) { m_settings.bodiesFolders = i; }));
    m_panelsExisting = new QFormLayout;
    m_panelsExisting->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    column->addLayout(m_panelsExisting);
    column->addWidget(new QLabel(tr("History")));
    QGridLayout* h = grid(column);
    check(h, 0, 0, tr("Play History button"), "playHistoryButton", &LayoutSettings::playHistoryButton,
          tr("A button that plays the History step by step"));
    check(h, 1, 0, tr("Fold the History bar into groups"), "foldHistoryGroups", &LayoutSettings::foldHistoryGroups,
          tr("Folders in the History bar fold into one step"));
    check(h, 2, 0, tr("Folder icon from its features"), "folderIconFromFeatures", &LayoutSettings::folderIconFromFeatures,
          tr("A History folder shows the icon of the features it holds"));
    check(h, 3, 0, tr("Go to end stops at the component's end"), "goToEndOfComponent", &LayoutSettings::goToEndOfComponent,
          tr("Go to End goes to the active component's last step"));
    check(h, 0, 1, tr("Group joints in the History"), "groupJointsInHistory", &LayoutSettings::groupJointsInHistory,
          tr("Joints in a folder of their own in the History"));
    check(h, 1, 1, tr("Origin and bodies on top"), "originAndBodiesOnTop", &LayoutSettings::originAndBodiesOnTop,
          tr("The Browser lists the origin and the bodies before the components"));
    check(h, 2, 1, tr("History steps by component"), "historyByComponent", &LayoutSettings::historyByComponent,
          tr("Each component's steps marked with its colour in the History"));
    check(h, 3, 1, tr("Feature menus act on the whole selection"), "featureMenusWholeSelection",
          &LayoutSettings::featureMenusWholeSelection, tr("A feature's menu acts on every selected feature"));
    column->addWidget(new QLabel(tr("Browser")));
    QGridLayout* b = grid(column);
    check(b, 0, 0, tr("Components start open"), "componentsStartOpen", &LayoutSettings::componentsStartOpen,
          tr("A component's row is open when the Browser shows it first"));
    check(b, 1, 0, tr("Eye beside the fold arrow"), "eyeBesideArrow", &LayoutSettings::eyeBesideArrow,
          tr("A row's eye at its left, beside the arrow"));
    check(b, 0, 1, tr("Browser rows start closed"), "rowsStartClosed", &LayoutSettings::rowsStartClosed,
          tr("Every row of the Browser starts closed"));
    check(b, 1, 1, tr("Features under bodies"), "featuresUnderBodies", &LayoutSettings::featuresUnderBodies,
          tr("A body's row holds the features that made it"));
  }

  void buildDialogs() {
    QVBoxLayout* column = section(tr("Dialogs"));
    QFormLayout* f = form(column);
    f->addRow(tr("OK and Cancel"), segments("okCancelTop", {tr("Bottom"), tr("Top")}, m_settings.okCancelTop ? 1 : 0,
                                             [this](int i) { m_settings.okCancelTop = i == 1; }));
    QGridLayout* g = grid(column);
    check(g, 0, 0, tr("Also in the view's corner"), "okCancelInCorner", &LayoutSettings::okCancelInCorner,
          tr("A command's OK and Cancel also at the view's bottom right"));
  }

  void buildToolbarTabs(const QVector<QPair<QString, QString>>& tabs) {
    QVBoxLayout* column = section(tr("Toolbar tabs"));
    auto* chips = new QGridLayout;
    chips->setHorizontalSpacing(6);
    chips->setVerticalSpacing(6);
    int i = 0;
    for (const auto& [id, name] : tabs) {
      auto* chip = new QPushButton(name);
      chip->setObjectName(QStringLiteral("layout_tab_") + id);
      chip->setCheckable(true);
      chip->setChecked(!m_settings.hiddenTabs.contains(id));
      chip->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; border-radius: 6px; padding: 4px 12px; } "
                                         "QPushButton:checked { border: 2px solid %2; font-weight: 600; }")
                              .arg(palette().color(QPalette::Mid).name(), palette().color(QPalette::WindowText).name()));
      chip->setToolTip(tr("Shows or hides the %1 tab").arg(name));
      const QString tabId = id;
      connect(chip, &QPushButton::toggled, this, [this, tabId](bool on) {
        m_settings.hiddenTabs.removeAll(tabId);
        if (!on) m_settings.hiddenTabs.append(tabId);
        changed();
      });
      chips->addWidget(chip, i / 7, i % 7);
      ++i;
    }
    column->addLayout(chips);
    QGridLayout* g = grid(column);
    check(g, 0, 0, tr("Keep the Sketch tab shown"), "keepSketchTab", &LayoutSettings::keepSketchTab,
          tr("The Sketch tab stays among the tabs when no sketch is open"));
    check(g, 1, 0, tr("Show search results in the ribbon"), "searchResultsInRibbon", &LayoutSettings::searchResultsInRibbon,
          tr("Search commands lists what it finds as buttons in the ribbon"));
    check(g, 2, 0, tr("Pinned commands out of menus"), "pinnedOutOfMenus", &LayoutSettings::pinnedOutOfMenus,
          tr("A command pinned to the toolbar leaves its group's menu"));
    check(g, 3, 0, tr("Show pinned constraints in the constraint grid"), "pinnedConstraintsInGrid",
          &LayoutSettings::pinnedConstraintsInGrid, tr("Pinned constraints also in Constraints' grid"));
    check(g, 0, 1, tr("Highlight active commands"), "highlightActiveCommands", &LayoutSettings::highlightActiveCommands,
          tr("A command's button stays lit while it runs"));
    check(g, 1, 1, tr("Show only the ribbon's tabs"), "onlyRibbonTabs", &LayoutSettings::onlyRibbonTabs,
          tr("The ribbon folds to its tabs; a tab opens its groups"));
    check(g, 2, 1, tr("Switch panel menus on hover"), "panelMenusOnHover", &LayoutSettings::panelMenusOnHover,
          tr("With a group's menu open, pointing at another group opens its menu"));
    check(g, 3, 1, tr("Open Search commands centered"), "searchCentered", &LayoutSettings::searchCentered,
          tr("Search commands opens in the middle of the window"));
    m_tabsExisting = new QFormLayout;
    m_tabsExisting->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    column->addLayout(m_tabsExisting);
  }

  void buildNavigationBar() {
    QVBoxLayout* column = section(tr("Navigation bar"));
    m_navigationExisting = new QFormLayout;
    m_navigationExisting->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    column->addLayout(m_navigationExisting);
    QFormLayout* f = form(column);
    f->addRow(tr("When idle"), segments("navigationIdle", {tr("Always visible"), tr("Fade"), tr("Hide until hovered")},
                                        m_settings.navigationIdle, [this](int i) { m_settings.navigationIdle = i; }));
    f->addRow(tr("Position"), segments("navigationPosition",
                                       {tr("Top left"), tr("Top"), tr("Top right"), tr("Bottom left"), tr("Bottom"), tr("Bottom right")},
                                       m_settings.navigationPosition, [this](int i) { m_settings.navigationPosition = i; }));
    f->addRow(tr("Size"), segments("navigationCompact", {tr("Compact"), tr("Regular")}, m_settings.navigationCompact ? 0 : 1,
                                   [this](int i) { m_settings.navigationCompact = i == 0; }));
    f->addRow(tr("Buttons"), segments("navigationLabels", {tr("Icons"), tr("Icons + labels")}, m_settings.navigationLabels ? 1 : 0,
                                      [this](int i) { m_settings.navigationLabels = i == 1; }));
    // The bar's items: shown ones in order, the others offered with +.
    column->addWidget(new QLabel(tr("Items (click one to remove it, + to add)")));
    m_shownItems = new QHBoxLayout;
    m_shownItems->setSpacing(4);
    column->addLayout(m_shownItems);
    m_availableItems = new QGridLayout;
    m_availableItems->setHorizontalSpacing(6);
    column->addLayout(m_availableItems);
    auto* reset = new QPushButton(tr("Reset to default"));
    reset->setObjectName(QStringLiteral("layout_navigationReset"));
    connect(reset, &QPushButton::clicked, this, [this] {
      m_settings.navigationItems.clear();
      rebuildItems();
      changed();
    });
    column->addWidget(reset, 0, Qt::AlignLeft);
    rebuildItems();
  }

public:
  // The navigation bar's items: id and name, the default ones first.
  static QVector<QPair<QString, QString>> navigationBarItems() {
    return {{QStringLiteral("view.look_at"), tr("Look At")},     {QStringLiteral("view.fit"), tr("Fit")},
            {QStringLiteral("view.home"), tr("Home")},           {QStringLiteral("view.visual_style"), tr("Display style")},
            {QStringLiteral("view.grid"), tr("Grid")},           {QStringLiteral("view.snap_grid"), tr("Snap to grid")},
            {QStringLiteral("view.comment_pins"), tr("Comment pins")}, {QStringLiteral("view.isolate"), tr("Isolate")},
            {QStringLiteral("view.perspective"), tr("Perspective")}, {QStringLiteral("view.orthographic"), tr("Orthographic")},
            {QStringLiteral("|"), tr("Separator")}};
  }
  static QStringList defaultNavigationItems() {
    return {QStringLiteral("view.look_at"), QStringLiteral("view.fit"), QStringLiteral("view.visual_style"),
            QStringLiteral("view.grid"), QStringLiteral("view.snap_grid"), QStringLiteral("view.comment_pins")};
  }
  std::function<void()> onChanged;

private:
  void rebuildItems() {
    for (QLayout* layout : {static_cast<QLayout*>(m_shownItems), static_cast<QLayout*>(m_availableItems)}) {
      while (QLayoutItem* item = layout->takeAt(0)) {
        delete item->widget();
        delete item;
      }
    }
    const QStringList shown = m_settings.navigationItems.isEmpty() ? defaultNavigationItems() : m_settings.navigationItems;
    const auto nameOf = [](const QString& id) {
      for (const auto& [itemId, name] : navigationBarItems()) {
        if (itemId == id) return name;
      }
      return id;
    };
    for (int i = 0; i < shown.size(); ++i) {
      auto* chip = new QPushButton(nameOf(shown.at(i)) + QStringLiteral("  ×"));
      chip->setObjectName(QStringLiteral("layout_navItem%1").arg(i));
      chip->setToolTip(tr("Removes it from the bar"));
      connect(chip, &QPushButton::clicked, this, [this, shown, i] {
        QStringList items = shown;
        items.removeAt(i);
        m_settings.navigationItems = items;
        rebuildItems();
        changed();
      });
      m_shownItems->addWidget(chip);
    }
    m_shownItems->addStretch(1);
    int column = 0;
    for (const auto& [id, name] : navigationBarItems()) {
      if (id != QLatin1String("|") && shown.contains(id)) continue;
      auto* chip = new QPushButton(name + QStringLiteral("  +"));
      chip->setObjectName(QStringLiteral("layout_navAdd_") + id);
      chip->setToolTip(tr("Adds it to the bar"));
      const QString itemId = id;
      connect(chip, &QPushButton::clicked, this, [this, shown, itemId] {
        QStringList items = shown;
        items.append(itemId);
        m_settings.navigationItems = items;
        rebuildItems();
        changed();
      });
      m_availableItems->addWidget(chip, column / 5, column % 5);
      ++column;
    }
  }

  LayoutSettings m_settings;
  QVBoxLayout* m_column = nullptr;
  QHBoxLayout* m_presets = nullptr;
  LayoutSchematic* m_schematic = nullptr;
  QFormLayout* m_windowExisting = nullptr;
  QFormLayout* m_panelsExisting = nullptr;
  QFormLayout* m_tabsExisting = nullptr;
  QFormLayout* m_navigationExisting = nullptr;
  QHBoxLayout* m_shownItems = nullptr;
  QGridLayout* m_availableItems = nullptr;
};

} // namespace mitcad
