#include <ArkModIC/utils/ArkSEModpackGlobals.h>
#include <ArkModIC/utils/ArkUtils.h>
#include <ArkModIC/utils/QTINCLUDE.h>
#include <Shlobj.h>
#include <ThreadedLoggerForCPP/LoggerFileSystem.hpp>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <ThreadedLoggerForCPP/LoggerThread.hpp>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <windows.h>

ArkUtils::ArkUtils() {}

ArkUtils::~ArkUtils() {}
std::string ArkUtils::getUsernameDirectory() {
  const char *username = std::getenv("USERNAME");
  std::string usernameStr = username ? username : "";
  return usernameStr;
}