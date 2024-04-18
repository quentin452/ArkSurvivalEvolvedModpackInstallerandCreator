#ifndef ArkSEModpackGlobals_H
#define ArkSEModpackGlobals_H
#include <ArkModIC/modmanager/ModDownloader.h>
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <thread>

#include <string>
class ArkSEModpackGlobals {
public:
  static LoggerThread LoggerInstance;
  static MainWindow *MainWindowInstance;
  static ModDownloader *ModDownloaderInstance;
  static WindowUtils *WindowUtilsInstance;
};
#endif // ArkSEModpackGlobals_H