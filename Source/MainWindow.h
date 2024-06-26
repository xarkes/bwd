#pragma once

#include <QMainWindow>

class LoginWidget;
class VaultUnlockWidget;
class VaultWidget;
class QStackedWidget;
class QLabel;

class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:
  MainWindow();

private slots:
  void sendNotification(QString data, int level);

private:
  void showLogin();
  void showUnlock();
  void showVault();

  void moveNotification();
  void hideNotification();
  void resizeEvent(QResizeEvent *e) override;
  void moveEvent(QMoveEvent *e) override;

  QStackedWidget* m_stack = nullptr;
  QLabel* m_notification = nullptr;

  LoginWidget* m_loginView = nullptr;
  VaultUnlockWidget* m_unlockView = nullptr;
  VaultWidget* m_vaultView = nullptr;
};
