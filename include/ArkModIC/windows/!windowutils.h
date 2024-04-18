#ifndef WINDOWUTILS_H
#define WINDOWUTILS_H

#include <QLineEdit>
#include <QMainWindow>
#include <QObject>
#include <QProcess>
#include <string>

class WindowUtils : public QObject {
  Q_OBJECT

public:
  explicit WindowUtils();
  ~WindowUtils();
  void loadTextFromFileToString(const QString &filePath,
                                QLineEdit *targetLineEdit);
  static void SetCurrentWindow(QMainWindow *windowUsedForNow,
                               QMainWindow *newWindow);

private:
private slots:
};

#endif // WINDOWUTILS_H