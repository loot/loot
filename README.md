# BOSSv3

## Introduction

See the [BOSS project page](http://code.google.com/p/better-oblivion-sorting-software/) for a general overview of BOSS.

BOSSv3 is being developed on a separate repository from the rest of BOSS's code as it is essentially a new program, and is being designed to address the shortcomings that are inherent in how BOSS approaches load order optimisation.

Modding for Bethesda's games has proved to be incredibly popular, with over 25,000 mods each available for Oblivion and Skyrim.

For BOSS to be as useful as possible, it needs to be able to sort as many mods as possible. For Skyrim, the backlog of mods that need adding to its masterlist is at 5,800+ and growing. Recruiting extra team members isn't a real solution, as I've already done that and frankly copy/pasting filenames into a massive text file is not a whole load of fun and any analysis takes a while to do properly.

The other obvious solution is to cut down on the number of mods that need to be added to the masterlist - instead of having every single mod listed, BOSS could be made to sort the 'simple' mods on its own, and then we could add to the masterlist only those that proved too tricky for it to position itself.

While that's being done, I might as well also make some improvements to other areas of BOSS.


## Install Structure

LOOT will be a self-contained installation that can be dropped anywhere. It will have an installer option that also installs some Start menu shortcuts and a Registry entry, but these will not be required for LOOT to function.

Directory structure will be:

```
/
    BOSS.exe
    resource/
        l10n/
            ...translation files...
        settings.yaml
        libespm.yaml
        ...CSS and JS files...
    docs/
        images/
            ...readme images...
        BOSS Readme.html
        ...other docs...
    Oblivion/
        masterlist.yaml
        userlist.yaml
        report.html
    ...other game folders with same structure as Oblivion...
```


## Required Libraries

BOSS uses the following libraries:

* Alphanum
* Boost
* Libespm
* Libloadorder
* PugiXML
* wxWidgets
* yaml-cpp

Also uses [polyfill.js](https://github.com/inexorabletash/polyfill/blob/master/polyfill.js), [storage.js](https://github.com/inexorabletash/polyfill/blob/master/storage.js) and [DOM-shim](https://github.com/Raynos/DOM-shim/) for Internet Explorer 8 compatibility.


## Misc

Here is the full list of BOSS members at the end of 2012. Any of these who lose
their membership status should still be credited in the BOSS readme:

Random007, Arthmoor, WrinklyNinja, PacificMorrowind, aellis, Vacuity, Gabba,
ZiggyX200, RiddlingLynx, AliTheLord, Tokc.D.K., Valda, Space Oden69, Televator,
Leandro Conde, Psymon, Loucifer, Torrello, Malonn, Skyline, Sharlikran, Red Eye,
iyumichan, Peste, Calen Ellefson, SilentSpike, Arkangel, zyp, v111, Chevenga,
rowynyew
