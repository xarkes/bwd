#include <QVBoxLayout>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QLabel>

#include "MainWindow.h"
#include "BWNetworkService.h"
#include "Views/LoginWidget.h"
#include "Views/VaultUnlockWidget.h"
#include "Views/VaultWidget.h"

MainWindow::MainWindow()
{
  m_stack = new QStackedWidget();
  setCentralWidget(m_stack);

  m_notification = new QLabel("Notification", this, Qt::ToolTip);
  m_notification->setContentsMargins(10, 10, 10, 10);

  showLogin();

  connect(Net(), &BWNetworkService::notify, this, &MainWindow::sendNotification);
}

void MainWindow::showLogin()
{
  if (!m_loginView) {
    m_loginView = new LoginWidget();
    connect(m_loginView, &LoginWidget::loginOk, [this](){
      showUnlock();
    });
    m_stack->layout()->addWidget(m_loginView);
  }
  static_cast<QStackedLayout*>(m_stack->layout())->setCurrentWidget(m_loginView);
  hideNotification();
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
    m_stack->layout()->addWidget(m_unlockView);
  }
  static_cast<QStackedLayout*>(m_stack->layout())->setCurrentWidget(m_unlockView);
  hideNotification();
}

void MainWindow::showVault()
{
  if (!m_vaultView) {
    m_vaultView = new VaultWidget();
    centralWidget()->layout()->addWidget(m_vaultView);
  }
  static_cast<QStackedLayout*>(m_stack->layout())->setCurrentWidget(m_vaultView);
  hideNotification();
}

void MainWindow::moveNotification()
{
  const int margin = 10;
  QPoint point = mapToGlobal(QPoint(size().width(), 0) -
                             QPoint(m_notification->size().width() + margin, - margin));
  m_notification->move(point);
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
  moveNotification();
  QMainWindow::resizeEvent(e);
}

void MainWindow::moveEvent(QMoveEvent *e)
{
  moveNotification();
  QMainWindow::moveEvent(e);
}

void MainWindow::sendNotification(QString data, int level)
{
  m_notification->setText(data);
  switch (level) {
    case BWNetworkService::NotificationLevel::Info:
      m_notification->setStyleSheet("background-color: #4040e0;");
      break;
    case BWNetworkService::NotificationLevel::Success:
      m_notification->setStyleSheet("background-color: #20e020;");
      break;
    case BWNetworkService::NotificationLevel::Error:
      m_notification->setStyleSheet("background-color: #e02020;");
      break;
  }
  m_notification->show();
  moveNotification();
}

void MainWindow::hideNotification()
{
  m_notification->setText("");
  m_notification->hide();
}
