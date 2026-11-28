// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

// Fork: Preferences › Design, as SolidCraft's: Units and precision, Body
// check, Drawings, Orientation, Imported files, Export, Folders, Origin,
// Linked designs and Assemblies. Units of new designs is the General page's
// setting (set through `applyUnits`); the rest is DesignPrefs ("design/").

#include <QComboBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>

#include "PreferenceKit.hpp"

namespace mitcad {

struct DesignPrefs {
  int lengthDecimals = 2;
  int angleDecimals = 1;
  int parameterRows = 50;
  bool stepValues = true;           // values step as they always did (ValueStep.hpp)
  bool valuesBesideHandles = true;
  int kernelCheck = 1;              // 0 After every step, 1 Once per feature, 2 Off
  bool thirdAngle = false;          // projection of new drawings (metric default: first angle)
  QString partTemplate = QStringLiteral("ISO A3");
  QString assemblyTemplate = QStringLiteral("ISO A2");
  int sheetColor = 0;               // 0 White, 1 Cream, 2 Gray, 3 Dark
  int sheetsSection = 0;            // 0 Multiple sheets only, 1 Always, 2 Never
  int trueSize = 1000;              // calibration × 1000
  bool yUp = false;                 // orientation of new designs
  bool offerStandUp = true;
  int unknownFiles = 0;             // 0 Note, 1 Ask, 2 Stand up, 3 Keep
  bool rememberStandUp = true;
  bool reuseImports = true;
  bool healImports = true;
  QString exportName;
  bool glbKeepsParts = true;
  QString designsFolder;            // "" the folder last used
  QString exportsFolder;
  bool topFrontRight = false;       // origin plane names
  int originPlaneSize = 100;        // percent
  bool hideOriginFolder = false;
  bool originAxesAcross = false;
  bool originItemUnderPointer = true;
  bool updateLinks = true;
  bool doubleClickActivates = false;  // a double-click renames, as before (BrowserController)
  bool insertListsOpenFirst = true;
  bool tabsNamedAfterFile = true;
  bool invertJointOrder = false;
  bool ballJointTrackball = true;
  bool pasteAtOrigin = false;

  static DesignPrefs load() {
    const QSettings store;
    DesignPrefs s;
    const QString g = QStringLiteral("design/");
    const auto flag = [&](const char* n, bool& v) { v = store.value(g + QLatin1String(n), v).toBool(); };
    const auto number = [&](const char* n, int& v, int low, int high) {
      bool ok = false;
      const int read = store.value(g + QLatin1String(n), v).toInt(&ok);
      if (ok) v = std::clamp(read, low, high);
    };
    const auto text = [&](const char* n, QString& v) { v = store.value(g + QLatin1String(n), v).toString(); };
    number("lengthDecimals", s.lengthDecimals, 0, 8);
    number("angleDecimals", s.angleDecimals, 0, 6);
    number("parameterRows", s.parameterRows, 10, 500);
    flag("stepValues", s.stepValues);
    flag("valuesBesideHandles", s.valuesBesideHandles);
    number("kernelCheck", s.kernelCheck, 0, 2);
    flag("thirdAngle", s.thirdAngle);
    text("partTemplate", s.partTemplate);
    text("assemblyTemplate", s.assemblyTemplate);
    number("sheetColor", s.sheetColor, 0, 3);
    number("sheetsSection", s.sheetsSection, 0, 2);
    number("trueSize", s.trueSize, 500, 2000);
    flag("yUp", s.yUp);
    flag("offerStandUp", s.offerStandUp);
    number("unknownFiles", s.unknownFiles, 0, 3);
    flag("rememberStandUp", s.rememberStandUp);
    flag("reuseImports", s.reuseImports);
    flag("healImports", s.healImports);
    text("exportName", s.exportName);
    flag("glbKeepsParts", s.glbKeepsParts);
    text("designsFolder", s.designsFolder);
    text("exportsFolder", s.exportsFolder);
    flag("topFrontRight", s.topFrontRight);
    number("originPlaneSize", s.originPlaneSize, 25, 400);
    flag("hideOriginFolder", s.hideOriginFolder);
    flag("originAxesAcross", s.originAxesAcross);
    flag("originItemUnderPointer", s.originItemUnderPointer);
    flag("updateLinks", s.updateLinks);
    flag("doubleClickActivates", s.doubleClickActivates);
    flag("insertListsOpenFirst", s.insertListsOpenFirst);
    flag("tabsNamedAfterFile", s.tabsNamedAfterFile);
    flag("invertJointOrder", s.invertJointOrder);
    flag("ballJointTrackball", s.ballJointTrackball);
    flag("pasteAtOrigin", s.pasteAtOrigin);
    return s;
  }
  void save() const {
    QSettings store;
    const QString g = QStringLiteral("design/");
    const auto put = [&](const char* n, const QVariant& v) { store.setValue(g + QLatin1String(n), v); };
    put("lengthDecimals", lengthDecimals);
    put("angleDecimals", angleDecimals);
    put("parameterRows", parameterRows);
    put("stepValues", stepValues);
    put("valuesBesideHandles", valuesBesideHandles);
    put("kernelCheck", kernelCheck);
    put("thirdAngle", thirdAngle);
    put("partTemplate", partTemplate);
    put("assemblyTemplate", assemblyTemplate);
    put("sheetColor", sheetColor);
    put("sheetsSection", sheetsSection);
    put("trueSize", trueSize);
    put("yUp", yUp);
    put("offerStandUp", offerStandUp);
    put("unknownFiles", unknownFiles);
    put("rememberStandUp", rememberStandUp);
    put("reuseImports", reuseImports);
    put("healImports", healImports);
    put("exportName", exportName);
    put("glbKeepsParts", glbKeepsParts);
    put("designsFolder", designsFolder);
    put("exportsFolder", exportsFolder);
    put("topFrontRight", topFrontRight);
    put("originPlaneSize", originPlaneSize);
    put("hideOriginFolder", hideOriginFolder);
    put("originAxesAcross", originAxesAcross);
    put("originItemUnderPointer", originItemUnderPointer);
    put("updateLinks", updateLinks);
    put("doubleClickActivates", doubleClickActivates);
    put("insertListsOpenFirst", insertListsOpenFirst);
    put("tabsNamedAfterFile", tabsNamedAfterFile);
    put("invertJointOrder", invertJointOrder);
    put("ballJointTrackball", ballJointTrackball);
    put("pasteAtOrigin", pasteAtOrigin);
  }
};

// A ruler 100 mm long at the calibration (True size calibration).
class CalibrationRuler : public QWidget {
public:
  explicit CalibrationRuler(const int* calibration) : m_calibration(calibration) {
    setMinimumHeight(uiRoomSize(44));
  }

protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const double pxPerMm = logicalDpiX() / 25.4 * (*m_calibration / 1000.0);
    const double length = std::min(width() - 20.0, 100.0 * pxPerMm);
    const QColor ink = palette().color(QPalette::WindowText);
    p.setPen(QPen(ink, 1.4));
    const double y = 10;
    p.drawLine(QPointF(4, y), QPointF(4 + length, y));
    for (int mm = 0; mm <= 100; mm += 10) {
      const double x = 4 + mm * length / 100.0;
      p.drawLine(QPointF(x, y), QPointF(x, y + (mm % 50 == 0 ? 12 : 7)));
    }
    p.drawText(QRectF(4 + length - 80, y + 12, 80, 20), Qt::AlignRight | Qt::AlignVCenter, QStringLiteral("100 mm"));
  }

private:
  const int* m_calibration;
};

class DesignPreferences : public PreferenceKitPage {
  Q_DECLARE_TR_FUNCTIONS(mitcad::DesignPreferences)

public:
  explicit DesignPreferences(const QString& units, QWidget* parent = nullptr)
      : PreferenceKitPage(QStringLiteral("design"), parent), m_prefs(DesignPrefs::load()), m_units(units) {
    buildUnits();
    buildBodyCheck();
    buildDrawings();
    buildOrientation();
    buildImported();
    buildExport();
    buildFolders();
    buildOrigin();
    buildLinked();
    buildAssemblies();
    finish();
  }

  std::function<void(const QString&)> applyUnits;

  void commit() {
    m_prefs.save();
    qDebug().noquote() << QStringLiteral("Preferences: design saved");
  }

private:
  QString lengthSample() const {
    return QStringLiteral("%1 %2").arg(12.5, 0, 'f', m_prefs.lengthDecimals).arg(m_units);
  }

  void buildUnits() {
    QVBoxLayout* column = section(tr("Units and precision"));
    QFormLayout* f = form(column);
    const QStringList units{QStringLiteral("mm"), QStringLiteral("cm"), QStringLiteral("m"), QStringLiteral("in"),
                            QStringLiteral("ft")};
    auto* lengthShown = new QLabel;
    QFont mono = lengthShown->font();
    mono.setFamily(QStringLiteral("monospace"));
    lengthShown->setFont(mono);
    f->addRow(tr("Units of new designs"),
              segments("units", units, std::max(0, int(units.indexOf(m_units))), [this, units, lengthShown](int i) {
                m_units = units.value(i);
                lengthShown->setText(lengthSample());
                if (applyUnits) applyUnits(m_units);
              }));
    const auto spinRow = [this, mono](int low, int high, int value, QLabel* shown, std::function<void(int)> apply) {
      auto* row = new QWidget;
      auto* line = new QHBoxLayout(row);
      line->setContentsMargins(0, 0, 0, 0);
      line->setSpacing(14);
      auto* spin = new QSpinBox;
      spin->setRange(low, high);
      spin->setValue(value);
      spin->setMinimumWidth(uiRoomSize(110));
      connect(spin, &QSpinBox::valueChanged, this, [this, apply](int v) {
        apply(v);
        changed();
      });
      line->addWidget(spin);
      if (shown != nullptr) {
        shown->setFont(mono);
        line->addWidget(shown);
      }
      line->addStretch(1);
      return row;
    };
    lengthShown->setText(lengthSample());
    f->addRow(tr("Length decimals"), spinRow(0, 8, m_prefs.lengthDecimals, lengthShown, [this, lengthShown](int v) {
                m_prefs.lengthDecimals = v;
                lengthShown->setText(lengthSample());
              }));
    auto* angleShown = new QLabel(QStringLiteral("%1°").arg(45.0, 0, 'f', m_prefs.angleDecimals));
    f->addRow(tr("Angle decimals"), spinRow(0, 6, m_prefs.angleDecimals, angleShown, [this, angleShown](int v) {
                m_prefs.angleDecimals = v;
                angleShown->setText(QStringLiteral("%1°").arg(45.0, 0, 'f', v));
              }));
    f->addRow(label(tr("Parameter list rows"), QString(), tr("How many parameters the Parameters list shows before it pages")),
              spinRow(10, 500, m_prefs.parameterRows, nullptr, [this](int v) { m_prefs.parameterRows = v; }));
    check(column, tr("Step values with arrows and wheel"), "stepValues", m_prefs.stepValues,
          [this](bool on) { m_prefs.stepValues = on; },
          tr("Up and Down and the mouse wheel step a value field by its unit"));
    check(column, tr("Show values beside their handles"), "valuesBesideHandles", m_prefs.valuesBesideHandles,
          [this](bool on) { m_prefs.valuesBesideHandles = on; }, tr("A dragged handle shows its value next to it"));
  }

  void buildBodyCheck() {
    QVBoxLayout* column = section(tr("Body check"));
    QFormLayout* f = form(column);
    f->addRow(tr("Full kernel check"),
              segments("kernelCheck", {tr("After every step"), tr("Once per feature"), tr("Off (experimental)")},
                       m_prefs.kernelCheck, [this](int i) { m_prefs.kernelCheck = i; }));
    auto* note = new QLabel(tr("After every step: checks each sweep, boolean and blend.\nOnce per feature: checks each "
                               "feature's result once.\nOff (experimental): no check."));
    note->setForegroundRole(QPalette::PlaceholderText);
    column->addWidget(note);
  }

  QComboBox* combo(const char* part, const QStringList& items, int index, std::function<void(int)> apply) {
    auto* box = new QComboBox;
    box->setObjectName(name(part));
    box->addItems(items);
    box->setCurrentIndex(std::max(0, index));
    box->setMinimumWidth(uiRoomSize(420));
    connect(box, &QComboBox::currentIndexChanged, this, [this, apply](int i) {
      apply(i);
      changed();
    });
    return box;
  }

  void buildDrawings() {
    QVBoxLayout* column = section(tr("Drawings"));
    QFormLayout* f = form(column);
    f->addRow(tr("Projection of new drawings"), segments("projection", {tr("First angle"), tr("Third angle")},
                                                         m_prefs.thirdAngle ? 1 : 0,
                                                         [this](int i) { m_prefs.thirdAngle = i == 1; }));
    const QStringList templates{QStringLiteral("ISO A4"), QStringLiteral("ISO A3"), QStringLiteral("ISO A2"),
                                QStringLiteral("ISO A1"), QStringLiteral("ISO A0"), QStringLiteral("ANSI A"),
                                QStringLiteral("ANSI B"), QStringLiteral("ANSI C"), QStringLiteral("ANSI D")};
    f->addRow(tr("Template for new part drawings"),
              combo("partTemplate", templates, int(templates.indexOf(m_prefs.partTemplate)),
                    [this, templates](int i) { m_prefs.partTemplate = templates.value(i); }));
    f->addRow(tr("Template for new assembly drawings"),
              combo("assemblyTemplate", templates, int(templates.indexOf(m_prefs.assemblyTemplate)),
                    [this, templates](int i) { m_prefs.assemblyTemplate = templates.value(i); }));
    f->addRow(tr("Sheet color"), combo("sheetColor", {tr("White"), tr("Cream"), tr("Gray"), tr("Dark")},
                                       m_prefs.sheetColor, [this](int i) { m_prefs.sheetColor = i; }));
    f->addRow(tr("Sheets section"), combo("sheetsSection", {tr("Multiple sheets only"), tr("Always"), tr("Never")},
                                          m_prefs.sheetsSection, [this](int i) { m_prefs.sheetsSection = i; }));
    auto* ruler = new CalibrationRuler(&m_prefs.trueSize);
    QWidget* calibration = slider("trueSize", 500, 2000, m_prefs.trueSize,
                                  [](int v) { return QStringLiteral("%1×").arg(v / 1000.0, 0, 'f', 3); },
                                  [this, ruler](int v) {
                                    m_prefs.trueSize = v;
                                    ruler->update();
                                  });
    calibration->setMinimumWidth(uiRoomSize(420));
    f->addRow(tr("True size calibration"), calibration);
    column->addWidget(ruler);
    auto* note = new QLabel(tr("Hold a ruler to the bar and move the calibration until the bar measures 100 mm: at "
                               "100 % the drawing sheet then shows 1 mm as 1 mm. The sheet color is for the screen "
                               "only; printed and exported sheets are always white."));
    note->setWordWrap(true);
    column->addWidget(note);
  }

  void buildOrientation() {
    QVBoxLayout* column = section(tr("Orientation"));
    const auto axes = [](bool yUp) {
      return [yUp](QPainter& p, const QRectF& r, const QPalette& pal) {
        const QPointF o = r.center() + QPointF(0, 12);
        QColor plane = pal.color(QPalette::WindowText);
        plane.setAlphaF(0.08f);
        p.setPen(Qt::NoPen);
        p.setBrush(plane);
        p.drawPolygon(QPolygonF{o + QPointF(-34, 0), o + QPointF(0, -14), o + QPointF(34, 0), o + QPointF(0, 14)});
        const auto arrow = [&p, &o](const QPointF& to, const QColor& color, const QString& name) {
          p.setPen(QPen(color, 2));
          p.drawLine(o, o + to);
          p.setPen(pal_text(color));
          p.drawText(QRectF(o.x() + to.x() * 1.25 - 10, o.y() + to.y() * 1.25 - 10, 20, 20), Qt::AlignCenter, name);
        };
        if (yUp) {
          arrow(QPointF(0, -38), QColor(46, 150, 52), QStringLiteral("Y"));
          arrow(QPointF(-30, 16), QColor(30, 70, 210), QStringLiteral("Z"));
        } else {
          arrow(QPointF(0, -38), QColor(30, 70, 210), QStringLiteral("Z"));
          arrow(QPointF(30, -14), QColor(46, 150, 52), QStringLiteral("Y"));
        }
        arrow(QPointF(32, 16), QColor(214, 48, 40), QStringLiteral("X"));
        p.setPen(Qt::NoPen);
        p.setBrush(pal.color(QPalette::WindowText));
        p.drawEllipse(o, 2.5, 2.5);
      };
    };
    choiceCards(column, "orientation", {{tr("Z up"), tr("default"), axes(false)}, {tr("Y up"), QString(), axes(true)}},
                m_prefs.yUp ? 1 : 0, [this](int i) { m_prefs.yUp = i == 1; }, 170, 150);
    check(column, tr("Offer to stand up Y-up files"), "offerStandUp", m_prefs.offerStandUp,
          [this](bool on) { m_prefs.offerStandUp = on; },
          tr("A file made Y up opens with an offer to turn it Z up"));
  }
  static QPen pal_text(const QColor& color) { return QPen(color.darker(120)); }

  void buildImported() {
    QVBoxLayout* column = section(tr("Imported files"));
    QFormLayout* f = form(column);
    auto* unknown = combo("unknownFiles", {tr("Note"), tr("Ask"), tr("Stand up"), tr("Keep as is")}, m_prefs.unknownFiles,
                          [this](int i) { m_prefs.unknownFiles = i; });
    unknown->setMinimumWidth(uiRoomSize(240));
    f->addRow(label(tr("Files from unknown programs"), QString(),
                    tr("What to do with a file whose program does not say which way is up")),
              unknown);
    const auto withButton = [this, column](const QString& title, const char* part, bool& field, const QString& button,
                                           std::function<void()> press) {
      auto* row = new QHBoxLayout;
      row->addWidget(makeCheck(title, part, field, [&field](bool on) { field = on; }));
      auto* b = new QPushButton(button);
      connect(b, &QPushButton::clicked, this, [press] { press(); });
      row->addWidget(b);
      row->addStretch(1);
      column->addLayout(row);
    };
    withButton(tr("Remember stand-up answers for files I've opened"), "rememberStandUp", m_prefs.rememberStandUp,
               tr("Forget remembered answers"), [] { QSettings().remove(QStringLiteral("design/standUpAnswers")); });
    withButton(tr("Reuse imports I've opened before"), "reuseImports", m_prefs.reuseImports, tr("Clear"),
               [] { QSettings().remove(QStringLiteral("design/importCache")); });
    check(column, tr("Heal imported parts that fail the check"), "healImports", m_prefs.healImports,
          [this](bool on) { m_prefs.healImports = on; });
  }

  void buildExport() {
    QVBoxLayout* column = section(tr("Export"));
    QFormLayout* f = form(column);
    auto* name_ = new QLineEdit(m_prefs.exportName);
    name_->setObjectName(name("exportName"));
    name_->setPlaceholderText(tr("e.g. {design}-{body}"));
    name_->setMinimumWidth(uiRoomSize(420));
    connect(name_, &QLineEdit::editingFinished, this, [this, name_] {
      m_prefs.exportName = name_->text();
      changed();
    });
    f->addRow(label(tr("Export file name"), QString(),
                    tr("The name exports start with: {design}, {body}, {component} and {date} are filled in")),
              name_);
    check(column, tr("GLB export keeps the assembly's parts"), "glbKeepsParts", m_prefs.glbKeepsParts,
          [this](bool on) { m_prefs.glbKeepsParts = on; }, tr("Each component stays its own node in a GLB file"));
  }

  QWidget* folder(const char* part, QString& field) {
    auto* row = new QWidget;
    auto* line = new QHBoxLayout(row);
    line->setContentsMargins(0, 0, 0, 0);
    auto* path = new QLineEdit(field);
    path->setObjectName(name(part));
    path->setPlaceholderText(tr("Folder last used"));
    path->setMinimumWidth(uiRoomSize(360));
    auto* browse = new QPushButton(tr("Browse…"));
    connect(path, &QLineEdit::editingFinished, this, [this, path, &field] {
      field = path->text();
      changed();
    });
    connect(browse, &QPushButton::clicked, this, [this, path, &field] {
      const QString chosen = QFileDialog::getExistingDirectory(this, tr("Choose a folder"), field);
      if (chosen.isEmpty()) return;
      field = chosen;
      path->setText(chosen);
      changed();
    });
    line->addWidget(path, 1);
    line->addWidget(browse);
    return row;
  }

  void buildFolders() {
    QVBoxLayout* column = section(tr("Folders"));
    QFormLayout* f = form(column);
    f->addRow(label(tr("Designs folder"), QString(), tr("Where Open and Save start; empty: the folder last used")),
              folder("designsFolder", m_prefs.designsFolder));
    f->addRow(label(tr("Exports folder"), QString(), tr("Where exports start; empty: the folder last used")),
              folder("exportsFolder", m_prefs.exportsFolder));
  }

  void buildOrigin() {
    QVBoxLayout* column = section(tr("Origin"));
    QFormLayout* f = form(column);
    f->addRow(tr("Origin plane names"), segments("planeNames", {tr("XY . XZ . YZ"), tr("Top . Front . Right")},
                                                 m_prefs.topFrontRight ? 1 : 0,
                                                 [this](int i) { m_prefs.topFrontRight = i == 1; }));
    QWidget* size = slider("originPlaneSize", 25, 400, m_prefs.originPlaneSize,
                           [](int v) { return QStringLiteral("%1 %").arg(v); },
                           [this](int v) { m_prefs.originPlaneSize = v; });
    size->setMinimumWidth(uiRoomSize(420));
    f->addRow(tr("Origin plane size"), size);
    check(column, tr("Hide the Origin folder in new designs"), "hideOriginFolder", m_prefs.hideOriginFolder,
          [this](bool on) { m_prefs.hideOriginFolder = on; });
    check(column, tr("Origin axes across the whole view"), "originAxesAcross", m_prefs.originAxesAcross,
          [this](bool on) { m_prefs.originAxesAcross = on; });
    check(column, tr("Show the Origin item under the pointer in the Browser"), "originItemUnderPointer",
          m_prefs.originItemUnderPointer, [this](bool on) { m_prefs.originItemUnderPointer = on; });
  }

  void buildLinked() {
    QVBoxLayout* column = section(tr("Linked designs"));
    check(column, tr("Update changed links when a tab comes back"), "updateLinks", m_prefs.updateLinks,
          [this](bool on) { m_prefs.updateLinks = on; });
    check(column, tr("Double-click a component to activate it"), "doubleClickActivates", m_prefs.doubleClickActivates,
          [this](bool on) { m_prefs.doubleClickActivates = on; },
          tr("A double-click on a component in the view or the Browser makes it the active one"));
    check(column, tr("Insert Design lists open designs first"), "insertListsOpenFirst", m_prefs.insertListsOpenFirst,
          [this](bool on) { m_prefs.insertListsOpenFirst = on; });
    check(column, tr("Name tabs and the Browser's top row after the design's file"), "tabsNamedAfterFile",
          m_prefs.tabsNamedAfterFile, [this](bool on) { m_prefs.tabsNamedAfterFile = on; });
  }

  void buildAssemblies() {
    QVBoxLayout* column = section(tr("Assemblies"));
    check(column, tr("Invert joint selection order"), "invertJointOrder", m_prefs.invertJointOrder,
          [this](bool on) { m_prefs.invertJointOrder = on; });
    check(column, tr("Turn parts on a ball joint like a trackball when dragged"), "ballJointTrackball",
          m_prefs.ballJointTrackball, [this](bool on) { m_prefs.ballJointTrackball = on; },
          tr("A component on a ball joint turns freely under the pointer"));
    check(column, tr("Paste components from another design at the origin"), "pasteAtOrigin", m_prefs.pasteAtOrigin,
          [this](bool on) { m_prefs.pasteAtOrigin = on; },
          tr("Pasted components land at the origin instead of where they were"));
  }

  DesignPrefs m_prefs;
  QString m_units;
};

} // namespace mitcad
