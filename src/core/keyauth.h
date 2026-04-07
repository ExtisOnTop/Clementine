#ifndef CORE_KEYAUTH_H_
#define CORE_KEYAUTH_H_

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSysInfo>

class KeyAuthApi : public QObject {
  Q_OBJECT

 public:
  KeyAuthApi(const QString& name, const QString& ownerid,
             const QString& secret, const QString& version,
             QObject* parent = nullptr);

  // Initialize the application session with KeyAuth
  void Init();

  // Validate a license key
  void License(const QString& key);

  bool is_initialized() const { return initialized_; }
  bool is_licensed() const { return licensed_; }
  QString error_message() const { return error_message_; }
  QString username() const { return username_; }
  QString expiry() const { return expiry_; }

 signals:
  void InitCompleted(bool success, const QString& message);
  void LicenseCompleted(bool success, const QString& message);

 private:
  void PostRequest(const QUrlQuery& params,
                   std::function<void(const QJsonObject&)> callback);
  QString GetHwid() const;

  QNetworkAccessManager* network_;
  QString name_;
  QString ownerid_;
  QString secret_;
  QString version_;
  QString session_id_;
  QString error_message_;
  QString username_;
  QString expiry_;
  bool initialized_;
  bool licensed_;
};

#endif  // CORE_KEYAUTH_H_
