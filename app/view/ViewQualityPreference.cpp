// SPDX-License-Identifier: MIT
// Fork: Preferences › Display › Smoothness and Anti-aliasing
// (view/ViewQualityPreference.hpp).
#include "view/ViewQualityPreference.hpp"

#include <QSettings>
#include <QtGlobal>

namespace mitcad {
namespace {

constexpr const char* kSmoothnessKey = "view/smoothness";
constexpr const char* kAntialiasingKey = "view/antialiasing";
constexpr int kDefaultSamples = 4;

int offeredSamples(int samples) {
  return samples == 0 || samples == 2 || samples == 4 || samples == 8 ? samples : kDefaultSamples;
}

} // namespace

QString smoothnessId(Smoothness choice) {
  return choice == Smoothness::Standard ? QStringLiteral("standard") : QStringLiteral("smooth");
}

std::optional<Smoothness> smoothnessFromId(const QString& id) {
  if (id == QLatin1String("smooth")) {
    return Smoothness::Smooth;
  }
  if (id == QLatin1String("standard")) {
    return Smoothness::Standard;
  }
  return std::nullopt;
}

Smoothness defaultSmoothness() {
  return qEnvironmentVariableIsEmpty("MITCAD_TEST_INPUT") ? Smoothness::Smooth : Smoothness::Standard;
}

Smoothness savedSmoothness() {
  return smoothnessFromId(QSettings().value(QLatin1String(kSmoothnessKey)).toString()).value_or(defaultSmoothness());
}

void applySavedSmoothness() { setViewSmoothness(savedSmoothness()); }

bool saveSmoothness(Smoothness choice) {
  if (savedSmoothness() == choice && viewSmoothness() == choice) {
    return false;
  }
  {
    QSettings settings;
    settings.setValue(QLatin1String(kSmoothnessKey), smoothnessId(choice));
  }
  setViewSmoothness(choice);
  emit ViewQualityNotifier::instance().smoothnessChanged();
  return true;
}

int savedAntialiasing() {
  bool ok = false;
  const int samples = QSettings().value(QLatin1String(kAntialiasingKey), kDefaultSamples).toInt(&ok);
  return ok ? offeredSamples(samples) : kDefaultSamples;
}

bool saveAntialiasing(int samples) {
  samples = offeredSamples(samples);
  if (savedAntialiasing() == samples) {
    return false;
  }
  {
    QSettings settings;
    settings.setValue(QLatin1String(kAntialiasingKey), samples);
  }
  emit ViewQualityNotifier::instance().antialiasingChanged(samples);
  return true;
}

ViewQualityNotifier& ViewQualityNotifier::instance() {
  static ViewQualityNotifier notifier;
  return notifier;
}

} // namespace mitcad
