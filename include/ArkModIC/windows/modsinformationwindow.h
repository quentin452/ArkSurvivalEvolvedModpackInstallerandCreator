#ifndef MODSINFORMATIONWINDOW_H
#define MODSINFORMATIONWINDOW_H

#include <ArkModIC/utils/QTINCLUDE.h>
#include <ArkModIC/windows/updatehandlerwithqwindow.h>

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
  QMap<uint64_t, QString> allModInfo;
  int totalRequests = 0;
  int receivedResponses = 0;
private slots:
  void onNetworkReply(QNetworkReply *reply);
};

#endif // MODSINFORMATIONWINDOW_H
