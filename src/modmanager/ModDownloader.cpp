#include "ui_mainwindow.h"
#include <ArkModIC/modmanager/ModDownloader.h>
#include <ArkModIC/utils/ArkModICWindowUtils.h>
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/utils/Configuration.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QRandomGenerator>
#include <QSettings>
#include <QStorageInfo>
#include <QTimer>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <lmcons.h>

ModDownloader::ModDownloader() {}

ModDownloader::~ModDownloader() {}

void ModDownloader::downloadMods(QString path, QStringList modIDs) {
  ArkSEModpackGlobals::MainWindowInstance->disableButtons();
  ArkSEModpackGlobals::MainWindowInstance->path = path;
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
                << QString::number(ArkSEModpackGlobals::MainWindowInstance
                                       ->depotOfArkSurvivalEvolvedOnSteam)
                << modID;
    }
    arguments << "+quit";
    process->setArguments(arguments);
    process->setWorkingDirectory(path);
    connect(process, &QProcess::finished, this,
            &ModDownloader::onProcessFinished);
    connect(process,
            QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &ModDownloader::onProcessErrorOccurred);
    process->start();
  } catch (const std::exception &e) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "An error occurred while executing the command: " +
            std::string(e.what()));
  }
}

void ModDownloader::onProcessFinished(int exitCode,
                                      QProcess::ExitStatus exitStatus) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process)
    return;
  if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
    QString sourcePath =
        ArkSEModpackGlobals::MainWindowInstance->gamePathQuery->text() +
        "/steamapps/workshop/content/" +
        QString::number(ArkSEModpackGlobals::MainWindowInstance
                            ->depotOfArkSurvivalEvolvedOnSteam) +
        "/";
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Source directory does not exist: " + sourcePath.toStdString());
      ArkSEModpackGlobals::MainWindowInstance->enableButtons();
      return;
    }
    QString destPath = ArkSEModpackGlobals::MainWindowInstance->path + "/Mods/";
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
      } else {
        if (!QFile::copy(filePath, destFilePath)) {
          ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
              LogLevel::ERRORING, __FILE__, __LINE__,
              "Failed to copy file: " + filePath.toStdString());
        }
      }
    }
    for (int i = 0;
         i <
         ArkSEModpackGlobals::MainWindowInstance->DirRecursivityRemovalDepth;
         ++i) {
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
  ArkSEModpackGlobals::MainWindowInstance->enableButtons();
}

void ModDownloader::onProcessErrorOccurred(QProcess::ProcessError error) {
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::ERRORING, __FILE__, __LINE__,
      "Error occurred in SteamCMD process: " + error);
  ArkSEModpackGlobals::MainWindowInstance->enableButtons();
}