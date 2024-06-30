#include <QPainter>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <qnamespace.h>
#include <qwidget.h>
#include <QLayout>

#include "BWField.h"

BWField::BWField(QString title, QWidget* parent) : QLabel(parent), m_title(title) 
{
  setMinimumHeight(font().pointSize() * 4);
  setAlignment(Qt::AlignmentFlag::AlignBottom);
}

void BWField::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  auto p = QPainter(this);
  p.setPen(QColorConstants::Blue);
  p.drawText(QPoint(0, font().pointSize()), m_title);
  p.setPen(QColorConstants::Black);
  p.drawText(QPoint(0, font().pointSize() * 3), text());
}

BWFieldConfidential::BWFieldConfidential(QString title, QWidget* parent) : BWField(title, parent) {
  m_revealed = false;
}

void BWFieldConfidential::toggle()
{
  m_revealed = !m_revealed;
  update();
}

void BWFieldConfidential::paintEvent(QPaintEvent* event)
{
  Q_UNUSED(event);
  auto p = QPainter(this);
  p.setPen(QColorConstants::Blue);
  p.drawText(QPoint(0, font().pointSize()), m_title);
  p.setPen(QColorConstants::Black);
  QString content = m_revealed ? text() : QString("●").repeated(14);
  p.drawText(QPoint(0, font().pointSize() * 3), content);
}
