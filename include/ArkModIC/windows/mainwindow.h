#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
#include <string>

#include <ArkModIC/windows/modsinformationwindow.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  void disableButtons();
  void enableButtons();

private:
  Ui::MainWindow *ui;
  QLineEdit *modsSteamIdListQuery;
  std::string LogFilePathForTheThread;
  ModsInformationWindow *modsInformationWindow;
  QString path;
  QLineEdit *gamePathQuery;

  void onProcessErrorOccurred(QProcess::ProcessError error);
  void onBrowseButtonClicked();
  void onInstallButtonClicked();
  void onRemoveModsBackupButtonClicked();
  void updateBackupInfo();
  void updateModsInfo();
  void onChooseModsFileButtonClicked();
  void onModsSteamIdListQueryChanged(const QString &modIds);
  void onGamePathQueryChanged(const QString &path);
  void onDeleteModsCheckBoxStateChanged(int state);
  void onBackupModsCheckBoxStateChanged(int state);
  void loadModIDsFromFile(const QString &filePath);
  void resetModsFileComboBox();
  void setModsFileComboBoxText();
  void onModsFileSelected(int index);
  void setupConnections();
  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void downloadMods(QString path, QStringList modIDs);
  void BackupMods(const QString &path);
private slots:
  void update();
  void onGoToModsInformationClicked();
};

#endif // MAINWINDOW_H
