![image](/resources/background/titlescreen.jpg)

# LianLianKan (连连看)
- A game where you connect stuff together. This goes by a lot of games like: Onet Classic, Animal Connect Game, Kawai,... but it's mostly known as Pikachu
where I live ;)
- The game is actually decades old at this point. It was available in most home computers back in the XP era but I really don't have no clue who installed it.

# Why are you remaking it
- [The original game's website](http://www.llk.cn/) is still available and still hosts download links for the game, including the original 1.0 version
which is the version I'm remaking and is also the version that was on most home computers at that time. However, there's a bug where after the first song
finished playing, the game will just give up and die. The website specifically mentions this bug and a workaround - disabling the music entirely. That's
not a solution I'm looking for however. The cheerful, bright music is arguably the most important to me. And so... I took the liberty of rewriting this
entire game in Qt and C++ just so I can play it in peace.

# Dependencies
- Qt5
- CMake

# Compiling 
- Feel free to open any issues, regardless of it's potential stupidity, because at the end of the day I'm also a dumbass xd
- Please note that this is linking agaisnt shared Qt libraries. If you want static linking, set ``-DCMAKE_PREFIX_PATH`` to where the static library is located

## Windows
- Go to [Qt's website](https://www.qt.io/) and download the online installer
- Install Qt5 (remember to select MinGW as the compiler, or whatever it is just select it alright)
- Open the Qt5 terminal (I'm using Qt 5.15.2 MinGW 8.1.0 64bit for reference) and then:
```sh
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```
- Copy the .exe into a new folder and type:
```sh
windeployqt llk.exe --no-translations --no-system-d3d-compiler
```

## Linux
- Install the Qt5 library using your preferred package manager, and then:
```sh
mkdir build
cd build
cmake ..
make
```

## Android
- Currently busy with school and I don't have much experience with Android, sorry. I'll figure it out one day, it was one of my main goal when starting this project afterall

## iOS
- Give me 99$ and I'll make one

## MacOS
- I don't have a Mac and I don't want to do any Hackintosh shenanigans
- If any of you have a Mac and is willing to compile them for me please do share it (pretty please)

# Credits
- CHEN PROGRAM STUDY for making this absolute legend of a game ;)
- Joe Jenkins for the River Flows in You cover (I know it's not canonical but I like it so why not hehe)
