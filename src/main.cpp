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
#include <tlhelp32.h>
#include <windows.h>

int ExitLoggerThread = 0;

void terminatePreviousInstances() {
  // Get the process ID of the current process
  DWORD currentProcessId = GetCurrentProcessId();

  // Create a snapshot of the current system processes
  HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  // Check if the snapshot was successfully created
  if (Process32First(hSnapShot, &pe32)) {
    // Iterate through the processes
    do {
      // Check if the process name matches ArkModIC.exe and if it's not the
      // current process
      if (_wcsicmp(pe32.szExeFile, L"ArkModIC.exe") == 0 &&
          pe32.th32ProcessID != currentProcessId) {
        // Open the process with the termination permission
        HANDLE hProcess =
            OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
        if (hProcess != NULL) {
          // Terminate the process
          TerminateProcess(hProcess, 1);
          CloseHandle(hProcess);
        }
      }
    } while (Process32Next(hSnapShot, &pe32));
  }

  // Close the snapshot handle
  CloseHandle(hSnapShot);
}

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
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__, "Running with admin privileges");
    QApplication app(argc, argv);
    char *steamcmdPath = std::getenv("steamcmd");
    if (steamcmdPath == nullptr) {
      ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
          LogLevel::ERRORING, __FILE__, __LINE__,
          "steamcmd variable not found in Windows environment. PLS ADD IT");
      ArkSEModpackGlobals::LoggerInstance.ExitLoggerThread();
      return 1;
    }
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
  }

  if (!fResult && dwLastError != ERROR_CANCELLED) {
    ArkSEModpackGlobals::LoggerInstance.logMessageAsync(
        LogLevel::INFO, __FILE__, __LINE__,
        "Failed to get admin privileges, error code: " + dwLastError);
  }

  return 1;
}

int main(int argc, char *argv[]) {
  terminatePreviousInstances();
  return LoggerThreadInit(argc, argv), runElevated(argc, argv);
}