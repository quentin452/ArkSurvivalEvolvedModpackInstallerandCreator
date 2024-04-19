#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <ArkModIC/utils/QTINCLUDE.h>
#include <string>
class Configuration {

public:
  Configuration();
  ~Configuration();

  static void readLastUsedModsFileFromConfig(QString &lastUsedModsFile);
  static void saveLastUsedModsFileToConfig(const QString &lastUsedModsFile);
  static void readSettingsFromConfigFile(QString &gamePath);

  static void readCheckboxStatesFromConfigFile(bool &deleteMods,
                                               bool &backupMods);
  static void saveCheckboxStatesToConfigFile(bool deleteMods, bool backupMods);
  static void saveSettingsToConfigFile(const QString &gamePath);

private:
  static std::string CONFIG_FILE_PATH;
  static std::string GAME_PATH_KEY;
  static std::string MODS_LIST_KEY;
  static std::string DELETE_MODS_KEY;
  static std::string BACKUP_MODS_KEY;
  static std::string LAST_USED_MODS_FILE_KEY;
};

#endif // CONFIGURATION_H