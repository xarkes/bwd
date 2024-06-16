#include "BWNetworkService.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QPasswordDigestor>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QNetworkReply>
#include <QPasswordDigestor>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <qjsonvalue.h>

#include "ThirdParty/aes.h"

static const char* BW_CL_NAME = "desktop_bwd";

BWNetworkService::BWNetworkService()
{
}

void BWNetworkService::preLogin(QString email, QString server)
{
  // TODO: Do input validation eg. we want to make sure that server is a valid URL etc.
  m_email = email;
  m_server = server;
  emit preLoginDone(true);
}

static QByteArray hkdfExpandSha256(QByteArray prk, QByteArray info, size_t outputByteSize) {
  const size_t hashLen = 32;
  Q_ASSERT(outputByteSize <= 255 * hashLen);
  Q_ASSERT(prk.length() >= hashLen);
  const int n = ceil(outputByteSize / hashLen);
  QByteArray previousT;
  QByteArray okm;
  for (int i = 0; i < n; i++) {
    QByteArray t;
    t.append(previousT);
    t.append(info);
    t.append(i + 1);
    previousT = QMessageAuthenticationCode::hash(t, prk, QCryptographicHash::Sha256);
    okm.append(previousT);
    if (okm.length() >= outputByteSize) {
      break;
    }
  }
  return okm.sliced(0, outputByteSize);
}

void BWNetworkService::login(QString password)
{
  qDebug() << "login";
  QNetworkAccessManager *mgr = new QNetworkAccessManager(this);

  // 1. First request to get the salt
  const QUrl url1(m_server + "/identity/accounts/prelogin");
  QNetworkRequest request1(url1);
  request1.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request1.setRawHeader("bitwarden-client-name", BW_CL_NAME);
  request1.setRawHeader("Accept", "application/json");
  QJsonObject obj;
  obj["email"] = m_email;
  QJsonDocument doc(obj);
  QNetworkReply* reply1 = mgr->post(request1, doc.toJson());
  QObject::connect(reply1, &QNetworkReply::finished, [=](){
    if (reply1->error() == QNetworkReply::NoError) {
      // 1b. Parse result and use it to derivate key
      QJsonDocument resp = QJsonDocument::fromJson(reply1->readAll());
      if (!resp.isObject()) {
        emit Net()->loginDone(false);
        return;
      }
      QJsonObject respo = resp.object();
      if (!respo.contains("Kdf") || !respo.contains("KdfIterations")) {
        emit Net()->loginDone(false);
        return;
      }
      auto kdf = respo["Kdf"].toInt();
      auto kdfiterations = respo["KdfIterations"].toInt();
      if (kdf != 0) {
        // TODO: Support argon2id
        qCritical() << "argon2id not supported!";
        emit Net()->loginDone(false);
        return;
      }
      m_masterKey = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), m_email.toUtf8(), kdfiterations, 32);

      // 2. Second request to unlock
      const QUrl url(m_server + "/identity/connect/token");
      QNetworkRequest request(url);
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=utf-8");
      request.setRawHeader("bitwarden-client-name", BW_CL_NAME);
      request.setRawHeader("Accept", "application/json");
  
      const int iterations = 1;
      QByteArray passwordHash = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, m_masterKey, password.toUtf8(), iterations, 32);
      QByteArray localHash = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, m_masterKey, password.toUtf8(), iterations + 1, 32);

      QString deviceIdentifier = "5756df8d-22c9-4da8-bab3-d27f8af38e91"; // XXX
      QString deviceName = "chrome"; // XXX

      QString content = "scope=api%20offline_access&client_id=web&deviceType=9&deviceIdentifier=" + deviceIdentifier + "&deviceName=" + deviceName + "&grant_type=password&username=" + QUrl::toPercentEncoding(m_email) + "&password=" + QUrl::toPercentEncoding(passwordHash.toBase64());

      QNetworkReply *reply = mgr->post(request, content.toUtf8());
      QObject::connect(reply, &QNetworkReply::finished, [=](){
        QByteArray contents = reply->readAll();
        QJsonParseError error;
        QJsonDocument response = QJsonDocument::fromJson(contents, &error);

        QString message;
        QString object;

        if (response.isObject()) {
          auto obj = response.object();
          if (obj.contains("ErrorModel")) {
            auto model = obj.value("ErrorModel");
            if (model.isObject()) {
              obj = model.toObject();
              message = obj.value("Message").toString();
              object = obj.value("Object").toString();
            }
          }
        }

        bool isError = (object == "Error" || reply->error() != QNetworkReply::NoError);
        if (isError) {
          if (!message.isEmpty()) {
            qDebug() << object << ": " << message;
          } else {
            qDebug() << contents;
          }
          emit Net()->loginDone(false);
        } else {
          if (response["token_type"].toString() != "Bearer") {
            emit Net()->loginDone(false);
          } else {
            m_accessToken = response["access_token"].toString().toUtf8();

            // Decrypt user key
            auto encrypted_key = EncryptedString(response["Key"].toString().toUtf8());
            if (encrypted_key.m_type != EncryptionType::AesCbc256_HmacSha256_B64) {
              qCritical() << "Unsupported algorithm!";
              return;
            }

            QByteArray stretchedKey = hkdfExpandSha256(m_masterKey, "enc", 32);
            QByteArray macKey = hkdfExpandSha256(m_masterKey, "mac", 32);
            m_key = encrypted_key.decryptToBytes(stretchedKey, macKey);

            // TODO: Maybe required for other encryption schemes
            // encrypted_key = EncryptedString(response["PrivateKey"].toString().toUtf8());
            // m_privateKey = encrypted_key.decryptToBytes(stretchedKey, stretchedKey.sliced(32));

            // Done, go forward
            emit Net()->loginDone(true);
            sync();
          }
        }
        reply->deleteLater();
      });

    } else {
      emit Net()->loginDone(false);
    }
    reply1->deleteLater();
  });
}

void BWNetworkService::sync()
{
  QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
  const QUrl url(m_server + "/api/sync?excludeDomains=true");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("bitwarden-client-name", BW_CL_NAME);
  request.setRawHeader("Accept", "application/json");
  request.setRawHeader("Authorization", "Bearer " + m_accessToken);

  QNetworkReply *reply = mgr->get(request);
  QObject::connect(reply, &QNetworkReply::finished, [=](){
    QByteArray contents = reply->readAll();
    QJsonParseError error;
    QJsonDocument response = QJsonDocument::fromJson(contents, &error);

    QJsonArray ciphers = response["Ciphers"].toArray();
    m_database = BWDatabase(ciphers);
    emit synced();
  });
}

void BWNetworkService::editEntry(BWDatabaseEntry* entry)
{
  QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
  const QUrl url(m_server + QString("/api/ciphers/%1").arg(entry->id));
  qDebug() << url;
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("bitwarden-client-name", BW_CL_NAME);
  request.setRawHeader("Accept", "application/json");
  request.setRawHeader("Authorization", "Bearer " + m_accessToken);
  QJsonObject obj;
  obj["type"] = entry->type;
  obj["folderId"] = entry->folderId.length() ? QJsonValue(entry->folderId) : QJsonValue::Null;
  obj["notes"] = entry->notes.toString();
  obj["name"] = entry->name.toString();

  if (entry->type == 1) {
    QJsonObject login;
    login["username"] = entry->username.toString();
    login["password"] = entry->password.toString();
    obj["login"] = login;
  } else {
    qCritical() << "Unsupported entry type! " << entry->type;
  }
  QJsonDocument doc(obj);
  QNetworkReply* reply = mgr->put(request, doc.toJson());
  QObject::connect(reply, &QNetworkReply::finished, [=](){
    QJsonDocument resp = QJsonDocument::fromJson(reply->readAll());
    qDebug() << "Edit entry: " << reply->error() << " " << resp;
  });
}

EncryptedString::EncryptedString(QString str)
{
  if (!str.length()) {
    return;
  }

  QList<QString> splits = str.split(".");
  QList<QString> pieces;

  if (splits.length() == 2) {
    bool ok;
    m_type = (EncryptionType)splits[0].toInt(&ok);
    if (!ok) {
      qCritical() << "Invalid encryption cipher received: " << str;
      return;
    }
    pieces = splits[1].split("|");
  } else {
    pieces = str.split("|");
    m_type = pieces.length() == 3 ? EncryptionType::AesCbc128_HmacSha256_B64 : EncryptionType::AesCbc256_B64;
  }

  switch (this->m_type) {
  case EncryptionType::AesCbc256_B64:
    m_iv = QByteArray::fromBase64(pieces[0].toLatin1());
    m_data = QByteArray::fromBase64(pieces[1].toLatin1());
    break;
  case EncryptionType::AesCbc128_HmacSha256_B64:
  case EncryptionType::AesCbc256_HmacSha256_B64:
    m_iv = QByteArray::fromBase64(pieces[0].toLatin1());
    m_data = QByteArray::fromBase64(pieces[1].toLatin1());
    m_mac = QByteArray::fromBase64(pieces[2].toLatin1());
    break;
  default:
    qCritical() << "Unsupported cipher for EncryptedString initialization: " << m_type;
    return;
  }
}

void EncryptedString::setClear(QString str)
{
  m_decrypted = str.toUtf8();

  if (m_type == EncryptionType::Unknown) {
    // This is the first time this string is being encrypted.
    // Arbitrarily choose the encryption type, and initialize an IV
    m_type = EncryptionType::AesCbc256_HmacSha256_B64;
    auto gen = QRandomGenerator();
    m_iv.resize(16);
    gen.generate(m_iv.begin(), m_iv.end());
  }

  switch (m_type) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
  {
    auto key = Net()->m_key.sliced(0, 32);
    auto macKey = Net()->m_key.sliced(32, 32);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (uint8_t*) key.data(), (uint8_t*) m_iv.data());
    m_data = m_decrypted;
    // PKCS7 padding
    int remain = 16 - m_data.length() % 16;
    for (int i = 0; i < remain; i++) {
      m_data.append(remain);
    }
    Q_ASSERT(m_data.length() % 16 == 0);
    AES_CBC_encrypt_buffer(&ctx, (uint8_t*) m_data.data(), m_data.length());

    QByteArray macData = m_iv + m_data;
    m_mac = QMessageAuthenticationCode::hash(macData, macKey, QCryptographicHash::Sha256);
    break;
  }
  default:
    qCritical() << "Unsupported cipher for encryption: " << m_type;
    return;
  }
}

QString EncryptedString::toString()
{
  if (!m_data.length()) {
    // String is empty, return empty string
    return "";
  }

  switch (m_type) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
    return QString("%1.%2|%3|%4").arg(m_type).arg(m_iv.toBase64()).arg(m_data.toBase64()).arg(m_iv.toBase64());
  default:
    qCritical() << "Unsupported cipher for formatting: " << m_type;
    return "";
  }
}

QString EncryptedString::decrypt()
{
  if (m_decrypted.length()) {
    return m_decrypted;
  }
  if (!m_data.length()) {
    // Nothing to decrypt
    return "";
  }

  // XXX: Probably only for AesCbc256_HmacSha256
  auto key = Net()->m_key.sliced(0, 32);
  auto macKey = Net()->m_key.sliced(32, 32);
  decryptToBytes(key, macKey);

  // Cut off PKCS7 padding for strings
  // XXX: Maybe wrong for non aes-cbc encryption
  auto len = m_decrypted[m_decrypted.length() - 1];
  if (len <= 16) {
    m_decrypted.truncate(m_decrypted.length() - len);
  }

  return m_decrypted;
}


static bool CompareHash(QByteArray a, QByteArray b)
{
  if (a.length() != b.length()) {
    return false;
  }
  uint8_t r = 0;
  for (size_t i = 0; i < a.length(); i++) {
    r += a[i] ^ b[i];
  }
  return r == 0;
}

// TODO: Need to understand why the original client uses such structure
// and the lifespan of decrypted information
QByteArray EncryptedString::decryptToBytes(QByteArray key, QByteArray mac)
{
  if (m_decrypted.length()) {
    return m_decrypted;
  }
  if (!m_data.length()) {
    // Nothing to decrypt
    return "";
  }

  switch (m_type) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
  {
    if (m_mac.length()) {
      // XXX: Is it expected that some entries have no mac? If so why use a mac at all?
      QByteArray macData = m_iv + m_data;
      QByteArray computedHash = QMessageAuthenticationCode::hash(macData, mac, QCryptographicHash::Sha256);
      if (!CompareHash(computedHash, m_mac)) {
        // XXX: Should we invalidate the data?
        qDebug() << "hmac verif failed";
        qDebug() << "   " << computedHash.toHex();
        qDebug() << "   " << m_mac.toHex();
        qDebug() << "   " << mac.toHex();
      }
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (uint8_t*) key.data(), (uint8_t*) m_iv.data());
    AES_CBC_decrypt_buffer(&ctx, (uint8_t*) m_data.data(), m_data.length());
    m_decrypted = m_data;
    this->m_data = this->m_iv = this->m_mac = "";
    break;
  }
  default:
    qCritical() << "Unsupported cipher! " << m_type;
  }
  return m_decrypted;
}

BWDatabase::BWDatabase(QJsonArray& ciphers)
{
  qDebug() << ciphers;
  for (QJsonValueRef cipher : ciphers) {
    QJsonObject obj = cipher.toObject();
    QJsonObject data = obj["Data"].toObject();
    BWDatabaseEntry entry{
      obj["Id"].toString(),
      obj["Type"].toInt(),
      EncryptedString(data["Name"].toString()),
      EncryptedString(data["Username"].toString()),
      EncryptedString(data["Password"].toString()),
      EncryptedString(data["Notes"].toString()),
      EncryptedString(data["Uri"].toString()),
      obj["FolderId"].toString()
    };
    entries.push_back(entry);
  }
}
