#include <ArkModIC/utils/ArkModICWindowUtils.h>
#include <ArkModIC/utils/QTINCLUDE.h>

quint64 ArkModICWindowUtils::getFolderSize(const QString &folderPath) {
  quint64 size = 0;
  QDir dir(folderPath);
  QFileInfoList list =
      dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
  foreach (QFileInfo fileInfo, list) {
    if (fileInfo.isDir()) {
      size += getFolderSize(fileInfo.absoluteFilePath());
    } else {
      size += fileInfo.size();
    }
  }
  return size;
}

QString ArkModICWindowUtils::formatSize(quint64 size) {
  QStringList units = {"B", "KB", "MB", "GB"};
  int unitIndex = 0;
  while (size >= 1024 && unitIndex < units.size() - 1) {
    size /= 1024;
    ++unitIndex;
  }
  return QString::number(size) + " " + units[unitIndex];
}