#!/bin/bash

set -e

if ! [ $(id -u) = 0 ]; then
   echo "This script must be run as root" 
   exit 1
fi

apt-get -q -y install nano mc zip unzip \
    curl \
    git \
    make cmake gcc build-essential \
    alsa alsa-base alsa-utils alsa-tools alsa-source libasound2-plugins libasound2-dev \
    ecasound \
    cmt swh-plugins tap-plugins \
    ladspa-sdk