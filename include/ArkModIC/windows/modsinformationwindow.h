#ifndef MODSINFORMATIONWINDOW_H
#define MODSINFORMATIONWINDOW_H

#include <ArkModIC/windows/updatehandlerwithqwindow.h>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRandomGenerator>
#include <QSettings>
#include <QStorageInfo>
#include <QStringList>
#include <QTimer>
#include <QUrlQuery>

QT_BEGIN_NAMESPACE
namespace Ui {
class ModsInformationWindow;
}
QT_END_NAMESPACE

class ModsInformationWindow : public UpdateHandlerWithQWindow {
  Q_OBJECT

public:
  explicit ModsInformationWindow(QWidget *parent = nullptr);
  ~ModsInformationWindow();

  void displayModInfo(const QMap<uint64_t, QString> &modInfoMap);
  void updateCode() override;
  void queryAndDisplayModInfo();

private:
  Ui::ModsInformationWindow *ui;
  QMap<uint64_t, QLabel *> modLabels;
private slots:
  void onNetworkReply(QNetworkReply *reply);
};

#endif // MODSINFORMATIONWINDOW_H
