#include <QPainter>
#include <QResizeEvent>
#include <QGraphicsDropShadowEffect>
#include <qnamespace.h>
#include <qwidget.h>

#include "BWLineEdit.h"

BWLineEdit::BWLineEdit(QString title, QWidget* parent) : m_title(title), QLineEdit(parent)
{
  QLineEdit::setFixedHeight(size().height() + font().pointSize());
  setAlignment(Qt::AlignmentFlag::AlignBottom);
  setTextMargins(m_margin, m_margin, m_margin, m_margin);
  m_shadow = new QGraphicsDropShadowEffect(this);
  m_shadow->setOffset(1,1);
  m_shadow->setBlurRadius(10);
  this->setGraphicsEffect(m_shadow);
}

void BWLineEdit::resizeEvent(QResizeEvent* event)
{
}

void BWLineEdit::paintEvent(QPaintEvent* event)
{
  if (hasFocus()) {
    m_shadow->setColor(QColorConstants::Blue);
  } else {
    m_shadow->setColor(QColorConstants::Black);
  }
  QLineEdit::paintEvent(event);

  auto p = QPainter(this);
  p.setPen(QColorConstants::Gray);
  p.drawText(QPoint(m_margin, p.font().pointSize() + m_margin), m_title);
}
