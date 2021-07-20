# BetterChess

## Special Thanks
Thank you to Github User [nagayev](https://github.com/nagayev) for assisting with the build instructions regarding CMAKE as well as some refactoring.

## About

This is a chess program that I created in the spring of 2021. I have attempted to make chess many times, and this is my final version (at least for now as of june 2021).

You can watch the video documenting the creation here: https://youtu.be/kwQk5A26cis

To make this be able to run, you will need to link both the SDL library, and the SDL_Image library. Eventually I will make this into a standalone download, but I believe that I will wait
until I implement an actual artificial intelligence to play against.

## Building

This project use CMake for building.  
You need to install cmake and sdl with sdl-image.  
Then type `cmake .` and you will get .vsproj file/Makefile or XCode project (depending on your OS)  
