*****************************
Installation & Uninstallation
*****************************

**LOOT requires Windows 7 or later.** The official 64-bit releases raise this requirement to a 64-bit version of Windows 10 (1809) or later.

LOOT can be installed either using its automated installer or manually. If you are using the installer, just run it and follow the wizard steps. If installing manually, extract the downloaded archive to a location of your choice.

The official 64-bit LOOT release requires the `MSVC 2019 x64 redistributable`_ to be installed, while the 32-bit release requires the `MSVC 2010 x86 redistributable`_ and the `MSVC 2019 x86 redistributable`_ to be installed.

The installer automatically downloads and installs the appropriate MSVC 2019 redistributable if you don't already have it installed, but if you use the LOOT archive then you will need to do so manually.

If LOOT was installed using the installer, then use the uninstaller linked to in the Start Menu to uninstall LOOT. If LOOT was installed manually:

1. Delete the files you extracted from the location you chose.
2. Delete the ``LOOT`` folder in your local application data folder, which can be accessed by entering ``%LOCALAPPDATA%`` into Windows' File Explorer.

.. _MSVC 2010 x86 redistributable: https://docs.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist?view=msvc-170#visual-studio-2010-vc-100-sp1-no-longer-supported
.. _MSVC 2019 x86 redistributable: https://aka.ms/vs/16/release/vc_redist.x86.exe
.. _MSVC 2019 x64 redistributable: https://aka.ms/vs/16/release/vc_redist.x64.exe
