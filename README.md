# rAVen-made-easC

This branch is the dynamic build version of rAVen [here](https://github.com/nots1dd/raven/blob/main/readme.md)

Check out the README there to further understand about the maths behind this AV, and more!

## EASC 

This build is possible thanks to [easC](https://github.com/nots1dd/easc) that provides with a boilerplate C project to build any project in a more dynamic fashion.

Who is this build meant for:

-> Developers / Potential contributors who want to test features on the fly within one music session of rAVen 

-> Anyone who is interested in learning how this framework is made or how it works

## Build 

```sh 
git clone -b rAVen-made-easC https://github.com/nots1dd/raven.git --single-branch
```

Check out [USAGE](https://github.com/nots1dd/blob/rAVen-made-easC?tab=readme-ov-file#usage) header in this README for information on dynamic and static building using easC

## Overview
This is a basic C framework built with dynamic library support using `dlopen`, `dlsym`, and `dlclose`. 
It supports hot-reloading, allowing library code to be updated without restarting the program.

## File structure 


```bash
rAVenDync/
├── .config/easC/
│    └── config.json
├── src/
│   └── main.c
├── lib/
│   ├── libraven.c
│   ├── libraven.h
│   └── recompile.sh
├── build/
├── init.sh
├── staticompile.sh
├── Makefile      [OPTIONAL]
├── .clang-format [OPTIONAL]
└── README.md
```

## Features
- Dynamic function loading and reloading.
- Easy recompile script for library changes.
- Optional .clang-format for code styling.
- Makefile for streamlined builds.

## Usage
To compile the project [DYNAMIC BUILD]:
```bash
./init.sh
```

To compile the project [STATIC BUILD]:
```bash 
./staticompile.sh
```

To run the project:
```bash
cd build && ./$output_binary
```

To reload the library at runtime, press 'r'. Press 'q' to quit.

**This goes without saying but, NO NEED TO KILL THE APPLICATION TO RELOAD CHANGES!!**

> [!NOTE]
> 
> Before attempting to hot reload, 
> 
> You need to do the following in order for the main event loop to grab the latest functions:
>
> ```sh 
> cd lib/
> ./recompile.sh
> ```
> If recompile.sh is successful, you should be able to hot reload with the new changes in the event loop 

## Requirements
- gcc/clang
- make (optional)
- clang-format (optional)

