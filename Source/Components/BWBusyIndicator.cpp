#include "BWBusyIndicator.h"

#include <QPainter>
#include <QPixmapCache>
#include <QGradient>
#include <QPropertyAnimation>
#include <qnamespace.h>

/* Inspired from https://github.com/anjinkristou/Qt-busy-indicator */

BWBusyIndicator::BWBusyIndicator(QWidget *parent) :
    QWidget(parent),
    m_startAngle(0)
{
  QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  policy.setHeightForWidth(true);
  setSizePolicy(policy);

  m_fillColor = palette().color(QPalette::WindowText);

  QPropertyAnimation anim;
  anim.setStartValue(m_startAngle);
  anim.startTimer(50, Qt::CoarseTimer);

  m_timer.setInterval(50);
  connect(&m_timer, SIGNAL(timeout()), this, SLOT(rotate()));
  m_timer.start();
}

void BWBusyIndicator::setVisible(bool visible)
{
  QWidget::setVisible(visible);
  if (visible) {
    m_timer.start();
  } else {
    m_timer.stop();
  }
}

void BWBusyIndicator::rotate()
{
  m_startAngle += 30;
  m_startAngle %= 360;
  update();
}

void BWBusyIndicator::setColor(QColor color)
{
  m_fillColor = color;
}

QPixmap BWBusyIndicator::generatePixmap(int side)
{
  QPixmap pixmap(QSize(side, side));
  pixmap.fill(QColor(255, 255, 255, 0));

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.translate(side / 2.0, side / 2.0);
  painter.scale(side / 200.0, side / 200.0);
  drawArcStyle(&painter);
  return pixmap;
}

void BWBusyIndicator::drawArcStyle(QPainter *painter)
{
  QColor color = m_fillColor;
  QConicalGradient gradient(0, 0, -m_startAngle);
  gradient.setColorAt(0, color);
  color.setAlpha(0);
  gradient.setColorAt(0.8, color);
  color.setAlpha(255);
  gradient.setColorAt(1, color);

  QPen pen;
  pen.setWidth(30);
  pen.setBrush(QBrush(gradient));
  painter->setPen(pen);

  painter->drawArc(-85, -85, 170, 170, 0 * 16, 360 * 16);
}

void BWBusyIndicator::paintEvent(QPaintEvent *)
{
  QString key = QString("%1:%2:%3:%4:0")
          .arg(metaObject()->className())
          .arg(width())
          .arg(height())
          .arg(m_startAngle);

  QPixmap pixmap;
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  int side = qMin(width(), height());

  if (!QPixmapCache::find(key, &pixmap)) {
      pixmap = generatePixmap(side);
      QPixmapCache::insert(key, pixmap);
  }

  painter.translate(width() / 2.0 - side / 2.0, height() / 2.0 - side / 2.0);
  painter.drawPixmap(0, 0, side, side, pixmap);
}

QSize BWBusyIndicator::minimumSizeHint() const
{
  return QSize(20, 20);
}

QSize BWBusyIndicator::sizeHint() const
{
  return QSize(30, 30);
}
