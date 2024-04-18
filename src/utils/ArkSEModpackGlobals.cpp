#include <ArkModIC/modmanager/ModDownloader.h>
#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/windows/!windowutils.h>
#include <ArkModIC/windows/mainwindow.h>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <string>

LoggerThread ArkSEModpackGlobals::LoggerInstance;
MainWindow *ArkSEModpackGlobals::MainWindowInstance = nullptr;
ModDownloader *ArkSEModpackGlobals::ModDownloaderInstance = nullptr;
WindowUtils *ArkSEModpackGlobals::WindowUtilsInstance = nullptr;