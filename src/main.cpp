#include <QApplication>
#include <QMessageBox>

#include <ArkModIC/ArkSEModpackGlobals.h>
#include <ArkModIC/mainwindow.h>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <Windows.h>
#include <iostream>
#include <shellapi.h>
#include <windows.h>

int ExitLoggerThread = 0;

int LoggerThreadInit(int argc, char *argv[]) {
  LoggerGlobals::UsernameDirectory = std::getenv("USERNAME");

  // this is the folder that contain your src files like main.cpp
  LoggerGlobals::SrcProjectDirectory = "src";

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
}
int runElevated(int argc, char *argv[]) {
  bool isElevated = false;
  BOOL fResult = FALSE;
  DWORD dwLastError = ERROR_SUCCESS;

  // Try to get the path of the current executable
  char exePath[MAX_PATH];
  GetModuleFileNameA(NULL, exePath, MAX_PATH);

  // Check if the process is already running with admin privileges
  HANDLE hToken = nullptr;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
    TOKEN_ELEVATION Elevation;
    DWORD dwSize;
    if (GetTokenInformation(hToken, TokenElevation, &Elevation,
                            sizeof(Elevation), &dwSize)) {
      isElevated = Elevation.TokenIsElevated;
    }
  }

  // If not elevated, try to re-run the executable with admin privileges
  if (!isElevated) {
    SHELLEXECUTEINFOA shExInfo = {0};
    shExInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.hwnd = nullptr;
    shExInfo.lpVerb = "runas";
    shExInfo.lpFile = exePath;
    shExInfo.lpParameters = "";
    shExInfo.lpDirectory = nullptr;
    shExInfo.nShow = SW_SHOW;
    shExInfo.hInstApp = nullptr;

    fResult = ShellExecuteExA(&shExInfo);
    dwLastError = fResult ? ERROR_SUCCESS : GetLastError();

    if (fResult) {
      // Wait for the new process to finish
      WaitForSingleObject(shExInfo.hProcess, INFINITE);
      CloseHandle(shExInfo.hProcess);
      return 0; // Exit the current process
    }
  } else {
    // If already elevated, run the main program logic here
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "Running with admin privileges");
    // Log messages after starting the logger thread
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "logger test");
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "logger test2");
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "logger test3");

    QApplication app(argc, argv);
    char *steamcmdPath = std::getenv("steamcmd");
    if (steamcmdPath == nullptr) {
      // Log an error message
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "steamcmd variable not found in Windows environment. PLS ADD IT");
      ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
      // Exit the application
      return -1;
    }
    try {
      MainWindow *window = new MainWindow();
      window->show();

      QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
        ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
            LogLevel::INFO, __FILE__, __LINE__, "logger test4");
        ExitLoggerThread = 1;
      });

      return app.exec();
    } catch (const std::exception &e) {
      QMessageBox::critical(nullptr, "Error", e.what());
    }
  }

  if (!fResult && dwLastError != ERROR_CANCELLED) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Failed to get admin privileges, error code: " + dwLastError);
  }

  if (ExitLoggerThread == 1) {
    ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
  }

  return -1;
}

int main(int argc, char *argv[]) {
  return LoggerThreadInit(argc, argv), runElevated(argc, argv);
}