#include "BWNetworkService.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QPasswordDigestor>
#include <QJsonObject>
#include <QJsonDocument>
#include <qcryptographichash.h>
#include <qjsondocument.h>
#include <qnetworkreply.h>
#include <qpassworddigestor.h>

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

void BWNetworkService::login(QString password)
{
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
        emit Net()->loginDone(false);
        return;
      }
      QByteArray masterKey = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, password.toUtf8(), m_email.toUtf8(), kdfiterations, 32);

      // 2. Second request to unlock
      const QUrl url(m_server + "/identity/connect/token");
      QNetworkRequest request(url);
      request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=utf-8");
      request.setRawHeader("bitwarden-client-name", BW_CL_NAME);
      request.setRawHeader("Accept", "application/json");
  
      const int iterations = 1;
      QByteArray passwordHash = QPasswordDigestor::deriveKeyPbkdf2(QCryptographicHash::Sha256, masterKey, password.toUtf8(), iterations, 32);

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
          qDebug() << "login ok";
          emit Net()->loginDone(true);
        }
        reply->deleteLater();
      });

    } else {
      emit Net()->loginDone(false);
    }
    reply1->deleteLater();
  });
}

