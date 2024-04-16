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
                                 "\\.ThreadedLoggerForCPPTest\\logging\\";
  LoggerGlobals::LogFilePath =
      "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
      "\\.ThreadedLoggerForCPPTest\\logging\\LuaCraftCPP.log";
  LoggerGlobals::LogFolderBackupPath =
      "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
      "\\.ThreadedLoggerForCPPTest\\logging\\LogBackup";
  LoggerGlobals::LogFileBackupPath =
      "C:\\Users\\" + LoggerGlobals::UsernameDirectory +
      "\\.ThreadedLoggerForCPPTest\\logging\\LogBackup\\LuaCraftCPP-";

  ArkSEModpackGlobals::LoggerInstance.StartLoggerThread(
      LoggerGlobals::LogFolderPath, LoggerGlobals::LogFilePath,
      LoggerGlobals::LogFolderBackupPath, LoggerGlobals::LogFileBackupPath);
  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(LogLevel::INFO,
                                                      "logger test");
  QApplication app(argc, argv);

  try {
    MainWindow *window = new MainWindow();
    window->show();

    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
      ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
    });

    return app.exec();
  } catch (const std::exception &e) {
    QMessageBox::critical(nullptr, "Error", e.what());
  }

  return -1;
}