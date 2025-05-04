.. _themes:

******
Themes
******

Themes can be selected in LOOT's settings dialog. Two themes are provided with LOOT: the default theme and a dark theme. The default theme can adapt to light and dark system colour schemes. You can also explicitly select its light and dark variants, though their behaviour varies across systems:

- On Windows selecting the opposite variant to your system colour scheme will use the selected colour scheme.
- On Linux selecting the opposite variant to your system colour scheme may not work correctly and result in a mix of the system colour scheme and elements of LOOT's variant theme.

Additional themes may be installed in the ``themes`` directory in LOOT's application data directory (at ``%LOCALAPPDATA%\LOOT`` on Windows). A theme is defined by a single ``<theme name>.theme.qss`` Qt Style Sheet file, or a pair of ``<theme name>-light.theme.qss`` and ``<theme name>-dark.theme.qss`` files that are used when the system colour scheme is light or dark respectively.
