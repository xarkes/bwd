#pragma once

#include <QPushButton>
#include "BWStyle.h"

class BWCategoryEntry : public QPushButton
{
  Q_OBJECT;

public:
  BWCategoryEntry(QString label, QString note="", QWidget* parent=nullptr);
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

protected:
  QString m_label;

  bool m_over = false;
  QColor m_colBgOver = BWS::buttonOverDark;
  QColor m_colText = BWS::textLight;
  QColor m_colFocus = BWS::main;

private:
  void updateCursor();
};
