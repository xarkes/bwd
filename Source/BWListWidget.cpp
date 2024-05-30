#include "BWListWidget.h"
#include <QPainter>

BWListItem::BWListItem(QString label) : m_label(label)
{ }

BWListWidget::BWListWidget(QString label, QWidget* parent) : QWidget(parent), m_label(label)
{ }

// void BWListWidget::onc

void BWListWidget::paintEvent(QPaintEvent* e)
{
  QPainter p(this);
  auto r = rect();
  r.adjust(0, 0, -20, -20);
  p.drawRect(r);

  p.drawText(0, font().pointSize(), m_label);

  int i = 0;
  for (auto item : m_items) {
    p.drawText(0, font().pointSize() * (i*2 + 3), item->m_label);
    i++;
  }
}
