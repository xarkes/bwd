#include <QPainter>
#include <QPaintEvent>

#include "BWEntry.h"

BWEntry::BWEntry(QString label, QString note, QWidget* parent) : QPushButton(parent), m_label(label), m_note(note) {
  setMouseTracking(true);
  setFont(QFont("Open Sans", 10, QFont::Weight::Normal));
  setContentsMargins(20, 10, 0, 0);
  setMinimumSize(QSize(label.length() * font().pointSize(), font().pointSize()*3 + contentsMargins().top() + contentsMargins().bottom()));
  setCheckable(true);
}

void BWEntry::mouseMoveEvent(QMouseEvent *e) {
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

void BWEntry::leaveEvent(QEvent *e)
{
  // if (rect().contains(mapFromGlobal(QCursor::pos())))
  m_over = false;
  update();
}

void BWEntry::updateCursor()
{
  if (m_over) {
    setCursor(Qt::PointingHandCursor);
  } else {
    setCursor(Qt::ArrowCursor);
  }
}

void BWEntry::paintEvent(QPaintEvent* event)
{
  QPainter p(this);

  const int focusBarWidth = 5;
  const bool overPaint = m_over;

  // Background
  p.fillRect(event->rect(), overPaint ? m_colBgOver : m_colBg);
  p.setPen(m_colText);
  p.drawText(contentsMargins().left(), font().pointSize() + contentsMargins().top(), m_label);
  p.setPen(m_colTextLow);
  p.drawText(contentsMargins().left(), font().pointSize()*2 + contentsMargins().top(), m_note);

  // Focus bar
  if (isChecked()) {
    p.fillRect(0, 0, focusBarWidth, event->rect().height(), m_colFocus);
  }
}
