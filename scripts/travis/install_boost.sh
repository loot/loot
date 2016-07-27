#!/bin/bash

BOOST_BASENAME=boost_1_61_0
BOOST_LIBRARIES=(libboost_atomic.a libboost_chrono.a libboost_date_time.a libboost_filesystem.a libboost_iostreams.a libboost_locale.a libboost_log.a libboost_log_setup.a libboost_regex.a libboost_system.a libboost_thread.a)

function isLibraryMissing {
  for LIBRARY in $BOOST_LIBRARIES; do
    if [[ ! -e ~/$BOOST_BASENAME/stage/lib/$LIBRARY ]]; then
      return 0
    fi
  done

  return 1
}

if isLibraryMissing; then
  cd ~

  wget https://downloads.sourceforge.net/project/boost/boost/1.61.0/${BOOST_BASENAME}.tar.bz2
  tar xf ${BOOST_BASENAME}.tar.bz2

  cd $BOOST_BASENAME
  ./bootstrap.sh
  ./b2 toolset=gcc-5 link=static variant=release address-model=64 cxxflags="-std=c++14 -fPIC" boost.locale.icu=off --with-log --with-date_time --with-thread --with-filesystem --with-locale --with-regex --with-system --with-iostreams
fi
