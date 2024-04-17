
#include "ui_modsinformationwindow.h"
#include <ArkModIC/mainwindow.h>
#include <ArkModIC/modsinformationwindow.h>

ModsInformationWindow::ModsInformationWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::ModsInformationWindow) {
  ui->setupUi(this);
  mainWindow = qobject_cast<MainWindow *>(
      parent);
  connect(ui->goToMainWindowButton, &QPushButton::clicked, this,
          &ModsInformationWindow::goToMainWindow);
}
ModsInformationWindow::~ModsInformationWindow() { delete ui; }

void ModsInformationWindow::goToMainWindow() {
  this->hide();
  mainWindow->show();
}