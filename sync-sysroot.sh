#!/usr/bin/env sh
BELA_IP=192.168.7.2

rsync -rvzLR --safe-links \
      root@$BELA_IP:/usr/lib/arm-linux-gnueabihf \
      root@$BELA_IP:/usr/lib/gcc/arm-linux-gnueabihf \
      root@$BELA_IP:/usr/include \
      root@$BELA_IP:/lib/arm-linux-gnueabihf \
      sysroot/

ln -sif $(pwd)/sysroot/usr/lib/arm-linux-gnueabihf/libperl.so.5.24 sysroot/usr/lib/arm-linux-gnueabihf/libperl.so

mkdir -p sysroot/usr/xenomai/include
mkdir -p sysroot/root/Bela/include
mkdir -p sysroot/usr/include/alsa
mkdir -p sysroot/usr/local/include

mkdir -p sysroot/root/Bela/lib
mkdir -p sysroot/usr/xenomai/lib
mkdir -p sysroot/usr/local/lib
mkdir -p sysroot/usr/lib/arm-linux-gnueabihf/

rsync -avz root@$BELA_IP:/usr/xenomai/include sysroot/usr/xenomai
rsync -avz root@$BELA_IP:/usr/include/alsa sysroot/usr/include
rsync -avz root@$BELA_IP:/root/Bela/include sysroot/root/Bela
rsync -avz root@$BELA_IP:/root/Bela/build/pru/pru_rtaudio_irq_bin.h sysroot/root/Bela/include
rsync -avz root@$BELA_IP:/root/Bela/build/pru/pru_rtaudio_bin.h sysroot/root/Bela/include
rsync -avz root@$BELA_IP:/usr/local/include/prussdrv.h sysroot/usr/local/include
rsync -avz root@$BELA_IP:/usr/local/include/seasocks sysroot/usr/local/include


rsync -avz root@$BELA_IP:/root/Bela/lib sysroot/root/Bela
rsync -avz root@$BELA_IP:/usr/xenomai/lib sysroot/usr/xenomai

rsync -avz root@$BELA_IP:/usr/local/lib/libpd.* sysroot/usr/local/lib
rsync -avz root@$BELA_IP:/usr/local/lib/libseasocks.* sysroot/usr/local/lib
rsync -avz root@$BELA_IP:/usr/local/lib/libprussdrv.* sysroot/usr/local/lib

rsync -avz root@$BELA_IP:/usr/lib/arm-linux-gnueabihf/libsndfile.* sysroot/usr/lib/arm-linux-gnueabihf/
rsync -avz root@$BELA_IP:/usr/lib/arm-linux-gnueabihf/libasound.* sysroot/usr/lib/arm-linux-gnueabihf/

rsync -avz root@$BELA_IP:/usr/include/ne10 sysroot/usr/include
rsync -avz root@$BELA_IP:/usr/include/math_neon.h sysroot/usr/include

rsync -avz root@$BELA_IP:/usr/lib/libNE10.* sysroot/usr/lib
rsync -avz root@$BELA_IP:/usr/lib/libmathneon.* sysroot/usr/lib

rsync -avz root@$BELA_IP:/usr/local/include/libpd sysroot/usr/local/include
rsync -avz root@$BELA_IP:/usr/local/lib/libpd.so* sysroot/usr/local/lib
