// SPDX-License-Identifier: AGPL-3.0-or-later
// Fork: the interface's motion (framework/Motion.hpp).
#include "framework/Motion.hpp"

#include <QEasingCurve>
#include <QPointer>
#include <QVariantAnimation>
#include <QtGlobal>

#include "framework/GlassCard.hpp"

namespace mitcad::motion {
namespace {

constexpr const char* kFadeName = "motionFade";

// The card's running fade, stopped and removed: a new one starts from the
// opacity the card has now.
void stopFade(GlassCard* card) {
  if (auto* running = card->findChild<QVariantAnimation*>(QLatin1String(kFadeName), Qt::FindDirectChildrenOnly)) {
    running->stop();
    running->deleteLater();
    running->setObjectName(QString());
  }
}

QVariantAnimation* fade(GlassCard* card, qreal to, int duration, QEasingCurve::Type curve) {
  stopFade(card);
  auto* animation = new QVariantAnimation(card);
  animation->setObjectName(QLatin1String(kFadeName));
  animation->setDuration(duration);
  animation->setStartValue(card->opacityLevel());
  animation->setEndValue(to);
  animation->setEasingCurve(curve);
  QObject::connect(animation, &QVariantAnimation::valueChanged, card,
                   [card](const QVariant& value) { card->setOpacityLevel(value.toReal()); });
  return animation;
}

} // namespace

bool enabled() {
  static const bool on = qEnvironmentVariableIsEmpty("MITCAD_TEST_INPUT");
  return on;
}

void fadeIn(GlassCard* card) {
  if (card == nullptr) {
    return;
  }
  const bool wasHidden = card->isHidden();
  if (!enabled()) {
    stopFade(card);
    card->setOpacityLevel(1.0);
    card->show();
    return;
  }
  if (wasHidden) {
    card->setOpacityLevel(0.0);
  }
  card->show();
  if (card->opacityLevel() >= 1.0) {
    stopFade(card);
    return;
  }
  fade(card, 1.0, kNormal, QEasingCurve::OutCubic)->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeOut(GlassCard* card, const std::function<void()>& done) {
  if (card == nullptr) {
    return;
  }
  if (!enabled() || card->isHidden()) {
    stopFade(card);
    card->hide();
    card->setOpacityLevel(1.0);
    if (done) {
      done();
    }
    return;
  }
  QVariantAnimation* animation = fade(card, 0.0, kFast, QEasingCurve::InQuad);
  QPointer<GlassCard> guarded(card);
  QObject::connect(animation, &QAbstractAnimation::finished, card, [guarded, done] {
    if (guarded) {
      guarded->hide();
      guarded->setOpacityLevel(1.0); // the next show starts opaque unless faded in
    }
    if (done) {
      done();
    }
  });
  animation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace mitcad::motion
