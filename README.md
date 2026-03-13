# strata
Synthesis Toolkit for Real-time Audio Tactile Applications

by Stratos Bichakis

developed with the support of UdK Wearable Computing team, Prof. Berit Greinke - ECDF

STRATA is a nested portable audio synthesis architecture. The system aims for modularity, providing a nested layered structure and portability, leveraging the benefits of using cmake and cross-compilation.

1. [BELA][1]
2. [BELA Trill][2]
3. [Synthesis Toolkit][3]
4. [RtAudio library for portable audio][4]
5. [Embedded Perl for generating SKINI commands][5]


[1]:https://bela.io/learn/
[2]:https://bela.io/products/trill/
[3]:https://ccrma.stanford.edu/software/stk/index.html
[4]:https://github.com/thestk/rtaudio
[5]:https://perldoc.perl.org/perlembed



# Getting started
### 0. Connect the Bela board over usb
### 1. build bela libraries first by remote connecting to bela 
type in a terminal
```shell
ssh root@bela.local 

```
```shell
cd Bela
make lib
```

<kbd>Ctrl</kbd> + <kbd>D</kbd> to close the connection

### 2. copy folders from Bela to a newly created folder named sysroot,
inside your local strata repository folder.
``` shell
./sync-sysroot.sh
```
500MB transfer takes a good while (~4 minutes)

### 3. install dependencies for OS X

```shell
brew install llvm lld cmake Ninja wget
```

### 4. compile stk using cmake
still strata folder
```shell
mkdir build && cd build
cmake -G Ninja ..
cmake --build . -j 8 --target stk && scp external/libstk.so root@bela.local:/usr/lib

```

### 5. copy stk rawwaves to bela

```shell
scp -r ../external/stk/rawwaves root@bela.local:.
```

### 6. compile one example
```shell
cmake --build . -j 8 --target rtsine && scp examples/minimal/rtsine root@bela.local:Bela/projects
```

### 7. try the sine example on Bela board
```shell
ssh root@bela.local 
./Bela/projects/rtsine
```


### 10. Build all examples
```shell
cmake --build .
```