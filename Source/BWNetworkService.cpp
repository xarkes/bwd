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
#include <QThreadPool>

static const char* BW_CL_NAME = "desktop_bwd";

BWNetworkService::BWNetworkService()
{
  m_reqmgr = new QNetworkAccessManager(this);
  connect(this, &BWNetworkService::loginPasswordDerived, this, &BWNetworkService::doLogin);
}

static QByteArray hkdfExpandSha256(QByteArray prk, QByteArray info, qsizetype outputByteSize) {
  const double hashLen = 32.0;
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

void BWNetworkService::preLogin(const QString& email, const QString& server)
{
  m_email = email;
  m_server = server;

  // Initiate first request to verify distant server is reachable
  // and get the salt for key derivation
  const QUrl url(m_server + "/identity/accounts/prelogin");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  request.setRawHeader("bitwarden-client-name", BW_CL_NAME);
  request.setRawHeader("Accept", "application/json");
  QJsonObject obj;
  obj["email"] = m_email;
  QJsonDocument doc(obj);
  m_reply = m_reqmgr->post(request, doc.toJson(QJsonDocument::JsonFormat::Compact));
  QObject::connect(m_reply, &QNetworkReply::finished, this, &BWNetworkService::preLoginReceived);
}

void BWNetworkService::preLoginReceived()
{
  if (m_reply->error() != QNetworkReply::NoError) {
    emit notify("Network error: " + m_reply->errorString(), NotificationLevel::Error);
    emit preLoginDone(false);
    return;
  }

  QJsonDocument resp = QJsonDocument::fromJson(m_reply->readAll());
  if (!resp.isObject()) {
    emit notify("Unexpected answer from server.", NotificationLevel::Error);
    emit preLoginDone(false);
    return;
  }
  QJsonObject respo = resp.object();
  if (!respo.contains("Kdf") || !respo.contains("KdfIterations")) {
    emit notify("Unexpected answer from server: no Kdf nor KdfIterations settings found.", NotificationLevel::Error);
    emit preLoginDone(false);
    return;
  }
  auto kdf = respo["Kdf"].toInt();
  if (kdf != 0) {
    // TODO: Support argon2id
    emit notify("Argon2id key derivation is not yet supported.", NotificationLevel::Error);
    emit preLoginDone(false);
    return;
  }

  m_kdfiterations = respo["KdfIterations"].toInt();
  emit preLoginDone(true);
}

void BWNetworkService::login(const QString& password)
{
  QThreadPool::globalInstance()->start([this, password](){
    m_masterKey = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), m_email.toUtf8(), m_kdfiterations, 32);
    const int iterations = 1;
    m_passwordHash = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, m_masterKey, password.toUtf8(), iterations, 32);
    m_localHash = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, m_masterKey, password.toUtf8(), iterations + 1, 32);
    emit loginPasswordDerived();
  });
}

void BWNetworkService::doLogin()
{
  const QUrl url(m_server + "/identity/connect/token");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=utf-8");
  request.setRawHeader("bitwarden-client-name", BW_CL_NAME);
  request.setRawHeader("Accept", "application/json");

  QString deviceIdentifier = "5756df8d-22c9-4da8-bab3-d27f8af38e91"; // XXX
  QString deviceName = "chrome"; // XXX

  QString content = "scope=api%20offline_access&client_id=web&deviceType=9&deviceIdentifier=" + deviceIdentifier + "&deviceName=" + deviceName + "&grant_type=password&username=" + QUrl::toPercentEncoding(m_email) + "&password=" + QUrl::toPercentEncoding(m_passwordHash.toBase64());

  m_reply->deleteLater();
  m_reply = m_reqmgr->post(request, content.toUtf8());
  QObject::connect(m_reply, &QNetworkReply::finished, this, &BWNetworkService::loginReceived);
}

void BWNetworkService::loginReceived()
{
  QByteArray contents = m_reply->readAll();
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

  bool isError = (object == "Error" || m_reply->error() != QNetworkReply::NoError);
  if (isError) {
    if (!message.isEmpty()) {
      emit notify(message, NotificationLevel::Error);
    } else {
      emit notify("Got empty answer from server.", NotificationLevel::Error);
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
        emit notify(QString("Unsupported algorithm: %1").arg(encrypted_key.m_type), NotificationLevel::Error);
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
    QJsonArray folders = response["Folders"].toArray();
    m_database = BWDatabase(ciphers, folders);
    emit synced();
  });
}

void BWNetworkService::editEntry(BWDatabaseEntry* entry)
{
  QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
  const QUrl url(m_server + QString("/api/ciphers/%1").arg(entry->id));
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
  QNetworkReply* reply = mgr->put(request, doc.toJson(QJsonDocument::JsonFormat::Compact));
  QObject::connect(reply, &QNetworkReply::finished, [=](){
    QJsonDocument resp = QJsonDocument::fromJson(reply->readAll());
    qDebug() << "Edit entry: " << reply->error() << " " << resp;
    // TODO: Notify the UI of the result
  });
}

BWDatabase::BWDatabase(QJsonArray& ciphers, QJsonArray& folders)
{
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
  for (QJsonValueRef folder : folders) {
    QJsonObject fObj = folder.toObject();
    BWDatabaseFolder dbFolder{
      fObj["Id"].toString(),
      EncryptedString(fObj["Name"].toString())
    };
    this->folders.push_back(dbFolder);
  }
}
