#include <QPainter>
#include <QPaintEvent>

#include "BWButton.h"

static const int margin = 20;

BWButton::BWButton(QString label, QWidget* parent) : QWidget(parent), m_label(label) {
  setMouseTracking(true);
  setFont(QFont("Open Sans", 12, QFont::Weight::Medium));
  setMinimumSize(QSize(label.length() * font().pointSize(), font().pointSize() + margin*2));
}

void BWButton::mouseMoveEvent(QMouseEvent *e) {
  bool changed = m_over;
	if (rect().marginsRemoved(QMargins()).contains(e->pos())) {
	  m_over = true;
	} else {
	  m_over = false;
	}
	changed = changed != m_over;
	updateCursor();
	if (changed) {
	  update();
	}
}

void BWButton::leaveEvent(QEvent *e)
{
  // if (rect().contains(mapFromGlobal(QCursor::pos())))
  m_over = false;
  update();
}

void BWButton::updateCursor()
{
  if (m_over) {
    setCursor(Qt::PointingHandCursor);
  } else {
    setCursor(Qt::ArrowCursor);
  }
}

void BWButton::paintEvent(QPaintEvent* event)
{
  QPainter p(this);

  // Background
  const bool overPaint = hasFocus() || m_over;
  p.fillRect(event->rect(), overPaint ? m_colBgOver : m_colBg);
  p.setPen(m_colText);
  p.drawText(margin, font().pointSize() + margin, m_label);
}
