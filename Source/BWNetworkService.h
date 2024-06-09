#pragma once

#include <QObject>
#include <QWidget>

class BWNetworkService;
static BWNetworkService* __net_instance = nullptr;

#define Net() BWNetworkService::instance()


enum EncryptionType {
  AesCbc256_B64 = 0,
  AesCbc128_HmacSha256_B64 = 1,
  AesCbc256_HmacSha256_B64 = 2,
  Rsa2048_OaepSha256_B64 = 3,
  Rsa2048_OaepSha1_B64 = 4,
  Rsa2048_OaepSha256_HmacSha256_B64 = 5,
  Rsa2048_OaepSha1_HmacSha256_B64 = 6,
};

class EncryptedString {
public:
  EncryptedString(QString str);
  QString decrypt();
  QByteArray decryptToBytes(QByteArray key, QByteArray mac="");

public:
  EncryptionType encType;

private:
  QByteArray m_iv;
  QByteArray m_data;
  QByteArray m_mac;
  QByteArray m_decrypted;
};

struct BWDatabaseEntry {
  EncryptedString name;
  EncryptedString username;
  EncryptedString password;
  EncryptedString notes;
  EncryptedString uri;
};

struct BWDatabase {
  BWDatabase() {};
  BWDatabase(QJsonArray& ciphers);
  QList<BWDatabaseEntry> entries;
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

  void preLogin(QString email, QString server);
  void login(QString password);
  void sync();

  BWDatabase& db() { return m_database; };

  QByteArray m_key; // TODO
  QByteArray m_privateKey;

signals:
  void preLoginDone(bool success);
  void loginDone(bool success);
  void synced();

protected:
  void run();

private:
  BWNetworkService();

  QString m_email;
  QString m_server;
  QByteArray m_masterKey;
  QByteArray m_accessToken;
  BWDatabase m_database;
};

