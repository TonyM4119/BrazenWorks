// SPDX-License-Identifier: MIT
// Fork: the product's name in the interface. The MitCad code says
// "MitCad" in its texts; BrandTranslator, asked before every other
// translator, swaps the upstream name for the product's (branding.json),
// in English and in every interface language, so those texts stay as
// upstream wrote them and merge cleanly.
#pragma once

#include <brand.hpp>

#include <QCoreApplication>
#include <QString>
#include <QTranslator>

namespace mitcad {

// The upstream name swapped for the product's in a text.
inline QString branded(QString text) {
  static const QString upstream = QString::fromUtf8(brand::kUpstreamName);
  static const QString product = QString::fromUtf8(brand::kDisplayName);
  if (upstream != product) text.replace(upstream, product);
  return text;
}

class BrandTranslator : public QTranslator {
 public:
  using QTranslator::QTranslator;
  bool isEmpty() const override { return false; }
  QString translate(const char* context, const char* source, const char* disambiguation,
                    int n) const override {
    // The text as the other translators give it (asked with this one out
    // of the way), then the name swapped; empty when there is nothing to
    // swap, so Qt goes on as without this translator.
    thread_local bool asking = false;
    if (asking || source == nullptr) return {};
    asking = true;
    const QString text = QCoreApplication::translate(context, source, disambiguation, n);
    asking = false;
    if (!text.contains(QString::fromUtf8(brand::kUpstreamName))) return {};
    return branded(text);
  }
};

// Installs BrandTranslator; call after the interface language's
// translators (the last installed is asked first).
inline void installBrandTranslator() {
  static BrandTranslator* translator = nullptr;
  if (translator) QCoreApplication::removeTranslator(translator);
  else translator = new BrandTranslator(QCoreApplication::instance());
  QCoreApplication::installTranslator(translator);
}

}  // namespace mitcad
