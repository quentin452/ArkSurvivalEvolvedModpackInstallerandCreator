#ifndef ArkSEModpackGlobals_H
#define ArkSEModpackGlobals_H
#include <ArkModIC/modmanager/ModDownloader.h>
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ArkModIC/windows/modsinformationwindow.h>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <string>
#include <thread>

class ArkSEModpackGlobals {
public:
  static LoggerThread LoggerInstance;
  static MainWindow *MainWindowInstance;
  static ModDownloader *ModDownloaderInstance;
  static WindowUtils *WindowUtilsInstance;
  static ModsInformationWindow *ModInformationWindowInstance;
};
#endif // ArkSEModpackGlobals_H