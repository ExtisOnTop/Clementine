#ifndef UI_LICENSEDIALOG_H_
#define UI_LICENSEDIALOG_H_

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class KeyAuthApi;

class LicenseDialog : public QDialog {
  Q_OBJECT

 public:
  explicit LicenseDialog(QWidget* parent = nullptr);

  // Returns true if license was validated successfully
  static bool Validate(QWidget* parent = nullptr);

 private slots:
  void OnActivateClicked();
  void OnInitCompleted(bool success, const QString& message);
  void OnLicenseCompleted(bool success, const QString& message);

 private:
  void SetStatus(const QString& text, bool is_error = false);

  KeyAuthApi* api_;
  QLineEdit* key_input_;
  QPushButton* activate_button_;
  QLabel* status_label_;
  QLabel* title_label_;
};

#endif  // UI_LICENSEDIALOG_H_
