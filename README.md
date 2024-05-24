# Photo Filter

A simple program to get me used to c++ again

the goal here is to open an image file and apply various filters in real time.

# Features

## Implemented

- Image loading

## Not Implemented

- Simple blur
- Gaussian blur
- Dithering
- Brightness
- Contrast
- Sharpening

# Project setup

## Submodules
This project uses submodules so be sure to clone using `--recurse-submodules` or run `git submodule init` and `git submodule update`

## Toolchain
I am developing this is VS 2022 and that is all I will support due to basic nature of this project.

## Graphics API
The main graphical API used for this is [SDL](https://wiki.libsdl.org/SDL3) and it is included as a submodule so make sure you set those up before complaining about this dependency being broken.

I am also using [SDL_Image](https://wiki.libsdl.org/SDL2_image/FrontPage) to load the images from disk