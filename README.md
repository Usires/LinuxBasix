# LinuxBasics
Personalised installation tool after basic distro install (based on apt/flatpak).

+ Made for Linux distros based on APT as a package manager (i.e. Debian, Ubuntu, Pop_OS! etc.)
+ Written in C with `ncurses` library for easy menu navigation
+ Relies mainly on function `system()` to call shell commands for installing, updating, unzipping etc.
+ Uses `wget` to download fonts as ZIP archives and unzips them to `~/.local/share/fonts`
+ Runs `fc-cache` after font installation

## Installed APT Packages
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

## Installed Flatpak packages
+ ca.desrt.dconf-editor
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

## Installed additional fonts
+ JetBrains Mono
+ Hack
