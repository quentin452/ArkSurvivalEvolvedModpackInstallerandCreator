#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLineEdit>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  bool onPushButtonClicked();
  void onBrowseButtonClicked();
  void downloadMods(QString path, QStringList modIDs);
  void onInstallButtonClicked();

private:
  Ui::MainWindow *ui;
  QLineEdit *lineEdit; 
  QLineEdit *modsLineEdit;
};

#endif // MAINWINDOW_H
