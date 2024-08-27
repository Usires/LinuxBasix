# LinuxBasix
Personalised installation tool for use after basic distro install (based on apt/flatpak). Easily portable and quickly compiled on any gcc-compatible environment.

+ Made for Linux distros based on APT as a package manager (i.e. Debian, Ubuntu, Pop_OS! etc.)
+ Written in C++ with `ncurses` library for easy menu navigation
+ Based on coding examples in the [NCURSES Howto by Pradeep Padala](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/index.html), v1.9 from 2005
+ C++ conversion based on Python version, conversion with support of Claude 3.5 by Anthropic
+ Uses `wget` to download fonts as ZIP archives and unzips them to `~/.local/share/fonts`
+ Runs `fc-cache` after font installation

## Compiling and usage

+ Download the C++ source file to your local machine.
+ Open your terminal app and change into the folder with the source code.
+ Make sure the ncurses libs are installed, i.e. with `sudo apt install libncurses-dev`. 
+ Compile the code: `g++ <name_of_the_source_code.cpp> -lncurses -Os`.
+ Run the program with `./a.out`.

## Pre-selected APT Packages in the code

+ 1password (via AgileBits repo, will be added)
+ htop
+ mc
+ gdu
+ neovim
+ tilix
+ zip/unzip
+ fortune-mod
+ build-essential
+ flatpak
+ preload
+ nala
+ gnome-software-plugin-flatpak
+ powertop
+ cmatrix
+ cool-retro-term

## Pre-selected Flatpak packages in the code

+ com.github.tchx84.Flatseal
+ com.discordapp.Discord
+ com.ktechpit.colorwall
+ com.mattjakeman.ExtensionManager
+ com.microsoft.Edge
+ com.valvesoftware.Steam
+ net.cozic.joplin_desktop
+ net.lutris.Lutris
+ org.DolphinEmu.dolphin-emu
+ org.duckstation.DuckStation
+ org.libretro.RetroArch
+ org.mozilla.Thunderbird
+ net.sf.VICE
+ net.fsuae.FS-UAE
+ org.audacityteam.Audacity
+ org.gimp.GIMP
+ org.gnome.Boxes
+ com.transmissionbt.Transmission
+ fr.handbrake.ghb

## Installed additional fonts

+ JetBrains Mono
+ Hack

