*****************************
Installation & Uninstallation
*****************************

Windows
=======

**LOOT requires a 64-bit version of Windows 10 (1809) or later.**

LOOT can be installed either using its automated installer or manually from a release archive. If you are using the installer, just run it and follow the wizard steps. If installing manually, extract the downloaded archive to a location of your choice.

LOOT requires the `MSVC 2019 x64 redistributable`_ to be installed.

The installer automatically downloads and installs the redistributable if you don't already have them installed, but if you use the release archive then you will need to do so manually.

If LOOT was installed using the installer, then use the uninstaller linked to in the Start Menu to uninstall LOOT. If LOOT was installed manually:

1. Delete the files you extracted from the location you chose.
2. Delete the ``LOOT`` folder in your local application data folder, which can be accessed by entering ``%LOCALAPPDATA%`` into Windows' File Explorer.

.. _MSVC 2019 x64 redistributable: https://aka.ms/vs/16/release/vc_redist.x64.exe

Linux
=====

**LOOT requires a 64-bit version of Linux.**

LOOT can be installed from a release archive, but this is not recommended as the archive does not include most of LOOT's runtime dependencies.
