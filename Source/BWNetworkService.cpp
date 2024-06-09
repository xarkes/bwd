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

          // qDebug() << "login ok";
          // qDebug() << response;

          if (response["token_type"].toString() != "Bearer") {
            emit Net()->loginDone(false);
          } else {
            m_accessToken = response["access_token"].toString().toUtf8();

            // Decrypt user key
            auto encrypted_key = EncryptedString(response["Key"].toString().toUtf8());
            if (encrypted_key.encType != EncryptionType::AesCbc256_HmacSha256_B64) {
              qCritical() << "Unsupported algorithm!";
              return;
            }

            // Apparently, master key needs to be stretched for Aesbc256_HmacSha256_B64
            QByteArray stretchedKey;
            stretchedKey.append(hkdfExpandSha256(m_masterKey, "enc", 32));
            stretchedKey.append(hkdfExpandSha256(m_masterKey, "mac", 32));

            m_key = encrypted_key.decryptToBytes(stretchedKey, stretchedKey.sliced(32));

            // XXX: ???
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

    // qDebug() << "Received reply";
    // qDebug() << response;

    QJsonArray ciphers = response["Ciphers"].toArray();
    m_database = BWDatabase(ciphers);
    emit synced();
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
    this->encType = (EncryptionType)splits[0].toInt(&ok);
    if (!ok) {
      return;
    }
    pieces = splits[1].split("|");
  } else {
    pieces = str.split("|");
    this->encType = pieces.length() == 3 ? EncryptionType::AesCbc128_HmacSha256_B64 : EncryptionType::AesCbc256_B64;
  }

  switch (this->encType) {
  case EncryptionType::AesCbc256_B64:
    this->m_iv = QByteArray::fromBase64(pieces[0].toLatin1());
    this->m_data = QByteArray::fromBase64(pieces[1].toLatin1());
    break;
  case EncryptionType::AesCbc128_HmacSha256_B64:
  case EncryptionType::AesCbc256_HmacSha256_B64:
    this->m_iv = QByteArray::fromBase64(pieces[0].toLatin1());
    this->m_data = QByteArray::fromBase64(pieces[1].toLatin1());
    this->m_mac = QByteArray::fromBase64(pieces[2].toLatin1());
    break;
  default:
    qCritical() << "Unsupported cipher! " << this->encType;
    return;
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
  decryptToBytes(Net()->m_key);

  // Cut off PKCS7 padding for strings
  // XXX: Maybe wrong for non aes-cbc encryption
  auto len = m_decrypted[m_decrypted.length() - 1];
  if (len <= 16) {
    m_decrypted.truncate(m_decrypted.length() - len);
  }

  return m_decrypted;
}

// TODO: Need to understand why the original client uses such structure
// and the lifespan of decrypted information
QByteArray EncryptedString::decryptToBytes(QByteArray key, QByteArray macKey)
{
  if (m_decrypted.length()) {
    return m_decrypted;
  }
  if (!m_data.length()) {
    // Nothing to decrypt
    return "";
  }

  switch (encType) {
  case EncryptionType::AesCbc256_HmacSha256_B64:
  {
    // XXX: Is this valid in every scenario for Bitwarden?
    QByteArray macData = m_iv + m_data;
    QByteArray computedHash = QMessageAuthenticationCode::hash(macData, macKey, QCryptographicHash::Sha256);
    // XXX: Implement proper cryptographic verification
    if (computedHash != m_mac) {
      qDebug() << "hmac verif failed" << computedHash << " " << m_mac;
    }

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, (uint8_t*) key.data(), (uint8_t*) m_iv.data());
    AES_CBC_decrypt_buffer(&ctx, (uint8_t*) m_data.data(), m_data.length());
    m_decrypted = m_data;

    this->m_data = this->m_iv = this->m_mac = "";
    break;
  }
  default:
    qCritical() << "Unsupported cipher! " << encType;
  }
  return m_decrypted;
}

BWDatabase::BWDatabase(QJsonArray& ciphers)
{
  qDebug() << ciphers;
  for (QJsonValueRef cipher : ciphers) {
    QJsonObject obj = cipher.toObject();
    QJsonObject data = obj["Data"].toObject();
    BWDatabaseEntry entry{EncryptedString(data["Name"].toString()), EncryptedString(data["Username"].toString()), EncryptedString(data["Password"].toString()), EncryptedString(data["Notes"].toString()), EncryptedString(data["Uri"].toString())};
    entries.push_back(entry);
  }
}
