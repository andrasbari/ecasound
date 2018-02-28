#!/bin/bash

set -e

confirm() {
    default_choice=""
    choices="[y,n]"

    if [ "$2" = "y" ]; then
        default_choice="y"
        choices="[Y,n]"
    elif [ "$2" = "n" ]; then
        default_choice="n"
        choices="[y,N]"
    fi

    read -r -p "${1:-Are you sure?} $choices " choice
    case "$choice" in
        [yY][eE][sS]|[yY]) 
            echo "y"
            ;;
        [nN][oO]|[nN]) 
            echo "n"
            ;;
        *)
            if [ -z "$default_choice" ]; then
                confirm "$1" "$2"
            else
                echo "$default_choice"
            fi
            ;;
    esac
}

if ! [ $(id -u) = 0 ]; then
    echo "This script must be run as root" 
    exit 1
fi

echo 'Update and upgrade'
apt-get -q -y update || { echo "apt update failed"; exit 1; }
apt-get -q -y upgrade || { echo "apt upgrade failed"; exit 1; }
apt-get -q -y autoremove || { echo "apt autoremove failed"; exit 1; }

cd /tmp

# Ubuntu toolchain
#add-apt-repository ppa:ubuntu-toolchain-r/ppa --y
#apt-get -q -y update

# ecasound required packages
apt-get -q -y install nano mc zip unzip \
    curl \
    git \
    make cmake gcc build-essential autoconf libreadline-dev libncurses-dev \
    mpg123 lame vorbis-tools faad \
    libsndfile1-dev libsndfile1 libaudiofile1 libsamplerate-dev python liblilv-dev \
    libjack-dev libcurl4-gnutls-dev liblo-dev gtk-doc \
    timidity mikmod \
    libtool pkg-config shtool \
    alsa alsa-base alsa-utils alsa-tools alsa-source libasound2-plugins libasound2-dev \
    ladspa-sdk \
    || { echo "failed to install required packages"; exit 1; }

# mpd required packages
apt-get -q -y install \
    build-essential gcc g++ clang automake libtool flex bison gdb \
    libboost-system-dev libboost-filesystem-dev libboost-thread-dev \
    pkg-config \
    libmad0-dev libmpg123-dev libid3tag0-dev \
    libflac-dev libvorbis-dev libopus-dev \
    libadplug-dev libaudiofile-dev libsndfile1-dev libfaad-dev \
    libfluidsynth-dev libgme-dev libmikmod2-dev libmodplug-dev \
    libmpcdec-dev libwavpack-dev libwildmidi-dev \
    libsidplay2-dev libsidutils-dev libresid-builder-dev \
    libavcodec-dev libavformat-dev \
    libmp3lame-dev libtwolame-dev libshine-dev \
    libsamplerate0-dev libsoxr-dev \
    libbz2-dev libcdio-paranoia-dev libiso9660-dev libmms-dev \
    libzzip-dev \
    libcurl4-gnutls-dev libyajl-dev libexpat-dev \
    libasound2-dev libao-dev libjack-jackd2-dev libopenal-dev \
    libpulse-dev libroar-dev libshout3-dev \
    libsndio-dev \
    libmpdclient-dev \
    libnfs-dev libsmbclient-dev \
    libupnp-dev \
    libavahi-client-dev \
    libsqlite3-dev \
    libsystemd-dev libwrap0-dev \
    libcppunit-dev xmlto \
    libboost-dev \
    libicu-dev \
    || { echo "failed to install required packages"; exit 1; }



# install liboil and dependecies
complie_liboil=$(confirm "Do you want to compile liboil from source?" "n")
if [ "$complie_liboil" = "y" ]; then
	cd /tmp

	apt-get install gtk-doc-tools

	wget https://liboil.freedesktop.org/download/liboil-0.3.17.tar.gz

	tar xvzf /tmp/liboil-0.3.17.tar.gz
	cd /tmp/liboil-0.3.17
	sh ./autogen.sh
	./configure
	make -j 8
	make install
fi


# install arts and dependecies (not finished!)
complie_arts=$(confirm "Do you want to compile arts from source?" "n")
if [ "$complie_arts" = "y" ]; then
	cd /tmp
	wget http://mirror.git.trinitydesktop.org/cgit/arts/snapshot/arts-r14.0.4.tar.gz

	tar xvzf /tmp/arts-r14.0.4.tar.gz
	cd /tmp/arts-r14.0.4
fi




# install ecasound
cd /tmp

git clone https://github.com/andrasbari/ecasound.git ecasound

cd ecasound

./autogen-vc.sh
# due to compile error, resample_secret_rabbit_code disabled
#samplebuffer.cpp: In member function ‘void SAMPLE_BUFFER::resample_secret_rabbit_code(SAMPLE_SPECS::sample_rate_t, SAMPLE_SPECS::sample_rate_t)’:
#samplebuffer.cpp:1381:20: error: cannot convert ‘SAMPLE_BUFFER::sample_t* {aka double*}’ to ‘float*’ in assignment
#     params.data_in = impl_repp->old_buffer_repp; 
#                    ^
#samplebuffer.cpp:1387:21: error: cannot convert ‘double*’ to ‘float*’ in assignment
#     params.data_out = buffer[c];
#                     ^
#samplebuffer.cpp: In function ‘void priv_alloc_sample_buf(SAMPLE_SPECS::sample_t**, size_t)’:
./configure --disable-libsamplerate
make -j 8
make install