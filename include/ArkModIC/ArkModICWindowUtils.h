#ifndef ArkModICWindowUtils_H
#define ArkModICWindowUtils_H

#include <QString>

class ArkModICWindowUtils {
public:
  static quint64 getFolderSize(const QString &folderPath);
  static QString formatSize(quint64 size);
};

#endif // ArkModICWindowUtils_H
