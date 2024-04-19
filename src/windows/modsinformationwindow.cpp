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
    : UpdateHandlerWithQWindow(parent), ui(new Ui::ModsInformationWindow) {
  ui->setupUi(this);
  connect(ui->goToMainWindowButton, &QPushButton::clicked, this, [=]() {
    WindowUtils::SetCurrentWindow(this,
                                  ArkSEModpackGlobals::MainWindowInstance);
  });
}

ModsInformationWindow::~ModsInformationWindow() { delete ui; }

void ModsInformationWindow::updateCode() {
  if (this->isActiveWindow()) {
    queryAndDisplayModInfo();
  }
}

void ModsInformationWindow::displayModInfo(
    const QMap<uint64_t, QString> &modInfoMap) {
  ui->textEdit->clear();
  if (modInfoMap.isEmpty()) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "Mod information map is empty.");
    return;
  }
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__, "Displaying mod information...");
  for (auto it = modInfoMap.begin(); it != modInfoMap.end(); ++it) {
    QString modInfo = QString("Mod ID: %1, Mod Title: %2")
                          .arg(QString::number(it.key()), it.value());
    ui->textEdit->append(modInfo);
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Mod information displayed: " + modInfo.toStdString());
  }
}

void ModsInformationWindow::queryAndDisplayModInfo() {
  QString modsList =
      ArkSEModpackGlobals::MainWindowInstance->modsSteamIdListQuery->text()
          .trimmed();
  QStringList modIds = modsList.split(",");
  modIds.removeAll("");

  for (const QString &modId : modIds) {
    QString url =
        QString("https://steamcommunity.com/sharedfiles/filedetails/?id=%1")
            .arg(modId);
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
    QString responseDataString = QString::fromUtf8(responseData);

    // Recherche du titre du mod dans le HTML
    QString modTitle;
    int titleStartIndex = responseDataString.indexOf("<title>") + 7;
    int titleEndIndex = responseDataString.indexOf("</title>");
    if (titleStartIndex != -1 && titleEndIndex != -1) {
      modTitle = responseDataString.mid(titleStartIndex,
                                        titleEndIndex - titleStartIndex);
    }

    // Recherche de l'ID du mod dans l'URL
    QString url = reply->url().toString();
    int idStartIndex = url.lastIndexOf("=") + 1;
    QString modIdString = url.mid(idStartIndex);
    bool conversionOK;
    uint64_t modId = modIdString.toULongLong(&conversionOK);

    if (conversionOK) {
      QMap<uint64_t, QString> modInfoMap;
      modInfoMap[modId] = modTitle;
      displayModInfo(modInfoMap);
    } else {
      QString errorMessage = "Failed to parse mod ID from URL: " + url;
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__, errorMessage.toStdString());
    }
  } else {
    QString errorMessage =
        "Failed to query mod information: " + reply->errorString();
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__, errorMessage.toStdString());
  }
  reply->deleteLater();
}