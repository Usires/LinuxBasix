#include <ncurses.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>

const int MAIN_MENU_ITEMS = 7;

std::vector<std::string> programs_to_install = {
    "curl", "git", "neovim", "htop", "neofetch", "tilix", "gdu", "nala", "mc",
    "zip", "unzip", "fortune-mod", "build-essential", "flatpak", "cpufetch",
    "preload", "gnome-software-plugin-flatpak", "cmatrix", "cool-retro-term",
    "powertop"
};

std::vector<std::string> flatpak_programs_to_install = {
    "com.spotify.Client", "org.videolan.VLC", "com.visualstudio.code",
    "com.github.tchx84.Flatseal", "com.discordapp.Discord",
    "com.ktechpit.colorwall", "com.mattjakeman.ExtensionManager", "com.microsoft.Edge",
    "com.valvesoftware.Steam", "net.cozic.joplin_desktop", "net.lutris.Lutris",
    "org.DolphinEmu.dolphin-emu", "org.duckstation.DuckStation", "org.libretro.RetroArch",
    "org.mozilla.Thunderbird", "net.sf.VICE", "net.fsuae.FS-UAE", "org.audacityteam.Audacity",
    "org.gimp.GIMP", "org.gnome.Boxes", "com.transmissionbt.Transmission", "fr.handbrake.ghb"
};

std::set<std::string> selected_flatpak_programs(flatpak_programs_to_install.begin(), flatpak_programs_to_install.end());

std::set<std::string> selected_apt_programs(programs_to_install.begin(), programs_to_install.end());


std::vector<std::string> main_menu_options = {
    "Select APT Packages",
    "Install APT Packages",
    "Select Flatpak Packages",
    "Install Flatpak Packages",
    "Install 1Password via AgileBit Repo",
    "Install additional fonts",
    "Exit (or press 'Q')"
};

void display_main_menu(WINDOW* stdscr, int highlight) {
    wclear(stdscr);
    init_pair(1, COLOR_WHITE, COLOR_BLUE);
    wbkgd(stdscr, COLOR_PAIR(1));
    box(stdscr, 0, 0);
    
    std::string program_name = "LinuxBasix.CPP -- Version 1.0";
    attron(A_BOLD);
    mvwprintw(stdscr, 1, 2, "%s", program_name.c_str());
    attroff(A_BOLD);

    mvwprintw(stdscr, 3, 2, "Main Menu");
    for (size_t i = 0; i < main_menu_options.size(); ++i) {
        if (i + 1 == highlight) {
            wattron(stdscr, A_REVERSE);
        }
        if (i + 1 == main_menu_options.size()) {
            mvwprintw(stdscr, 6 + i, 5, "%zu. %s", i + 1, main_menu_options[i].c_str());
        } else {
            mvwprintw(stdscr, 5 + i, 5, "%zu. %s", i + 1, main_menu_options[i].c_str());
        }
        
        wattroff(stdscr, A_REVERSE);
    }

    int height, width;
    getmaxyx(stdscr, height, width);
    std::string version_info = "Uses ncurses library " + std::string(NCURSES_VERSION);
    std::string copyright_text = "(c) 2024 github.com/Usires. Made in C++ with the help of ChatGPT-4o.";
    std::string packer_text = "Packed with UPX 3.96, (c) 1996-2020 by Markus Oberhumer, Laszlo Molnar & John Reiser";
    mvwprintw(stdscr, height - 4, 2, "%s", packer_text.c_str());
    mvwprintw(stdscr, height - 3, 2, "%s", version_info.c_str());
    mvwprintw(stdscr, height - 2, 2, "%s", copyright_text.c_str());

    wrefresh(stdscr);
}

void execute_command(const std::vector<std::string>& command) {
    std::vector<char*> args;
    for (const auto& arg : command) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);

    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args.data());
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            std::cerr << "Command " << command[0] << " failed with return code " << WEXITSTATUS(status) << "\n";
        }
    }
}

void execute_code_block(WINDOW* stdscr, int option) {
    wclear(stdscr);
    wrefresh(stdscr);

    std::vector<std::vector<std::string>> commands;
    if (option == 2) {
        commands = {
            {"clear"},
            {"sudo", "apt", "update"},
            {"sudo", "apt", "install", "-y"}
        };
        commands[2].insert(commands[2].end(), selected_apt_programs.begin(), selected_apt_programs.end());
        commands.push_back({"flatpak", "-v", "remote-add", "--if-not-exists", "flathub", "https://dl.flathub.org/repo/flathub.flatpakrepo"});
    /* else if (option == 3) {
        commands = {{"echo", "Returning to Main Menu."}}; */
    } else if (option == 4) {
        commands = {
            {"clear"},
            {"flatpak", "install"}
        };
        commands[1].insert(commands[1].end(), selected_flatpak_programs.begin(), selected_flatpak_programs.end());
    } else if (option == 5) {
        commands = {
            /* {"sudo", "apt", "update"},
            {"sudo", "apt", "install", "-y", "curl"}, */
            {"clear"},
            {"sh", "-c", "curl -sS https://downloads.1password.com/linux/keys/1password.asc | sudo gpg --dearmor --output /usr/share/keyrings/1password-archive-keyring.gpg"},
            {"sh", "-c", "echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/1password-archive-keyring.gpg] https://downloads.1password.com/linux/debian/amd64 stable main' | sudo tee /etc/apt/sources.list.d/1password.list"},
            {"sudo", "mkdir", "-p", "/etc/debsig/policies/AC2D62742012EA22/"},
            {"sh", "-c", "curl -sS https://downloads.1password.com/linux/debian/debsig/1password.pol | sudo tee /etc/debsig/policies/AC2D62742012EA22/1password.pol"},
            {"sudo", "mkdir", "-p", "/usr/share/debsig/keyrings/AC2D62742012EA22"},
            {"sh", "-c", "curl -sS https://downloads.1password.com/linux/keys/1password.asc | sudo gpg --dearmor --output /usr/share/debsig/keyrings/AC2D62742012EA22/debsig.gpg"},
            {"sh", "-c", "sudo apt update && printf '\\n' && sudo apt install -y 1password"}
        };
     } else if (option == 6) {
        commands = {
            {"clear"},
            {"echo", "Installing addtional fonts. \n"},
            {"wget", "https://github.com/source-foundry/Hack/releases/download/v3.003/Hack-v3.003-ttf.zip"},
            {"wget", "https://download.jetbrains.com/fonts/JetBrainsMono-1.0.3.zip"},
            {"sh", "-c", "for i in *.zip; do unzip -u \"$i\" -d ~/.local/share/fonts && rm \"$i\"; done"},
            // {"rm", "-v", "*.zip"},
            {"fc-cache", "-r", "-v"}
        };    
     }

    endwin();
    for (const auto& command : commands) {
        execute_command(command);
    }
    std::cout << "Press any key to return to the main menu...";
    std::cin.get();
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    stdscr = initscr();
    keypad(stdscr, TRUE);
}

/* void display_apt_programs(WINDOW* stdscr) {
    std::vector<std::string> sorted_programs = programs_to_install;
    std::sort(sorted_programs.begin(), sorted_programs.end());

    int height, width;
    getmaxyx(stdscr, height, width);
    int win_height = sorted_programs.size() + 8;
    int win_width = std::max(static_cast<int>(max_element(sorted_programs.begin(), sorted_programs.end(), 
        [](const std::string& a, const std::string& b){ return a.size() < b.size(); })->size()) + 2, 40);

    WINDOW* win = newwin(win_height, win_width, (height - win_height) / 2, (width - win_width) / 2);
    box(win, 0, 0);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 1, "APT Programs");
    wattroff(win, A_BOLD);

    for (size_t i = 0; i < sorted_programs.size(); ++i) {
        mvwprintw(win, 3 + i, 2, "%s", sorted_programs[i].c_str());
    }

    mvwprintw(win, win_height - 2, 1, "Press any key to return...");
    wrefresh(win);
    wgetch(win);
    wclear(win);
    wrefresh(win);
    delwin(win);
} */

void select_programs(WINDOW* stdscr, const std::vector<std::string>& programs_to_sort, std::set<std::string>& selected_programs) {
    std::vector<std::string> sorted_programs = programs_to_sort;
    std::sort(sorted_programs.begin(), sorted_programs.end());

    int height, width;
    getmaxyx(stdscr, height, width);
    int win_height = sorted_programs.size() + 6;
    int win_width = std::max(static_cast<int>(std::max_element(sorted_programs.begin(), sorted_programs.end(), 
        [](const std::string& a, const std::string& b){ return a.size() < b.size(); })->size()) + 10, 50);

    WINDOW* win = newwin(win_height, win_width, (height - win_height) / 2, (width - win_width) / 2);
    keypad(win, TRUE);
    box(win, 0, 0);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 1, "Select software to install: ");
    wattroff(win, A_BOLD);

    int highlight = 0;
    while (true) {
        for (size_t i = 0; i < sorted_programs.size(); ++i) {
            if (static_cast<int>(i) == highlight) {
                wattron(win, A_REVERSE);
            }
            if (selected_programs.count(sorted_programs[i])) {
                mvwprintw(win, 3 + i, 2, "[x] %s", sorted_programs[i].c_str());
            } else {
                mvwprintw(win, 3 + i, 2, "[ ] %s", sorted_programs[i].c_str());
            }
            if (static_cast<int>(i) == highlight) {
                wattroff(win, A_REVERSE);
            }
        }

        mvwprintw(win, win_height - 2, 1, "Press Enter to select/unselect, q to quit");
        wrefresh(win);

        int key = wgetch(win);
        if (key == KEY_UP) {
            highlight = (highlight - 1 + sorted_programs.size()) % sorted_programs.size();
        } else if (key == KEY_DOWN) {
            highlight = (highlight + 1) % sorted_programs.size();
        } else if (key == 10) { // Enter key
            const std::string& program = sorted_programs[highlight];
            if (selected_programs.count(program)) {
                selected_programs.erase(program);
            } else {
                selected_programs.insert(program);
            }
        } else if (key == 'q') {
            break;
        }
    }

    wclear(win);
    wrefresh(win);
    delwin(win);
}

/* void select_apt_programs(WINDOW* stdscr) {
    std::vector<std::string> sorted_programs = programs_to_install;
    std::sort(sorted_programs.begin(), sorted_programs.end());

    int height, width;
    getmaxyx(stdscr, height, width);
    int win_height = sorted_programs.size() + 6;
    int win_width = std::max(static_cast<int>(max_element(sorted_programs.begin(), sorted_programs.end(), 
        [](const std::string& a, const std::string& b){ return a.size() < b.size(); })->size()) + 10, 50);

    WINDOW* win = newwin(win_height, win_width, (height - win_height) / 2, (width - win_width) / 2);
    keypad(win, TRUE);
    box(win, 0, 0);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 1, "Select APT Programs");
    wattroff(win, A_BOLD);

    int highlight = 0;
    while (true) {
        for (size_t i = 0; i < sorted_programs.size(); ++i) {
            if (static_cast<int>(i) == highlight) {
                wattron(win, A_REVERSE);
            }
            if (selected_apt_programs.count(sorted_programs[i])) {
                mvwprintw(win, 3 + i, 2, "[x] %s", sorted_programs[i].c_str());
            } else {
                mvwprintw(win, 3 + i, 2, "[ ] %s", sorted_programs[i].c_str());
            }
            wattroff(win, A_REVERSE);
        }

        mvwprintw(win, win_height - 2, 1, "Press Enter to select/unselect, q to quit");
        wrefresh(win);

        int key = wgetch(win);
        if (key == KEY_UP) {
            highlight = (highlight - 1 + sorted_programs.size()) % sorted_programs.size();
        } else if (key == KEY_DOWN) {
            highlight = (highlight + 1) % sorted_programs.size();
        } else if (key == 10) { // Enter key
            const std::string& program = sorted_programs[highlight];
            if (selected_apt_programs.count(program)) {
                selected_apt_programs.erase(program);
            } else {
                selected_apt_programs.insert(program);
            }
        } else if (key == 'q') {
            break;
        }
    }

    wclear(win);
    wrefresh(win);
    delwin(win);
} */

/* void select_flatpak_programs(WINDOW* stdscr) {
    std::vector<std::string> sorted_programs = flatpak_programs_to_install;
    std::sort(sorted_programs.begin(), sorted_programs.end());

    int height, width;
    getmaxyx(stdscr, height, width);
    int win_height = sorted_programs.size() + 6;
    int win_width = std::max(static_cast<int>(max_element(sorted_programs.begin(), sorted_programs.end(), 
        [](const std::string& a, const std::string& b){ return a.size() < b.size(); })->size()) + 10, 50);

    WINDOW* win = newwin(win_height, win_width, (height - win_height) / 2, (width - win_width) / 2);
    keypad(win, TRUE);
    box(win, 0, 0);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 1, "Select Flatpak Programs");
    wattroff(win, A_BOLD);

    int highlight = 0;
    while (true) {
        for (size_t i = 0; i < sorted_programs.size(); ++i) {
            if (static_cast<int>(i) == highlight) {
                wattron(win, A_REVERSE);
            }
            if (selected_flatpak_programs.count(sorted_programs[i])) {
                mvwprintw(win, 3 + i, 2, "[x] %s", sorted_programs[i].c_str());
            } else {
                mvwprintw(win, 3 + i, 2, "[ ] %s", sorted_programs[i].c_str());
            }
            wattroff(win, A_REVERSE);
        }

        mvwprintw(win, win_height - 2, 1, "Press Enter to select/unselect, q to quit");
        wrefresh(win);

        int key = wgetch(win);
        if (key == KEY_UP) {
            highlight = (highlight - 1 + sorted_programs.size()) % sorted_programs.size();
        } else if (key == KEY_DOWN) {
            highlight = (highlight + 1) % sorted_programs.size();
        } else if (key == 10) { // Enter key
            const std::string& program = sorted_programs[highlight];
            if (selected_flatpak_programs.count(program)) {
                selected_flatpak_programs.erase(program);
            } else {
                selected_flatpak_programs.insert(program);
            }
        } else if (key == 'q') {
            break;
        }
    }

    wclear(win);
    wrefresh(win);
    delwin(win);
} */

void main_menu() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    WINDOW* stdscr = initscr();
    keypad(stdscr, TRUE);

    int highlight_main = 1;
    while (true) {
        display_main_menu(stdscr, highlight_main);
        int key = wgetch(stdscr);

        if (key == KEY_UP) {
            highlight_main = highlight_main - 1 > 0 ? highlight_main - 1 : MAIN_MENU_ITEMS;
        } else if (key == KEY_DOWN) {
            highlight_main = highlight_main + 1 <= MAIN_MENU_ITEMS ? highlight_main + 1 : 1;
        } else if (key == 10) { // Enter key
            if (highlight_main == MAIN_MENU_ITEMS) {
                break; // Exit the program
            } else {
                if (highlight_main == 1) {
                    select_programs(stdscr, programs_to_install, selected_apt_programs);
                } else if (highlight_main == 2) {
                    execute_code_block(stdscr, highlight_main);
                } else if (highlight_main == 3) {
                    select_programs(stdscr, flatpak_programs_to_install, selected_flatpak_programs);
                } else {
                    execute_code_block(stdscr, highlight_main);
                }
            }
        } else if (key == 'q') {
            break;
        }
    }

    endwin();
}

int main() {
    main_menu();
    return 0;
}
