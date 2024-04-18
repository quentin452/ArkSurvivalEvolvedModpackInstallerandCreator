#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/windows/!windowutils.h>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QRandomGenerator>
#include <QSettings>
#include <QStorageInfo>
#include <QTimer>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>

WindowUtils::WindowUtils() {}

WindowUtils::~WindowUtils() {}

void WindowUtils::SetCurrentWindow(QMainWindow *windowUsedForNow,
                                   QMainWindow *newWindow) {
  windowUsedForNow->hide();
  newWindow->show();
}
void WindowUtils::loadTextFromFileToString(const QString &filePath,
                                           QLineEdit *targetLineEdit) {
  QFile file(filePath);
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream in(&file);
    QString text = in.readAll();
    file.close();
    if (!text.isEmpty()) {
      targetLineEdit->setText(text);
    } else {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "The file is empty or contains invalid data.");
    }
  } else {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        ("Failed to open the file: " + file.errorString()).toStdString());
  }
}