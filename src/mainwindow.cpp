// TODO : ADD AN OPTION TO BACKUP FOLDER
// TODO : PREVENT THE USER TO CLICK ON INSTALL MOD BUTTON AGAIN IF STEAMCMD IS
// RUNNING
// TODO : ADD DIFFERENT LANGUAGE
// TODO : ADD CONFIG FILE TO SAVE THINGS SUCH HAS COCHE CASE AND GAME DIRECTORIE
// TODO : ADD steamid mod LIST BACKUP
// TODO : ADD AN COCHE CASE TO BACKUP MODS BEFORE REMOVING Mods CONTENTS

#include "ui_mainwindow.h"
#include <ArkModIC/ArkSEModpackGlobals.h>
#include <ArkModIC/mainwindow.h>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QProcess>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <filesystem>
#include <iostream>
#include <lmcons.h>
#include <windows.h>
int DirRecursivityRemovalDepth = 3;
int depotOfArkSurvivalEvolvedOnSteam = 346110;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  gamePathQuery = this->findChild<QLineEdit *>("gamePathQuery");
  modsSteamIdListQuery = this->findChild<QLineEdit *>("modsSteamIdListQuery");
  gamePathQuery->setText("C:/Users/iamacatfr/Desktop/test");
  modsSteamIdListQuery->setText("2783538786,2715085686");
  connect(ui->browseButton, &QPushButton::clicked, this,
          &MainWindow::onBrowseButtonClicked);
  connect(ui->installButton, &QPushButton::clicked, this,
          &MainWindow::onInstallButtonClicked);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::onBrowseButtonClicked() {
  QString directory = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), QDir::homePath(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!directory.isEmpty()) {
    gamePathQuery->setText(directory);
  }
}
QString getCurrentUsername() {
  WCHAR username[UNLEN + 1];
  DWORD size = UNLEN + 1;
  if (!GetUserNameW(username, &size)) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "Failed to get current username");
    return QString();
  }
  QString usernameStr = QString::fromWCharArray(username);
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__,
      "Current Username: " + usernameStr.toStdString());
  return usernameStr;
}
void MainWindow::downloadMods(QString path, QStringList modIDs) {
  this->path = path;
  try {
    QString steamcmdPath = "steamcmd.exe";
    QProcess *process = new QProcess(this);
    process->setProgram("cmd.exe");
    QStringList arguments;
    arguments << "/c"
              << "start"
              << "/wait"
              << "steamcmd.exe"
              << "+force_install_dir" << path << "+login"
              << "anonymous";
    foreach (QString modID, modIDs) {
      arguments << "+workshop_download_item"
                << QString::number(depotOfArkSurvivalEvolvedOnSteam) << modID;
    }
    arguments << "+quit";
    process->setArguments(arguments);
    process->setWorkingDirectory(path);
    connect(process, &QProcess::finished, this, &MainWindow::onProcessFinished);
    process->start();
  } catch (const std::exception &e) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "An error occurred while executing the command: " +
            std::string(e.what()));
  }
}

void MainWindow::onProcessFinished(int exitCode,
                                   QProcess::ExitStatus exitStatus) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process)
    return;
  if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
    QString sourcePath =
        gamePathQuery->text() + "/steamapps/workshop/content/" +
        QString::number(depotOfArkSurvivalEvolvedOnSteam) + "/";
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Source directory does not exist: " + sourcePath.toStdString());
      return;
    }
    QString destPath = this->path + "/Mods/";
    if (!QDir(destPath).exists()) {
      QDir().mkpath(destPath);
    }
    QDirIterator it(sourcePath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString filePath = it.next();
      QString relativePath = sourceDir.relativeFilePath(filePath);
      QString destFilePath = destPath + QDir::separator() + relativePath;

      if (QFileInfo(filePath).isDir()) {
        QDir(destFilePath).mkpath(".");
      }
    }
    for (int i = 0; i < DirRecursivityRemovalDepth; ++i) {
      QDir parentDir = sourceDir;
      parentDir.cdUp();
      sourceDir = parentDir;
    }
    if (sourceDir.removeRecursively()) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__,
          "Source directory deleted successfully: " + sourcePath.toStdString());
    } else {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Failed to delete source directory: " + sourcePath.toStdString());
    }
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Files copied from directory " + sourcePath.toStdString() + " to " +
            destPath.toStdString());
  } else {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__, "Failed to install mods");
  }

  process->deleteLater();
}
void MainWindow::onCopyProcessFinished(int exitCode,
                                       QProcess::ExitStatus exitStatus) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process)
    return;
  process->deleteLater();
}
void MainWindow::onInstallButtonClicked() {
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__, "Install Mods...");
  QString path = gamePathQuery->text();
  QString mods = modsSteamIdListQuery->text();
  QStringList modList = mods.split(",");
  bool deleteMods = ui->deleteModsCheckBox->isChecked();
  if (deleteMods) {
    QString modsFolderPath = path + "/Mods/";
    QDir modsDir(modsFolderPath);
    if (modsDir.exists()) {
      if (!modsDir.removeRecursively()) {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__,
            "Failed to delete Mods folder: " + modsFolderPath.toStdString());
        return;
      }
    }
  }
  downloadMods(path, modList);
}