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

# Ubuntu toolchain
install_ubuntu_toolchain=$(confirm "Do you want to install Ubuntu Toolchain PPA?" "n")
if [ "$install_ubuntu_toolchain" = "y" ]; then
	add-apt-repository ppa:ubuntu-toolchain-r/ppa --y
fi

echo 'Update and upgrade'
apt-get -q -y update || { echo "apt update failed"; exit 1; }
apt-get -q -y upgrade || { echo "apt upgrade failed"; exit 1; }
apt-get -q -y autoremove || { echo "apt autoremove failed"; exit 1; }

cd /tmp

# ecasound required packages
apt-get -q -y install nano mc zip unzip \
    curl \
    git \
    make cmake gcc build-essential autoconf \
    mpg123 lame vorbis-tools faad \
    libreadline-dev libncurses-dev libaudiofile-dev \
    libsndfile1-dev libsndfile1 libaudiofile1 libsamplerate-dev liblilv-dev \
    libjack-dev libcurl4-gnutls-dev liblo-dev \
    timidity mikmod \
    libtool pkg-config shtool \
    alsa alsa-base alsa-utils alsa-tools alsa-source libasound2-plugins libasound2-dev \
    ladspa-sdk \
    || { echo "failed to install required packages"; exit 1; }

install_python=$(confirm "Do you want to install python for pyecasound?" "n")
if [ "$install_python" = "y" ]; then
	apt-get -q -y install python \
		|| { echo "failed to install required packages"; exit 1; }
fi

install_ruby=$(confirm "Do you want to install ruby for rubyecasound?" "n")
if [ "$install_ruby" = "y" ]; then
	apt-get -q -y install ruby \
		|| { echo "failed to install required packages"; exit 1; }
fi


# compile libsamplerate from source (not finished)
compile_libsamplerate=$(confirm "Do you want to compile libsamplerate?" "n")
if [ "$compile_libsamplerate" = "y" ]; then
	# dependencies
	apt-get -q -y install libsndfile1 libsndfile1-dev fftw3 fftw3-dev alsa alsa-base alsa-utils alsa-tools alsa-source \
	    || { echo "failed to install required packages"; exit 1; }
fi


# compile liboil from source
complie_liboil=$(confirm "Do you want to compile liboil from source?" "y")
if [ "$complie_liboil" = "y" ]; then
	cd /tmp

	apt-get -q -y install gtk-doc-tools

	wget https://liboil.freedesktop.org/download/liboil-0.3.17.tar.gz

	tar xvzf /tmp/liboil-0.3.17.tar.gz
	cd /tmp/liboil-0.3.17
	sh ./autogen.sh
	./configure
	make -j 8
	make install
fi


# compile and install ecasound
cd /tmp

git clone https://github.com/andrasbari/ecasound.git ecasound

cd ecasound

./autogen-vc.sh
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
cd /tmp/ecasound
./configure --disable-libsamplerate
make -j 8
make install