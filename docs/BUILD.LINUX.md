# Build Instructions using GCC

Linux binaries can be built for LOOT, and these instructions are for doing so on
Ubuntu Desktop 14.04, though they may also apply to other versions and
distributions.

Most of the procedure for building the API, tests and metadata validator can be
found in the `.travis.yml` file, which is the configuration file for LOOT's
Travis CI instance. However, Travis instances have a few more libraries and
utilities by default than Ubuntu Desktop 14.04 does, and the procedure does not
cover building the GUI application, so these differences will be covered here.

Linux builds of the GUI application should be considered officially
**unsupported and unmaintained**, though contributions are welcome.

## Installing Missing Dependencies

To install Node.js (for building the HTML UI) and the libcurl and X11
development libraries (for libgit2 and the UI respectively), run the following:

```
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv 68576280
sudo apt-add-repository 'deb https://deb.nodesource.com/node_4.x precise main'
sudo apt-get update
sudo apt-get install nodejs npm libcurl4-openssl-dev libx11-dev
sudo ln -s /usr/bin/nodejs /usr/bin/node
sudo npm install -g bower vulcanize@0.7.11
```

If you wish to build the GUI application, you must also manually download the
Chromium Embedded Framework binary archive, then extract it and run

```
mkdir build && cd build
cmake ..
make
```

from the extracted folder.

## Runtime Differences

Not all LOOT's features have been implemented for Linux builds. Issues labelled
`linux` on LOOT's issue tracker cover such missing features where they can be
implemented. Unavoidable platform differences are documented here:

* On Windows, LOOT can detect game installs using their Registry entries. On
  Linux this is obviously not possible, so either game paths will have to be
  entered manually in LOOT's settings dialog when it is run, or LOOT will need
  to be installed beside a game's Data folder for that game to be detected.
