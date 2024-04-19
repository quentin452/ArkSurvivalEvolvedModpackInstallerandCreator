#ifndef UPDATEHANDLER_H
#define UPDATEHANDLER_H
#include <ArkModIC/utils/QTINCLUDE.h>

class UpdateHandlerWithQWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit UpdateHandlerWithQWindow(QWidget *parent = nullptr);
  virtual void updateCode();

private:
  QTimer *timer;
  void update();
};

#endif // UPDATEHANDLER_H
