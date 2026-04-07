#include "keyauth.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSysInfo>
#include <QUrlQuery>

KeyAuthApi::KeyAuthApi(const QString& name, const QString& ownerid,
                       const QString& secret, const QString& version,
                       QObject* parent)
    : QObject(parent),
      network_(new QNetworkAccessManager(this)),
      name_(name),
      ownerid_(ownerid),
      secret_(secret),
      version_(version) {}

QString KeyAuthApi::GetHwid() const {
  return QSysInfo::machineUniqueId().isEmpty()
             ? QSysInfo::machineHostName()
             : QString::fromUtf8(QSysInfo::machineUniqueId());
}

QJsonObject KeyAuthApi::PostRequest(const QUrlQuery& params) {
  QNetworkRequest request;
  request.setUrl(QUrl("https://keyauth.win/api/1.3/"));
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "application/x-www-form-urlencoded");

  QNetworkReply* reply = network_->post(request, params.toString().toUtf8());

  QEventLoop loop;
  connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QJsonObject result;
  if (reply->error() != QNetworkReply::NoError) {
    result["success"] = false;
    result["message"] = reply->errorString();
  } else {
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isObject()) {
      result = doc.object();
    } else {
      result["success"] = false;
      result["message"] = QString("Invalid response from server");
    }
  }

  reply->deleteLater();
  return result;
}

bool KeyAuthApi::Init(QString* error_out) {
  QUrlQuery params;
  params.addQueryItem("type", "init");
  params.addQueryItem("name", name_);
  params.addQueryItem("ownerid", ownerid_);
  params.addQueryItem("ver", version_);

  QJsonObject response = PostRequest(params);
  bool success = response["success"].toBool();

  if (success) {
    session_id_ = response["sessionid"].toString();
  } else if (error_out) {
    *error_out = response["message"].toString();
  }

  return success;
}

bool KeyAuthApi::License(const QString& key, QString* error_out) {
  if (session_id_.isEmpty()) {
    if (error_out) *error_out = "Not initialized";
    return false;
  }

  QUrlQuery params;
  params.addQueryItem("type", "license");
  params.addQueryItem("key", key);
  params.addQueryItem("hwid", GetHwid());
  params.addQueryItem("sessionid", session_id_);
  params.addQueryItem("name", name_);
  params.addQueryItem("ownerid", ownerid_);

  QJsonObject response = PostRequest(params);
  bool success = response["success"].toBool();

  if (success) {
    QJsonObject info = response["info"].toObject();
    username_ = info["username"].toString();

    QJsonArray subs = info["subscriptions"].toArray();
    if (!subs.isEmpty()) {
      expiry_ = subs[0].toObject()["expiry"].toString();
    }
  } else if (error_out) {
    *error_out = response["message"].toString();
  }

  return success;
}
