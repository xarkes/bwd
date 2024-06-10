#include <QPainter>
#include <QPaintEvent>

#include "BWEntry.h"

BWEntry::BWEntry(QString label, QString note, QWidget* parent) : QPushButton(parent), m_label(label), m_note(note) {
  setMouseTracking(true);
  setFont(QFont("Open Sans", 10, QFont::Weight::Normal));
  setContentsMargins(10, 10, 0, 0);
  setMinimumSize(QSize(label.length() * font().pointSize(), font().pointSize()*3.5 + contentsMargins().top() + contentsMargins().bottom()));
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

  icon().paint(&p, contentsMargins().left(), contentsMargins().top(), iconSize().rwidth(), iconSize().rheight());

  p.setPen(m_colText);
  p.drawText(contentsMargins().left() + iconSize().rwidth() + 10, font().pointSize() + contentsMargins().top(), m_label);
  p.setPen(m_colTextLow);
  p.drawText(contentsMargins().left() + iconSize().rwidth() + 10, font().pointSize()*2.5 + contentsMargins().top(), m_note);

  // Focus bar
  if (isChecked()) {
    p.fillRect(0, 0, focusBarWidth, event->rect().height(), m_colFocus);
  }
}
