#!/bin/bash
# This script prepares a fresh install of Ubuntu Server 12.04 so that the steps
# in .travis.yml can then be run.

sudo rm -rf /var/lib/apt/lists/*
sudo apt-get update

sudo apt-get install -y python-software-properties git build-essential libcurl4-openssl-dev

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv 68576280
sudo apt-add-repository -y 'deb https://deb.nodesource.com/node_4.x precise main'
sudo apt-get update
sudo apt-get install -y nodejs
