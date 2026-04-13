#ifndef CORE_KEYAUTH_H_
#define CORE_KEYAUTH_H_

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QJsonObject>

class KeyAuthApi : public QObject {
  Q_OBJECT

 public:
  KeyAuthApi(const QString& name, const QString& ownerid,
             const QString& secret, const QString& version,
             QObject* parent = nullptr);

  // Initialize the application session with KeyAuth (blocking)
  bool Init(QString* error_out = nullptr);

  // Validate a license key (blocking)
  bool License(const QString& key, QString* error_out = nullptr);

  QString username() const { return username_; }
  QString expiry() const { return expiry_; }

 private:
  QJsonObject PostRequest(const QUrlQuery& params);
  QString GetHwid() const;

  QNetworkAccessManager* network_;
  QString name_;
  QString ownerid_;
  QString secret_;
  QString version_;
  QString session_id_;
  QString username_;
  QString expiry_;
};

#endif  // CORE_KEYAUTH_H_
