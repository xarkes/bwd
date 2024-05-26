#pragma once

#include <QWidget>
#include "BWStyle.h"

class BWButton : public QWidget
{
  Q_OBJECT;

public:
  BWButton(QString label, QWidget* parent=nullptr);
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

protected:
  QString m_label;
  bool m_over = false;
  QColor m_colBg = BWS::button;
  QColor m_colBgOver = BWS::buttonOver;
  QColor m_colText = BWS::text;

private:
  void updateCursor();
};
