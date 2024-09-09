/*
 * LinuxBasix -- Copyright (c) 2024, Dirk Steiger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * For more information, please visit: https://github.com/Usires
 */

#include <cstdio>
#include <ncurses.h>
#include <clocale>
#include <cstdlib>
#include <utility>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <fstream>
#include <sys/utsname.h>

using namespace std;

// Configuration structure
struct Configuration
{
    vector<string> main_menu_options;
    vector<string> programs_to_install;
    vector<string> flatpak_programs_to_install;
};

// Abstract classes for better testability
class SystemInfo
{
public:
    virtual string getKernelVersion() = 0;
    virtual vector<string> checkPackageManagers() = 0;
    virtual ~SystemInfo() = default;
};

class FileSystem
{
public:
    virtual bool appendToFile(const string& filename, const vector<string>& lines) = 0;
    virtual ~FileSystem() = default;
};

class CommandExecutor
{
public:
    virtual void execute(const vector<string>& command) = 0;
    virtual ~CommandExecutor() = default;
};

// Concrete implementations
class RealSystemInfo final : public SystemInfo
{
public:
    string getKernelVersion() override
    {
        utsname buffer{};
        if (uname(&buffer) != 0)
        {
            return {"Unknown"};
        }
        return {buffer.release};
    }

    vector<string> checkPackageManagers() override
    {
        vector<string> packageManagers = {"apt", "pacman", "yum", "dnf", "zypper", "snap"};
        vector<string> availablePackageManagers;

        for (const auto& manager : packageManagers)
        {
            if (commandExists(manager))
            {
                availablePackageManagers.push_back(manager);
            }
        }
        return availablePackageManagers;
    }

private:
    bool static commandExists(const string& command)
    {
        const string path = "/usr/bin/" + command;
        const ifstream file(path.c_str());
        return file.good();
    }
};

class RealFileSystem final : public FileSystem
{
public:
    bool appendToFile(const string& filename, const vector<string>& lines) override
    {
        ofstream file(filename, ios_base::app);
        if (!file.is_open())
        {
            return false;
        }
        for (const auto& line : lines)
        {
            file << line << "\n";
        }
        return true;
    }
};

class RealCommandExecutor final : public CommandExecutor
{
public:
    void execute(const vector<string>& command) override
    {
        vector<char*> args;
        args.reserve(command.size());

        for (const auto& arg : command)
        {
            args.push_back(const_cast<char*>(arg.c_str()));
        }
        args.push_back(nullptr);

        if (const pid_t pid = fork(); pid == 0)
        {
            execvp(args[0], args.data());
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("fork");
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
            {
                cerr << "Command " << command[0] << " failed with return code " << WEXITSTATUS(status) << "\n";
            }
        }
    }
};

string join(const vector<string>& vec, const string& delimiter)
{
    string result;
    if (vec.empty()) result = "None";

    for (size_t i = 0; i < vec.size(); ++i)
    {
        if (i > 0) result += delimiter;
        result += vec[i];
    }
    return result;
}

// LinuxBasix class
class LinuxBasix
{
    Configuration config;
    SystemInfo& systemInfo;
    FileSystem& fileSystem;
    CommandExecutor& commandExecutor;
    set<string> selected_flatpak_programs;
    set<string> selected_apt_programs;
    set<string> selected_package_manager;
    vector<string> user_added_programs;

public:
    LinuxBasix(Configuration  cfg, SystemInfo& si, FileSystem& fs, CommandExecutor& ce)
        : config(move(cfg)), systemInfo(si), fileSystem(fs), commandExecutor(ce)
    {
        selected_flatpak_programs = set(config.flatpak_programs_to_install.begin(),
                                                config.flatpak_programs_to_install.end());
        selected_apt_programs = set(config.programs_to_install.begin(), config.programs_to_install.end());
    }

    void run()
    {
        initscr();
        cbreak();
        noecho();
        curs_set(0); // Cursor not visible
        setlocale(LC_ALL, "UTF8");
        start_color();

        init_pair(1, COLOR_WHITE, COLOR_BLUE); // Main window
        init_pair(2, COLOR_WHITE, COLOR_RED); // Sub menu set #1
        init_pair(3, COLOR_BLACK, COLOR_BLACK); // Shadow color
        init_pair(4, COLOR_WHITE, COLOR_MAGENTA); // Sub menu set #2
        init_pair(5, COLOR_WHITE, COLOR_GREEN); // Sub menu set #3

        WINDOW* stdscr = initscr();
        keypad(stdscr, TRUE);

        main_menu(stdscr);

        endwin();
    }

private:
    void main_menu(WINDOW* stdscr)
    {
        int highlight_main = 1;
        const auto MAIN_MENU_ITEMS = static_cast<int8_t>(config.main_menu_options.size());

        while (true)
        {
            display_main_menu(stdscr, highlight_main);
            wrefresh(stdscr);

            switch (wgetch(stdscr))
            {
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

    void display_main_menu(WINDOW* stdscr, int highlight) const;

    void add_custom_programs(const WINDOW* stdscr)
    {
        int height, width;
        getmaxyx(stdscr, height, width);

        constexpr int win_height = 20;
        constexpr int win_width = 60;
        const int start_y = (height - win_height) / 2;
        const int start_x = (width - win_width) / 2;

        WINDOW* popup = newwin(win_height, win_width, start_y, start_x);
        WINDOW* shadow = newwin(win_height, win_width, start_y + 1, start_x + 2);

        wbkgd(shadow, COLOR_PAIR(3));
        wrefresh(shadow);

        keypad(popup, TRUE);
        wbkgd(popup, COLOR_PAIR(2));
        box(popup, 0, 0);

        wattron(popup, A_BOLD);
        mvwprintw(popup, 1, 2, "Add custom programs for repo installation (max 20)");
        mvwprintw(popup, 2, 2, "Enter program name (':q' = quit, ':c' = clear list):");
        wattroff(popup, A_BOLD);

        int input_y = 5;
        mvwhline(popup, input_y, 2, '_', win_width - 4); // Dotted line to type on

        while (user_added_programs.size() < 21)
        {
            char program[50] = {};
            wattron(popup, A_BOLD);
            mvwprintw(popup, input_y, 2, "[%lu]  ", user_added_programs.size() + 1);
            wattroff(popup, A_BOLD);

            echo();
            curs_set(1); // Make cursor visible
            mvwgetstr(popup, input_y, 7, program);
            curs_set(0); // Hide cursor
            noecho();

            string prog_str(program);
            if (prog_str == ":q") break;
            if (prog_str == ":c")
            {
                user_added_programs.clear();
                break;
            }

            if (!prog_str.empty())
            {
                user_added_programs.push_back(prog_str);
                input_y += 2;
                if (input_y >= win_height - 3) input_y = 5;
            }
            mvwhline(popup, input_y, 2, '_', win_width - 4); // Dotted line to type on
        }

        delwin(shadow);
        delwin(popup);
    }

    void handle_menu_selection(WINDOW* stdscr, const int highlight_main)
    {
        if (highlight_main == static_cast<int>(config.main_menu_options.size()))
        {
            endwin();
            exit(0); // Exit the program
        }

        switch (highlight_main)
        {
        case 1:
            select_programs(stdscr, config.programs_to_install, selected_apt_programs, 2, "packages");
            break;
        case 3:
            add_custom_programs(stdscr);
            break;
        case 4:
            select_programs(stdscr, config.flatpak_programs_to_install, selected_flatpak_programs, 4, "Flatpaks");
            break;
        case 9:
            select_programs(stdscr, systemInfo.checkPackageManagers(), selected_package_manager, 5, "package manager");
            break;
        case 10:
            append_to_bashrc_and_edit();
            break;
        default:
            execute_code_block(stdscr, highlight_main);
            break;
        }
    }

    void static select_programs(const WINDOW* stdscr, const vector<string>& programs_to_sort,
                                set<string>& selected_programs,
                                const int menu_color, const string& program_type)
    {
        vector<string> sorted_programs = programs_to_sort;
        sort(sorted_programs.begin(), sorted_programs.end());

        int height, width;
        getmaxyx(stdscr, height, width);
        const int win_height = min(static_cast<int>(sorted_programs.size()) + 6, height - 2);
        const int win_width = min(max(static_cast<int>(max_element(sorted_programs.begin(), sorted_programs.end(),
                                                                   [](const string& a, const string& b)
                                                                   {
                                                                       return a.size() < b.size();
                                                                   })->size()) + 10, 50), width - 2);

        const int start_y = (height - win_height) / 2;
        const int start_x = (width - win_width) / 2;

        WINDOW* win = newwin(win_height, win_width, start_y, start_x);
        WINDOW* shadow_win = newwin(win_height, win_width, start_y + 1, start_x + 2);

        // Draw shadow
        wattron(shadow_win, COLOR_PAIR(3));
        wbkgd(shadow_win, COLOR_PAIR(3));
        for (int i = 0; i < win_height; ++i)
        {
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
        const int max_display = win_height - 4;

        while (true)
        {
            for (int i = 0; i < max_display && i + start_idx < static_cast<int>(sorted_programs.size()); ++i)
            {
                if (i + start_idx == highlight)
                {
                    wattron(win, A_REVERSE);
                }
                string display_str = (selected_programs.count(sorted_programs[i + start_idx]) ? "[+] " : "[ ] ") +
                    sorted_programs[i + start_idx];
                mvwprintw(win, i + 3, 2, "%-*s", win_width - 4, display_str.c_str());
                wattroff(win, A_REVERSE);
            }

            mvwprintw(win, win_height - 1, 1, "Space: select/unselect, Enter: confirm, q: quit");
            wrefresh(win);

            switch (wgetch(win))
            {
            case KEY_UP:
                if (highlight > 0)
                {
                    --highlight;
                    if (highlight < start_idx) --start_idx;
                }
                break;
            case KEY_DOWN:
                if (highlight < static_cast<int>(sorted_programs.size()) - 1)
                {
                    ++highlight;
                    if (highlight >= start_idx + max_display) ++start_idx;
                }
                break;
            case ' ':
                {
                    if (const string& program = sorted_programs[highlight]; selected_programs.count(program))
                    {
                        selected_programs.erase(program);
                    }
                    else
                    {
                        selected_programs.insert(program);
                    }
                }
                break;
            case 10: // Enter key
            case 'q':
                delwin(shadow_win);
                delwin(win);
                return;
            default:
                break;
            }
        }
    }

    void execute_code_block(WINDOW* stdscr, const int option)
    {
        wclear(stdscr);
        wrefresh(stdscr);
        curs_set(2);

        vector<vector<string>> commands;
        if (option == 2)
        {
            commands = {
                {"clear"},
                {"sudo", "apt-get", "update"},
                {"sudo", "apt-get", "install", "--ignore-missing"}
            };
            commands[2].insert(commands[2].end(), selected_apt_programs.begin(), selected_apt_programs.end());
            commands[2].insert(commands[2].end(), user_added_programs.begin(), user_added_programs.end());
            // commands[2].emplace_back("--ignore-missing");

            commands.push_back({
                "flatpak", "-v", "remote-add", "--if-not-exists", "flathub",
                "https://dl.flathub.org/repo/flathub.flatpakrepo"
            });
        }
        else if (option == 5)
        {
            commands = {
                {"clear"},
                {"flatpak", "install"}
            };
            commands[1].insert(commands[1].end(), selected_flatpak_programs.begin(), selected_flatpak_programs.end());
        }
        else if (option == 6)
        {
            commands = {
                {"clear"},
                {
                    "wget", "https://downloads.1password.com/linux/debian/amd64/stable/1password-latest.deb",
                    "https://github.com/fastfetch-cli/fastfetch/releases/download/2.21.3/fastfetch-linux-amd64.deb"
                },
                {"sh", "-c", "sudo apt-get install ./1password-latest.deb ./fastfetch-linux-amd64.deb"},
                {"rm", "./1password-latest.deb", "./fastfetch-linux-amd64.deb"}
            };
        }
        else if (option == 7)
        {
            commands = {
                {"clear"},
                {"echo", "Installing SynthShell from Github.com \n\n"},
                {"git", "clone", "--recursive", "https://github.com/andresgongora/synth-shell.git"},
                {"sh", "-c", "cd ./synth-shell && ./setup.sh"}
            };
        }
        else if (option == 8)
        {
            commands = {
                {"clear"},
                {"echo", "Installing additional fonts. \n"},
                {"wget", "https://github.com/source-foundry/Hack/releases/download/v3.003/Hack-v3.003-ttf.zip"},
                {"wget", "https://download.jetbrains.com/fonts/JetBrainsMono-1.0.3.zip"},
                {"sh", "-c", R"(for i in *.zip; do unzip -u "$i" -d ~/.local/share/fonts && rm "$i"; done)"},
                {"fc-cache", "-r", "-v"}
            };
        }

        endwin();
        for (const auto& command : commands)
        {
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

    void append_to_bashrc_and_edit() const
    {
        const char* home = getenv("HOME");
        if (!home)
        {
            cerr << "Unable to get HOME directory" << endl;
            return;
        }

        const string bashrc_path = string(home) + "/.bashrc";

        const vector<string> lines_to_add = {
            "\n# Added by LinuxBasix",
            "alias ll='ls -la'",
            "alias ls='ls -l'",
            "alias cd..='cd ..'",
            "fastfetch",
            "echo ''",
            "fortune -s",
            "echo ''"
        };

        if (fileSystem.appendToFile(bashrc_path, lines_to_add))
        {
            cout << "Lines added to .bashrc successfully." << endl;
        }
        else
        {
            cerr << "Unable to open .bashrc for appending" << endl;
            return;
        }

        endwin(); // End ncurses mode temporarily

        const string command = "vim " + bashrc_path;
        if (const int result = system(command.c_str()); result == -1)
        {
            cerr << "Error: Failed to execute vim" << endl;
        }
        else if (WIFEXITED(result) && WEXITSTATUS(result) != 0)
        {
            cerr << "Warning: vim exited with status " << WEXITSTATUS(result) << endl;
        }
        cout << "Press any key to return to the main menu...";
        cin.get();

        initscr(); // Reinitialize ncurses
        cbreak();
        noecho();
        curs_set(0);
        refresh();
    }
};

void LinuxBasix::display_main_menu(WINDOW* stdscr, const int highlight) const
{
    wclear(stdscr);
    wbkgd(stdscr, COLOR_PAIR(1));

    // box(stdscr, 0, 0);

    const vector<string> ASCII_ART = {
        " _     _                 ______           _      ",
        "| |   (_)                | ___ \\         (_)     ",
        "| |    _ _ __  _   ___  _| |_/ / __ _ ___ ___  __",
        R"(| |   | | '_ \| | | \ \/ | ___ \/ _` / __| \ \/ /)",
        "| |___| | | | | |_| |>  <| |_/ | (_| \\__ | |>  < ",
        R"(\_____|_|_| |_|\__,_/_/\_\____/ \__,_|___|_/_/\_\)"
    };

    const string program_name = "Version 2.42-240904";

    attron(A_BOLD);

    for (size_t i = 0; i < ASCII_ART.size(); ++i) {
        mvprintw(static_cast<int>(i) + 1, 2, "%s", ASCII_ART[i].c_str());
    }
    mvwprintw(stdscr, 6, 52, "%s", program_name.c_str());

    mvwprintw(stdscr, 8, 2, "MAIN MENU");
    attroff(A_BOLD);

    for (size_t i = 0; i < config.main_menu_options.size(); ++i)
    {
        if (i + 1 == highlight)
        {
            wattron(stdscr, A_REVERSE);
        }

        const char letter = i < 26 ? static_cast<char>('A' + static_cast<int>(i)) : '?';

        if (i == config.main_menu_options.size() - 1)
        {
            // If it's the last item, print it on a line one line apart from the rest
            mvwprintw(stdscr, static_cast<int>(11 + i), 5, "%c.   %s", letter, config.main_menu_options[i].c_str());
        }
        else
        {
            // For all other items, print normally
            mvwprintw(stdscr, static_cast<int>(10 + i), 5, "%c.   %s", letter, config.main_menu_options[i].c_str());
        }
        wattroff(stdscr, A_REVERSE);
    }

    const int height(getmaxy(stdscr));

    // Get the kernel version
    string kernelVersion = systemInfo.getKernelVersion();

    // Check for available package managers
    const vector<string> availablePackageManagers = systemInfo.checkPackageManagers();

    const string version_info = "Uses ncurses library " + string(NCURSES_VERSION) +
        ", (c) 1993-2024 Free Software Foundation, Inc.";
    const string copyright_text = "(c) 2024 github.com/Usires. Made in C++ with support of Claude 3.5 and ChatGPT-4o";
    // string packer_text = "Packed with UPX 3.96, (c) 1996-2020 by Markus Oberhumer, Laszlo Molnar & John Reiser";
    const string kernel = "Current Linux Kernel version: " + string(kernelVersion);
    const string packetmanagers = "Detected packet managers (* = selected): " + join(availablePackageManagers, " | ");
    const string customprograms = "Manually added repo packages: " + join(user_added_programs, " | ");

    // mvwprintw(stdscr, height - 2, 2, "%s", packer_text.c_str());
    mvwprintw(stdscr, height - 3, 2, "%s", version_info.c_str());
    mvwprintw(stdscr, height - 4, 2, "%s", copyright_text.c_str());
    attron(A_BOLD);
    mvwprintw(stdscr, height - 6, 2, "%s", kernel.c_str());
    mvwprintw(stdscr, height - 7, 2, "%s", packetmanagers.c_str());
    mvwprintw(stdscr, height - 8, 2, "%s", customprograms.c_str());

    attroff(A_BOLD);
}

int main() // int argc, char* argv[] -- for future cli options, not implemented yet
{
    const Configuration config = {
        // main_menu_options
        {
            "Select original repo packages",
            "Install original repo packages",
            "Add repo packages manually",
            "Select Flatpak packages",
            "Install Flatpak packages",
            "Install 1Password (latest) and Fastfetch (v2.21.3)",
            "Install SynthShell scripts (cloning from Github.com)",
            "Install additional fonts (JetBrains Mono / Hack)",
            "Select package manager for repo packages",
            "Add startup items to ~/.bashrc (with check in Nvim)",
            "Add padding for GTK 3.0/4.0 terminal emulators (CSS patch, 10 pixels)",
            "Exit (or press 'Q')"
        },
        // programs_to_install
        {
            "curl", "git", "neovim", "htop", "tilix", "gdu", "nala", "mc",
            "zip", "unzip", "fortune-mod", "build-essential", "flatpak", "preload",
            "cmatrix", "cool-retro-term", "powertop", "upx-ucl", "fonts-powerline"
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
