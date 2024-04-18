#include <ArkModIC/ArkUtils.h>
#include <ArkModIC/Configuration.h>
#include <QFile>
#include <QSettings>
#include <QString>
#include <ThreadedLoggerForCPP/LoggerGlobals.hpp>
#include <windows.h>
ArkUtils utils;
std::string Configuration::CONFIG_FILE_PATH =
    "C:/Users/" + utils.getUsernameDirectory() + "/.ArkModIC/config.ini";
std::string Configuration::GAME_PATH_KEY = "GamePath";
std::string Configuration::MODS_LIST_KEY = "ModsList";
std::string Configuration::DELETE_MODS_KEY = "DeleteMods";
std::string Configuration::BACKUP_MODS_KEY = "BackupMods";
std::string Configuration::LAST_USED_MODS_FILE_KEY = "LastUsedModsFile";

Configuration::Configuration() {}

Configuration::~Configuration() {}

void Configuration::readLastUsedModsFileFromConfig(QString &lastUsedModsFile) {
  QSettings settings(QString::fromStdString(CONFIG_FILE_PATH),
                     QSettings::IniFormat);
  QString filePath =
      settings.value(QString::fromStdString(LAST_USED_MODS_FILE_KEY))
          .toString();
  QFile file(filePath);
  if (file.exists()) {
    lastUsedModsFile = filePath;
  } else {
    lastUsedModsFile = "";
  }
}
void Configuration::saveLastUsedModsFileToConfig(
    const QString &lastUsedModsFile) {
  QSettings settings(QString::fromStdString(CONFIG_FILE_PATH),
                     QSettings::IniFormat);
  settings.setValue(QString::fromStdString(LAST_USED_MODS_FILE_KEY),
                    lastUsedModsFile);
}

void Configuration::readSettingsFromConfigFile(QString &gamePath) {
  QSettings settings(QString::fromStdString(CONFIG_FILE_PATH),
                     QSettings::IniFormat);
  gamePath = settings.value(QString::fromStdString(GAME_PATH_KEY)).toString();
}

void Configuration::readCheckboxStatesFromConfigFile(bool &deleteMods,
                                                     bool &backupMods) {
  QSettings settings(QString::fromStdString(CONFIG_FILE_PATH),
                     QSettings::IniFormat);
  deleteMods =
      settings.value(QString::fromStdString(DELETE_MODS_KEY), false).toBool();
  backupMods =
      settings.value(QString::fromStdString(BACKUP_MODS_KEY), false).toBool();
}

void Configuration::saveCheckboxStatesToConfigFile(bool deleteMods,
                                                   bool backupMods) {
  QSettings settings(QString::fromStdString(CONFIG_FILE_PATH),
                     QSettings::IniFormat);
  settings.setValue(QString::fromStdString(DELETE_MODS_KEY), deleteMods);
  settings.setValue(QString::fromStdString(BACKUP_MODS_KEY), backupMods);
}

void Configuration::saveSettingsToConfigFile(const QString &gamePath) {
  QSettings settings(QString::fromStdString(CONFIG_FILE_PATH),
                     QSettings::IniFormat);
  settings.setValue(QString::fromStdString(GAME_PATH_KEY), gamePath);
}