# rAVen 

> [!NOTE]
> 
> rAVen is currently a WIP
> 

Just making songs look cool

## Deps

clang (or any C compiler really)

bash

## C libs used

-> raylib

-> math, complex, assert for FFT

-> libavformat for metadata extraction

Other standard libs

## UNDERSTANDING FFT

Fast Fourier Transform is very cool and blends both maths and algorithms pretty well

Check the comments in the [FFT](https://github.com/nots1dd/rAVen/blob/main/fft.c) C file

## Building

> [!WARNING]
> 
> Build Script is not available right now
> 
> It is going under some changes
> 
> Use Makefile instead
> 

Using the build script:

```sh 
chmod +x build.sh 
./build.sh
./raven
```

Using Makefile:

```sh
make
./raven
```

Using CMake:

```sh 
cmake -S . -B build/
cmake --build build/ 
./build/raven 
```

![raven1](https://github.com/user-attachments/assets/40ab07df-4f43-406d-b30f-77b133780d12)
![raven2](https://github.com/user-attachments/assets/3aa6de17-62b4-4f64-9b35-31ec6d0fbb5b)
![raven3](https://github.com/user-attachments/assets/51a291b7-12d6-41b1-af3d-52759791a093)



rAVen is free and open source software licensed under GNU Lesser General Public License [LICENSE](https://github.com/nots1dd/raven/blob/main/LICENSE)
