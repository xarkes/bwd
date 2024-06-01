#pragma once

#include <QObject>
#include <QWidget>

class BWNetworkService;
static BWNetworkService* __net_instance = nullptr;

#define Net() BWNetworkService::instance()

class BWNetworkService : public QWidget {
  Q_OBJECT;

public:
  static BWNetworkService* instance() {
    if (!__net_instance) {
      __net_instance = new BWNetworkService();
    }
    return __net_instance;
  }

  void preLogin(QString email, QString server);
  void login(QString password);

signals:
  void preLoginDone(bool success);
  void loginDone();

protected:
  void run();

private:
  BWNetworkService();

  QString m_email;
  QString m_server;
};

