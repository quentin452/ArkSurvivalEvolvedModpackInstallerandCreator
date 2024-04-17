// TODO : ADD DIFFERENT LANGUAGE
// TODO : ADD CONFIG FILE TO SAVE THINGS SUCH HAS COCHE CASE AND GAME DIRECTORIE
// TODO : REMAKE UI
// TODO : ADD informations to know how many space take every mods
// TODO : ADD A WAY TO KNOW WHICH MOD IS THIS ID BY EXAMPLE 2715085686 by making
// a list (gui) of installed mods in your Ark Survival Evolved GamePath
// TODO : add a gui that can be used when backuping txts for steam modid
#include "ui_mainwindow.h"
#include <ArkModIC/ArkModICWindowUtils.h>
#include <ArkModIC/ArkSEModpackGlobals.h>
#include <ArkModIC/mainwindow.h>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QStorageInfo>
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
  QString username = QString::fromStdString(LoggerGlobals::UsernameDirectory);
  gamePathQuery->setText("C:/Users/" + username + "/Desktop/test");
  modsSteamIdListQuery->setText("2783538786,2715085686");
  connect(ui->browseButton, &QPushButton::clicked, this,
          &MainWindow::onBrowseButtonClicked);
  connect(ui->installButton, &QPushButton::clicked, this,
          &MainWindow::onInstallButtonClicked);
  connect(ui->removeModsBackupButton, &QPushButton::clicked, this,
          &MainWindow::onRemoveModsBackupButtonClicked);
  connect(ui->chooseModsFileButton, &QPushButton::clicked, this,
          &MainWindow::onChooseModsFileButtonClicked);
  ui->warningLabel->setText("");
  ui->warningLabel->setStyleSheet("color: orange");
  ui->warningLabel2->setText("");
  ui->warningLabel2->setStyleSheet("color: red");
  connect(gamePathQuery, &QLineEdit::textChanged, this,
          &MainWindow::onGamePathQueryChanged);
  onGamePathQueryChanged(gamePathQuery->text());
  connect(modsSteamIdListQuery, &QLineEdit::textChanged, this,
          &MainWindow::onModsSteamIdListQueryChanged);
  onModsSteamIdListQueryChanged(modsSteamIdListQuery->text());
  updateBackupInfo();
  updateModsInfo();
}
MainWindow::~MainWindow() { delete ui; }

void MainWindow::onBrowseButtonClicked() {
  QString directory = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), QDir::homePath(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!directory.isEmpty()) {
    gamePathQuery->setText(directory);
    onGamePathQueryChanged(gamePathQuery->text());
  }
}

void MainWindow::downloadMods(QString path, QStringList modIDs) {
  disableButtons();
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
    connect(process,
            QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            this, &MainWindow::onProcessErrorOccurred);
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
      enableButtons();
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
      } else {
        if (!QFile::copy(filePath, destFilePath)) {
          ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
              LogLevel::ERRORING, __FILE__, __LINE__,
              "Failed to copy file: " + filePath.toStdString());
        }
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
  updateBackupInfo();
  updateModsInfo();
  enableButtons();
}
void MainWindow::onCopyProcessFinished(int exitCode,
                                       QProcess::ExitStatus exitStatus) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process)
    return;
  process->deleteLater();
}
void MainWindow::onInstallButtonClicked() {
  onModsSteamIdListQueryChanged(modsSteamIdListQuery->text());
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__, "Install Mods...");

  QString path = gamePathQuery->text();
  QString mods = modsSteamIdListQuery->text();
  QStringList modList = mods.split(",");

  if (mods.contains(" ")) {
    return;
  }

  bool deleteMods = ui->deleteModsCheckBox->isChecked();
  bool backupMods = ui->backupModsCheckBox->isChecked();

  if (backupMods) {
    QString modsFolderPath = path + "/Mods/";
    QString backupFolderPath =
        "C:\\Users\\" + username + "\\.ArkModIC\\Mods.old\\";

    QDir modsDir(modsFolderPath);
    QDir backupDir(backupFolderPath);

    if (!backupDir.exists()) {
      if (!backupDir.mkpath(".")) {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__,
            "Failed to create backup folder: " +
                backupFolderPath.toStdString());
        enableButtons();
        return;
      }
    }
    QString timestamp =
        QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString zipFileName = backupFolderPath + "Mods_" + timestamp + ".zip";

    if (!QDir(modsFolderPath).exists()) {
      if (!QDir().mkdir(modsFolderPath)) {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__,
            "Failed to create folder for mods backup: " +
                modsFolderPath.toStdString());
        enableButtons();
        return;
      }
    }
    QProcess::execute("7z a \"" + zipFileName + "\" \"" + modsFolderPath +
                      "\"");
    QFile zipFile(zipFileName);
    if (!zipFile.exists()) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Failed to create mods backup: " + zipFileName.toStdString());
      enableButtons();
      return;
    }
  }
  if (deleteMods) {
    QString modsFolderPath = path + "/Mods/";
    QDir modsDir(modsFolderPath);
    if (modsDir.exists()) {
      if (!modsDir.removeRecursively()) {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__,
            "Failed to delete Mods folder: " + modsFolderPath.toStdString());
        enableButtons();
        return;
      }
    }
  }
  downloadMods(path, modList);
}

void MainWindow::disableButtons() {
  QList<QPushButton *> allButtons = findChildren<QPushButton *>();
  for (QPushButton *button : allButtons) {
    button->setEnabled(false);
  }
}

void MainWindow::enableButtons() {
  QList<QPushButton *> allButtons = findChildren<QPushButton *>();
  for (QPushButton *button : allButtons) {
    button->setEnabled(true);
  }
}
void MainWindow::onRemoveModsBackupButtonClicked() {
  QMessageBox::StandardButton reply;
  reply = QMessageBox::question(
      this, "Supprimer la sauvegarde des mods",
      "Êtes-vous sûr de vouloir supprimer la sauvegarde de tous vos mods ?",
      QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    QString backupFolderPath =
        "C:\\Users\\" + username + "\\.ArkModIC\\Mods.old\\";
    QDir backupDir(backupFolderPath);
    if (backupDir.exists()) {
      if (!backupDir.removeRecursively()) {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__,
            "Failed to delete Mods backup folder: " +
                backupFolderPath.toStdString());
        return;
      }
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__,
          "Mods backup folder deleted successfully: " +
              backupFolderPath.toStdString());
    } else {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Mods backup folder does not exist: " +
              backupFolderPath.toStdString());
    }
  }
}
void MainWindow::onModsSteamIdListQueryChanged(const QString &modIds) {
  if (modIds.contains(" ")) {
    ui->warningLabel2->setText("Error: The mod ID list contains spaces.");
  } else {
    ui->warningLabel2->clear();
  }
}
void MainWindow::onGamePathQueryChanged(const QString &path) {
  QString shooterGamePath =
      path + "/ShooterGame/Binaries/Win64/ShooterGame.exe";
  if (!QFile::exists(shooterGamePath)) {
    ui->warningLabel->setText(
        "Warning: ShooterGame.exe file "
        "not found in the specified directory.(ShooterGame.exe is the "
        "executable of Ark Survival Evolved)");
  } else {
    ui->warningLabel->clear();
  }
}
void MainWindow::updateBackupInfo() {
  QString backupFolderPath =
      "C:\\Users\\" + username + "\\.ArkModIC\\Mods.old\\";
  quint64 backupSize = ArkModICWindowUtils::getFolderSize(backupFolderPath);
  QString backupSizeText = ArkModICWindowUtils::formatSize(backupSize);
  ui->backupSizeLabel->setText("Backup Size from .ArkModIC Folder: " +
                               backupSizeText);
}

void MainWindow::updateModsInfo() {
  QString modsFolderPath = gamePathQuery->text() + "/Mods/";
  quint64 modsSize = ArkModICWindowUtils::getFolderSize(modsFolderPath);
  QString modsSizeText = ArkModICWindowUtils::formatSize(modsSize);
  ui->modsSizeLabel->setText("Mods Size from Ark Surival Evolved: " +
                             modsSizeText);
}

void MainWindow::onProcessErrorOccurred(QProcess::ProcessError error) {
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::ERRORING, __FILE__, __LINE__,
      "Error occurred in SteamCMD process: " + error);
  enableButtons();
}

void MainWindow::onChooseModsFileButtonClicked() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Choose Mods File"), QDir::homePath(), tr("Text Files (*.txt)"));
  if (!filePath.isEmpty()) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
      QTextStream in(&file);
      QString modIds = in.readAll();
      modsSteamIdListQuery->setText(modIds);
      QString saveFolder =
          "C:/Users/" + username + "/.ArkModIC/ModsIdsListSave";
      QDir dir(saveFolder);
      if (!dir.exists()) {
        dir.mkpath(saveFolder);
      }
      QString savePath = saveFolder + "/" + QFileInfo(filePath).fileName();
      QFile::copy(filePath, savePath);
      file.close();
    }
  }
}