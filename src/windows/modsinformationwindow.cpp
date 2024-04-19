#include "ui_modsinformationwindow.h"
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRandomGenerator>
#include <QSettings>
#include <QStorageInfo>
#include <QTimer>
#include <QUrlQuery>
#include <cstdint>

ModsInformationWindow::ModsInformationWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ModsInformationWindow) {
  ui->setupUi(this);
  connect(ui->goToMainWindowButton, &QPushButton::clicked, this, [=]() {
    WindowUtils::SetCurrentWindow(this,
                                  ArkSEModpackGlobals::MainWindowInstance);
  });
}

ModsInformationWindow::~ModsInformationWindow() { delete ui; }

void ModsInformationWindow::update() { queryAndDisplayModInfo(); }

void ModsInformationWindow::displayModInfo(const QStringList &modInfoList) {
  ui->textEdit->clear();
  for (const QString &modInfo : modInfoList) {
    ui->textEdit->append(modInfo);
  }
}

void ModsInformationWindow::queryAndDisplayModInfo() {
  QString modsList =
      ArkSEModpackGlobals::MainWindowInstance->modsSteamIdListQuery->text()
          .trimmed();
  QStringList modIds = modsList.split(",");
  modIds.removeAll("");

  for (const QString &modId : modIds) {
    uint64_t workshopId =
        modId.toULongLong(); // Déclarer workshopId comme uint64_t
    QUrl url("https://api.steampowered.com/ISteamRemoteStorage/"
             "GetPublishedFileDetails/v1/");
    QUrlQuery query;
    query.addQueryItem("itemcount", "1");
    query.addQueryItem(
        "publishedfileids[0]",
        QString::number(workshopId)); // Supprimer le troisième argument
    url.setQuery(query);

    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this,
            &ModsInformationWindow::onNetworkReply);
    manager->get(request);
  }

  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__,
      "Querying and displaying mod information...");
}

void ModsInformationWindow::onNetworkReply(QNetworkReply *reply) {
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDocument = QJsonDocument::fromJson(responseData);

    // Récupérer les détails du mod à partir de la réponse JSON
    QJsonObject jsonObject = jsonDocument.object();
    QJsonObject response = jsonObject["response"].toObject();
    QJsonArray publishedFileDetails =
        response["publishedfiledetails"].toArray();

    // Vérifier si des détails de mod ont été retournés
    if (!publishedFileDetails.isEmpty()) {
      QJsonObject modDetails = publishedFileDetails.first().toObject();
      QString modTitle = modDetails["title"].toString();
      uint64_t modId = modDetails["publishedfileid"].toDouble();

      // Afficher les informations du mod (vous pouvez les enregistrer dans les
      // journaux ici)
      QString modInfo = QString("Mod ID: %1, Mod Title: %2")
                            .arg(QString::number(modId), modTitle);
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__, modInfo.toStdString());
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__,
          "Mod information displayed: " + modInfo.toStdString());
    }
  } else {
    // Gérer les erreurs de requête réseau
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "Failed to query mod information: " +
            reply->errorString().toStdString());
  }

  reply->deleteLater();
}