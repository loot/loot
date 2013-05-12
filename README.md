# BOSSv3

## Introduction

See the [BOSS project page](http://code.google.com/p/better-oblivion-sorting-software/) for a general overview of BOSS.

BOSSv3 is being developed on a separate repository from the rest of BOSS's code as it is essentially a new program, and is being designed to address the shortcomings that are inherent in how BOSS approaches load order optimisation.

Modding for Bethesda's games has proved to be incredibly popular, with over 25,000 mods each available for Oblivion and Skyrim.

For BOSS to be as useful as possible, it needs to be able to sort as many mods as possible. For Skyrim, the backlog of mods that need adding to its masterlist is at 5,800+ and growing. Recruiting extra team members isn't a real solution, as I've already done that and frankly copy/pasting filenames into a massive text file is not a whole load of fun and any analysis takes a while to do properly.

The other obvious solution is to cut down on the number of mods that need to be added to the masterlist - instead of having every single mod listed, BOSS could be made to sort the 'simple' mods on its own, and then we could add to the masterlist only those that proved too tricky for it to position itself.

While that's being done, I might as well also make some improvements to other areas of BOSS.


## Design Notes

BOSS should be able to identify a reasonably correct load order for a group of arbitrary plugins based on their contents. However, there are always some plugins that must be positioned in certain ways relative to other plugins due to author intent that cannot be determined from their contents. There is also generally an element of user preference in setting a load order. BOSS should also be able to provide mechanisms for dealing with such plugins and preferences in addition to its basic 'reasonably correct' sorting.

BOSSv3's masterlist will therefore be used to provide unordered data sets for plugins that require them, and the userlist used to allow users to override the automatic sorting and to provide additional metadata themselves. They will be quite different to the current masterlist and userlist, and will use a new language/format.

If the 'impact' of a plugin is defined as the number of its edits that get applied to a person's game divided by the total number of edits it makes, then an 'optimum' load order is one that maximises the average impact of its constituent plugins whilst satisfying all explicit dependencies. A 'correct' load order is not necessarily an optimum load order though, as some plugins may be made with the intent that they are overridden by other plugins.

For flexibility, the 'masterlist' data file that gets updated by the BOSS Team should have its online location as a configurable option within BOSS, so that should its location change for whatever reason, users would be able to redirect their copy of BOSS to look in the new location. It might be a good idea to download it via a diff, or allow compression somehow, as the Skyrim masterlist is 1.7 MB. Also need to figure out how to do version checks, because currently it scans a web page for the version number, which isn't exactly robust or flexible.

BOSSv3 will also have an API for accessing BOSS data and functionality. The more general functionality found in v2's API has already been forked to libloadorder, which v3 shall make use of.

Unlike v2, v3 will not have a built-in updater/update checker. It's just too much of a headache to code and manage for the payoff it gives, especially since notifications can be sent to users via the masterlist.

When run, BOSS shall output its results to a report file, which shall then be interpreted by the UI to display the BOSS Log.

BOSSv3 won't have a command line interface, to simplify things. It also allows BOSSv3 to have a greater focus on users making sure their load order is correct, which is something that is sadly lacking from most users at the moment. Most users don't look any further than assuming BOSS has set their order correctly, which is unfortunate. 


## Roadmap/To Do
    
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
- [ ] Develop UI - report viewer.
- [ ] Error handling.
- [ ] Checks for cyclic dependencies and incompatibilities.
- [ ] Setting load order.
- [ ] Optimisations to load ordering.
- [ ] Implement logging.
- [ ] Add a quick header-only plugin read for use when loading the metadata editor, so that existing Bash Tags can also be displayed.
- [ ] Add checks for "Deactivate" tag compliance?
- [ ] Make validity checks non-fatal.
- [ ] Add a massive "RUN THE GAME LAUNCHER IF YOUR GAME IS NOT DETECTED" message somewhere.
- [ ] Generalise Total Conversion support, so that any TC for any of the supported 'base' games can be used with BOSS.
- [ ] Write XHTML report generator.
- [ ] Write report CSS.
- [ ] Write report Javascript.


## BOSS Report

Once BOSS has applied a load order, it will display a report, similar to the BOSS Log, that covers the following:

* The number of messages, broken down into the total, the number of warnings and the number of errors.
* The masterlist version used.
* Any parser or validity checker errors encountered.
* If there were any changes since BOSS was last run.
* The new load order.
* The messages attached to plugins, version numbers read, CRCs calculated and whether or not each plugin is active.

The current BOSS Log has a number of useful filters. Depending on the UI used for the report, some or all of these filters may be useful for implementation:

* Hide versions
* Hide whether a plugin is active or not.
* Hide CRCs
* Hide "say" messages.
* Hide Bash Tag suggestions.
* Hide all messages.
* Hide "do not clean" messages.
* Hide inactive plugins.
* Hide messageless plugins.

The report data will be stored in a YAML file, which could use an extended form of the metadata file syntax, with the following additions:

* Plugins get 'version' (string) and 'crc' (int) nodes added to them.
* A new top-level node that holds a 'masterlist version' (string), a 'masterlist updated' (boolean) and a 'report changed' (boolean) node.

There are a whole slew of options when it comes to how the report UI will be implemented. Using a native interface isn't a realistic option because it's so difficult to get anything that isn't very limited implemented, and the report is mostly static so wouldn't take advantage of native controls anyway. A HTML/CSS/JS approach is therefore preferable. The neatest way to do this would be to write the HTML as XHTML using PugiXML, then include external CSS & JS that gets distributed with BOSS.


## Optimisation

Some ideas that have yet to be tested or implemented:

* If no changes have been made to the installed plugins since previous sorting, then the same order could be applied. This would require CRC checks to make sure plugins haven't been edited though, so it's not clear without profiling whether it would improve or worsen performance.
* If plugins have only been removed since last sort, the same order could be used, except that some conditions might be evaluated differently in their absence, so again this isn't clear-cut.
* If plugins have been added but those plugins contain only new records, then they can be positioned alphabetically at the end without any trouble.
* Masterlist updating using the Subversion API to get only diffs of the file. This would require the masterlists to be always hosted in an SVN repository, which reduces flexibility somewhat, and I've always found SVN to be really slow in practice. If the API isn't too complicated, it might be worth a shot though.
* Masterlist updating could also just use a straightforward HTTP GET request, and if this method is chosen then the Accept-Encoding header should be sent with appropriate values so that the server can use compression when serving the file. Unfortunately, Google Code doesn't seem to use it, though other sites do.


## Data Files Format

BOSSv3's settings file, masterlist, userlist and previous run logs will all be written in YAML. This allows me to take advantage of existing parsing libraries and the format's flexibility. It's also not that much more verbose than v2's masterlist format, once data structure changes are taken into account.

A custom parser is required for the evaluation of conditions though. The syntax has been made more human readable and compound conditions now evaluate according to standard logic.

The masterlist and userlist will use the same parser, and simply combined by merging or replacing plugin metadata where depending on the type of metadata.

Further details can be found in the 'docs/BOSS Metadata File Syntax.html' file. An example settings file can be found at 'examples/settings.yaml'.


## Install Structure

LOOT will be a self-contained installation that can be dropped anywhere. It will have an installer option that also installs some Start menu shortcuts and a Registry entry, but these will not be required for LOOT to function.

The directory structure will be identical to that which BOSS currently has, with the exception that the text and ini files will be replaced by YAML files.


## Required Libraries

BOSS uses the following libraries:

* Alphanum
* Boost
* Libespm
* Libloadorder
* wxWidgets
* yaml-cpp


## Misc

Here is the full list of BOSS members at the end of 2012. Any of these who lose
their membership status should still be credited in the BOSS readme:

Random007, Arthmoor, WrinklyNinja, PacificMorrowind, aellis, Vacuity, Gabba,
ZiggyX200, RiddlingLynx, AliTheLord, Tokc.D.K., Valda, Space Oden69, Televator,
Leandro Conde, Psymon, Loucifer, Torrello, Malonn, Skyline, Sharlikran, Red Eye,
iyumichan, Peste, Calen Ellefson, SilentSpike, Arkangel, zyp, v111, Chevenga,
rowynyew
