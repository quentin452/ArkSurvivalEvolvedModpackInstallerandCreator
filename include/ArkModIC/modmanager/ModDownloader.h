#ifndef MODDOWNLOADER_H
#define MODDOWNLOADER_H

#include <QObject>
#include <QProcess>
#include <string>

class ModDownloader : public QObject {
  Q_OBJECT

public:
  explicit ModDownloader();
  ~ModDownloader();
  void downloadMods(QString path, QStringList modIDs);

public slots:
  void onProcessErrorOccurred(QProcess::ProcessError error);
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
};

#endif // MODDOWNLOADER_H