#pragma once

#include <QMainWindow>

#include "LoginWidget.h"
#include "VaultUnlockWidget.h"
#include "VaultWidget.h"

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
