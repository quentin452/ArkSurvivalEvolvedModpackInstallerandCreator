
#include "ui_modsinformationwindow.h"
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>


ModsInformationWindow::ModsInformationWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ModsInformationWindow) {
  ui->setupUi(this);
  mainWindow = qobject_cast<MainWindow *>(parent);
  connect(ui->goToMainWindowButton, &QPushButton::clicked, this,
          [=]() { WindowUtils::SetCurrentWindow(this, mainWindow); });
}

ModsInformationWindow::~ModsInformationWindow() { delete ui; }