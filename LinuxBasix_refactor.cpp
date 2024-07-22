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

// Configuration structure
struct Configuration {
    vector<string> main_menu_options;
    vector<string> programs_to_install;
    vector<string> flatpak_programs_to_install;
};

// Abstract classes for better testability
class SystemInfo {
public:
    virtual string getKernelVersion() = 0;
    virtual vector<string> checkPackageManagers() = 0;
    virtual ~SystemInfo() = default;
};

class FileSystem {
public:
    virtual bool appendToFile(const string& filename, const vector<string>& lines) = 0;
    virtual ~FileSystem() = default;
};

class CommandExecutor {
public:
    virtual void execute(const vector<string>& command) = 0;
    virtual ~CommandExecutor() = default;
};

// Concrete implementations
class RealSystemInfo : public SystemInfo {
public:
    string getKernelVersion() override {
        struct utsname buffer;
        if (uname(&buffer) != 0) {
            return "Unknown";
        }
        return string(buffer.release);
    }

    vector<string> checkPackageManagers() override {
        vector<string> packageManagers = {"apt", "pacman", "yum", "dnf", "zypper", "snap"};
        vector<string> availablePackageManagers;

        for (const auto& manager : packageManagers) {
            if (commandExists(manager)) {
                availablePackageManagers.push_back(manager);
            }
        }
        return availablePackageManagers;
    }

private:
    bool commandExists(const string& command) {
        string path = "/usr/bin/" + command;
        ifstream file(path.c_str());
        return file.good();
    }
};

class RealFileSystem : public FileSystem {
public:
    bool appendToFile(const string& filename, const vector<string>& lines) override {
        ofstream file(filename, ios_base::app);
        if (!file.is_open()) {
            return false;
        }
        for (const auto& line : lines) {
            file << line << "\n";
        }
        return true;
    }
};

class RealCommandExecutor : public CommandExecutor {
public:
    void execute(const vector<string>& command) override {
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
};

string join(const vector<string>& vec, const string& delimiter) {
    string result;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) result += delimiter;
        result += vec[i];
    }
    return result;
}


// LinuxBasix class
class LinuxBasix {
private:
    Configuration config;
    SystemInfo& systemInfo;
    FileSystem& fileSystem;
    CommandExecutor& commandExecutor;
    set<string> selected_flatpak_programs;
    set<string> selected_apt_programs;
    set<string> selected_package_manager;

public:
    LinuxBasix(const Configuration& cfg, SystemInfo& si, FileSystem& fs, CommandExecutor& ce)
        : config(cfg), systemInfo(si), fileSystem(fs), commandExecutor(ce) {
        selected_flatpak_programs = set<string>(config.flatpak_programs_to_install.begin(), config.flatpak_programs_to_install.end());
        selected_apt_programs = set<string>(config.programs_to_install.begin(), config.programs_to_install.end());
    }

    void run() {
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        start_color();
        
        init_pair(1, COLOR_WHITE, COLOR_BLUE);
        init_pair(2, COLOR_WHITE, COLOR_RED);
        init_pair(4, COLOR_WHITE, COLOR_MAGENTA);
        init_pair(3, COLOR_BLACK, COLOR_BLACK);
        init_pair(5, COLOR_WHITE, COLOR_GREEN);

        WINDOW* stdscr = initscr();
        keypad(stdscr, TRUE);

        main_menu(stdscr);

        endwin();
    }

private:
    void main_menu(WINDOW* stdscr) {
        int highlight_main = 1;
        int MAIN_MENU_ITEMS = config.main_menu_options.size();

        while (true) {
            display_main_menu(stdscr, highlight_main);
            wrefresh(stdscr);
            int key = wgetch(stdscr);

            switch (key) {
                case KEY_UP:
                    highlight_main = highlight_main - 1 > 0 ? highlight_main - 1 : MAIN_MENU_ITEMS;
                    break;
                case KEY_DOWN:
                    highlight_main = highlight_main + 1 <= MAIN_MENU_ITEMS ? highlight_main + 1 : 1;
                    break;
                case 10: // Enter key
                    handle_menu_selection(stdscr, highlight_main);
                    break;
                case 'q':
                case 27: // ESC key
                    return;
                default:
                    break;
            }
        }
    }

    void display_main_menu(WINDOW* stdscr, int highlight) {
    	wclear(stdscr);
    	wbkgd(stdscr, COLOR_PAIR(1));

    	// box(stdscr, 0, 0);
    
   	const string program_name = "LinuxBasix // Version 2.2-240722 (C++ edition)";
    
    	attron(A_BOLD);
    	mvwprintw(stdscr, 1, 2, "%s", program_name.c_str());
    	mvwprintw(stdscr, 3, 2, "Main Menu");
    	attroff(A_BOLD);

   	for (size_t i = 0; i < config.main_menu_options.size(); ++i) {
        	if (i + 1 == highlight) {
            	wattron(stdscr, A_REVERSE);
        	}
        	if (i == config.main_menu_options.size() - 1) {
            	// If it's the last item, print it on a line one line apart from the rest
            	mvwprintw(stdscr, 6 + i, 5, "%zu. %s", i + 1, config.main_menu_options[i].c_str());
        	} else {
            	// For all other items, print normally
            	mvwprintw(stdscr, 5 + i, 5, "%zu. %s", i + 1, config.main_menu_options[i].c_str());
    	    	}
       		wattroff(stdscr, A_REVERSE);
    	}   

    	int height, width;
    	getmaxyx(stdscr, height, width);

     	// Get the kernel version
    	string kernelVersion = systemInfo.getKernelVersion();
    
    	// Check for available package managers
    	vector<string> availablePackageManagers = systemInfo.checkPackageManagers();

    	string version_info = "Uses ncurses library " + string(NCURSES_VERSION) + ", (c) 1993-2024 Free Software Foundation, Inc.";
    	string copyright_text = "(c) 2024 github.com/Usires. Made in C++ with support of Claude 3.5 and ChatGPT-4o";
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

    void handle_menu_selection(WINDOW* stdscr, int highlight_main) {
        if (highlight_main == static_cast<int>(config.main_menu_options.size())) {
            endwin();
            exit(0); // Exit the program
        }
        
        switch (highlight_main) {
            case 1:
                select_programs(stdscr, config.programs_to_install, selected_apt_programs, 2, "packages");
                break;
            case 3:
                select_programs(stdscr, config.flatpak_programs_to_install, selected_flatpak_programs, 4, "Flatpaks");
                break;
            case 7:
                select_programs(stdscr, systemInfo.checkPackageManagers(), selected_package_manager, 5, "package manager");
                break;
            case 8:
                append_to_bashrc_and_edit();
                break;
            default:
                execute_code_block(stdscr, highlight_main);
                break;
        }
    }

    void select_programs(WINDOW* stdscr, const vector<string>& programs_to_sort, set<string>& selected_programs, int menu_color, const string& program_type) {
    vector<string> sorted_programs = programs_to_sort;
    sort(sorted_programs.begin(), sorted_programs.end());

        int height, width;
        getmaxyx(stdscr, height, width);
        int win_height = min(static_cast<int>(sorted_programs.size()) + 6, height - 2);
        int win_width = min(max(static_cast<int>(max_element(sorted_programs.begin(), sorted_programs.end(), 
        [](const string& a, const string& b){ return a.size() < b.size(); })->size()) + 10, 50), width - 2);

        int start_y = (height - win_height) / 2;
        int start_x = (width - win_width) / 2;

        WINDOW* win = newwin(win_height, win_width, start_y, start_x);
        WINDOW* shadow_win = newwin(win_height, win_width, start_y + 1, start_x + 2);

        // Draw shadow
        wattron(shadow_win, COLOR_PAIR(3));
        wbkgd(shadow_win, COLOR_PAIR(3));
        for (int i = 0; i < win_height; ++i) {
            mvwhline(shadow_win, i, 0, ' ', win_width);
        }
        wattroff(shadow_win, COLOR_PAIR(3));
        wrefresh(shadow_win);

        keypad(win, TRUE);
        wbkgd(win, COLOR_PAIR(menu_color));
        box(win, 0, 0);
        wattron(win, A_BOLD);
        mvwprintw(win, 1, 1, "Select %s:", program_type.c_str());
        wattroff(win, A_BOLD);

        int highlight = 0;
        int start_idx = 0;
        int max_display = win_height - 4;

        while (true) {
            for (int i = 0; i < max_display && (i + start_idx) < static_cast<int>(sorted_programs.size()); ++i) {
                if (i + start_idx == highlight) {
                    wattron(win, A_REVERSE);
                }
                string display_str = (selected_programs.count(sorted_programs[i + start_idx]) ? "[+] " : "[ ] ") + sorted_programs[i + start_idx];
                mvwprintw(win, i + 3, 2, "%-*s", win_width - 4, display_str.c_str());
                wattroff(win, A_REVERSE);
            }

            mvwprintw(win, win_height - 1, 1, "Space: select/unselect, Enter: confirm, q: quit");
            wrefresh(win);

            int key = wgetch(win);
            switch (key) {
                case KEY_UP:
                    if (highlight > 0) {
                        --highlight;
                        if (highlight < start_idx) --start_idx;
                    }
                    break;
                case KEY_DOWN:
                    if (highlight < static_cast<int>(sorted_programs.size()) - 1) {
                        ++highlight;
                        if (highlight >= start_idx + max_display) ++start_idx;
                    }
                    break;
                case ' ':
                    {
                        const string& program = sorted_programs[highlight];
                        if (selected_programs.count(program)) {
                            selected_programs.erase(program);
                        } else {
                            selected_programs.insert(program);
                        }
                    }
                    break;
                case 10: // Enter key
                case 'q':
                    delwin(shadow_win);
                    delwin(win);
                    return;
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
                {"wget", "https://downloads.1password.com/linux/debian/amd64/stable/1password-latest.deb"},
                {"sh", "-c", "sudo apt install ./1password-latest.deb"},
                {"rm", "./1password-latest.deb"}
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
            commandExecutor.execute(command);
        }
        cout << "Press any key to return to the main menu...";
        cin.get();
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        keypad(stdscr, TRUE);
    }

    void append_to_bashrc_and_edit() {
        const char* home = getenv("HOME");
        if (!home) {
            cerr << "Unable to get HOME directory" << endl;
            return;
        }

        string bashrc_path = string(home) + "/.bashrc";

        vector<string> lines_to_add = {
            "\n# Added by LinuxBasix",
            "alias ll='ls -la'",
            "alias ls='ls -l'",
            "neofetch",
            "echo ''",
            "fortune -s",
            "echo ''"
        };

        if (fileSystem.appendToFile(bashrc_path, lines_to_add)) {
            cout << "Lines added to .bashrc successfully." << endl;
        } else {
            cerr << "Unable to open .bashrc for appending" << endl;
            return;
        }

        endwin();  // End ncurses mode temporarily

        string command = "vim " + bashrc_path;
        int result = system(command.c_str());  // Store the return value
        if (result == -1) {
            cerr << "Error: Failed to execute vim" << endl;
        } else if (WIFEXITED(result) && WEXITSTATUS(result) != 0) {
            cerr << "Warning: vim exited with status " << WEXITSTATUS(result) << endl;
        }
        cout << "Press any key to return to the main menu...";
        cin.get();
        
        initscr();  // Reinitialize ncurses
        cbreak();
        noecho();
        curs_set(0);
        refresh();
    }
};


int main(int argc, char *argv[]) {
    Configuration config = {
        // main_menu_options
        {
            "Select original repo packages",
            "Install original repo packages",
            "Select Flatpak packages",
            "Install Flatpak packages",
            "Install 1Password (latest .deb from 1password.com)",
            "Install additional fonts",
            "Select package manager",
            "Add additional login items to ~/.bashrc",
            "Exit (or press 'Q')"
        },
        // programs_to_install
        {
            "curl", "git", "neovim", "htop", "fastfetch", "tilix", "gdu", "nala", "mc",
            "zip", "unzip", "fortune-mod", "build-essential", "flatpak", "preload",
            "cmatrix", "cool-retro-term", "powertop", "upx-ucl", "code"
        },
        // flatpak_programs_to_install
        {
            "com.spotify.Client", "org.videolan.VLC",
            "com.github.tchx84.Flatseal", "com.discordapp.Discord",
            "com.ktechpit.colorwall", "com.mattjakeman.ExtensionManager", "com.microsoft.Edge",
            "com.valvesoftware.Steam", "net.cozic.joplin_desktop", "net.lutris.Lutris",
            "org.DolphinEmu.dolphin-emu", "org.duckstation.DuckStation", "org.libretro.RetroArch",
            "org.mozilla.Thunderbird", "net.sf.VICE", "net.fsuae.FS-UAE", "org.audacityteam.Audacity",
            "org.gimp.GIMP", "org.gnome.Boxes", "com.transmissionbt.Transmission", "fr.handbrake.ghb"
        }
    };

    RealSystemInfo systemInfo;
    RealFileSystem fileSystem;
    RealCommandExecutor commandExecutor;

    LinuxBasix app(config, systemInfo, fileSystem, commandExecutor);
    app.run();

    return 0;
}