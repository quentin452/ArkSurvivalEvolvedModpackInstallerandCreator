#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

  void readSettingsFromConfigFile(QString &gamePath, QString &modsList);
  void readCheckboxStatesFromConfigFile(bool &deleteMods, bool &backupMods);
  void saveCheckboxStatesToConfigFile(bool deleteMods, bool backupMods);
  void saveSettingsToConfigFile(const QString &gamePath,
                                const QString &modsList);
  void onDeleteModsCheckBoxStateChanged(int state);
  void onBackupModsCheckBoxStateChanged(int state);
  void loadModIDsFromFile(const QString &filePath);
  void updateModsFileComboBox();
  void onModsFileSelected(int index);

private:
  Ui::MainWindow *ui;
  QLineEdit *gamePathQuery;
  QLineEdit *modsSteamIdListQuery;
  std::string LogFilePathForTheThread;
  QString path;
  QString username;
  std::string CONFIG_FILE_PATH;
  std::string GAME_PATH_KEY;
  std::string MODS_LIST_KEY;
  std::string DELETE_MODS_KEY;
  std::string BACKUP_MODS_KEY;
private slots:
  void update();
};

#endif // MAINWINDOW_H
