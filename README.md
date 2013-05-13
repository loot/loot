# BOSSv3

## Introduction

See the [BOSS project page](http://code.google.com/p/better-oblivion-sorting-software/) for a general overview of BOSS.

BOSSv3 is being developed on a separate repository from the rest of BOSS's code as it is essentially a new program, and is being designed to address the shortcomings that are inherent in how BOSS approaches load order optimisation.

Modding for Bethesda's games has proved to be incredibly popular, with over 25,000 mods each available for Oblivion and Skyrim.

For BOSS to be as useful as possible, it needs to be able to sort as many mods as possible. For Skyrim, the backlog of mods that need adding to its masterlist is at 5,800+ and growing. Recruiting extra team members isn't a real solution, as I've already done that and frankly copy/pasting filenames into a massive text file is not a whole load of fun and any analysis takes a while to do properly.

The other obvious solution is to cut down on the number of mods that need to be added to the masterlist - instead of having every single mod listed, BOSS could be made to sort the 'simple' mods on its own, and then we could add to the masterlist only those that proved too tricky for it to position itself.

While that's being done, I might as well also make some improvements to other areas of BOSS.


## Idea Notes

It might be a good idea to download masterlists via a diff, or allow compression somehow, as the Skyrim masterlist is 1.7 MB. Also need to figure out how to do version checks, because currently it scans a web page for the version number, which isn't exactly robust or flexible.

Unlike v2, v3 will not have a built-in updater/update checker. It's just too much of a headache to code and manage for the payoff it gives, especially since notifications can be sent to users via the masterlist.

It would be good if Nehrim support could be handled as an offshoot of Oblivion support and the framework generalised so that any TC for any of the supported games could be used.

I'm considering using glog to handle logging, should really get that implemented soon and remove all the messy ofstream debug stuff.


## To Do
    
- [x] Write new masterlist, userlist, settings file parsers.
- [x] Implement per-game handling.
- [x] Write API.
- [x] Tie together automatic 'simple' sorting with masterlist, userlist data parsing and usage.
- [x] Initialisation and finishing routines (read settings file, write log file, etc.).
- [x] Remove game detection override.
- [x] Include libespm initialisation into Game constructor.
- [x] Implement reading of plugin versions from their description field.
- [x] Develop UI - main window.
- [x] Develop UI - settings window.
- [ ] Develop sorting algorithm.
- [ ] Work out how to implement masterlist updating and write the code.
- [x] Develop UI - metadata editor window.
- [x] Develop UI - report viewer.
- [x] Error handling.
- [ ] Checks for cyclic dependencies and incompatibilities.
- [ ] Setting load order.
- [ ] Optimisations to load ordering.
- [ ] Implement logging.
- [x] Add a quick header-only plugin read for use when loading the metadata editor, so that existing Bash Tags can also be displayed.
- [x] Make validity checks non-fatal.
- [ ] Add a massive "RUN THE GAME LAUNCHER IF YOUR GAME IS NOT DETECTED" message somewhere.
- [ ] Generalise Total Conversion support, so that any TC for any of the supported 'base' games can be used with BOSS.
- [x] Write XHTML report generator.
- [ ] Write report CSS.
- [ ] Write report Javascript.
- [ ] Double-check ghosted file support.
- [x] Add a setting that lets users choose whether to use the viewer window or their own browser when viewing BOSS's report.
- [x] Somehow implement filter settings memory for the BOSS report.
- [ ] Implement checking of details part of report against previous report.


## Optimisation

Some ideas that have yet to be tested or implemented:

* If no changes have been made to the installed plugins since previous sorting, then the same order could be applied. This would require CRC checks to make sure plugins haven't been edited though, so it's not clear without profiling whether it would improve or worsen performance.
* If plugins have only been removed since last sort, the same order could be used, except that some conditions might be evaluated differently in their absence, so again this isn't clear-cut.
* If plugins have been added but those plugins contain only new records, then they can be positioned alphabetically at the end without any trouble.
* Masterlist updating using the Subversion API to get only diffs of the file. This would require the masterlists to be always hosted in an SVN repository, which reduces flexibility somewhat, and I've always found SVN to be really slow in practice. If the API isn't too complicated, it might be worth a shot though.
* Masterlist updating could also just use a straightforward HTTP GET request, and if this method is chosen then the Accept-Encoding header should be sent with appropriate values so that the server can use compression when serving the file. Unfortunately, Google Code doesn't seem to use it, though other sites do.


## Install Structure

LOOT will be a self-contained installation that can be dropped anywhere. It will have an installer option that also installs some Start menu shortcuts and a Registry entry, but these will not be required for LOOT to function.

Directory structure will be:

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
