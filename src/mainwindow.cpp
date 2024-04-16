#include <ArkModIC/mainwindow.h>
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QProcess>
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

            // Créez un objet QProcess
            QProcess *process = new QProcess(this);

            // Connectez les signaux et les slots pour gérer la sortie de la commande
            connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    [=](int exitCode, QProcess::ExitStatus exitStatus) {
                        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                            qDebug() << "Mod with ID" << modID << "installed successfully";
                            // Lancez le processus de décompression ici
                            QProcess *unzipProcess = new QProcess(this);
                            QString unzipCmd = "unzip \"" + depotPath + "\" -d \"" + extractedPath + "\"";
                            unzipProcess->start(unzipCmd);
                            connect(unzipProcess, &QProcess::finished, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                                if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
                                    qDebug() << "Mod with ID" << modID << "unzipped successfully";
                                } else {
                                    qDebug() << "Failed to unzip mod with ID" << modID;
                                }
                                unzipProcess->deleteLater();
                            });
                        } else {
                            qDebug() << "Failed to install mod with ID" << modID;
                        }
                        process->deleteLater();
                    });

            // Exécutez la commande steamcmd
            QString cmd = "steamcmd +login anonymous +force_install_dir \"" + path + "\" +download_depot 346110 " + modID;
            process->start(cmd);

        } catch (const std::exception &e) {
            qDebug() << "An error occurred while executing the command:" << e.what();
            // Gérer l'erreur ici
        }
    }
}


void MainWindow::onInstallButtonClicked() {

  QString path = lineEdit->text();
  QString mods = modsLineEdit->text();

  // split la chaine en liste
  QStringList modList = mods.split(",");

  downloadMods(path, modList);
}