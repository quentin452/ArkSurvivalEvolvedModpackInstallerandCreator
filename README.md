# ArkSurvivalEvolvedModpackInstallerandCreator
a cpp app that can install a list of mods(modpack) on client and server for easier modpack download for the game ArkSE

How to build : 

1 : show this tutorial : https://youtu.be/XiMplRfuFJc (install QT , Configure Qt in path (Windows) and install CMAKE)

How to execute an already builded file : 

1 : show this tutorial : https://youtu.be/XiMplRfuFJc (install QT and Configure Qt in path (Windows))

2: Download SteamCmd and add the SteamCmd environment variable in Windows

    Open the Start menu and search for "Edit the system environment variables" or "Edit environment variables for your account".

    Click on the result to open the System Properties window.

    In the System Properties window, click on the "Environment Variables" button.

    In the Environment Variables window, under the "System Variables" section, click on the "New" button.

    In the "New System Variable" window, enter "steamcmd" as the "Variable name".

    For the "Variable value", enter the full path to the directory where you extracted or installed SteamCmd. For example, if you installed SteamCmd in "C:\Program Files\Steam\steamcmd\steamcmd.exe", enter that as the value.

    Click "OK" to save the new environment variable.

    Next, you need to add the SteamCmd directory to the system's PATH variable. In the "System Variables" section, locate the "Path" variable, click on it, and then click "Edit".

    In the "Edit Environment Variable" window, click on "New" and enter the full path to the SteamCmd directory (e.g., "C:\Program Files\Steam\steamcmd").

    Click "OK" to save the changes to the PATH variable.

    After making these changes, you may need to restart your computer or any running command prompts or applications for the new environment variables to take effect.

Now, the SteamCmd executable should be accessible from any directory in your command prompt or applications that rely on the SteamCmd environment variable. If you encounter any issues, double-check that the paths are correct and that you have the necessary permissions to modify system environment variables.