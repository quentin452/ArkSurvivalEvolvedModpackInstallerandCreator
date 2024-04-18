#ifndef ArkSEModpackGlobals_H
#define ArkSEModpackGlobals_H
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <thread>

#include <string>
class ArkSEModpackGlobals {
public:
  static LoggerThread LoggerInstance;
};
#endif // ArkSEModpackGlobals_H