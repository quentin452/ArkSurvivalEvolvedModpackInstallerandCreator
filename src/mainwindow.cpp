#include "ui_mainwindow.h"
#include <ArkModIC/ArkSEModpackGlobals.h>
#include <ArkModIC/mainwindow.h>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QDirIterator>
#include <QProcess>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <filesystem>
#include <iostream>
#include <lmcons.h>
#include <windows.h>
namespace fs = std::filesystem;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  std::cout << "Construct MainWindow" << std::endl;
  ui->setupUi(this);
  lineEdit = this->findChild<QLineEdit *>("lineEdit");
  modsLineEdit = this->findChild<QLineEdit *>("modsLineEdit");

  // Définir les valeurs par défaut
  lineEdit->setText("C:/Users/iamacatfr/Desktop/test");
  modsLineEdit->setText("2783538786");

  connect(ui->browseButton, &QPushButton::clicked, this,
          &MainWindow::onBrowseButtonClicked);
  connect(ui->installButton, &QPushButton::clicked, this,
          &MainWindow::onInstallButtonClicked);
}

MainWindow::~MainWindow() { delete ui; }

bool MainWindow::onPushButtonClicked() { return false; }

void MainWindow::onBrowseButtonClicked() {
  QString directory = QFileDialog::getExistingDirectory(
      this, tr("Select Directory"), QDir::homePath(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (!directory.isEmpty()) {
    lineEdit->setText(directory);
  }
}
QString getCurrentUsername() {
  WCHAR username[UNLEN + 1];
  DWORD size = UNLEN + 1;
  if (!GetUserNameW(username, &size)) {
    // Log d'erreur si la récupération du nom d'utilisateur échoue
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "Failed to get current username");
    return QString(); // Retourne une chaîne vide en cas d'erreur
  }
  QString usernameStr = QString::fromWCharArray(username);
  // Log du nom d'utilisateur récupéré
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__,
      "Current Username: " + usernameStr.toStdString());
  return usernameStr;
}
void MainWindow::downloadMods(QString path, QStringList modIDs) {
  this->path = path;

  foreach (QString modID, modIDs) {
    try {
      QProcess *process = new QProcess(this);
      process->setProgram("steamcmd.exe");

      // Commande pour télécharger le mod
      QStringList arguments;
      arguments << "+login"
                << "anonymous"
                << "+force_install_dir" << path << "+workshop_download_item"
                << "346110" << modID;

      // Commande pour quitter steamcmd après le téléchargement
      arguments << "+quit";

      process->setArguments(arguments);

      connect(process, &QProcess::finished, this,
              &MainWindow::onProcessFinished);

      process->start();
    } catch (const std::exception &e) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "An error occurred while executing the command: " +
              std::string(e.what()));
    }
  }
}
void MainWindow::onProcessFinished(int exitCode,
                                   QProcess::ExitStatus exitStatus) {
  QProcess *process = qobject_cast<QProcess *>(sender());
  if (!process)
    return;

  if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
    // Chemin source
    QString sourcePath = "C:/Users/" + getCurrentUsername() +
                         "/Desktop/test/steamapps/workshop/content/346110/";

    // Vérifier que le répertoire source existe
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
      // Log d'erreur si le répertoire source n'existe pas
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "Source directory does not exist: " + sourcePath.toStdString());
      return;
    }

    // Chemin destination
    QString destPath =
        this->path + "/Mods/"; // Utilisation de la variable membre path

    // Vérifier que le répertoire de destination existe ou le créer
    if (!QDir(destPath).exists()) {
      QDir().mkpath(destPath);
    }

    // Récupérer récursivement les fichiers et dossiers dans le répertoire
    // source
    QDirIterator it(sourcePath, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot,
                    QDirIterator::Subdirectories);
    while (it.hasNext()) {
      QString filePath = it.next();
      QString relativePath = sourceDir.relativeFilePath(filePath);
      QString destFilePath = destPath + QDir::separator() + relativePath;

      if (QFileInfo(filePath).isDir()) {
        // Créer le répertoire dans le répertoire de destination
        QDir(destFilePath).mkpath(".");
      } else {
        // Copier le fichier dans le répertoire de destination
        if (!QFile::copy(filePath, destFilePath)) {
          // Log d'erreur en cas d'échec de la copie
          ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
              LogLevel::ERRORING, __FILE__, __LINE__,
              "Failed to copy file: " + filePath.toStdString());
        }
      }
    }

    // Log de réussite de la copie des fichiers
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Files copied from directory " + sourcePath.toStdString() + " to " +
            destPath.toStdString());
  } else {
    // Log d'échec de l'installation du mod
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

  // Supprimez le processus de copie une fois qu'il a terminé son travail
  process->deleteLater();
}
void MainWindow::onInstallButtonClicked() {
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__, "Install Mods...");
  QString path = lineEdit->text();
  QString mods = modsLineEdit->text();

  // split la chaine en liste
  QStringList modList = mods.split(",");

  downloadMods(path, modList);
}
