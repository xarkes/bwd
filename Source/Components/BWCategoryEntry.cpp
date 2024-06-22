#include <QPainter>
#include <QPaintEvent>

#include "BWCategoryEntry.h"

BWCategoryEntry::BWCategoryEntry(QString label, QWidget* parent) : QPushButton(parent), m_label(label) {
  setMouseTracking(true);
  setFont(QFont("Open Sans", 10, QFont::Weight::Normal));
  setContentsMargins(5, 5, 5, 5);
  setMinimumSize(QSize(label.length() * font().pointSize(), font().pointSize()*3 + contentsMargins().top() + contentsMargins().bottom()));
  setCheckable(true);
}

void BWCategoryEntry::mouseMoveEvent(QMouseEvent *e) {
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

void BWCategoryEntry::leaveEvent(QEvent *e)
{
  Q_UNUSED(e);
  // if (rect().contains(mapFromGlobal(QCursor::pos())))
  m_over = false;
  update();
}

void BWCategoryEntry::updateCursor()
{
  if (m_over) {
    setCursor(Qt::PointingHandCursor);
  } else {
    setCursor(Qt::ArrowCursor);
  }
}

void BWCategoryEntry::paintEvent(QPaintEvent* event)
{
  QPainter p(this);

  const bool paintBg = isChecked() || m_over;

  // Background
  if (paintBg) {
    p.fillRect(event->rect(), m_colBgOver);
  }
  p.setPen(m_colText);
  p.drawText(contentsMargins().left(), contentsMargins().top() + height() / 2, m_label);
}
