// SPDX-License-Identifier: AGPL-3.0-or-later
// Fork: Preferences › Graphics › Graphics card: Windows' own choice of
// graphics card for this program (Settings › System › Display › Graphics,
// HKCU\Software\Microsoft\DirectX\UserGpuPreferences by the program's path),
// which Windows reads when the program starts. 1 High performance, 2 Power
// saving; 0 (Automatic) leaves the choice to Windows again.
#include <windows.h>

#include <iterator>

#include <QDir>
#include <QSettings>
#include <QString>

namespace mitcad {

void applyWindowsGpuPreference(int card) {
  wchar_t path[32768] = {};
  const DWORD length = GetModuleFileNameW(nullptr, path, DWORD(std::size(path)));
  if (length == 0) {
    return;
  }
  const QString program = QDir::toNativeSeparators(QString::fromWCharArray(path, int(length)));
  QSettings gpu(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\DirectX\\UserGpuPreferences"),
                QSettings::NativeFormat);
  if (card == 1 || card == 2) {
    gpu.setValue(program, QStringLiteral("GpuPreference=%1;").arg(card == 1 ? 2 : 1));
  } else if (gpu.contains(program)) {
    gpu.remove(program);
  }
}

} // namespace mitcad
