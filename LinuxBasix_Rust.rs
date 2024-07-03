use std::process::Command;
use std::fs;
use std::io::{self, Write};
use std::collections::HashSet;

const MAIN_MENU_ITEMS: usize = 9;

const MAIN_MENU_OPTIONS: [&str; MAIN_MENU_ITEMS] = [
    "Select original repo packages",
    "Install original repo packages",
    "Select Flatpak packages",
    "Install Flatpak packages",
    "Install 1Password (via AgileBit repo)",
    "Install additional fonts",
    "Select package manager",
    "Copy configs from Github repo to HOME",
    "Exit (or press 'Q')"
];

const PROGRAMS_TO_INSTALL: [&str; 20] = [
    "curl", "git", "neovim", "htop", "neofetch", "tilix", "gdu", "nala", "mc",
    "zip", "unzip", "fortune-mod", "build-essential", "flatpak", "preload",
    "cmatrix", "cool-retro-term", "powertop", "upx-ucl", "code"
];

const FLATPAK_PROGRAMS_TO_INSTALL: [&str; 21] = [
    "com.spotify.Client", "org.videolan.VLC",
    "com.github.tchx84.Flatseal", "com.discordapp.Discord",
    "com.ktechpit.colorwall", "com.mattjakeman.ExtensionManager", "com.microsoft.Edge",
    "com.valvesoftware.Steam", "net.cozic.joplin_desktop", "net.lutris.Lutris",
    "org.DolphinEmu.dolphin-emu", "org.duckstation.DuckStation", "org.libretro.RetroArch",
    "org.mozilla.Thunderbird", "net.sf.VICE", "net.fsuae.FS-UAE", "org.audacityteam.Audacity",
    "org.gimp.GIMP", "org.gnome.Boxes", "com.transmissionbt.Transmission", "fr.handbrake.ghb"
];

struct ProgramState {
    selected_flatpak_programs: HashSet<String>,
    selected_apt_programs: HashSet<String>,
    selected_package_manager: Option<String>
}

fn get_kernel_version() -> String {
    let output = Command::new("uname")
        .arg("-r")
        .output()
        .expect("Failed to execute uname command");
    String::from_utf8_lossy(&output.stdout).trim().to_string()
}

fn command_exists(command: &str) -> bool {
    fs::metadata(format!("/usr/bin/{}", command)).is_ok()
}

fn check_package_managers() -> Vec<String> {
    let package_managers = vec!["apt", "pacman", "yum", "dnf", "zypper", "snap"];
    package_managers.into_iter()
        .filter(|&manager| command_exists(manager))
        .map(String::from)
        .collect()
}

fn display_main_menu(highlight: usize) {
    println!("LinuxBasix // Version 2.1 (Rust edition)");
    println!("Main Menu");
    for (i, option) in MAIN_MENU_OPTIONS.iter().enumerate() {
        if i + 1 == highlight {
            print!("> ");
        } else {
            print!("  ");
        }
        println!("{}. {}", i + 1, option);
    }
    println!("\nCurrent Linux Kernel version: {}", get_kernel_version());
    println!("Detected packet managers: {}", check_package_managers().join(" "));
}

fn execute_command(command: &[&str]) {
    let status = Command::new(command[0])
        .args(&command[1..])
        .status()
        .expect("Failed to execute command");

    if !status.success() {
        eprintln!("Command {:?} failed with exit code {:?}", command, status.code());
    }
}

fn select_programs(programs: &[String], selected_programs: &mut HashSet<String>) {
    let mut sorted_programs: Vec<_> = programs.iter().map(|s| s.to_string()).collect();
    sorted_programs.sort();

    loop {
        println!("Select programs:");
        for (i, program) in sorted_programs.iter().enumerate() {
            let status = if selected_programs.contains(program) { "[+]" } else { "[ ]" };
            println!("{} {} {}", i + 1, status, program);
        }
        println!("Press the number to select/unselect, q to quit");

        let mut input = String::new();
        io::stdout().flush().unwrap();
        io::stdin().read_line(&mut input).unwrap();

        match input.trim() {
            "q" => break,
            n if n.parse::<usize>().is_ok() => {
                let index = n.parse::<usize>().unwrap() - 1;
                if index < sorted_programs.len() {
                    let program = &sorted_programs[index];
                    if selected_programs.contains(program) {
                        selected_programs.remove(program);
                    } else {
                        selected_programs.insert(program.clone());
                    }
                }
            }
            _ => println!("Invalid input"),
        }
    }
}

fn main_menu(state: &mut ProgramState) {
    let mut highlight = 1;

    loop {
        display_main_menu(highlight);

        let mut input = String::new();
        io::stdout().flush().unwrap();
        io::stdin().read_line(&mut input).unwrap();

        match input.trim() {
            "q" | "Q" => break,
            "w" => highlight = if highlight > 1 { highlight - 1 } else { MAIN_MENU_ITEMS },
            "s" => highlight = if highlight < MAIN_MENU_ITEMS { highlight + 1 } else { 1 },
            "\n" => {
                 match highlight {
                    1 => {
                        let programs: Vec<String> = PROGRAMS_TO_INSTALL.iter().map(|&s| s.to_string()).collect();
                        select_programs(&programs, &mut state.selected_apt_programs);
                        },
                    3 => {
                        let flatpaks: Vec<String> = FLATPAK_PROGRAMS_TO_INSTALL.iter().map(|&s| s.to_string()).collect();
                        select_programs(&flatpaks, &mut state.selected_flatpak_programs);
                        },
                    7 => {
                        let available_managers = check_package_managers();
                        let mut selected = HashSet::new();
                        if let Some(current) = &state.selected_package_manager {
                            selected.insert(current.clone());
                            }
                        select_programs(&available_managers, &mut selected);
                        state.selected_package_manager = selected.into_iter().next();
                        },
                    9 => break,
                    _ => println!("Not implemented yet"),
                }
            }
            _ => println!("Invalid input"),
        }
    }
}

fn main() {
    let mut state = ProgramState {
        selected_flatpak_programs: HashSet::new(),
        selected_apt_programs: HashSet::new(),
        selected_package_manager: None
    };

    main_menu(&mut state);
}
