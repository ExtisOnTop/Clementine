/* This file is part of Clementine.
   Copyright 2012, Arnaud Bienner <arnaud.bienner@gmail.com>
   Copyright 2014, Krzysztof Sobiecki <sobkas@gmail.com>
   Copyright 2014, John Maguire <john.maguire@gmail.com>

   Clementine is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Clementine is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Clementine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "appearance.h"

#include <QApplication>
#include <QSettings>

const char* Appearance::kSettingsGroup = "Appearance";
const char* Appearance::kUseCustomColorSet = "use-custom-set";
const char* Appearance::kForegroundColor = "foreground-color";
const char* Appearance::kBackgroundColor = "background-color";

const QPalette Appearance::kDefaultPalette = QPalette();

Appearance::Appearance(QObject* parent) : QObject(parent) {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  QPalette p = QApplication::palette();
  background_color_ =
      s.value(kBackgroundColor, p.color(QPalette::WindowText)).value<QColor>();
  foreground_color_ =
      s.value(kForegroundColor, p.color(QPalette::Window)).value<QColor>();
}

void Appearance::LoadUserTheme() {
  QSettings s;
  s.beginGroup(kSettingsGroup);
  bool use_a_custom_color_set = s.value(kUseCustomColorSet).toBool();

  if (use_a_custom_color_set) {
    ChangeForegroundColor(foreground_color_);
    ChangeBackgroundColor(background_color_);
  } else {
    ApplyModernDarkTheme();
  }
}

void Appearance::ApplyModernDarkTheme() {
  QPalette dark;

  // Core surface colors
  QColor bgDark(30, 30, 30);        // #1e1e1e
  QColor bgPanel(37, 37, 37);       // #252525
  QColor bgInput(42, 42, 42);       // #2a2a2a
  QColor fgPrimary(224, 224, 224);   // #e0e0e0
  QColor fgSecondary(208, 208, 208); // #d0d0d0
  QColor fgMuted(136, 136, 136);     // #888888
  QColor accent(110, 158, 251);      // #6e9efb  soft blue accent
  QColor border(58, 58, 58);         // #3a3a3a

  // Window
  dark.setColor(QPalette::Window, bgDark);
  dark.setColor(QPalette::WindowText, fgPrimary);

  // Base (list/tree/table backgrounds)
  dark.setColor(QPalette::Base, bgDark);
  dark.setColor(QPalette::AlternateBase, QColor(34, 34, 34));

  // Text
  dark.setColor(QPalette::Text, fgSecondary);
  dark.setColor(QPalette::BrightText, Qt::white);

  // Buttons
  dark.setColor(QPalette::Button, bgPanel);
  dark.setColor(QPalette::ButtonText, fgPrimary);

  // Selection
  dark.setColor(QPalette::Highlight, accent);
  dark.setColor(QPalette::HighlightedText, Qt::white);

  // Tooltips
  dark.setColor(QPalette::ToolTipBase, QColor(45, 45, 45));
  dark.setColor(QPalette::ToolTipText, fgPrimary);

  // Decorative roles
  dark.setColor(QPalette::Light, QColor(60, 60, 60));
  dark.setColor(QPalette::Midlight, QColor(50, 50, 50));
  dark.setColor(QPalette::Mid, border);
  dark.setColor(QPalette::Dark, QColor(22, 22, 22));
  dark.setColor(QPalette::Shadow, QColor(10, 10, 10));

  // Links
  dark.setColor(QPalette::Link, accent);
  dark.setColor(QPalette::LinkVisited, QColor(140, 130, 200));

  // Disabled states
  dark.setColor(QPalette::Disabled, QPalette::WindowText, fgMuted);
  dark.setColor(QPalette::Disabled, QPalette::Text, fgMuted);
  dark.setColor(QPalette::Disabled, QPalette::ButtonText, fgMuted);
  dark.setColor(QPalette::Disabled, QPalette::Highlight, QColor(50, 50, 50));
  dark.setColor(QPalette::Disabled, QPalette::HighlightedText, fgMuted);

  QApplication::setPalette(dark);
}

void Appearance::ResetToSystemDefaultTheme() {
  QApplication::setPalette(kDefaultPalette);
}

void Appearance::ChangeForegroundColor(const QColor& color) {
  // Get the application palette
  QPalette p = QApplication::palette();

  // Modify the palette
  p.setColor(QPalette::WindowText, color);
  p.setColor(QPalette::Text, color);

  // Make the modified palette the new application's palette
  QApplication::setPalette(p);
  foreground_color_ = color;
}

void Appearance::ChangeBackgroundColor(const QColor& color) {
  // Get the application palette
  QPalette p = QApplication::palette();

  // Modify the palette
  p.setColor(QPalette::Window, color);
  p.setColor(QPalette::Base, color);

  // Make the modified palette the new application's palette
  QApplication::setPalette(p);
  background_color_ = color;
}
