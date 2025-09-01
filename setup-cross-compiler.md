# Cross compiling 

``` shell
rsync -rzLR --safe-links \
      root@bela.local:/usr/lib/arm-linux-gnueabihf \
      root@bela.local:/usr/lib/gcc/arm-linux-gnueabihf \
      root@bela.local:/usr/include \
      root@bela.local:/lib/arm-linux-gnueabihf \
      sysroot/ 
```
500MB transfer takes a good while (~4 minutes)

```shell
brew install llvm  

```
```
`brew --prefix llvm`/bin/clang++ \
    --target=arm-linux-gnueabihf \
    --sysroot=/Users/stratosbichakis/workspace/ongoing/BELA/cross-compiling/sysroot \
    -isysroot=/Users/stratosbichakis/workspace/ongoing/BELA/cross-compiling/sysroot/sysroot \
    -isystem=/Users/stratosbichakis/workspace/ongoing/BELA/cross-compiling/sysroot/sysroot/usr/include/c++/6.3.0 \
    -isystem=/Users/stratosbichakis/workspace/ongoing/BELA/cross-compiling/sysroot/sysroot/usr/include/carm-linux-gnueabihf/c++/6.3.0 \
    -L/Users/stratosbichakis/workspace/ongoing/BELA/cross-compiling/sysroot/sysroot/usr/lib/gcc/arm-linux-gnueabihf/6.3.0 \
    -B/Users/stratosbichakis/workspace/ongoing/BELA/cross-compiling/sysroot/sysroot/usr/lib/gcc/arm-linux-gnueabihf/6.3.0 \
    --gcc-toolchain=`brew --prefix arm-linux-gnueabihf-binutils` \
    -o testcross \
    testcross.cpp
```

need to make bela lib first - in bela



- need to create symbolic link for perl in  
sysroot/usr/lib/arm-linux-gnueabihf 
```
ln -s sysroot/usr/lib/arm-linux-gnueabihf/libperl.so.5.24 sysroot/usr/lib/arm-linux-gnueabihf/libperl.so
```

- need to copy files from bela sysroot to 
```
/usr/local/linaro
```

- build and copy stk library from cmake-build folder

```shell
cmake --build . --target stk && scp external/libstk.so root@bela.local:/usr/lib
```

- copy stk rawwaves to bela
```shell
scp -r ../external/stk/rawwaves root@bela.local:.
```

