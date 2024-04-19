#include "ui_modsinformationwindow.h"
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/utils/QTINCLUDE.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>
#include <cstdint>
ModsInformationWindow::ModsInformationWindow(QWidget *parent)
    : UpdateHandlerWithQWindow(parent), ui(new Ui::ModsInformationWindow) {
  ui->setupUi(this);
  connect(ui->goToMainWindowButton, &QPushButton::clicked, this, [=]() {
    WindowUtils::SetCurrentWindow(this,
                                  ArkSEModpackGlobals::MainWindowInstance);
  });
  verticalLayout = new QVBoxLayout();
}

ModsInformationWindow::~ModsInformationWindow() { delete ui; }

void ModsInformationWindow::updateCode() {
  if (this->isActiveWindow()) {
    queryAndDisplayModInfo();
  }
}
void ModsInformationWindow::displayModInfo(
    const QMap<uint64_t, QString> &modInfoMap) {
  for (auto label : modLabels.values()) {
    delete label;
  }
  modLabels.clear();
  QLayoutItem *child;
  while ((child = ui->modColumnsLayout->takeAt(0)) != nullptr) {
    delete child->widget();
    delete child;
  }
  QLayout *layout = ui->modColumnsLayout->layout();
  if (layout) {
    QLayoutItem *item;
    while ((item = layout->takeAt(0)) != nullptr) {
      delete item->widget();
      delete item;
    }
  }
  for (auto it = modInfoMap.begin(); it != modInfoMap.end(); ++it) {
    QString labelText = QString("Mod ID: %1, Mod Title: %2")
                            .arg(QString::number(it.key()), it.value());
    QLabel *label = new QLabel(labelText);
    ui->modColumnsLayout->addWidget(label);
    modLabels.insert(it.key(), label);
  }
}

void ModsInformationWindow::queryAndDisplayModInfo() {
  QString gamePath =
      ArkSEModpackGlobals::MainWindowInstance->gamePathQuery->text().trimmed();
  QString modsDirectory = gamePath + "/Mods";
  QDir modsDir(modsDirectory);
  QStringList modDirs = modsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  totalRequests = modDirs.size();
  receivedResponses = 0;
  allModInfo.clear();
  for (const QString &modDir : modDirs) {
    QString modId = modDir;
    QString url =
        QString("https://steamcommunity.com/sharedfiles/filedetails/?id=%1")
            .arg(modId);
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, manager,
            &QNetworkAccessManager::deleteLater);
    QNetworkReply *reply = manager->get(request);
    connect(manager, &QNetworkAccessManager::finished, this,
            &ModsInformationWindow::onNetworkReply);
  }
}

void ModsInformationWindow::onNetworkReply(QNetworkReply *reply) {
  reply->deleteLater();
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
    QString modTitle;
    int titleStartIndex = responseDataString.indexOf("<title>") + 7;
    int titleEndIndex = responseDataString.indexOf("</title>");
    if (titleStartIndex != -1 && titleEndIndex != -1) {
      modTitle = responseDataString.mid(titleStartIndex,
                                        titleEndIndex - titleStartIndex);
    }
    bool conversionOK;
    uint64_t modIdValue = modId.toULongLong(&conversionOK);
    if (conversionOK) {
      allModInfo[modIdValue] = modTitle;
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
  receivedResponses++;
  if (receivedResponses == totalRequests) {
    displayModInfo(allModInfo);
    sender()->deleteLater();
  }
}