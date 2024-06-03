#include <ArkModIC/utils/ArkModICWindowUtils.h>
#include <ArkModIC/utils/QTINCLUDE.h>

void ArkModICWindowUtils::processDirectory(const QFileInfoList &list, quint64 &size, QList<QFuture<quint64>> &futures) {
    foreach (QFileInfo fileInfo, list) {
        if (fileInfo.isDir()) {
            QFuture<quint64> future = QtConcurrent::run(&ArkModICWindowUtils::getFolderSize, fileInfo.absoluteFilePath());
            futures.append(future);
        } else {
            size += fileInfo.size();
        }
    }
}

quint64 ArkModICWindowUtils::getFolderSize(const QString &folderPath) {
    quint64 size = 0;
    QDir dir(folderPath);
    QFileInfoList list = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden);
    QList<QFuture<quint64>> futures;
    QFutureWatcher<quint64> watcher;
    QEventLoop loop;

    // Connect the finished signal to the event loop quit slot
    QObject::connect(&watcher, &QFutureWatcher<void>::finished, &loop, &QEventLoop::quit);

    // Process the directory entries
    processDirectory(list, size, futures);

    // Run the event loop and process each future as it completes
    for (auto &future : futures) {
        watcher.setFuture(future);
        size += future.result();
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