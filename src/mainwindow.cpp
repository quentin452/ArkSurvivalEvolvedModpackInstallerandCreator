#include "ui_mainwindow.h"
#include <ArkModIC/ArkSEModpackGlobals.h>
#include <ArkModIC/mainwindow.h>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <iostream>
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

void MainWindow::downloadMods(QString path, QStringList modIDs) {
  foreach (QString modID, modIDs) {
    try {
      QString depotPath = path + "/depotcache/" + modID + "/" + modID + ".zip";
      QString extractedPath = path + "/Mods/" + modID;

      // Log de début du téléchargement du mod
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__,
          "Downloading mod with ID: " + modID.toStdString());

      // Créez un objet QProcess
      QProcess *process = new QProcess(this);

      // Connectez les signaux et les slots pour gérer la sortie de la commande
      connect(
          process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          [=](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
              // Log de réussite de l'installation du mod
              ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                  LogLevel::INFO, __FILE__, __LINE__,
                  "Mod with ID " + modID.toStdString() +
                      " installed successfully");

              // Lancez le processus de décompression ici uniquement si le
              // téléchargement a réussi
              QProcess *unzipProcess = new QProcess(this);
              QString unzipCmd =
                  "unzip \"" + depotPath + "\" -d \"" + extractedPath + "\"";
              unzipProcess->start(unzipCmd);
              connect(
                  unzipProcess, &QProcess::finished,
                  [=](int unzipExitCode, QProcess::ExitStatus unzipExitStatus) {
                    if (unzipExitCode == 0 &&
                        unzipExitStatus == QProcess::NormalExit) {
                      // Log de réussite de la décompression du mod
                      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                          LogLevel::INFO, __FILE__, __LINE__,
                          "Mod with ID " + modID.toStdString() +
                              " unzipped successfully");

                    } else {
                      // Log d'échec de la décompression du mod
                      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                          LogLevel::ERROR, __FILE__, __LINE__,
                          "Failed to unzip mod with ID " + modID.toStdString());
                    }
                    unzipProcess->deleteLater();
                  });
            } else {
              // Log d'échec de l'installation du mod
              ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                  LogLevel::ERROR, __FILE__, __LINE__,
                  "Failed to install mod with ID " + modID.toStdString());
            }
            process->deleteLater();
          });

      // Exécutez la commande steamcmd
      QString cmd = "steamcmd +login anonymous +force_install_dir \"" + path +
                    "\" +download_depot 346110 " + modID;
      process->start(cmd);

    } catch (const std::exception &e) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERROR, __FILE__, __LINE__,
          "An error occurred while executing the command: " +
              std::string(e.what()));
    }
  }
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