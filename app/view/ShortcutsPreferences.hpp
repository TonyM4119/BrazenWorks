// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Shortcuts, as SolidCraft's: a filter with Done,
// Export and Import, Keep model keys out of sketches, Modifier keys, and
// every command by its group with its key as a chip, + to record another,
// ↶ back to its default and × to clear it. The keys are the command
// registry's (ViewController gives the rows and applies the changes:
// `conflict`, `apply`); the page's own choices are under "shortcutsPrefs/".

#include <QFile>
#include <QFileDialog>
#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeySequenceEdit>
#include <QLineEdit>
#include <QComboBox>
#include <QSettings>
#include <QToolButton>

#include "PreferenceKit.hpp"

namespace mitcad {

class ShortcutsPreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::ShortcutsPreferences)

public:
  struct Row {
    QString id;
    QString name;
    QString group;             // its caption ("SOLID")
    QKeySequence key;          // as saved
    QKeySequence defaultKey;
  };

  explicit ShortcutsPreferences(const QVector<Row>& rows, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("shortcuts"), parent), m_rows(rows) {
    const QSettings store;
    m_keepModelKeysOut = store.value(QStringLiteral("shortcutsPrefs/keepModelKeysOut"), false).toBool();
    m_hidePreviewKey = std::clamp(store.value(QStringLiteral("shortcutsPrefs/hidePreviewKey"), 0).toInt(), 0, 3);
    for (const Row& row : m_rows) m_keys.insert(row.id, row.key);
    buildTop();
    buildRows();
    finish();
  }

  // Another command already on `key` ("" none): its name.
  std::function<QString(const QString& id, const QKeySequence& key)> conflict;
  // The keys changed here, by command.
  std::function<void(const QHash<QString, QKeySequence>&)> apply;

  void commit() {
    QSettings store;
    store.setValue(QStringLiteral("shortcutsPrefs/keepModelKeysOut"), m_keepModelKeysOut);
    store.setValue(QStringLiteral("shortcutsPrefs/hidePreviewKey"), m_hidePreviewKey);
    QHash<QString, QKeySequence> changedKeys;
    for (const Row& row : m_rows) {
      const QKeySequence now = m_keys.value(row.id);
      if (now != row.key) changedKeys.insert(row.id, now);
    }
    if (!changedKeys.isEmpty() && apply) apply(changedKeys);
    for (Row& row : m_rows) row.key = m_keys.value(row.id);
    qDebug().noquote() << QStringLiteral("Preferences: shortcuts saved, %1 changed").arg(changedKeys.size());
  }

private:
  void buildTop() {
    auto* column = m_column;
    auto* top = new QHBoxLayout;
    top->setSpacing(10);
    m_filter = new QLineEdit;
    m_filter->setObjectName(name("filter"));
    m_filter->setPlaceholderText(tr("Filter commands or keys"));
    m_filter->setClearButtonEnabled(true);
    connect(m_filter, &QLineEdit::textChanged, this, [this] { refilter(); });
    top->addWidget(m_filter, 1);
    auto* done = new QPushButton(tr("Done"));
    done->setObjectName(name("done"));
    done->setDefault(false);
    done->setStyleSheet(QStringLiteral("QPushButton { background: %1; color: %2; border-radius: 6px; padding: 5px 14px; "
                                       "font-weight: 600; }")
                            .arg(palette().color(QPalette::Highlight).name(),
                                 palette().color(QPalette::HighlightedText).name()));
    connect(done, &QPushButton::clicked, this, [this] {
      stopRecording();
      m_filter->clear();
    });
    top->addWidget(done);
    auto* exportButton = new QPushButton(tr("⤒  Export"));
    exportButton->setObjectName(name("export"));
    connect(exportButton, &QPushButton::clicked, this, [this] { exportKeys(); });
    top->addWidget(exportButton);
    auto* importButton = new QPushButton(tr("⤓  Import"));
    importButton->setObjectName(name("import"));
    connect(importButton, &QPushButton::clicked, this, [this] { importKeys(); });
    top->addWidget(importButton);
    column->addLayout(top);
    check(column, tr("Keep model keys out of sketches"), "keepModelKeysOut", m_keepModelKeysOut,
          [this](bool on) { m_keepModelKeysOut = on; });
    auto* modifiers = new QLabel(tr("Modifier Keys"));
    column->addWidget(modifiers);
    auto* reselect = new QHBoxLayout;
    reselect->addWidget(new QLabel(tr("Hide preview for reselect")));
    auto* key = new QComboBox;
    key->setObjectName(name("hidePreviewKey"));
    key->addItems({tr("Ctrl + modifiers"), tr("Ctrl"), tr("Alt"), tr("Shift")});
    key->setCurrentIndex(m_hidePreviewKey);
    auto* reset = new QPushButton(tr("Reset"));
    reset->setEnabled(m_hidePreviewKey != 0);
    connect(key, &QComboBox::currentIndexChanged, this, [this, reset](int i) {
      m_hidePreviewKey = i;
      reset->setEnabled(i != 0);
      changed();
    });
    connect(reset, &QPushButton::clicked, this, [key] { key->setCurrentIndex(0); });
    reselect->addWidget(key);
    reselect->addWidget(reset);
    reselect->addStretch(1);
    column->addLayout(reselect);
    auto* rule = new QFrame;
    rule->setFrameShape(QFrame::HLine);
    rule->setStyleSheet(QStringLiteral("color: %1;").arg(palette().color(QPalette::Mid).name()));
    column->addWidget(rule);
  }

  // One row: the name at the left; at the right the key's chip and +, or
  // "Not assigned", then ↶ and ×.
  struct RowWidgets {
    QWidget* row = nullptr;
    QWidget* keys = nullptr;
    QLabel* chip = nullptr;
    QLabel* none = nullptr;
    QToolButton* add = nullptr;
    QToolButton* reset = nullptr;
    QToolButton* clear = nullptr;
    QKeySequenceEdit* recorder = nullptr;
    QLabel* warning = nullptr;
  };

  void buildRows() {
    QString group;
    for (const Row& row : m_rows) {
      if (row.group != group) {
        group = row.group;
        auto* caption = new QLabel(group);
        caption->setObjectName(name("group") + QLatin1Char('_') + group);
        QFont bold = caption->font();
        bold.setBold(true);
        bold.setLetterSpacing(QFont::AbsoluteSpacing, 0.5);
        caption->setFont(bold);
        m_column->addSpacing(8);
        m_column->addWidget(caption);
        m_captions.insert(group, caption);
      }
      RowWidgets w;
      w.row = new QWidget;
      w.row->setObjectName(name("row") + QLatin1Char('_') + row.id);
      w.row->setAttribute(Qt::WA_Hover);
      w.row->setAttribute(Qt::WA_StyledBackground);
      w.row->setProperty("shortcutRow", true);
      auto* line = new QHBoxLayout(w.row);
      line->setContentsMargins(8, 8, 8, 8);
      line->addWidget(new QLabel(row.name), 1);
      w.warning = new QLabel;
      w.warning->setProperty("shortcutWarning", true);
      w.warning->hide();
      line->addWidget(w.warning);
      w.chip = new QLabel;
      w.chip->setProperty("shortcutChip", true);
      w.chip->setCursor(Qt::PointingHandCursor);
      w.none = new QLabel(tr("Not assigned"));
      w.none->setForegroundRole(QPalette::PlaceholderText);
      w.none->setCursor(Qt::PointingHandCursor);
      const auto tool = [this](const QString& text, const QString& tip) {
        auto* b = new QToolButton;
        b->setText(text);
        b->setToolTip(tip);
        b->setAutoRaise(true);
        b->setFocusPolicy(Qt::NoFocus);
        return b;
      };
      w.add = tool(QStringLiteral("+"), tr("Record a key"));
      w.reset = tool(QStringLiteral("↶"), tr("Back to the default key"));
      w.clear = tool(QStringLiteral("×"), tr("No key"));
      w.recorder = new QKeySequenceEdit;
      w.recorder->setMaximumWidth(uiRoomSize(140));
      w.recorder->hide();
      line->addWidget(w.chip);
      line->addWidget(w.none);
      line->addWidget(w.recorder);
      line->addWidget(w.add);
      line->addWidget(w.reset);
      line->addWidget(w.clear);
      const QString id = row.id;
      w.chip->installEventFilter(this);
      w.none->installEventFilter(this);
      w.chip->setProperty("shortcutId", id);
      w.none->setProperty("shortcutId", id);
      connect(w.add, &QToolButton::clicked, this, [this, id] { startRecording(id); });
      connect(w.reset, &QToolButton::clicked, this, [this, id] { setKey(id, defaultOf(id)); });
      connect(w.clear, &QToolButton::clicked, this, [this, id] { setKey(id, QKeySequence()); });
      connect(w.recorder, &QKeySequenceEdit::editingFinished, this, [this, id] {
        const RowWidgets& rw = m_widgets[id];
        const QKeySequence recorded = rw.recorder->keySequence();
        stopRecording();
        if (!recorded.isEmpty()) setKey(id, QKeySequence(recorded[0]));
      });
      m_column->addWidget(w.row);
      m_widgets.insert(id, w);
      showKey(id);
    }
    // One style sheet for every row (one per row would make Preferences slow
    // to open with hundreds of commands).
    if (QWidget* content = m_column->parentWidget()) {
      content->setStyleSheet(QStringLiteral("QWidget[shortcutRow=\"true\"] { border-bottom: 1px solid %1; } "
                                            "QWidget[shortcutRow=\"true\"]:hover { background: rgba(128,128,128,26); "
                                            "border-radius: 6px; } "
                                            "QLabel[shortcutChip=\"true\"] { border: 1px solid %2; border-radius: 5px; "
                                            "padding: 1px 8px; background: rgba(128,128,128,30); } "
                                            "QLabel[shortcutWarning=\"true\"] { color: #b4441c; }")
                                 .arg(palette().color(QPalette::Mid).lighter(120).name(),
                                      palette().color(QPalette::Mid).name()));
    }
  }

  bool eventFilter(QObject* watched, QEvent* event) override {
    if (event->type() == QEvent::MouseButtonRelease) {
      const QString id = watched->property("shortcutId").toString();
      if (!id.isEmpty()) {
        startRecording(id);
        return true;
      }
    }
    return PreferenceKitPage::eventFilter(watched, event);
  }

  QKeySequence defaultOf(const QString& id) const {
    for (const Row& row : m_rows) {
      if (row.id == id) return row.defaultKey;
    }
    return {};
  }

  void showKey(const QString& id) {
    RowWidgets& w = m_widgets[id];
    const QKeySequence key = m_keys.value(id);
    const bool has = !key.isEmpty();
    w.chip->setText(key.toString(QKeySequence::NativeText));
    w.chip->setVisible(has && w.recorder->isHidden());
    w.none->setVisible(!has && w.recorder->isHidden());
    w.add->setVisible(has);
    w.reset->setEnabled(key != defaultOf(id));
    w.clear->setEnabled(has);
    const QString other = has && conflict ? conflict(id, key) : QString();
    w.warning->setText(other.isEmpty() ? QString() : tr("Also %1").arg(other));
    w.warning->setVisible(!other.isEmpty());
  }

  void setKey(const QString& id, const QKeySequence& key) {
    m_keys.insert(id, key);
    showKey(id);
    changed();
  }

  void startRecording(const QString& id) {
    stopRecording();
    RowWidgets& w = m_widgets[id];
    m_recording = id;
    w.chip->hide();
    w.none->hide();
    w.recorder->clear();
    w.recorder->show();
    w.recorder->setFocus();
  }
  void stopRecording() {
    if (m_recording.isEmpty()) return;
    const QString id = m_recording;
    m_recording.clear();
    m_widgets[id].recorder->hide();
    showKey(id);
  }

  void refilter() {
    const QString text = m_filter->text().trimmed();
    QHash<QString, bool> groupShown;
    for (const Row& row : m_rows) {
      const QString key = m_keys.value(row.id).toString(QKeySequence::NativeText);
      const bool shown = text.isEmpty() || row.name.contains(text, Qt::CaseInsensitive) ||
                         key.contains(text, Qt::CaseInsensitive) || row.group.contains(text, Qt::CaseInsensitive);
      m_widgets[row.id].row->setVisible(shown);
      if (shown) groupShown[row.group] = true;
    }
    for (auto it = m_captions.cbegin(); it != m_captions.cend(); ++it) {
      it.value()->setVisible(groupShown.value(it.key()));
    }
  }

  void exportKeys() {
    const QString path = QFileDialog::getSaveFileName(this, tr("Export shortcuts"), QStringLiteral("shortcuts.json"),
                                                      tr("Shortcuts (*.json)"));
    if (path.isEmpty()) return;
    QJsonObject keys;
    for (const Row& row : m_rows) {
      const QKeySequence key = m_keys.value(row.id);
      if (!key.isEmpty()) keys.insert(row.id, key.toString(QKeySequence::PortableText));
    }
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      file.write(QJsonDocument(QJsonObject{{QStringLiteral("purecadShortcuts"), keys}}).toJson());
      qDebug().noquote() << QStringLiteral("Preferences: shortcuts exported to %1").arg(path);
    }
  }
  void importKeys() {
    const QString path = QFileDialog::getOpenFileName(this, tr("Import shortcuts"), QString(), tr("Shortcuts (*.json)"));
    if (path.isEmpty()) return;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    const QJsonObject keys = QJsonDocument::fromJson(file.readAll()).object().value(QStringLiteral("purecadShortcuts")).toObject();
    for (const Row& row : m_rows) {
      if (keys.contains(row.id)) {
        m_keys.insert(row.id, QKeySequence(keys.value(row.id).toString(), QKeySequence::PortableText));
      } else {
        m_keys.insert(row.id, QKeySequence());
      }
      showKey(row.id);
    }
    changed();
    qDebug().noquote() << QStringLiteral("Preferences: shortcuts imported from %1").arg(path);
  }

  QVector<Row> m_rows;
  QHash<QString, QKeySequence> m_keys;
  QHash<QString, RowWidgets> m_widgets;
  QHash<QString, QLabel*> m_captions;
  QLineEdit* m_filter = nullptr;
  QString m_recording;
  bool m_keepModelKeysOut = false;
  int m_hidePreviewKey = 0;
};

} // namespace mitcad
