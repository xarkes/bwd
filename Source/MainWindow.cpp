#include <QVBoxLayout>

#include "MainWindow.h"
#include "LoginWidget.h"
#include "VaultUnlockWidget.h"
#include "VaultWidget.h"

MainWindow::MainWindow()
{
  // auto *widget = new LoginWidget();
  // auto *widget = new VaultUnlockWidget();
  auto *widget = new VaultWidget();
  setCentralWidget(widget);
}
