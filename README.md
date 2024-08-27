# LinuxBasix
Personalised installation tool after basic distro install (based on apt/flatpak).

+ Made for Linux distros based on APT as a package manager (i.e. Debian, Ubuntu, Pop_OS! etc.)
+ Written in C++ with `ncurses` library for easy menu navigation
+ Based on coding examples in the NCURSES Howto by Pradeep Padala, v1.9 from 2005
+ C++ conversion based on Python version, conversion with support of Claude 3.5 by Anthropic
+ Uses `wget` to download fonts as ZIP archives and unzips them to `~/.local/share/fonts`
+ Runs `fc-cache` after font installation

## Pre-selected APT Packages

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

## Pre-selected Flatpak packages

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

## Compiling and usage

+ Download the C++ source file to your local machine.
+ Open your terminal app and change into the folder with the source code.
+ Compile the code: `g++ <name_of_the_source_code.cpp> -lncurses -Os`.
+ Run the programm with `./a.out`.


