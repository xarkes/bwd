#include <QApplication>

#include "MainWindow.h"
#include "BWNetworkService.h"

int main(int argc, char* argv[])
{
  QApplication app(argc, argv);
  Net();

  MainWindow main = MainWindow();
  main.show();

  return app.exec();
}
