#include <QLineEdit>
#include <QLabel>
#include <qgraphicseffect.h>
#include <qpushbutton.h>
#include <qwidget.h>

class QPushButton;

class BWField : public QLabel
{
  Q_OBJECT;

public:
  BWField(QString title, QWidget* parent=nullptr);
  void paintEvent(QPaintEvent* event) override;

protected:
  QString m_title;
};

class BWFieldConfidential : public BWField
{
  Q_OBJECT;

public:
  BWFieldConfidential(QString title, QWidget* parent=nullptr);
  void paintEvent(QPaintEvent* event) override;
  void toggle();

private:
  bool m_revealed = false;
};
