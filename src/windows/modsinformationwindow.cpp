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

  internetStatusLabel = new QLabel("Internet: Connexion active");
  internetStatusLabel->setStyleSheet("color: green;");
  ui->verticalLayout->insertWidget(0, internetStatusLabel);
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
    QLabel *imageLabel = new QLabel;
    QNetworkAccessManager *imageManager = new QNetworkAccessManager(this);
    connect(
        imageManager, &QNetworkAccessManager::finished,
        [=, id = it.key()](QNetworkReply *reply) mutable {
          if (reply->error() == QNetworkReply::NoError) {
            QByteArray imageData = reply->readAll();
            QPixmap pixmap;
            bool loadSuccess = pixmap.loadFromData(imageData);
            if (loadSuccess) {
              imageLabel->setPixmap(pixmap.scaledToWidth(100));
            } else {
              ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                  LogLevel::ERRORING, __FILE__, __LINE__,
                  ("Failed to load image for mod ID: " + QString::number(id) +
                   ", Error: Failed to load image data")
                      .toStdString());
            }
          } else {
            ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                LogLevel::ERRORING, __FILE__, __LINE__,
                ("Failed to load image for mod ID: " + QString::number(id) +
                 ", Error: " + reply->errorString())
                    .toStdString());
          }
          reply->deleteLater();
        });
    QString url =
        QString("https://steamcommunity.com/sharedfiles/filedetails/?id=%1")
            .arg(it.key());
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, manager,
            &QNetworkAccessManager::deleteLater);
    QNetworkReply *reply = manager->get(request);
    connect(manager, &QNetworkAccessManager::finished, this,
            [=](QNetworkReply *reply) {
              if (reply->error() == QNetworkReply::NoError) {
                QString htmlContent = QString::fromUtf8(reply->readAll());
                QString imageUrl = extractImageUrlFromHtml(htmlContent);
                ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                    LogLevel::INFO, __FILE__, __LINE__,
                    "Extracted Image URL:" + imageUrl.toStdString());
                QNetworkRequest imageRequest(imageUrl);
                imageManager->get(imageRequest);
                if (!imageUrl.isEmpty()) { // Vérification de l'URL de l'image
                  QNetworkRequest imageRequest(imageUrl);
                  imageManager->get(imageRequest);
                } else {
                  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                      LogLevel::ERRORING, __FILE__, __LINE__,
                      ("Empty or invalid image URL for mod ID: " +
                       QString::number(it.key()))
                          .toStdString());
                }
              } else {
                ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
                    LogLevel::ERRORING, __FILE__, __LINE__,
                    ("Failed to fetch mod details for mod ID: " +
                     QString::number(it.key()) +
                     ", Error: " + reply->errorString())
                        .toStdString());
              }
              reply->deleteLater();
            });
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(imageLabel);
    hLayout->addWidget(label);
    ui->modColumnsLayout->addLayout(hLayout);
    modLabels.insert(it.key(), label);
  }
}

QString
ModsInformationWindow::extractImageUrlFromHtml(const QString &htmlContent) {
  QString imageUrl;
  QRegularExpression imageMainRegex(
      "<img[^>]*id=\"previewImageMain\"[^>]*src=['\"]([^'\"]+)['\"][^>]*>");
  QRegularExpressionMatch mainMatch = imageMainRegex.match(htmlContent);
  if (mainMatch.hasMatch()) {
    imageUrl = mainMatch.captured(1);
  } else {
    QRegularExpression imageRegex(
        "<img[^>]*id=\"previewImage\"[^>]*src=['\"]([^'\"]+)['\"][^>]*>");
    QRegularExpressionMatch match = imageRegex.match(htmlContent);
    if (match.hasMatch()) {
      imageUrl = match.captured(1);
    } else {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__,
          "No image URL found in HTML content.");
    }
  }
  return imageUrl;
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
    internetStatusLabel->setText("Internet: Connexion active");
    internetStatusLabel->setStyleSheet("color: green;");
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
    internetStatusLabel->setText("Internet: Connexion inactive(some mods "
                                 "informations couldn't be viewed)");
    internetStatusLabel->setStyleSheet("color: red;");
  }
  reply->deleteLater();
  receivedResponses++;
  if (receivedResponses == totalRequests) {
    displayModInfo(allModInfo);
    sender()->deleteLater();
  }
}
