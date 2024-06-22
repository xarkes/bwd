#include <QVBoxLayout>
#include <QStackedWidget>
#include <QStackedLayout>

#include "MainWindow.h"
#include "Views/LoginWidget.h"
#include "Views/VaultUnlockWidget.h"
#include "Views/VaultWidget.h"

MainWindow::MainWindow()
{
  auto stack = new QStackedWidget();
  setCentralWidget(stack);
  showLogin();
}

void MainWindow::showLogin()
{
  if (!m_loginView) {
    m_loginView = new LoginWidget();
    connect(m_loginView, &LoginWidget::loginOk, [this](){
      showUnlock();
    });
    centralWidget()->layout()->addWidget(m_loginView);
  }
  static_cast<QStackedLayout*>(centralWidget()->layout())->setCurrentWidget(m_loginView);
}

void MainWindow::showUnlock()
{
  if (!m_unlockView) {
    m_unlockView = new VaultUnlockWidget();
    connect(m_unlockView, &VaultUnlockWidget::back, [this](){
      showLogin();
    });
    connect(m_unlockView, &VaultUnlockWidget::unlocked, [this](){
      showVault();
    });
    centralWidget()->layout()->addWidget(m_unlockView);
  }
  static_cast<QStackedLayout*>(centralWidget()->layout())->setCurrentWidget(m_unlockView);
}

void MainWindow::showVault()
{
  if (!m_vaultView) {
    m_vaultView = new VaultWidget();
    centralWidget()->layout()->addWidget(m_vaultView);
  }
  static_cast<QStackedLayout*>(centralWidget()->layout())->setCurrentWidget(m_vaultView);
}
