.. _themes:

******
Themes
******

Themes can be selected in LOOT's settings dialog. Two themes are provided with LOOT: the default theme and a dark theme.

Additional themes may be installed in the ``resources/themes`` directory, relative to ``LOOT.exe``. A theme can have two files:

* ``<theme name>.theme.qss`` is a Qt Style Sheet file that is required to be present for LOOT to find the theme.
* ``<theme name>.palette.toml`` is a TOML file that can optionally be provided alongside the QSS file to define the theme's color scheme.

Palette TOML
============

If provided, a palette TOML file allows the following `color roles`_ to be defined for the ``active``, ``inactive`` and ``disabled`` color groups:

* window
* windowText
* base
* alternateBase
* toolTipBase
* toolTipText
* placeholderText
* text
* button
* buttonText
* brightText
* light
* midlight
* dark
* mid
* highlight
* highlightedText
* link
* linkVisited

All color roles take a string value that can be in any of the formats documented `here`_.

All of the color groups and roles are optional, but at least the active group's button and window colors should be provided for best results.

Example
-------

::

    [active]
    button = "#424242"
    window = "#212121"
    windowText = "#E0E0E0"
    light = "#303030"
    dark = "#757575"
    mid = "#595959"
    text = "#FFF"
    brightText = "#595959"
    base = "#303030"
    placeholderText = "#B3FFFFFF"
    link = "#2979FF"

    [disabled]
    windowText = "#4DE0E0E0"
    text = "#4DFFFFFF"
    brightText = "#4D595959"
    placeholderText = "#4DFFFFFF"
    link = "#4D2979FF"


.. _color roles: https://doc.qt.io/qt-6/qpalette.html#ColorRole-enum
.. _here: https://doc.qt.io/qt-6/qcolor.html#setNamedColor
