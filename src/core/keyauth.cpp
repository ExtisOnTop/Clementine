#include "keyauth.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QUuid>

KeyAuthApi::KeyAuthApi(const QString& name, const QString& ownerid,
                       const QString& secret, const QString& version,
                       QObject* parent)
    : QObject(parent),
      network_(new QNetworkAccessManager(this)),
      name_(name),
      ownerid_(ownerid),
      secret_(secret),
      version_(version),
      initialized_(false),
      licensed_(false) {}

QString KeyAuthApi::GetHwid() const {
  return QSysInfo::machineUniqueId().isEmpty()
             ? QSysInfo::machineHostName()
             : QString::fromUtf8(QSysInfo::machineUniqueId());
}

void KeyAuthApi::PostRequest(
    const QUrlQuery& params,
    std::function<void(const QJsonObject&)> callback) {
  QNetworkRequest request;
  request.setUrl(QUrl("https://keyauth.win/api/1.3/"));
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");

  QNetworkReply* reply = network_->post(request, params.toString().toUtf8());

  QEventLoop loop;
  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  if (reply->error() != QNetworkReply::NoError) {
    QJsonObject err;
    err["success"] = false;
    err["message"] = reply->errorString();
    callback(err);
  } else {
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    callback(doc.object());
  }

  reply->deleteLater();
}

void KeyAuthApi::Init() {
  QUrlQuery params;
  params.addQueryItem("type", "init");
  params.addQueryItem("name", name_);
  params.addQueryItem("ownerid", ownerid_);
  params.addQueryItem("ver", version_);

  PostRequest(params, [this](const QJsonObject& response) {
    bool success = response["success"].toBool();
    QString message = response["message"].toString();

    if (success) {
      session_id_ = response["sessionid"].toString();
      initialized_ = true;
    }

    error_message_ = success ? "" : message;
    emit InitCompleted(success, message);
  });
}

void KeyAuthApi::License(const QString& key) {
  if (!initialized_) {
    error_message_ = "Not initialized";
    emit LicenseCompleted(false, error_message_);
    return;
  }

  QUrlQuery params;
  params.addQueryItem("type", "license");
  params.addQueryItem("key", key);
  params.addQueryItem("hwid", GetHwid());
  params.addQueryItem("sessionid", session_id_);
  params.addQueryItem("name", name_);
  params.addQueryItem("ownerid", ownerid_);

  PostRequest(params, [this](const QJsonObject& response) {
    bool success = response["success"].toBool();
    QString message = response["message"].toString();

    if (success) {
      licensed_ = true;
      QJsonObject info = response["info"].toObject();
      username_ = info["username"].toString();

      QJsonArray subs = info["subscriptions"].toArray();
      if (!subs.isEmpty()) {
        expiry_ = subs[0].toObject()["expiry"].toString();
      }
    }

    error_message_ = success ? "" : message;
    emit LicenseCompleted(success, message);
  });
}
