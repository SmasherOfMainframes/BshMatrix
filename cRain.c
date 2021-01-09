#include <stdlib.h>		// strtol
#include <unistd.h>		// usleep()
#include <time.h>		// srand(time(0))
#include <stdbool.h>	// BOOL
#include <string.h>		// strcmp
#include <locale.h>		// for setting locale
#include <argp.h>		// For command line processing
#include <ncursesw/ncurses.h>

#define DEFAULT_HEAD_COLOR "WHITE"
#define DEFAULT_TAIL_COLOR "GREEN"

/* --------------------------------------------------
-------------------- MR. WORLD WIDE -----------------
-------------------------------------------------- */

//FILE* dbg;

// Num rows and columns, set in main().
int COLS;
int ROWS;

// Data struct for each column
struct Column{
	unsigned int speed;		// How many ticks until the droplet falls
	int tick;				// Keeps track of time for each column
	int index;				// How many times has tick reached index
	int padding;			// How many leading " " are in the string
	int length;				// Determines the length of the "droplet" string
	bool is_blank;			// Is the "droplet" string blank or regular?

	struct Droplet* bottom;
	struct Droplet* top;
};

struct Droplet {
	int tail;
	int head;
	int length;
	char prev_char[4];
	bool is_blank;
	struct Droplet* next;
};

struct config {
	char col_hd[16];
	char col_tl[16];
	char chset_name[64];
	bool chset_flag;
	unsigned int delay;
};

char charset[256][4] = {
	" ", "a", "b", "c", "d", 
	"e", "f", "g", "h", "i", 
	"j", "k", "l", "m", "n", 
	"o", "p", "q", "r", "s",
	"t", "u", "v", "w", "x",
	"y", "z", 
	"A", "B", "C", "D",
	"E", "F", "G", "H", "I", 
	"J", "K", "L", "M", "N", 
	"O", "P", "Q", "R", "S",
	"T", "U", "V", "W", "X",
	"Y", "Z",
	"0", "1", "2", "3", "4",
	"5", "6", "7", "8", "9", 
	"!", "@", "#", "$", "%", 
	"^", "&", "*"
};
int charset_len = 71;

/* --------------------------------------------------
---------------------- PROTOTYPES -------------------
-------------------------------------------------- */

void set_column(struct Column* col);

void init_columns(struct Column* column);

void init_droplet(struct Droplet* droplet, struct Droplet* next, struct Column* column);

void make_it_rain(struct Column* column);

void set_h_color(char* head);
void set_t_color(char* tail);

void set_charset(char* name);

int is_keypressed();

/* --------------------------------------------------
------------------------- ARGP ----------------------
-------------------------------------------------- */

static int parse_opt (int key, char* arg, struct argp_state* state) {
	struct config* config = state->input;
	switch (key){
		case 'h':
		{
			strncpy(config->col_hd, arg, 16);
			break;
			
		}
		case 't':
		{
			strncpy(config->col_tl, arg, 16);
			break;
		}
		case 'c':
		{
			strncpy(config->chset_name, arg, 64);
			config->chset_flag = true;
			break;
		}
		case 'd':
		{
			config->delay = atoi(arg);
			break;
		}
		default :
			break;

	}
	return 0;
}

/* --------------------------------------------------
------------------------- MAIN ----------------------
-------------------------------------------------- */

int main(int argc, char* argv[]){
	// Needed for unicode support. MUST BE FIRST.
	setlocale(LC_ALL, "en_CA.UTF-8");

	// ----- ARGP ----- //

	struct config config;
	strncpy(config.col_hd, DEFAULT_HEAD_COLOR, 16);
	strncpy(config.col_tl, DEFAULT_TAIL_COLOR, 16);
	config.delay = 2500;
	config.chset_flag = false;

	struct argp_option options[] = {
		{ 0, 'h', "Head color", 0, "Set head color." },
		{ 0, 't', "Tail color", 0, "Set tail color." },
		{ 0, 'c', "Charset", 0, "Set character set." },
		{ 0, 'd', "Delay", 0, "How fast columns descend. Reccomended 2000 - 4000" },
		{ 0 }
	};
	struct argp argp = { options, parse_opt, 0, 0 };
	argp_parse(&argp, argc, argv, 0, 0, &config);

	// ----- NCURSE INIT ----- //

	initscr();	
	start_color();
	use_default_colors();	// allows transparent background
	curs_set(0);			// turns off cursor
	nodelay(stdscr, TRUE);	// Turns getch() into a non-blocking call, allows key press to quit

	srand(time(0));

	// ----- CONFIG SETUP ----- //

	set_h_color(config.col_hd);
	set_t_color(config.col_tl);
	if(config.chset_flag) {
		set_charset(config.chset_name);
	}

	// Sets ROWS and COLS to size of terminal.
	getmaxyx(stdscr, ROWS, COLS);

	// Array of Column structs, used to hold column-specific data
	struct Column columns[COLS];
	// Initialize the starting values of each column.
	init_columns(columns);

//	dbg = fopen("/home/smigii/Code/projects/cRain/debug.txt", "w");

	// ------ MAIN LOOP -------- //
	while( !(is_keypressed()) ){
		make_it_rain(columns);
		refresh();
		usleep(config.delay);
	}

	// ----- Goodbye ----- //
	endwin();
//	fclose(dbg);
	return 0;
}

/* --------------------------------------------------
---------------------- FONCTIONS --------------------
-------------------------------------------------- */

void set_column(struct Column* column){
	column->speed		= rand()%5 + 10;
	column->tick		= 0;
	column->index		= 0;
	column->length 		= rand()%(int)(0.8*ROWS) + 3;
	column->padding		= rand()%(int)(0.5*ROWS) + 3;
	column->is_blank 	= (rand()%10 < 6) ? true : false;
}

void init_columns(struct Column* column){
	for(size_t i = 0; i < COLS; i++){
		set_column(&column[i]);

		column[i].bottom = (struct Droplet*)malloc(sizeof(struct Droplet));
		column[i].top = column[i].bottom;

		init_droplet(column[i].bottom, NULL, &column[i]);
	}
}

void init_droplet(struct Droplet* droplet, struct Droplet* next, struct Column* column){
	droplet->length = column->length;
	droplet->is_blank = column->is_blank;
	droplet->head = -1;
	droplet->tail = -1;
	droplet->next = next;
}

void make_it_rain(struct Column* column){
	for(size_t c = 0; c < COLS; c++){
		column[c].tick++;

		if(column[c].tick == column[c].speed) {
			column[c].index++;
			column[c].tick = 0;

			struct Droplet* droplet = column[c].bottom;
			while(droplet != NULL){

				// Move the head down one position, unless it would fall off the screen.
				if(droplet->head+1 < ROWS){
					droplet->head++;
					int head = droplet->head;

					// Unfortunately, there does not seem to be a way to just change the color of
					// a specific cell with ncurses, so we have to reprint the whole character and
					// adjust the color then. :(
					if(head > 0) {
						attron(COLOR_PAIR(2));
						mvprintw(head - 1, c, "%s", droplet->prev_char);
					}
					char str[4] = " ";
					if(!droplet->is_blank)
						strncpy(str, charset[(rand()%(charset_len)) + 1], 4);
					strncpy(droplet->prev_char, str, 4);

					// Sets color to Head Color, unless droplet reaches the bottom
					(droplet->head + 1 >= ROWS) ? attron(COLOR_PAIR(2)) : attron(COLOR_PAIR(1));
					mvprintw(head, c, "%s", str);
				}

				// Move the tail down one position and clear the character at that position,
				// unless it would fall off the screen. If it does, then it's time to delete the Droplet.
				if(droplet->head >= droplet->length && droplet->tail+1 < ROWS){
					droplet->tail++;
					mvprintw(droplet->tail, c, " ");
					// End
					droplet = droplet->next;
				} else if(droplet->head < droplet->length && droplet->tail+1 < ROWS){
					droplet = droplet->next;
				} else {
					// Reassign current droplet -> free old bottom -> set bottom to reassigned current droplet.
					droplet = droplet->next;
					free(column[c].bottom);
					column[c].bottom = droplet;
				}

			}

			// Reshuffle Column attributes
			if(column[c].index == column[c].length + column[c].padding){
				set_column(&column[c]);
				column[c].top->next = (struct Droplet*)malloc(sizeof(struct Droplet));
				column[c].top = column[c].top->next;
				init_droplet(column[c].top, NULL, &column[c]);
			}
		}
	}
}

void set_h_color(char* head){
	if(strcmp(head, "WHITE") == 0){
		init_pair(1, 15, -1);
	} else if(strcmp(head, "BLACK") == 0){
		init_pair(1, 0, -1);
	}else if(strcmp(head, "GREEN") == 0){
		init_pair(1, 10, -1);
	}else if(strcmp(head, "YELLOW") == 0){
		init_pair(1, 11, -1);
	}else if(strcmp(head, "BLUE") == 0){
		init_pair(1, 12, -1);
	}else if(strcmp(head, "MAGENTA") == 0){
		init_pair(1, 13, -1);
	}else if(strcmp(head, "CYAN") == 0){
		init_pair(1, 14, -1);
	}else if(strcmp(head, "RED") == 0){
		init_pair(1, 9, -1);
	}
}
void set_t_color(char* tail){
	if(strcmp(tail, "WHITE") == 0){
		init_pair(2, 7, -1);
	} else if(strcmp(tail, "BLACK") == 0){
		init_pair(2, 0, -1);
	}else if(strcmp(tail, "GREEN") == 0){
		init_pair(2, 10, -1);
	}else if(strcmp(tail, "YELLOW") == 0){
		init_pair(2, 3, -1);
	}else if(strcmp(tail, "BLUE") == 0){
		init_pair(2, 4, -1);
	}else if(strcmp(tail, "MAGENTA") == 0){
		init_pair(2, 5, -1);
	}else if(strcmp(tail, "CYAN") == 0){
		init_pair(2, 6, -1);
	}else if(strcmp(tail, "RED") == 0){
		init_pair(2, 1, -1);
	}
}

void set_charset(char* name){
	// Build the path string so we can check if the user has created the ~/.config/cRain/charsets file
	char home[] = "/home/";
	char path1[] = "/.config/cRain/charsets";
	char user[32];
	strncpy(user, getlogin(), 16);

	char path[64];
	strncat(path, home, 7);
	strncat(path, user, 32);
	strncat(path, path1, 24);

	int match_flag = 0;

	// If there is a charset file, then open er up and checky checky
	if(!access(path, R_OK)) {
		FILE *fp = fopen(path, "r");
		char buff_line[256];

		while (fgets(buff_line, 256, fp) != NULL) {
			// First we parse through the names of the charsets given in the charsets file
			// and check for a match against param name
			char buff_name[64];
			int idx = 0;
			while (buff_line[idx] != ' ') {
				buff_name[idx] = buff_line[idx];
				idx++;
			}
			buff_name[idx] = '\0';

			idx++;

			// If we have a matchy match, update the charset array.
			if (!strcmp(buff_name, name)) {
				match_flag = 1;
				// Reset the default character set
				charset_len = 1;
				charset[0][0] = ' ';
				// LOOP THROUGH ALL CHARS
				char buff_char[4];
				int i = 0;
				while (buff_line[idx - 1] != '\0') {
					if (buff_line[idx] == ' ' || buff_line[idx] == '\0') {
						buff_char[i] = '\0';
						idx++;
						strcpy(charset[charset_len], buff_char);
						charset_len++;
						i = 0;
					} else {
						buff_char[i] = buff_line[idx];
						idx++;
						i++;
					}
				}
				charset_len -= 2;    // lol why does this stabilize everything??
				break;
			}
		}
		if(!match_flag) {
			printf("Not a valid charset, fucko.\n");
			printf("Continuing with default lame charset in 3 seconds.\n");
			usleep(3000000);
		}
	}
}

int is_keypressed()
{
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}
