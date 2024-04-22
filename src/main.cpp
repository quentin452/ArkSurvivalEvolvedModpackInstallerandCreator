#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/utils/Configuration.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>
#include <QApplication>
#include <QMessageBox>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <Windows.h>
#include <cstdlib>
#include <iostream>
#include <shellapi.h>
#include <tlhelp32.h>

void terminatePreviousInstances() {
  DWORD currentProcessId = GetCurrentProcessId();
  HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);
  if (Process32First(hSnapShot, &pe32)) {
    do {
      if (_wcsicmp(pe32.szExeFile, L"ArkModIC.exe") == 0 &&
          pe32.th32ProcessID != currentProcessId) {
        HANDLE hProcess =
            OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL) {
          TerminateProcess(hProcess, 1);
          CloseHandle(hProcess);
        }
      }
    } while (Process32Next(hSnapShot, &pe32));
  }
  CloseHandle(hSnapShot);
}

void initLoggerThread() {
  LoggerGlobals::UsernameDirectory = std::getenv("USERNAME");
  LoggerGlobals::SrcProjectDirectory = "src";
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

  ArkSEModpackGlobals::LoggerInstance.StartLoggerThread(
      LoggerGlobals::LogFolderPath, LoggerGlobals::LogFilePath,
      LoggerGlobals::LogFolderBackupPath, LoggerGlobals::LogFileBackupPath);
}

bool elevatePrivileges(const char *exePath) {
  bool isElevated = false;
  HANDLE hToken = nullptr;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
    TOKEN_ELEVATION Elevation;
    DWORD dwSize;
    if (GetTokenInformation(hToken, TokenElevation, &Elevation,
                            sizeof(Elevation), &dwSize)) {
      isElevated = Elevation.TokenIsElevated;
    }
  }
  if (!isElevated) {
    SHELLEXECUTEINFOA shExInfo = {0};
    shExInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
    shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExInfo.lpVerb = "runas";
    shExInfo.lpFile = exePath;
    shExInfo.lpParameters = "";
    shExInfo.nShow = SW_SHOW;
    if (ShellExecuteExA(&shExInfo)) {
      WaitForSingleObject(shExInfo.hProcess, INFINITE);
      CloseHandle(shExInfo.hProcess);
      return true;
    }
  }
  return isElevated;
}

int main(int argc, char *argv[]) {
  terminatePreviousInstances();
  initLoggerThread();
  // if (!QSslSocket::supportsSsl()) {
  //  ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
  //       LogLevel::WARNING, __FILE__, __LINE__,
  //      "SSL not supported!");
  //  return 1;
  //}
  char exePath[MAX_PATH];
  GetModuleFileNameA(nullptr, exePath, MAX_PATH);
  if (!elevatePrivileges(exePath)) {
 ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
       LogLevel::ERRORING, __FILE__, __LINE__,
       "Failed to get admin privileges");
  return 1;
  }
  QApplication app(argc, argv);
  char *steamcmdPath = std::getenv("steamcmd");
  if (steamcmdPath == nullptr) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::ERRORING, __FILE__, __LINE__,
        "steamcmd variable not found in Windows environment. PLS ADD IT");
    return 1;
  }
  try {
    ArkSEModpackGlobals::MainWindowInstance = new MainWindow();
    ArkSEModpackGlobals::ModInformationWindowInstance =
        new ModsInformationWindow();
    ArkSEModpackGlobals::MainWindowInstance->show();
    ArkSEModpackGlobals::ModDownloaderInstance = new ModDownloader();
    ArkSEModpackGlobals::WindowUtilsInstance = new WindowUtils();
    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::INFO, __FILE__, __LINE__, "Exiting application...");
      QObject::disconnect(&app, &QApplication::aboutToQuit, nullptr, nullptr);
      ArkSEModpackGlobals::MainWindowInstance->close();
      ArkSEModpackGlobals::ModInformationWindowInstance->close();
      qApp->quit();
      ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
    });
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "Starting application...");
    return app.exec();
  } catch (const std::exception &e) {
    QMessageBox::critical(nullptr, "Error", e.what());
    ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
    return 1;
  }
}
