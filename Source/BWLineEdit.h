#include <QLineEdit>
#include <QLabel>
#include <qgraphicseffect.h>
#include <qwidget.h>

class BWLineEdit : public QLineEdit
{
  Q_OBJECT;

public:
  BWLineEdit(QString title, QWidget* parent=nullptr);
  void resizeEvent(QResizeEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

private:
  QString m_title;
  QGraphicsDropShadowEffect* m_shadow;
  const int m_margin = 5;
};
