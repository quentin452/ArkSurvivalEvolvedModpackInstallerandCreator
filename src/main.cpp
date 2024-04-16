#include <QApplication>
#include <QMessageBox>

#include <ArkModIC/ArkSEModpackGlobals.h>
#include <ArkModIC/mainwindow.h>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>

int main(int argc, char *argv[]) {
  LoggerGlobals::UsernameDirectory = std::getenv("USERNAME");

  // Create Log File and folder
  LoggerGlobals::LogFolderPath = "C:\\Users\\" +
                                 LoggerGlobals::UsernameDirectory +
                                 "\\.ArkModIC\\logging\\";
  LoggerGlobals::LogFilePath = "C:\\Users\\" +
                               LoggerGlobals::UsernameDirectory +
                               "\\.ArkModIC\\logging\\ArkModIC.log";
  LoggerGlobals::LogFolderBackupPath = "C:\\Users\\" +
                                       LoggerGlobals::UsernameDirectory +
                                       "\\.ArkModIC\\logging\\LogBackup";
  LoggerGlobals::LogFileBackupPath =
      "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
      "\\.ArkModIC\\logging\\LogBackup\\ArkModIC-";

  // Start the logger thread
  ArkSEModpackGlobals::LoggerInstance.StartLoggerThread(
      LoggerGlobals::LogFolderPath, LoggerGlobals::LogFilePath,
      LoggerGlobals::LogFolderBackupPath, LoggerGlobals::LogFileBackupPath);

  // Log messages after starting the logger thread
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(LogLevel::INFO,
                                                      "logger test");
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(LogLevel::INFO,
                                                      "logger test2");
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(LogLevel::INFO,
                                                      "logger test3");

  QApplication app(argc, argv);

  try {
    MainWindow *window = new MainWindow();
    window->show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(LogLevel::INFO,
                                                          "logger test4");
      ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
    });

    return app.exec();
  } catch (const std::exception &e) {
    QMessageBox::critical(nullptr, "Error", e.what());
  }

  return -1;
}
