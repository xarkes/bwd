#pragma once

#include <QWidget>

class QPushButton;
class BWLineEdit;

class LoginWidget : public QWidget {
  Q_OBJECT;

public:
  LoginWidget();

signals:
  void loginOk();

private:
  void showLoginScreen();
  void showIdentifyScreen();
  void setLoadingScreen(bool);

  BWLineEdit *m_inputEmail;
  BWLineEdit *m_inputServer;
  QPushButton *m_buttonLogin;
};
