# rAVen

> [!NOTE]
> rAVen is currently a **Work in Progress (WIP)**.

### Just making AUDIO look and feel cool

---

## Table of Contents
1. [Dependencies](#deps)
2. [C Libraries Used](#c-libs-used)
3. [Understanding FFT](#understanding-fft)
4. [Building rAVen](#building)
   - [Using Build Script](#using-build-script)
   - [Using Makefile](#using-makefile)
   - [Using CMake](#using-cmake)
5. [FAQ](#faq)
6. [License](#license)

---

## <a id="deps"></a>Deps

- **Clang / GCC**
- **Bash**
- [C libs](#c-libs-used)

---

## <a id="c-libs-used"></a>C Libraries Used

- **raylib**: For rendering and visualizing audio
- **math, complex, assert**: For FFT (Fast Fourier Transform)
- **libavformat**: For metadata extraction
- Other standard C libraries

---

## <a id="understanding-fft"></a>Understanding FFT

Fast Fourier Transform (FFT) is a powerful algorithm that blends math and algorithms beautifully.

To dive deeper, check out the comments in the [FFT.c](https://github.com/nots1dd/rAVen/blob/main/fft.c) file to understand how FFT is used to analyze audio samples.

---

## <a id="building"></a>Building rAVen

> [!WARNING]
> The build script is currently unavailable as it's undergoing changes.
> For now, use the **Makefile**.

> [!IMPORTANT]
> 
> DYNAMIC BUILD for rAVen is on another branch
> 
> rAVen-made-easC is made possible by [easC](https://github.com/nots1dd/easc) framework:
> An effective lightweight framework that allows for dynamic build and handling of any C project
> 
> Check out the branch [here](https://github.com/nots1dd/rAVen/blob/rAVen-made-easC/readme.md)
> 

### <a id="using-build-script"></a>Using the Build Script:
```bash
chmod +x build.sh
./build.sh
./raven samples/sample-15s.wav
```

### <a id="using-makefile"></a>Using Makefile:
```bash
make
./raven
```

### <a id="using-cmake"></a>Using CMake:
```bash
cmake -S . -B build/
cmake --build build/
./build/raven samples/sample-15s.wav
```

---

## <a id="faq"></a>FAQ

### 1. How does the math work?

Check out the [fft.c](https://github.com/nots1dd/raven/blob/main/fft.c) file to understand how **Fast Fourier Transform** is used to analyze audio samples.

---

### 2. What audio formats are supported?

rAVen supports all formats supported by [RAYLIB](https://github.com/raysan5/raylib). 

> [!NOTE]
> Although RAYLIB supports `.flac`, it's **not enabled by default**. To enable `.flac` support:
>
> - Go to `src/config.h` in raylib and uncomment the following line:
>   ```c
>   //#define SUPPORT_FILEFORMAT_FLAC 1
>   ```
> Then follow the raylib documentation for building raylib with this change.

### 3. What color pallette does rAVen follow?

rAVen uses [GRUVBOX](https://github.com/morhetz/gruvbox) color schema
This color schema is something I use daily and I have come to really like it 

There may be a time in the future of rAVen where I may make the theme **customizable** but for now, hope you like GRUVBOX :)

---

### 3. Will rAVen integrate with audio services like PipeWire, ALSA, or PulseAudio?

rAVen aims to support these services eventually. The first priority will be **PipeWire**, with plans to explore **ALSA** and **PulseAudio** integration in the future.

---

## <a id="license"></a>License

rAVen is free and open-source software, licensed under the **GNU Lesser General Public License**. See the full [LICENSE](https://github.com/nots1dd/raven/blob/main/LICENSE) for more details.

---

### Screenshots

![raven1](https://github.com/user-attachments/assets/40ab07df-4f43-406d-b30f-77b133780d12)
![raven2](https://github.com/user-attachments/assets/3aa6de17-62b4-4f64-9b35-31ec6d0fbb5b)
![raven3](https://github.com/user-attachments/assets/51a291b7-12d6-41b1-af3d-52759791a093)

---
