#pragma once

#include <QMainWindow>

class LoginWidget;
class VaultUnlockWidget;
class VaultWidget;

class MainWindow : public QMainWindow
{
  Q_OBJECT;

public:
  MainWindow();

private:
  void showLogin();
  void showUnlock();
  void showVault();

  LoginWidget* m_loginView = nullptr;
  VaultUnlockWidget* m_unlockView = nullptr;
  VaultWidget* m_vaultView = nullptr;
};
