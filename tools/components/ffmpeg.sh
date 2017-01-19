#!/bin/bash
RED='\033[01;31m'
RESET='\033[0m'
INSTALL_SDIR='/usr/src/ffmpegscript'
INSTALL_DDIR='/usr/local/cpffmpeg'
# # te="[dag] name=Dag RPM Repository for Red Hat Enterprise Linux
# # baseurl=http://apt.sw.be/redhat/el$releasever/en/$basearch/dag
# gpgcheck=1
# enabled=1"
# sudo echo "$te" > /etc/yum.repos.d/dag.repo
export cpu=`cat "/proc/cpuinfo" | grep "processor"|wc -l`
export TMPDIR=$HOME/tmp
_package='FFMPEG'
clear
sleep 2
ffmpeg_source=$_package
ldconfig
echo -e $RED"Installation of $_package ....... started"$RESET
cd $INSTALL_SDIR
echo "Checking for old source"
   git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
   cd ffmpeg/
   ldconfig
   ./configure --prefix=$INSTALL_DDIR --enable-shared --enable-nonfree \
		--enable-gpl --enable-pthreads  --enable-decoder=liba52 \
		--enable-libopencore-amrwb --enable-libmp3lame \
		--enable-libtheora --enable-libvorbis  --enable-libx264  --enable-libxvid \
		--extra-cflags=-I/usr/local/cpffmpeg/include/ --extra-ldflags=-L/usr/local/cpffmpeg/lib \
		 --enable-version3 --extra-version=syslint
   make -j$cpu
   make tools/qt-faststart   
   make install
   cp -vf tools/qt-faststart /usr/local/cpffmpeg/bin/
   ln -sf /usr/local/cpffmpeg/bin/ffmpeg /usr/local/bin/ffmpeg
   ln -sf /usr/local/cpffmpeg/bin/ffmpeg /usr/bin/ffmpeg
   ln -sf /usr/local/cpffmpeg/bin/qt-faststart /usr/local/bin/qt-faststart
   ln -sf /usr/local/cpffmpeg/bin/qt-faststart /usr/bin/qt-faststart
   ldconfig
   /usr/bin/ffmpeg -formats

echo -e $RED"Installation of $_package ....... Completed"$RESET
sleep 2
