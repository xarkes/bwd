#pragma once

#include <QObject>
#include <QWidget>

#include "EncryptedString.h"

class QNetworkAccessManager;
class QNetworkReply;

class BWNetworkService;
static BWNetworkService* __net_instance = nullptr;

#define Net() BWNetworkService::instance()

struct BWDatabaseEntry {
  QString id;
  int type;
  EncryptedString name;
  EncryptedString username;
  EncryptedString password;
  EncryptedString notes;
  EncryptedString uri;
  QString folderId;
};

struct BWDatabaseFolder {
  QString id;
  EncryptedString name;
};

struct BWDatabase {
  BWDatabase() {};
  BWDatabase(QJsonArray& ciphers, QJsonArray& folders);
  QList<BWDatabaseEntry> entries;
  QList<BWDatabaseFolder> folders;
};


class BWNetworkService : public QWidget {
  Q_OBJECT;

public:
  static BWNetworkService* instance() {
    if (!__net_instance) {
      __net_instance = new BWNetworkService();
    }
    return __net_instance;
  }

  void preLogin(const QString& email, const QString& server);
  void login(const QString& password);
  void sync();
  void editEntry(BWDatabaseEntry* entry);

  BWDatabase& db() { return m_database; };

  QByteArray m_key; // TODO
  QByteArray m_privateKey;

  enum NotificationLevel {
    Info = 0,
    Success = 1,
    Error = 2
  };

signals:
  void preLoginDone(bool success);
  void loginPasswordDerived();
  void loginDone(bool success);
  void synced();
  void notify(QString text, NotificationLevel level);

protected:
  void run();

private slots:
  void preLoginReceived();
  void doLogin();
  void loginReceived();

private:
  BWNetworkService();

  QString m_email;
  QString m_server;
  QByteArray m_masterKey;
  QByteArray m_accessToken;
  BWDatabase m_database;
  int m_kdfiterations;

  QByteArray m_passwordHash;
  QByteArray m_localHash;

  QNetworkAccessManager* m_reqmgr;
  QNetworkReply* m_reply;
};

