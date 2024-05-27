#pragma once

#include <QPushButton>
#include "BWStyle.h"

class BWEntry : public QPushButton
{
  Q_OBJECT;

public:
  BWEntry(QString label, QString note="", QWidget* parent=nullptr);
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;

protected:
  QString m_label;
  QString m_note;

  bool m_over = false;
  QColor m_colBg = BWS::button;
  QColor m_colBgOver = BWS::buttonOver;
  QColor m_colText = BWS::text;
  QColor m_colTextLow = BWS::textLow;
  QColor m_colFocus = BWS::main;

private:
  void updateCursor();
};
