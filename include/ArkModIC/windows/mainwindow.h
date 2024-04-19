#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <ArkModIC/windows/modsinformationwindow.h>
#include <QCloseEvent>
#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
#include <string>

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
  bool isDownloading = false;

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
  bool BackupMods(const QString &path);
private slots:
  void update();

protected:
  void closeEvent(QCloseEvent *event) override {
    if (isDownloading) {
      event->ignore();
    } else {
      event->accept();
    }
  }
};

#endif // MAINWINDOW_H
