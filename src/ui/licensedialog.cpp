#include "licensedialog.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QFont>
#include <QSettings>
#include <QTimer>

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
  setFixedSize(420, 280);
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

  setStyleSheet(
      "QDialog { background-color: #1e1e1e; }"
      "QLabel { color: #e0e0e0; background: transparent; }"
      "QLineEdit {"
      "  background-color: #2a2a2a; color: #e0e0e0;"
      "  border: 1px solid #3a3a3a; border-radius: 8px;"
      "  padding: 10px 14px; font-size: 13px;"
      "}"
      "QLineEdit:focus { border: 1px solid #6e9efb; }"
      "QPushButton {"
      "  background-color: #6e9efb; color: #ffffff;"
      "  border: none; border-radius: 8px;"
      "  padding: 10px 24px; font-size: 13px; font-weight: 600;"
      "}"
      "QPushButton:hover { background-color: #85adfb; }"
      "QPushButton:pressed { background-color: #5a8ae6; }"
      "QPushButton:disabled { background-color: #333333; color: #666666; }");

  QVBoxLayout* layout = new QVBoxLayout(this);
  layout->setContentsMargins(32, 28, 32, 28);
  layout->setSpacing(16);

  // Title
  QLabel* title = new QLabel("Enter License Key");
  QFont title_font = title->font();
  title_font.setPointSize(16);
  title_font.setWeight(QFont::DemiBold);
  title->setFont(title_font);
  title->setStyleSheet("color: #ffffff;");
  title->setAlignment(Qt::AlignCenter);
  layout->addWidget(title);

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
  status_label_->setStyleSheet("font-size: 11px; color: #888888;");
  layout->addWidget(status_label_);

  layout->addStretch();

  connect(activate_button_, &QPushButton::clicked, this,
          &LicenseDialog::OnActivateClicked);
  connect(key_input_, &QLineEdit::returnPressed, this,
          &LicenseDialog::OnActivateClicked);

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
  QApplication::processEvents();

  // Step 1: Init
  QString error;
  if (!api_->Init(&error)) {
    SetStatus("Connection failed: " + error, true);
    activate_button_->setEnabled(true);
    activate_button_->setText("Activate");
    return;
  }

  // Step 2: Validate license
  activate_button_->setText("Validating...");
  SetStatus("Checking license...");
  QApplication::processEvents();

  if (!api_->License(key, &error)) {
    SetStatus("Invalid license: " + error, true);
    activate_button_->setEnabled(true);
    activate_button_->setText("Activate");
    return;
  }

  // Success - save key and close
  QSettings s;
  s.beginGroup(kSettingsGroup);
  s.setValue(kSettingsKey, key);

  SetStatus("License activated successfully!");
  QApplication::processEvents();

  // Small delay so user sees the success message
  QTimer::singleShot(500, this, &QDialog::accept);
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
