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
  const int DirRecursivityRemovalDepth = 3;
  const int depotOfArkSurvivalEvolvedOnSteam = 346110;
  QString path;
  QLineEdit *gamePathQuery;

private:
  Ui::MainWindow *ui;
  QLineEdit *modsSteamIdListQuery;
  std::string LogFilePathForTheThread;
  ModsInformationWindow *modsInformationWindow;

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
  void resetModsFileComboBox();
  void setModsFileComboBoxText();
  void onModsFileSelected(int index);
  void setupConnections();
  void BackupMods(const QString &path);
private slots:
  void update();
};

#endif // MAINWINDOW_H
