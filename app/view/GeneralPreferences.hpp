// SPDX-License-Identifier: MIT
#pragma once

// Fork: Preferences › General, as SolidCraft's: Profiles (named setups of
// the settings, saved as settings files, framework/SettingsTransfer.hpp),
// then sections the page's existing controls are moved into (they keep
// their saving), Updates among them (SolidCraft has no page of its own for
// them).

#include <QEvent>
#include <QGroupBox>
#include <QLineEdit>
#include <QMenu>
#include <QTimer>
#include <QPushButton>

#include "PreferenceKit.hpp"

namespace mitcad {

// A small tag beside a section's caption ("0 saved"), kept beside the
// caption's words as the box's font and title change.
class SectionTagPlacer : public QObject {
public:
  SectionTagPlacer(QGroupBox* box, QLabel* tag) : QObject(box), m_box(box), m_tag(tag) {
    box->installEventFilter(this);
  }

protected:
  bool eventFilter(QObject*, QEvent* event) override {
    if (event->type() == QEvent::Show || event->type() == QEvent::Resize || event->type() == QEvent::FontChange ||
        event->type() == QEvent::LayoutRequest) {
      const QFontMetrics metrics(m_box->font());
      m_tag->adjustSize();
      m_tag->move(metrics.horizontalAdvance(m_box->title()) + 14, std::max(0, (metrics.height() - m_tag->height()) / 2));
      m_tag->raise();
    }
    return false;
  }

private:
  QGroupBox* m_box;
  QLabel* m_tag;
};

class GeneralPreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::GeneralPreferences)

public:
  struct Profiles {
    QStringList saved;    // the names of the saved setups
    QString active;       // the one last saved or loaded; empty: none
    QStringList held;     // what a setup holds (the groups exported by default)
    QStringList computer; // what stays with this computer
  };

  // Set by Preferences: a setup saved under a name (false: not saved),
  // loaded, deleted, and a settings file imported.
  std::function<bool(const QString&)> saveProfile;
  std::function<void(const QString&)> loadProfile;
  std::function<void(const QString&)> deleteProfile;
  std::function<void()> importFile;

  explicit GeneralPreferences(const Profiles& profiles, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("general"), parent), m_profiles(profiles) {
    QVBoxLayout* column = addSection(tr("Profiles"), savedText());
    m_savedTag = m_lastTag;
    auto* nameRow = new QHBoxLayout;
    nameRow->setSpacing(8);
    auto* name = new QLineEdit;
    name->setObjectName(QStringLiteral("generalProfileName"));
    name->setPlaceholderText(tr("Name this setup"));
    name->setMinimumWidth(uiRoomSize(280));
    auto* save = new QPushButton(tr("Save as new"));
    save->setObjectName(QStringLiteral("generalProfileSave"));
    save->setEnabled(false);
    save->setToolTip(tr("Keeps the settings below under this name, to come back to or share"));
    const QColor accent = palette().color(QPalette::Highlight);
    QColor faded = accent;
    faded.setAlpha(110);
    save->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: white; border: none; border-radius: 6px; "
                                       "padding: 6px 18px; font-weight: 600; } QPushButton:disabled { background: %2; }")
                            .arg(accent.name(), faded.name(QColor::HexArgb)));
    auto* import = new QPushButton(QStringLiteral("⤓  ") + tr("Import…"));
    import->setObjectName(QStringLiteral("generalProfileImport"));
    import->setToolTip(tr("Takes the settings from a file saved on this or another computer"));
    connect(name, &QLineEdit::textChanged, save, [save](const QString& text) { save->setEnabled(!text.trimmed().isEmpty()); });
    connect(save, &QPushButton::clicked, this, [this, name] {
      const QString chosen = name->text().trimmed();
      if (chosen.isEmpty() || !saveProfile || !saveProfile(chosen)) return;
      name->clear();
      if (!m_profiles.saved.contains(chosen)) m_profiles.saved << chosen;
      m_profiles.active = chosen;
      showSaved();
    });
    connect(name, &QLineEdit::returnPressed, save, &QPushButton::click);
    connect(import, &QPushButton::clicked, this, [this] {
      if (importFile) importFile();
    });
    nameRow->addWidget(name);
    nameRow->addWidget(save);
    nameRow->addWidget(import);
    nameRow->addStretch(1);
    column->addLayout(nameRow);
    QFormLayout* rows = form(column);
    m_savedChips = new QWidget;
    m_savedChips->setObjectName(QStringLiteral("generalProfiles"));
    new QHBoxLayout(m_savedChips);
    m_savedChips->layout()->setContentsMargins(0, 0, 0, 0);
    m_savedChips->layout()->setSpacing(6);
    m_savedLabel = new QLabel(tr("Saved"));
    rows->addRow(m_savedLabel, m_savedChips);
    rows->addRow(label(tr("Held"), QString(), tr("What a saved setup holds and brings back")),
                 staticChips(m_profiles.held, false));
    rows->addRow(label(tr("This computer"), QString(),
                       tr("Folders, programs and recent files stay with this computer: another may not have them")),
                 staticChips(m_profiles.computer, true));
    showSaved();
  }

  // A section, with a tag beside its caption when `tagText` is given.
  QVBoxLayout* addSection(const QString& title, const QString& tagText = QString()) {
    QVBoxLayout* column = section(title);
    m_lastTag = nullptr;
    if (!tagText.isEmpty()) {
      auto* box = static_cast<QGroupBox*>(column->parentWidget());
      m_lastTag = tag(tagText, palette().color(QPalette::PlaceholderText));
      m_lastTag->setParent(box);
      m_lastTag->setProperty("purecadSectionTag", true);
      new SectionTagPlacer(box, m_lastTag);
    }
    return column;
  }
  QFormLayout* addForm(QVBoxLayout* column) { return form(column); }
  QWidget* rowLabel(const QString& title, const QString& caption = QString(), const QString& tip = QString()) {
    return label(title, caption, tip);
  }
  QLabel* infoMark(const QString& tip) { return info(tip); }
  QLabel* tagLabel(const QString& text, const QColor& tint) { return tag(text, tint); }
  // A line of quiet text under a row.
  QLabel* note(QVBoxLayout* column, const QString& text) {
    auto* line = new QLabel(text);
    line->setWordWrap(true);
    line->setForegroundRole(QPalette::PlaceholderText);
    column->addWidget(line);
    return line;
  }
  // A check box moved here from elsewhere, with its ⓘ.
  void adopt(QVBoxLayout* column, QCheckBox* box, const QString& tip = QString()) {
    auto* row = new QHBoxLayout;
    row->setSpacing(6);
    row->addWidget(box);
    const QString said = tip.isEmpty() ? box->toolTip() : tip;
    if (!said.isEmpty()) row->addWidget(info(said));
    row->addStretch(1);
    column->addLayout(row);
  }
  void done() { finish(); }

private:
  QString savedText() const { return tr("%n saved", nullptr, int(m_profiles.saved.size())); }

  QWidget* staticChips(const QStringList& names, bool locked) {
    auto* row = new QWidget;
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    line->setSpacing(6);
    const QPalette pal = palette();
    for (const QString& text : names) {
      auto* chip = new QLabel((locked ? QStringLiteral("🔒 ") : QString()) + text);
      chip->setStyleSheet(QStringLiteral("QLabel { border: 1px solid %1; border-radius: 10px; padding: 2px 11px; "
                                         "color: %2; font-weight: %3; }")
                              .arg(locked ? pal.color(QPalette::Mid).name() : pal.color(QPalette::Dark).name(),
                                   locked ? pal.color(QPalette::PlaceholderText).name()
                                          : pal.color(QPalette::WindowText).name(),
                                   locked ? QStringLiteral("400") : QStringLiteral("600")));
      line->addWidget(chip);
    }
    line->addStretch(1);
    return row;
  }

  // The saved setups as chips: a click loads one, its menu deletes it.
  void showSaved() {
    if (m_savedTag != nullptr) {
      m_savedTag->setText(savedText());
      m_savedTag->adjustSize();
    }
    QLayout* line = m_savedChips->layout();
    while (QLayoutItem* item = line->takeAt(0)) {
      delete item->widget();
      delete item;
    }
    const QPalette pal = palette();
    for (const QString& profile : m_profiles.saved) {
      auto* chip = new QPushButton(profile);
      const bool active = profile == m_profiles.active;
      chip->setToolTip(tr("Load the settings saved as %1").arg(profile));
      chip->setStyleSheet(QStringLiteral("QPushButton { border: 1px solid %1; border-radius: 10px; padding: 2px 12px; "
                                         "background: %2; font-weight: %3; } QPushButton:hover { background: %4; }")
                              .arg(active ? pal.color(QPalette::WindowText).name() : pal.color(QPalette::Mid).name(),
                                   active ? pal.color(QPalette::Mid).lighter(140).name() : pal.color(QPalette::Base).name(),
                                   active ? QStringLiteral("600") : QStringLiteral("400"),
                                   preferenceHoverTint(pal).name()));
      chip->setContextMenuPolicy(Qt::CustomContextMenu);
      connect(chip, &QPushButton::clicked, this, [this, profile] {
        if (loadProfile) loadProfile(profile);
      });
      connect(chip, &QWidget::customContextMenuRequested, this, [this, chip, profile](const QPoint& at) {
        QMenu menu;
        QAction* load = menu.addAction(tr("Load"));
        QAction* remove = menu.addAction(tr("Delete"));
        QAction* picked = menu.exec(chip->mapToGlobal(at));
        if (picked == load && loadProfile) {
          loadProfile(profile);
        } else if (picked == remove && deleteProfile) {
          deleteProfile(profile);
          m_profiles.saved.removeAll(profile);
          if (m_profiles.active == profile) m_profiles.active.clear();
          QTimer::singleShot(0, this, [this] { showSaved(); });
        }
      });
      line->addWidget(chip);
    }
    static_cast<QHBoxLayout*>(line)->addStretch(1);
    m_savedLabel->setVisible(!m_profiles.saved.isEmpty());
    m_savedChips->setVisible(!m_profiles.saved.isEmpty());
  }

  Profiles m_profiles;
  QLabel* m_lastTag = nullptr;
  QLabel* m_savedTag = nullptr;
  QLabel* m_savedLabel = nullptr;
  QWidget* m_savedChips = nullptr;
};

} // namespace mitcad
