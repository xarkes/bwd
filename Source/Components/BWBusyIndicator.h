#pragma once

#include <QTimer>
#include <QWidget>

class BWBusyIndicator : public QWidget
{
  Q_OBJECT;
public:
  explicit BWBusyIndicator(QWidget *parent = 0);

  void paintEvent(QPaintEvent *);
  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void setColor(QColor color);
  void setVisible(bool);

private slots:
  void rotate();

private:
  QPixmap generatePixmap(int sideLength);
  void drawArcStyle(QPainter *painter);

  QTimer m_timer;
  int m_startAngle;

  QColor m_fillColor;
};
