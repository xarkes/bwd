#include <QApplication>

#include "MainWindow.h"
#include "BWNetworkService.h"

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);

  // Set app name for QSettings
  QCoreApplication::setOrganizationName("bwd");
  QCoreApplication::setOrganizationDomain("bwd.xarkes.com");
  QCoreApplication::setApplicationName("Bwd");

  Net();

  MainWindow main = MainWindow();
  main.show();

  return app.exec();
}
