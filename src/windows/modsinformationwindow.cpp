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
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPlainTextEdit>
#include <QProcess>
#include <QRandomGenerator>
#include <QSettings>
#include <QStorageInfo>
#include <QTextEdit>
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
  // Supprimer les widgets individuels de modLabels
  for (auto label : modLabels.values()) {
    delete label;
  }
  modLabels.clear();

  // Create a new vertical layout
  QVBoxLayout *verticalLayout = new QVBoxLayout();

  // Add each mod label directly to the vertical layout
  for (auto it = modInfoMap.begin(); it != modInfoMap.end(); ++it) {
    QString labelText = QString("Mod ID: %1, Mod Title: %2")
                            .arg(QString::number(it.key()), it.value());
    QLabel *label = new QLabel(labelText);
    verticalLayout->addWidget(label);
    modLabels.insert(it.key(), label);
  }
  // Set the new vertical layout to modColumnsLayout
  ui->modColumnsLayout->addLayout(verticalLayout);
}
void ModsInformationWindow::queryAndDisplayModInfo() {
  QString modsList =
      ArkSEModpackGlobals::MainWindowInstance->modsSteamIdListQuery->text()
          .trimmed();
  QStringList modIds = modsList.split(",");
  modIds.removeAll("");

  // Créer une carte pour associer les ID de mod aux réponses réseau
  QMap<QString, QNetworkReply *> modIdToReplyMap;

  for (const QString &modId : modIds) {
    QString url =
        QString("https://steamcommunity.com/sharedfiles/filedetails/?id=%1")
            .arg(modId);
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    // Associer chaque ID de mod à sa réponse réseau
    QNetworkReply *reply = manager->get(request);
    modIdToReplyMap[modId] = reply;

    // Connecter la réponse au slot de traitement
    connect(manager, &QNetworkAccessManager::finished, this,
            &ModsInformationWindow::onNetworkReply);
  }

  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
      LogLevel::INFO, __FILE__, __LINE__,
      "Querying and displaying mod information...");
}

void ModsInformationWindow::onNetworkReply(QNetworkReply *reply) {
  // Récupérer l'ID de mod correspondant à cette réponse
  QString url = reply->url().toString();
  QRegularExpression idRegex("\\b(id=)(\\d+)\\b");
  QRegularExpressionMatch match = idRegex.match(url);
  if (!match.hasMatch()) {
    reply->deleteLater();
    return;
  }
  QString modId = match.captured(2);

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

    // Afficher les informations du mod
    QMap<uint64_t, QString> modInfoMap;
    bool conversionOK;
    uint64_t modIdValue = modId.toULongLong(&conversionOK);
    if (conversionOK) {
      modInfoMap[modIdValue] = modTitle;
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

  // Supprimer la réponse une fois traitée
  reply->deleteLater();
}