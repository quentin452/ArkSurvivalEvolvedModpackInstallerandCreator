#ifndef MODSINFORMATIONWINDOW_H
#define MODSINFORMATIONWINDOW_H

#include <ArkModIC/mainwindow.h>
#include <QLineEdit>
#include <QMainWindow>
#include <QProcess>
#include <string>

QT_BEGIN_NAMESPACE
namespace Ui {
class ModsInformationWindow;
}
QT_END_NAMESPACE

class MainWindow;

class ModsInformationWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit ModsInformationWindow(QWidget *parent = nullptr);
  ~ModsInformationWindow();
public slots:
  void goToMainWindow();

private:
  Ui::ModsInformationWindow *ui;
  MainWindow *mainWindow;
};

#endif // MODSINFORMATIONWINDOW_H
