// TODO : ADD DIFFERENT LANGUAGE
// TODO : REMAKE UI
// TODO : ADD informations to know how many space take every mods
// TODO : ADD A WAY TO KNOW WHICH MOD IS THIS ID BY EXAMPLE 2715085686 by making
// a list (gui) of installed mods in your Ark Survival Evolved GamePath
// TODO : add a way to open path on window explorer from the app (for easy debug
// or just easily find things)
#include "ui_mainwindow.h"
#include <ArkModIC/modmanager/ModDownloader.h>
#include <ArkModIC/utils/ArkModICWindowUtils.h>
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/utils/Configuration.h>
#include <ArkModIC/utils/QTINCLUDE.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <lmcons.h>

MainWindow::MainWindow(QWidget *parent)
    : UpdateHandlerWithQWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  QString gamePath, modsList;
  bool deleteMods, backupMods;
  Configuration::readSettingsFromConfigFile(gamePath);
  Configuration::readCheckboxStatesFromConfigFile(deleteMods, backupMods);

  gamePathQuery = this->findChild<QLineEdit *>("gamePathQuery");
  modsSteamIdListQuery = this->findChild<QLineEdit *>("modsSteamIdListQuery");
  gamePathQuery->setText(gamePath);
  modsSteamIdListQuery->setReadOnly(true);
  ui->deleteModsCheckBox->setChecked(deleteMods);
  ui->backupModsCheckBox->setChecked(backupMods);
  ui->warningLabel->setText("");
  ui->warningLabel->setStyleSheet("color: orange");
  ui->warningLabel2->setText("");
  ui->warningLabel2->setStyleSheet("color: red");

  setupConnections();

  QString lastUsedModsFile;
  Configuration::readLastUsedModsFileFromConfig(lastUsedModsFile);
  if (!lastUsedModsFile.isEmpty()) {
    ui->modsFileComboBox->setCurrentText(lastUsedModsFile);
  }
  setModsFileComboBoxText();
}

void MainWindow::setupConnections() {
  connect(ui->goToModsInformationButton, &QPushButton::clicked, this, [=]() {
    WindowUtils::SetCurrentWindow(
        this, ArkSEModpackGlobals::ModInformationWindowInstance);
  });
  connect(ui->browseButton, &QPushButton::clicked, this,
          &MainWindow::onBrowseButtonClicked);
  connect(ui->installButton, &QPushButton::clicked, this,
          &MainWindow::onInstallButtonClicked);
  connect(ui->removeModsBackupButton, &QPushButton::clicked, this,
          &MainWindow::onRemoveModsBackupButtonClicked);
  connect(ui->chooseModsFileButton, &QPushButton::clicked, this,
          &MainWindow::onChooseModsFileButtonClicked);
  connect(ui->deleteModsCheckBox, &QCheckBox::stateChanged, this,
          &MainWindow::onDeleteModsCheckBoxStateChanged);
  connect(ui->backupModsCheckBox, &QCheckBox::stateChanged, this,
          &MainWindow::onBackupModsCheckBoxStateChanged);
  connect(ui->modsFileComboBox,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &MainWindow::onModsFileSelected);
  connect(modsSteamIdListQuery, &QLineEdit::textChanged, this,
          &MainWindow::updateModsInfo);
  connect(gamePathQuery, &QLineEdit::textChanged, this,
          &MainWindow::onGamePathQueryChanged);
  connect(modsSteamIdListQuery, &QLineEdit::textChanged, this,
          &MainWindow::onModsSteamIdListQueryChanged);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::updateCode() {
  if (this->isActiveWindow()) {
    resetModsFileComboBox();
    updateBackupInfo();
    updateModsInfo();
  }
}
void MainWindow::onDeleteModsCheckBoxStateChanged(int state) {
  bool deleteMods = (state == Qt::Checked);
  bool backupMods = ui->backupModsCheckBox->isChecked();
  Configuration::saveCheckboxStatesToConfigFile(deleteMods, backupMods);
}

void MainWindow::onBackupModsCheckBoxStateChanged(int state) {
  bool deleteMods = ui->deleteModsCheckBox->isChecked();
  bool backupMods = (state == Qt::Checked);
  Configuration::saveCheckboxStatesToConfigFile(deleteMods, backupMods);
}

void MainWindow::onBrowseButtonClicked() {
  QString directory = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), QDir::homePath(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!directory.isEmpty()) {
    gamePathQuery->setText(directory);
    onGamePathQueryChanged(gamePathQuery->text());
  }
  Configuration::saveSettingsToConfigFile(gamePathQuery->text());
}

void MainWindow::onInstallButtonClicked() {
  try {
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
      if (!BackupMods(path)) {
        throw std::runtime_error("Failed to backup mods");
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
          throw std::runtime_error("Failed to delete Mods folder");
        }
      }
    }
    ArkSEModpackGlobals::ModDownloaderInstance->downloadMods(path, modList);
    Configuration::saveCheckboxStatesToConfigFile(deleteMods, backupMods);
    Configuration::saveSettingsToConfigFile(gamePathQuery->text());
  } catch (const std::exception &e) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "An error occurred while installing mods: " + std::string(e.what()));
    enableButtons();
    QMessageBox::critical(this, "Error",
                          "An error occurred while installing mods: " +
                              QString(e.what()));
  }
}

bool MainWindow::BackupMods(const QString &path) {
  QString modsFolderPath = path + "/Mods/";
  QString backupFolderPath =
      "C:\\Users\\" + QString::fromStdString(LoggerGlobals::UsernameDirectory) +
      "\\.ArkModIC\\Mods.old\\";

  QDir modsDir(modsFolderPath);
  QDir backupDir(backupFolderPath);
  QDirIterator it(modsFolderPath, QDir::Files, QDirIterator::Subdirectories);
  if (!modsDir.exists() || !it.hasNext()) {
    return true;
  }

  if (!backupDir.exists()) {
    if (!backupDir.mkpath(".")) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Failed to create backup folder: " + backupFolderPath.toStdString());
      return false;
    }
  }
  QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
  QString zipFileName = backupFolderPath + "Mods_" + timestamp + ".zip";

  if (!QDir(modsFolderPath).exists()) {
    if (!QDir().mkdir(modsFolderPath)) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Failed to create folder for mods backup: " +
              modsFolderPath.toStdString());
      return false;
    }
  }
  QProcess::execute("7z a \"" + zipFileName + "\" \"" + modsFolderPath + "\"");
  QFile zipFile(zipFileName);
  if (!zipFile.exists()) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "Failed to create mods backup: " + zipFileName.toStdString());
    return false;
  }
  return true;
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
      this, "Remove Mods Backups",
      "Are you sure you want to delete the backup of all your mods ?",
      QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    QString backupFolderPath =
        "C:\\Users\\" +
        QString::fromStdString(LoggerGlobals::UsernameDirectory) +
        "\\.ArkModIC\\Mods.old\\";
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
      "C:\\Users\\" + QString::fromStdString(LoggerGlobals::UsernameDirectory) +
      "\\.ArkModIC\\Mods.old\\";
  quint64 backupSize = ArkModICWindowUtils::getFolderSize(backupFolderPath);
  QString backupSizeText = ArkModICWindowUtils::formatSize(backupSize);
  ui->backupSizeLabel->setText("Backup Size from .ArkModIC Folder: " +
                               backupSizeText);
}
void MainWindow::updateModsInfo() {
    QFuture<void> future = QtConcurrent::run([this] {
        QElapsedTimer timer;
        timer.start();
        QString modsFolderPath = gamePathQuery->text() + "/Mods/";
        quint64 modsSize = ArkModICWindowUtils::getFolderSize(modsFolderPath);
        QString modsSizeText = ArkModICWindowUtils::formatSize(modsSize);
        QString modsList = modsSteamIdListQuery->text().trimmed();
        QStringList modIds = modsList.split(",");
        modIds.removeAll("");
        int numberOfMods = modIds.count();
        QDir modsDir(modsFolderPath);
        QStringList modFolders = modsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        int numberOfModFolders = modFolders.size();

        // Use Qt::QueuedConnection to ensure the UI update is performed in the main thread
        QMetaObject::invokeMethod(this, [=] {
            ui->modsSizeLabel->setText("Mods Size from Ark Survival Evolved: " + modsSizeText);
            ui->numberOfModsLabel->setText("Number of Mods in the txt list: " + QString::number(numberOfMods));
            ui->numberOfModFilesLabel->setText("Number of Mods in the Mods folder: " + QString::number(numberOfModFolders));
        }, Qt::QueuedConnection);
    });
}

void MainWindow::onChooseModsFileButtonClicked() {
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Choose Mods File"), QDir::homePath(), tr("Text Files (*.txt)"));
  if (!filePath.isEmpty()) {
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
      QTextStream in(&file);
      QString modIds = in.readAll();
      file.close();
      modsSteamIdListQuery->setText(modIds);
      Configuration::saveLastUsedModsFileToConfig(filePath);
      ui->modsFileComboBox->setCurrentText(QFileInfo(filePath).fileName());
      QString saveFolder =
          "C:/Users/" +
          QString::fromStdString(LoggerGlobals::UsernameDirectory) +
          "/.ArkModIC/ModsIdsListSave";
      QDir().mkpath(saveFolder);
      QString saveFilePath = saveFolder + "/" + QFileInfo(filePath).fileName();
      if (!QFile::copy(filePath, saveFilePath)) {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::ERRORING, __FILE__, __LINE__,
            "Failed to copy file to save folder: " + filePath.toStdString());
      }
    }
  }
}

void MainWindow::resetModsFileComboBox() {
  QString saveFolder =
      "C:/Users/" + QString::fromStdString(LoggerGlobals::UsernameDirectory) +
      "/.ArkModIC/ModsIdsListSave";
  QDir dir(saveFolder);
  QStringList filters;
  filters << "*.txt";
  QStringList modFiles = dir.entryList(filters, QDir::Files, QDir::Name);
  disconnect(ui->modsFileComboBox, &QComboBox::currentIndexChanged, this,
             &MainWindow::onModsFileSelected);
  ui->modsFileComboBox->clear();
  ui->modsFileComboBox->addItem("");
  if (!modFiles.isEmpty()) {
    ui->modsFileComboBox->addItems(modFiles);
  }
  connect(ui->modsFileComboBox, &QComboBox::currentIndexChanged, this,
          &MainWindow::onModsFileSelected);
  setModsFileComboBoxText();
  QString lastUsedModsFile;
  Configuration::readLastUsedModsFileFromConfig(lastUsedModsFile);
  ui->modsFileComboBox->setCurrentText(lastUsedModsFile);
  ui->lastUsedModsFileLabel->setText("Last Used Mod File: " + lastUsedModsFile);
}

void MainWindow::setModsFileComboBoxText() {
  QString lastUsedModsFile;
  Configuration::readLastUsedModsFileFromConfig(lastUsedModsFile);
  ui->modsFileComboBox->setCurrentText(lastUsedModsFile);
  QFile file(lastUsedModsFile);
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    QString content = in.readAll();
    ui->modsSteamIdListQuery->setText(content);
    file.close();
  }
}

void MainWindow::onModsFileSelected(int index) {
  QString saveFolder =
      "C:/Users/" + QString::fromStdString(LoggerGlobals::UsernameDirectory) +
      "/.ArkModIC/ModsIdsListSave";
  QString filePath = saveFolder + "/" + ui->modsFileComboBox->currentText();
  ArkSEModpackGlobals::WindowUtilsInstance->loadTextFromFileToString(
      filePath, modsSteamIdListQuery);
  onModsSteamIdListQueryChanged(modsSteamIdListQuery->text());
  Configuration::saveLastUsedModsFileToConfig(filePath);
}
