#ifndef ArkModICWindowUtils_H
#define ArkModICWindowUtils_H
#include <ArkModIC/utils/QTINCLUDE.h>

class ArkModICWindowUtils : public QObject {
    Q_OBJECT

public:
    static quint64 getFolderSize(const QString &folderPath);
    static QString formatSize(quint64 size);

private:
    static void processDirectory(const QFileInfoList &list, quint64 &size, QList<QFuture<quint64>> &futures);
};

#endif // ArkModICWindowUtils_H
