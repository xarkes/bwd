#include "BWNetworkService.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <qjsondocument.h>

BWNetworkService::BWNetworkService()
{
}

void BWNetworkService::preLogin(QString email, QString server)
{
  m_email = email;
  m_server = server;
  emit preLoginDone(true);
}

void BWNetworkService::login(QString password)
{
  QNetworkAccessManager *mgr = new QNetworkAccessManager(this);
  const QUrl url(m_server + "/identity/connect/token");
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=utf-8");
  request.setRawHeader("bitwarden-client-name", "desktop_bwd");
  request.setRawHeader("Accept", "application/json");
  
  QString deviceIdentifier = "5756df8d-22c9-4da8-bab3-d27f8af38e91"; // XXX
  QString deviceName = "chrome"; // XXX

  QString content = "scope=api%20offline_access&client_id=web&deviceType=9&deviceIdentifier=" + deviceIdentifier + "&deviceName=" + deviceName + "&grant_type=password&username=" + QUrl::toPercentEncoding(m_email) + "&password=" + QUrl::toPercentEncoding(password);
  
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
    } else {
      qDebug() << "login ok";
    }

    reply->deleteLater();
    emit Net()->loginDone();
  });
}

