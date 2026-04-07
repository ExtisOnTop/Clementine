#include "licensedialog.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QSettings>

#include "core/keyauth.h"

// KeyAuth application credentials
static const char* kKeyAuthAppName = "Extis691's Application";
static const char* kKeyAuthOwnerId = "35j3goqJPZ";
static const char* kKeyAuthSecret =
    "34143e0b5366eb9c4d80230f99d5ad39b978bf8454edb354520da8673e4aa730";
static const char* kKeyAuthVersion = "1.0";
static const char* kSettingsGroup = "License";
static const char* kSettingsKey = "saved_key";

LicenseDialog::LicenseDialog(QWidget* parent)
    : QDialog(parent),
      api_(new KeyAuthApi(kKeyAuthAppName, kKeyAuthOwnerId, kKeyAuthSecret,
                          kKeyAuthVersion, this)) {
  setWindowTitle("Clementine - License Activation");
  setFixedSize(420, 260);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  // Styling
  setStyleSheet(
      "QDialog {"
      "  background-color: #1e1e1e;"
      "}"
      "QLabel {"
      "  color: #e0e0e0;"
      "  background: transparent;"
      "}"
      "QLineEdit {"
      "  background-color: #2a2a2a;"
      "  color: #e0e0e0;"
      "  border: 1px solid #3a3a3a;"
      "  border-radius: 8px;"
      "  padding: 10px 14px;"
      "  font-size: 13px;"
      "}"
      "QLineEdit:focus {"
      "  border: 1px solid #6e9efb;"
      "}"
      "QPushButton {"
      "  background-color: #6e9efb;"
      "  color: #ffffff;"
      "  border: none;"
      "  border-radius: 8px;"
      "  padding: 10px 24px;"
      "  font-size: 13px;"
      "  font-weight: 600;"
      "}"
      "QPushButton:hover {"
      "  background-color: #85adfb;"
      "}"
      "QPushButton:pressed {"
      "  background-color: #5a8ae6;"
      "}"
      "QPushButton:disabled {"
      "  background-color: #333333;"
      "  color: #666666;"
      "}");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(32, 28, 32, 28);
  layout->setSpacing(16);

  // Title
  title_label_ = new QLabel("Enter License Key");
  QFont title_font = title_label_->font();
  title_font.setPointSize(16);
  title_font.setWeight(QFont::DemiBold);
  title_label_->setFont(title_font);
  title_label_->setStyleSheet("color: #ffffff;");
  title_label_->setAlignment(Qt::AlignCenter);
  layout->addWidget(title_label_);

  // Subtitle
  QLabel* subtitle = new QLabel("Enter your license key to activate Clementine");
  subtitle->setStyleSheet("color: #888888; font-size: 12px;");
  subtitle->setAlignment(Qt::AlignCenter);
  layout->addWidget(subtitle);

  layout->addSpacing(4);

  // Key input
  key_input_ = new QLineEdit();
  key_input_->setPlaceholderText("XXXXX-XXXXX-XXXXX-XXXXX");
  key_input_->setAlignment(Qt::AlignCenter);
  layout->addWidget(key_input_);

  // Activate button
  activate_button_ = new QPushButton("Activate");
  activate_button_->setCursor(Qt::PointingHandCursor);
  layout->addWidget(activate_button_);

  // Status label
  status_label_ = new QLabel();
  status_label_->setAlignment(Qt::AlignCenter);
  status_label_->setWordWrap(true);
  status_label_->setStyleSheet("font-size: 11px;");
  layout->addWidget(status_label_);

  layout->addStretch();

  // Connections
  connect(activate_button_, &QPushButton::clicked, this,
          &LicenseDialog::OnActivateClicked);
  connect(key_input_, &QLineEdit::returnPressed, this,
          &LicenseDialog::OnActivateClicked);
  connect(api_, &KeyAuthApi::InitCompleted, this,
          &LicenseDialog::OnInitCompleted);
  connect(api_, &KeyAuthApi::LicenseCompleted, this,
          &LicenseDialog::OnLicenseCompleted);

  // Load saved key
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QString saved_key = s.value(kSettingsKey).toString();
  if (!saved_key.isEmpty()) {
    key_input_->setText(saved_key);
  }
}

void LicenseDialog::OnActivateClicked() {
  QString key = key_input_->text().trimmed();
  if (key.isEmpty()) {
    SetStatus("Please enter a license key.", true);
    return;
  }

  activate_button_->setEnabled(false);
  activate_button_->setText("Connecting...");
  SetStatus("Initializing...");

  api_->Init();
}

void LicenseDialog::OnInitCompleted(bool success, const QString& message) {
  if (!success) {
    SetStatus("Connection failed: " + message, true);
    activate_button_->setEnabled(true);
    activate_button_->setText("Activate");
    return;
  }

  activate_button_->setText("Validating...");
  SetStatus("Checking license...");
  api_->License(key_input_->text().trimmed());
}

void LicenseDialog::OnLicenseCompleted(bool success, const QString& message) {
  if (success) {
    // Save the key for next launch
    QSettings s;
    s.beginGroup(kSettingsGroup);
    s.setValue(kSettingsKey, key_input_->text().trimmed());

    SetStatus("License activated successfully!");
    accept();
  } else {
    SetStatus("Invalid license: " + message, true);
    activate_button_->setEnabled(true);
    activate_button_->setText("Activate");
  }
}

void LicenseDialog::SetStatus(const QString& text, bool is_error) {
  status_label_->setText(text);
  status_label_->setStyleSheet(
      is_error ? "color: #ff6b6b; font-size: 11px;"
               : "color: #888888; font-size: 11px;");
}

bool LicenseDialog::Validate(QWidget* parent) {
  LicenseDialog dialog(parent);
  return dialog.exec() == QDialog::Accepted;
}
