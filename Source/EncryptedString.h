#pragma once

#include <QString>
#include <QtConcurrent/QtConcurrent>

enum EncryptionType {
  AesCbc256_B64 = 0,
  AesCbc128_HmacSha256_B64 = 1,
  AesCbc256_HmacSha256_B64 = 2,
  Rsa2048_OaepSha256_B64 = 3,
  Rsa2048_OaepSha1_B64 = 4,
  Rsa2048_OaepSha256_HmacSha256_B64 = 5,
  Rsa2048_OaepSha1_HmacSha256_B64 = 6,
  Unknown = 7
};

class EncryptedString {
public:
  EncryptedString(QString str);

  QString decrypt();
  QByteArray decryptToBytes(QByteArray key, QByteArray mac="");
  QFuture<QString> decryptAsync();

  void setClear(QString str);
  QString toString();

public:
  EncryptionType m_type = Unknown;

private:
  QByteArray m_iv;
  QByteArray m_data;
  QByteArray m_mac;
  QByteArray m_decrypted;
};
