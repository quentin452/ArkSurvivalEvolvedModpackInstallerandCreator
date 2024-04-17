#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void onBrowseButtonClicked();
  void downloadMods(QString path, QStringList modIDs);
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void onCopyProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void onInstallButtonClicked();
  void disableButtons();
  void enableButtons();
  void onRemoveModsBackupButtonClicked();
  void updateBackupInfo();
  void updateModsInfo();
  void onGamePathQueryChanged(const QString &path);
  bool copyRecursively(const QString &srcPath, const QString &dstPath);

private:
  Ui::MainWindow *ui;
  QLineEdit *gamePathQuery;
  QLineEdit *modsSteamIdListQuery;
  std::string LogFilePathForTheThread;
  QString path;
};

#endif // MAINWINDOW_H
