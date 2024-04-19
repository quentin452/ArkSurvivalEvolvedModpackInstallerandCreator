#include <ArkModIC/windows/updatehandlerwithqwindow.h>

UpdateHandlerWithQWindow::UpdateHandlerWithQWindow(QWidget *parent)
    : QMainWindow(parent) {
  timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &UpdateHandlerWithQWindow::update);
  timer->start(1000); // 1000 ms interval
}

void UpdateHandlerWithQWindow::update() { updateCode(); }

void UpdateHandlerWithQWindow::updateCode() {}