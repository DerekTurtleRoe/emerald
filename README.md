# Emerald
A fork of Amethyst v4, which was a a lightweight source code editor with tree-view navigation. This fork is to make it compile on the supported operating systems and also add some fixes and improvements.

## Features

- Simple and easy to use
- Config files are simple .bml files
- Tree-view on the left, editor on the right
- Free and open-source via the ISC license
- Cross-platform on Windows, Mac, Linux, and BSD

## Compiling

### Windows
If you are on Windows 10 or 11, download and install [MSYS2](https://www.msys2.org/). Then, follow these instructions:

1. Open an MSYS2 MSYS window and run:

`pacman -Syu`

2. Download and install all dependancies for the GTK3 Windows build:

```
pacman -S --needed base-devel mingw-w64-x86_64-toolchain mingw-w64-x86_64-binutils mingw-w64-x86_64-gtk3 mingw-w64-x86_64-gtksourceview3
```
Then run: `make hiro=gtk3`

### Linux
TODO

### BSD
TODO

### macOS
TODO
    
## FAQ

#### What is this?
As it says above, this is a fork of Amethyst v4, which was a very simple text editor for Windows, Mac, Linux, and BSD by the late Near.

#### What can it do?
It can edit text...and that is about it. It has a tree-view on the left and the editor on the right.

#### Why make the fork?

A few reasons.

First, just because I wanted to practice and challenge my skills.

Secondly, the beauty is in the simplicity of the application, but I had issues getting it to run on nearly every platform for several reasons. My goals are to get it working on every platform it orginally ran on, make compiled binaries for them, and then release it. I also kind of want to try using it as a daily driver for a bit and see how it goes.

## Dependancies
- GTK2 and/or GTK3
- gtksourceview
- nall and hiro (these are included in the repo, and are modified from the original source code to build the GTK3 version on modern Windows, these were originally git submodules)

## Authors

- The late Near, rest in peace

## Acknowledgements

 - SuperMikeMan100 on Discord, who helped greatly with fixing the Windows version
 - Luke Usher, who provided screenshots and some information
 - The ares Discord, for putting up with my persistence and annoying questions
 - An anonymous user on the internet who had some backed up archives of the source code and some binaries which I downloaded years ago, however I haven't been able to find said links again. But if that user ever reads this, thank you for backing them up!
 - Another anonymous user who had some archives of older versions of Amethyst source code, thank you!
 - [This](https://codeberg.org/neoninteger/amethyst) fork of Amethyst, for being an inspiration and for being the only other fork I know of


## License

[ISC license](https://choosealicense.com/licenses/isc)

