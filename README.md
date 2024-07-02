# LinuxBasix
Personalised installation tool after basic distro install (based on apt/flatpak).

+ Made for Linux distros based on APT as a package manager (i.e. Debian, Ubuntu, Pop_OS! etc.)
+ Written in C++ with `ncurses` library for easy menu navigation
+ Based on coding examples in the NCURSES Howto by Pradeep Padala, v1.9 from 2005
+ C++ conversion based on Python version, conversion with support of ChatGPT-4o by OpenAI
+ Binary packed with UPX 3.96, (c) 1996-2020 by Markus Oberhumer, Laszlo Molnar & John Reiser
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
+ neofetch
+ cpufetch
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
+ com.visualstudio.code
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
