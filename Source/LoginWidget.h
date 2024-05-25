#pragma once

#include <QWidget>

class QPushButton;
class BWLineEdit;

class LoginWidget : public QWidget {
  Q_OBJECT;
public:
  LoginWidget();

private:
  BWLineEdit *m_inputEmail;
  QPushButton *m_buttonLogin;
};
