*****************************
Installation & Uninstallation
*****************************

**LOOT requires Windows 7 or later.**

**Windows 7 users** must ensure that they have `enabled TLS 1.2 support`_ to support updating the masterlists from LOOT's official repositories on GitHub.

LOOT can be installed either using its automated installer or manually. If you are using the installer, just run it and follow the wizard steps. If installing manually, extract the downloaded archive to a location of your choice, then download and install the `MSVC 2015 x86 redistributable`_ if you don't already have it installed.

If LOOT was installed using the installer, then use the uninstaller linked to in the Start Menu to uninstall LOOT. If LOOT was installed manually:

1. Delete the files you extracted from the location you chose.
2. Delete the ``LOOT`` folder in your local application data folder, which can be accessed by entering ``%LOCALAPPDATA%`` into Windows' File Explorer.

.. _enabled TLS 1.2 support: https://support.microsoft.com/en-us/help/3140245/update-to-enable-tls-1-1-and-tls-1-2-as-a-default-secure-protocols-in
.. _MSVC 2015 x86 redistributable: https://download.microsoft.com/download/6/A/A/6AA4EDFF-645B-48C5-81CC-ED5963AEAD48/vc_redist.x86.exe
