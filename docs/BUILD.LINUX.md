# Build Instructions using GCC

Linux binaries can be built for LOOT, and these instructions are for doing so on
Ubuntu 12.04, though they may also apply to other versions and
distributions.

## Building

Most of the procedure for building the API, tests and metadata validator can be
found in the `.travis.yml` file, which is the configuration file for LOOT's
Travis CI instance. However, Travis instances have a few more libraries and
utilities by default than Ubuntu 12.04 does, so installation of them will be 
covered here.

Linux builds of the GUI application should be considered officially
**unsupported and unmaintained**, though contributions are welcome.

## Installing Missing Dependencies

### Base Dependencies

```
sudo apt-get install python-software-properties git build-essential libcurl4-openssl-dev
```

### UI Dependencies

```
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv 68576280
sudo apt-add-repository 'deb https://deb.nodesource.com/node_4.x precise main'
sudo apt-get update
sudo apt-get install nodejs
```

## Runtime Differences

Not all LOOT's features have been implemented for Linux builds. Issues labelled
`linux` on LOOT's issue tracker cover such missing features where they can be
implemented. Unavoidable platform differences are documented here:

* On Windows, LOOT can detect game installs using their Registry entries. On
  Linux this is obviously not possible, so either game paths will have to be
  entered manually in LOOT's settings dialog when it is run, or LOOT will need
  to be installed beside a game's Data folder for that game to be detected.
