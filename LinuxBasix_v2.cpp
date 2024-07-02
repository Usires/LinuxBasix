#include <ncurses.h>
#include <cstdlib>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <sstream>
#include <sys/utsname.h>

using namespace std;

const int MAIN_MENU_ITEMS = 9;

const vector<string> main_menu_options = {
    "Select original repo packages",
    "Install original repo packages",
    "Select Flatpak packages",
    "Install Flatpak packages",
    "Install 1Password (via AgileBit repo)",
    "Install additional fonts",
    "Select package manager",
    "Copy configs from Github repo to HOME",
    "Exit (or press 'Q')"
};

const vector<string> programs_to_install = {
   "curl", "git", "neovim", "htop", "neofetch", "tilix", "gdu", "nala", "mc",
   "zip", "unzip", "fortune-mod", "build-essential", "flatpak", "preload",
   "cmatrix", "cool-retro-term", "powertop", "upx-ucl", "code"
};

const vector<string> flatpak_programs_to_install = {
    "com.spotify.Client", "org.videolan.VLC",
    "com.github.tchx84.Flatseal", "com.discordapp.Discord",
    "com.ktechpit.colorwall", "com.mattjakeman.ExtensionManager", "com.microsoft.Edge",
    "com.valvesoftware.Steam", "net.cozic.joplin_desktop", "net.lutris.Lutris",
    "org.DolphinEmu.dolphin-emu", "org.duckstation.DuckStation", "org.libretro.RetroArch",
    "org.mozilla.Thunderbird", "net.sf.VICE", "net.fsuae.FS-UAE", "org.audacityteam.Audacity",
    "org.gimp.GIMP", "org.gnome.Boxes", "com.transmissionbt.Transmission", "fr.handbrake.ghb"
};

set<string> selected_flatpak_programs(flatpak_programs_to_install.begin(), flatpak_programs_to_install.end());
set<string> selected_apt_programs(programs_to_install.begin(), programs_to_install.end());
set<string> selected_package_manager;

// Function to get the Linux kernel version
string getKernelVersion() {
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        return "Unknown";
    }
    return string(buffer.release);
}

// Function to check if a command exists
bool commandExists(const string& command) {
    string path = "/usr/bin/" + command;
    ifstream file(path.c_str());
    return file.good();
}

// Function to get the availability of popular package managers
vector<string> checkPackageManagers() {
    vector<string> packageManagers = {"apt", "pacman", "yum", "dnf", "zypper", "snap"};
    vector<string> availablePackageManagers;

    for (const auto& manager : packageManagers) {
        if (commandExists(manager)) {
            availablePackageManagers.push_back(manager);
        }
    }
    return availablePackageManagers;
}

// Function to join a vector of strings into a single string with a delimiter
string join(const vector<string>& vec, const string& delimiter) {
    ostringstream oss;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i != 0) {
            oss << delimiter;
        }
        oss << vec[i];
    }
    return oss.str();
}

void display_main_menu(WINDOW* stdscr, int highlight) {
    wclear(stdscr);
    wbkgd(stdscr, COLOR_PAIR(1));

    // wborder(stdscr, ACS_VLINE, ACS_VLINE, ACS_HLINE, ACS_HLINE, ACS_ULCORNER, ACS_URCORNER, ACS_LLCORNER, ACS_LRCORNER);
    box(stdscr, 0, 0);
    
    string program_name = "LinuxBasix // Version 2.1 (C++ edition)";
    
    attron(A_BOLD);
    mvwprintw(stdscr, 1, 2, "%s", program_name.c_str());
    mvwprintw(stdscr, 3, 2, "Main Menu");
    attroff(A_BOLD);

   for (size_t i = 0; i < main_menu_options.size(); ++i) {
        if (i + 1 == highlight) {
            wattron(stdscr, A_REVERSE);
        }
        if (i == main_menu_options.size() - 1) {
            // If it's the last item, print it on a line one line apart from the rest
            mvwprintw(stdscr, 6 + i, 5, "%zu. %s", i + 1, main_menu_options[i].c_str());
        } else {
            // For all other items, print normally
            mvwprintw(stdscr, 5 + i, 5, "%zu. %s", i + 1, main_menu_options[i].c_str());
        }
        wattroff(stdscr, A_REVERSE);
    }   

    int height, width;
    getmaxyx(stdscr, height, width);

     // Get the kernel version
    string kernelVersion = getKernelVersion();
    
    // Check for available package managers
    vector<string> availablePackageManagers = checkPackageManagers();
    
    /* cout << "Available Package Managers: ";
    for (const auto& manager : availablePackageManagers) {
        cout << manager << " ";
    } */ 

    string version_info = "Uses ncurses library " + string(NCURSES_VERSION) + ", (c) 1993-2024 Free Software Foundation, Inc.";
    string copyright_text = "(c) 2024 github.com/Usires. Made in C++ with the help of ChatGPT-4o.";
    string packer_text = "Packed with UPX 3.96, (c) 1996-2020 by Markus Oberhumer, Laszlo Molnar & John Reiser";
    string kernel = "Current Linux Kernel version: " + string(kernelVersion);
    string packetmanagers = "Detected packet managers: " + join(availablePackageManagers, " ");

    mvwprintw(stdscr, height - 2, 2, "%s", packer_text.c_str());
    mvwprintw(stdscr, height - 3, 2, "%s", version_info.c_str());
    mvwprintw(stdscr, height - 4, 2, "%s", copyright_text.c_str());
    
    attron(A_BOLD);
    mvwprintw(stdscr, height - 7, 2, "%s", kernel.c_str());
    mvwprintw(stdscr, height - 6, 2, "%s", packetmanagers.c_str());
    attroff(A_BOLD);

    wrefresh(stdscr);
}

void execute_command(const vector<string>& command) {
    vector<char*> args;
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
            cerr << "Command " << command[0] << " failed with return code " << WEXITSTATUS(status) << "\n";
        }
    }
}

void execute_code_block(WINDOW* stdscr, int option) {
    wclear(stdscr);
    wrefresh(stdscr);

    vector<vector<string>> commands;
    if (option == 2) {
        commands = {
            {"clear"},
            {"sudo", "apt", "update"},
            {"sudo", "apt", "install", "-y"}
        };
        commands[2].insert(commands[2].end(), selected_apt_programs.begin(), selected_apt_programs.end());
        commands.push_back({"flatpak", "-v", "remote-add", "--if-not-exists", "flathub", "https://dl.flathub.org/repo/flathub.flatpakrepo"});
    } else if (option == 4) {
        commands = {
            {"clear"},
            {"flatpak", "install"}
        };
        commands[1].insert(commands[1].end(), selected_flatpak_programs.begin(), selected_flatpak_programs.end());

    } else if (option == 5) {
        commands = {
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
            {"echo", "Installing additional fonts. \n"},
            {"wget", "https://github.com/source-foundry/Hack/releases/download/v3.003/Hack-v3.003-ttf.zip"},
            {"wget", "https://download.jetbrains.com/fonts/JetBrainsMono-1.0.3.zip"},
            {"sh", "-c", "for i in *.zip; do unzip -u \"$i\" -d ~/.local/share/fonts && rm \"$i\"; done"},
            {"fc-cache", "-r", "-v"}
        };    
     }

    endwin();
    for (const auto& command : commands) {
        execute_command(command);
    }
    cout << "Press any key to return to the main menu...";
    cin.get();
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
}

void select_programs(WINDOW* stdscr, const vector<string>& programs_to_sort, set<string>& selected_programs, int menu_color, const string& program_type) {
    vector<string> sorted_programs = programs_to_sort;
    sort(sorted_programs.begin(), sorted_programs.end());

    int height, width;
    getmaxyx(stdscr, height, width);
    int win_height = sorted_programs.size() + 6;
    int win_width = max(static_cast<int>(max_element(sorted_programs.begin(), sorted_programs.end(), 
        [](const string& a, const string& b){ return a.size() < b.size(); })->size()) + 10, 50);

    int start_y = (height - win_height) / 2;
    int start_x = (width - win_width) / 2;

    // Create windows (first main, then shadow)

    WINDOW* win = newwin(win_height, win_width, start_y, start_x);
    WINDOW* shadow_win = newwin(win_height, win_width, start_y + 1, start_x + 2);

    // Draw shadow
    
    wattron(shadow_win, COLOR_PAIR(3)); // Color pair 3 is the shadow color
    
    for (int i = 0; i < win_height; ++i) {
        for (int j = 0; j < win_width; ++j) {
            mvwaddch(shadow_win, i, j, ' ');
        }
    }
    
    wattroff(shadow_win, COLOR_PAIR(3));
    wrefresh(shadow_win);

    keypad(win, TRUE);
    wbkgd(win, COLOR_PAIR(menu_color)); 
    box(win, 0, 0);
    wattron(win, A_BOLD);
    mvwprintw(win, 1, 1, "Select %s: ", program_type.c_str());
    wattroff(win, A_BOLD);

    int highlight = 0;
    while (true) {
        for (size_t i = 0; i < sorted_programs.size(); ++i) {
            if (static_cast<int>(i) == highlight) {
                wattron(win, A_REVERSE);
            }
            if (selected_programs.count(sorted_programs[i])) {
                mvwprintw(win, 3 + i, 2, "[+] %s", sorted_programs[i].c_str());
            } else {
                mvwprintw(win, 3 + i, 2, "[ ] %s", sorted_programs[i].c_str());
            }
            if (static_cast<int>(i) == highlight) {
                wattroff(win, A_REVERSE);
            }
        }

        mvwprintw(win, win_height - 2, 1, "Press Space to select/unselect, q to quit");
        wrefresh(win);

        int key = wgetch(win);
        if (key == KEY_UP) {
            highlight = (highlight - 1 + sorted_programs.size()) % sorted_programs.size();
        } else if (key == KEY_DOWN) {
            highlight = (highlight + 1) % sorted_programs.size();
        } else if (key == ' ') { // Space key
            const string& program = sorted_programs[highlight];
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

    wclear(shadow_win);
    wrefresh(shadow_win);
    delwin(shadow_win);
}


void main_menu() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    start_color();
    
    init_pair(1, COLOR_WHITE, COLOR_BLUE); // Main menu colors
    init_pair(2, COLOR_WHITE, COLOR_CYAN); // Sub menu color set 1
    init_pair(4, COLOR_WHITE, COLOR_MAGENTA); // Sub menu color set 2
    init_pair(3, COLOR_BLACK, COLOR_BLACK); // Shadow color

    WINDOW* stdscr = initscr();
    keypad(stdscr, TRUE);

    int highlight_main = 1;

    while (true) {
        display_main_menu(stdscr, highlight_main);
        int key = wgetch(stdscr);

        switch (key) {
            case KEY_UP:
                highlight_main = highlight_main - 1 > 0 ? highlight_main - 1 : MAIN_MENU_ITEMS;
                break;

            case KEY_DOWN:
                highlight_main = highlight_main + 1 <= MAIN_MENU_ITEMS ? highlight_main + 1 : 1;
                break;

            case 10: // Enter key
                if (highlight_main == MAIN_MENU_ITEMS) {
                    endwin();
                    return; // Exit the program
                } else {
                    switch (highlight_main) {
                        case 1:
                            select_programs(stdscr, programs_to_install, selected_apt_programs, 2, "packages");
                            break;
                        case 2:
                            execute_code_block(stdscr, highlight_main);
                            break;
                        case 3:
                            select_programs(stdscr, flatpak_programs_to_install, selected_flatpak_programs, 4, "Flatpaks");
                            break;
                        case 7:
                            select_programs(stdscr, checkPackageManagers(), selected_package_manager, 2, "package manager");
                            break;
                        default:
                            execute_code_block(stdscr, highlight_main);
                            break;
                    }
                }
                break;

            case 'q':
            case 27: // ESC key
                endwin();
                return; // Exit the program

            default:
                break;
        }
    }
}

int main() {
    main_menu();
    return 0;
}
