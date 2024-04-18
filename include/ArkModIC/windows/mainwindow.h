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
  void onChooseModsFileButtonClicked();
  void onModsSteamIdListQueryChanged(const QString &modIds);
  void onProcessErrorOccurred(QProcess::ProcessError error);
  void onGamePathQueryChanged(const QString &path);
  void onDeleteModsCheckBoxStateChanged(int state);
  void onBackupModsCheckBoxStateChanged(int state);
  void loadModIDsFromFile(const QString &filePath);
  void resetModsFileComboBox();
  void setModsFileComboBoxText();
  void onModsFileSelected(int index);

private:
  Ui::MainWindow *ui;
  QLineEdit *gamePathQuery;
  QLineEdit *modsSteamIdListQuery;
  std::string LogFilePathForTheThread;
  QString path;
  QString username;
  ModsInformationWindow *modsInformationWindow;
private slots:
  void update();
  void onGoToModsInformationClicked();
};

#endif // MAINWINDOW_H
