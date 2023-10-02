
#include <stdlib.h>
#include <ncurses.h>

#define WIDTH 50
#define HEIGHT 10

int startx = 0;
int starty = 0;

char *choices[] = { 
			"Install 1password (via AgileBits repo)",
			"Install basics with apt (incl. flatpak)",
			"Install basics with flatpak",
			"Install additional fonts",
			"Show and preselect apt packages",
			"Show and preselect flatpak packages",
			"Exit",
		  };

struct apt_package {
	char p_name[30];
	int p_status;
}apts;

struct flat_package {
	char f_name[30];
	int f_status;
}flats;


int n_choices = sizeof(choices) / sizeof(char *);
void print_menu(WINDOW *menu_win, int highlight);

int main()
{	
	int highlight = 1;

menu_start:	

	WINDOW *menu_win;
	WINDOW *select_win;

	int choice = 0;
	int c;

	initscr();
	start_color();
	init_pair(1, COLOR_BLUE, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	clear();
	noecho();
	cbreak();	/* Line buffering disabled. pass on everything */
	startx = (80 - WIDTH) / 3;
	starty = (24 - HEIGHT) / 3;
		
	menu_win = newwin(HEIGHT, WIDTH, starty, startx);

	keypad(menu_win, TRUE);
	attron(COLOR_PAIR(1) | A_REVERSE | A_BOLD);
	mvprintw(0, 0, "LinuxBasics 1.1 -- Use arrow keys to go up and down, press enter to select...");
	attroff(COLOR_PAIR(1));
	refresh();
	
	print_menu(menu_win, highlight);
	while(1)
	{	c = wgetch(menu_win);
		switch(c)
		{	case KEY_UP:
				if(highlight == 1)
					highlight = n_choices;
				else
					--highlight;
				break;
			case KEY_DOWN:
				if(highlight == n_choices)
					highlight = 1;
				else 
					++highlight;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				refresh();
				break;
		}
		print_menu(menu_win, highlight);
		if(choice != 0)	/* User did a choice come out of the infinite loop */
			break;
	}	
	// mvwprintw(shell_win, 1, 1, "You chose choice %d with choice string %s\n", choice, choices[choice - 1]);
	// wrefresh(shell_win);
	// getch();
	// clrtoeol();
	// refresh();
	// endwin();

	switch (choice) {
		case 1:
			endwin();
			system("clear && printf '1password install...\n'");
			system("sudo apt update");
			system("sudo apt install curl");
			system("curl -sS https://downloads.1password.com/linux/keys/1password.asc | sudo gpg --dearmor --output /usr/share/keyrings/1password-archive-keyring.gpg");
			system("echo 'deb [arch=amd64 signed-by=/usr/share/keyrings/1password-archive-keyring.gpg] https://downloads.1password.com/linux/debian/amd64 stable main' | sudo tee /etc/apt/sources.list.d/1password.list");
			system("sudo mkdir -p /etc/debsig/policies/AC2D62742012EA22/");
			system("curl -sS https://downloads.1password.com/linux/debian/debsig/1password.pol | sudo tee /etc/debsig/policies/AC2D62742012EA22/1password.pol");
			system("sudo mkdir -p /usr/share/debsig/keyrings/AC2D62742012EA22");
			system("curl -sS https://downloads.1password.com/linux/keys/1password.asc | sudo gpg --dearmor --output /usr/share/debsig/keyrings/AC2D62742012EA22/debsig.gpg");
			system("sudo apt update && printf '\n' && sudo apt install 1password");
			break;
		case 2:
			endwin();
			system("clear && printf 'APT install...\n\n'");
			system("sudo apt update && printf '\n'");
			system("sudo apt install htop mc gdu neovim tilix zip unzip fortune-mod build-essential flatpak neofetch cpufetch preload nala gnome-software-plugin-flatpak");
			system("flatpak remote-add --if-not-exists flathub https://dl.flathub.org/repo/flathub.flatpakrepo");
			break;
		case 3:
			endwin();
			system("clear && printf 'Flatpak install...\n\n'");
			system("flatpak install ca.desrt.dconf-editor com.github.tchx84.Flatseal com.discordapp.Discord com.ktechpit.colorwall com.mattjakeman.ExtensionManager com.microsoft.Edge com.valvesoftware.Steam com.visualstudio.code net.cozic.joplin_desktop net.lutris.Lutris org.DolphinEmu.dolphin-emu org.duckstation.DuckStation org.libretro.RetroArch org.mozilla.Thunderbird net.sf.VICE net.fsuae.FS-UAE");
			break;
		case 4:
			endwin();
			system("clear && printf 'Installing additional fonts...\n\n'");
			system("wget https://github.com/source-foundry/Hack/releases/download/v3.003/Hack-v3.003-ttf.zip");
			system("wget https://download.jetbrains.com/fonts/JetBrainsMono-1.0.3.zip");
			system("for i in *.zip; do unzip \"$i\" -d ~/.local/share/fonts; done");
			system("rm *.zip");
			system("fc-cache -r -v");
			break;
		case 5:
			select_win = newwin(25,40,3,35);
			box(select_win, 0, 0);
			wrefresh(select_win);
			break;
		case 6:
			select_win = newwin(25,40,3,35); 
			box(select_win, 0, 0);
			wrefresh(select_win);
			break;
		default:
			// refresh();
			endwin();
			return 0;
	}
	
	printf("\nPress ENTER to continue...\n");
	getchar();

	goto menu_start;
}


void print_menu(WINDOW *menu_win, int highlight)
{
	int x, y, i;	

	x = 2;
	y = 2;
	box(menu_win, 0, 0);
	for(i = 0; i < n_choices; ++i)
	{	if(highlight == i + 1) /* High light the present choice */
		{	wattron(menu_win, COLOR_PAIR(2) | A_REVERSE | A_BOLD); 
			mvwprintw(menu_win, y, x, "%s", choices[i]);
			wattroff(menu_win, COLOR_PAIR(2) | A_REVERSE | A_BOLD);
		}
		else
			mvwprintw(menu_win, y, x, "%s", choices[i]);
		++y;
	}
	wrefresh(menu_win);
}
