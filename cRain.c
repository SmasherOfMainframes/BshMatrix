#include <stdlib.h>		// strtol
#include <unistd.h>		// usleep()
#include <time.h>		// srand(time(0))
#include <stdbool.h>	// BOOL
#include <string.h>		// strcmp
#include <locale.h>		// for setting locale
#include <argp.h>		// For command line processing
#include <ncursesw/ncurses.h>

#define DEFAULT_HEAD_COLOR "white"
#define DEFAULT_TAIL_COLOR "green"

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
	bool async;
	unsigned int speed;
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

void mir_loop(struct Column* column, size_t c);
void make_it_rain(struct Column* column);
void make_it_rain_async(struct Column* column);

void set_color(char* color, short pair);

void set_speed(struct config* config);

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
		case 'a':
		{
			config->async = true;
			break;
		}
		case 's':
		{
			config->speed = atoi(arg);
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
	config.speed = 5;
	config.async = false;
	config.chset_flag = false;

	struct argp_option options[] = {
		{ 0, 'h', "Head color",	0, "Set color color." },
		{ 0, 't', "Tail color",	0, "Set tail color." },
		{ 0, 'c', "Charset",	0, "Set character set." },
		{ 0, 'a', 0,			0, "Sets droplets to fall synchronously or asynchronously." },
		{ 0, 's', "Speed",		0, "How fast columns descend, from 0 - 10." },
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

	set_color(config.col_hd, 1);
	set_color(config.col_tl, 2);
	set_speed(&config);
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
	if(config.async){
		while( !(is_keypressed()) ){
			make_it_rain_async(columns);
			refresh();
			usleep(config.speed);
		}
	} else {
		while( !(is_keypressed()) ){
			make_it_rain(columns);
			refresh();
			usleep(config.speed);
		}
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
	column->speed		= rand()%4 + 3;
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

void mir_loop(struct Column* column, size_t c){
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
void make_it_rain_async(struct Column* column){
	for(size_t c = 0; c < COLS; c++){
		column[c].tick++;

		if(column[c].tick == column[c].speed) {
			column[c].index++;
			column[c].tick = 0;

			mir_loop(column, c);
		}
	}
}
void make_it_rain(struct Column* column){
	for(size_t c = 0; c < COLS; c++){
		column[c].index++;

		mir_loop(column, c);
	}

}

void set_speed(struct config* config){
	int n = 10 - config->speed;
	// Magic numbers, they give a nice range on my machine so that's
	// what you're getting.
	if(config->async){
		config->speed = 3000 + (n * 800);
	} else {
		config->speed = 13000 + (n * 3400);
	}
}

void set_color(char* color, short pair){
	for(int i = 0; color[i]; i++){
		color[i] = tolower(color[i]);
	}

	if(strcmp(color, "white") == 0){
		init_pair(pair, 15, -1);
	} else if(strcmp(color, "black") == 0){
		init_pair(pair, 0, -1);
	}else if(strcmp(color, "green") == 0){
		init_pair(pair, 10, -1);
	}else if(strcmp(color, "yellow") == 0){
		init_pair(pair, 11, -1);
	}else if(strcmp(color, "blue") == 0){
		init_pair(pair, 12, -1);
	}else if(strcmp(color, "magenta") == 0){
		init_pair(pair, 13, -1);
	}else if(strcmp(color, "cyan") == 0){
		init_pair(pair, 14, -1);
	}else if(strcmp(color, "red") == 0){
		init_pair(pair, 9, -1);
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