#!/bin/bash
RED='\033[01;31m'
RESET='\033[0m'
INSTALL_SDIR='/usr/src/ffmpegscript'
SOURCE_URL='http://mirror.ffmpeginstaller.com/source'
INSTALL_DDIR='/usr/local/cpffmpeg'
export cpu=`cat "/proc/cpuinfo" | grep "processor"|wc -l`
export TMPDIR=$HOME/tmp
_package=' '
clear
sleep 2
echo -e $RED"Installation of $_package ....... started"$RESET


make -j$cpu
make install

echo -e $RED"Installation of $_package ....... Completed"$RESET
sleep 2
