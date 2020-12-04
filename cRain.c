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

/* TODO:
 * 0. Why the fuck are we looping through each cell and performing all those checks?????
 *      We should be keeping track of where the column is, then modifying that column...
 * 1. Check for ~/.config/cRain folder, where charsets and config will live.
 *
 * HOW DOES THIS TRASH WORK???
 *
 * The Matrix is mainly built up of structs called Column. Each Column struct holds
 * data on the column of the matrix. 
 * 
 * Every loop of the while loop in int main, each columns TICK attribute 
 * increases by one. Once TICK reaches the value of SPEED, the INDEX attribute
 * increases by one. So, SPEED determines how many ticks it takes to increase
 * INDEX. 
 *
 * Every time INDEX is incremented, the top row of The Matrix is fed a new character.
 * What character appears depends on whether the IS_BLANK attribute is true or false.
 * If true, then every time INDEX is increased, a " " or int value 32 is added to the
 * Matrix. If false, then a random value (1 <= n < charset size) is added to the head of the 
 * falling string, or, if INDEX is currently less than the value of PADDING, a " " is added.
 * 
 * Padding refers to how many leading zeroes will be added before the regular 
 * characters are added. This prevents long, continuous strings from displaying.
 *
 * Also, whenever INDEX is incremented, a new value is added to the bottom of the droplet,
 * and the top value is replaced with " ". This keeps values in place while the droplet falls.
 *
 * This continues until INDEX reaches the value LENGTH. When this happens, the Column
 * struct is reshuffled with new, randomized values and the current droplet will continue
 * to fall down while a new one starts at the top of the column.
 *
 * Finally, a random number of non-" " characters are selected to be changed to a new
 * value, to give the falling strings some fun randomness.
 * 
 * UPDATED : Sept 20 2020, 5am
 *
 */

/* --------------------------------------------------
-------------------- MR. WORLD WIDE -----------------
-------------------------------------------------- */

// Data struct for each column
struct Column{
	unsigned int speed;		// How many ticks until the droplet falls
	int tick;				// Keeps track of time for each column
	int index;				// How many times has tick reached index
	int padding;			// How many leading " " are in the string
	int length;				// Determines the length of the "droplet" string
	bool is_blank;			// Is the "droplet" string blank or regular?
};

// Data struct of The Matrix, which holds value
// This should be replaced by a simple matrix, it used
// to hold color data aswell. But alas, i don't wanna.
struct Matrix{
	int val;
};

char charset[256][4] = {
	" ", "a", "b", "c", "d", 
	"e", "f", "g", "h", "i", 
	"j", "k", "l", "m", "n", 
	"o", "p", "q", "r", "s",
	"t", "u", "v", "w", "x",
	"y", "z", 
	"A", "B", "C", "E", 
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

struct config {
	char col_hd[16];
	char col_tl[16];
	char chset_name[64];
	bool chset_flag;
	unsigned int delay;
};


/* --------------------------------------------------
---------------------- PROTOTYPES -------------------
-------------------------------------------------- */

void set_col(struct Column* col, int rows);

void set_all_cols(struct Column* column, int cols, int rows);

void move_cols(struct Column* column, struct Matrix* matrix, int cols, int rows);

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

	}
	return 0;
}

/* --------------------------------------------------
------------------------- MAIN ----------------------
-------------------------------------------------- */

int main(int argc, char* argv[]){
	// Needed for unicode support. MUST BE FIRST.
	setlocale(LC_ALL, "en_CA.UTF-8");

	// ----- TESTY ----- //

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

	// Using built in ncurses function to get the terminal size,
	// which is needed for program logic.
	int COLS;
	int ROWS;
	getmaxyx(stdscr, ROWS, COLS);

	// Array of Column structs, used to hold column-specific data
	struct Column columns[COLS];
	// Initialize the starting values of each column.
	set_all_cols(columns, COLS, ROWS);

	// THE MATRIX
	struct Matrix thematrix[COLS][ROWS];
	// Set the matrix to all " "
	for(size_t i = 0; i < ROWS; i++){
		for(size_t j = 0; j < COLS; j++){
			thematrix[j][i].val = 0;
		}
	}

	// ------ MAIN LOOP -------- //
	while( !(is_keypressed()) ){
		move_cols(columns, thematrix[0], COLS, ROWS);
		refresh();
		usleep(config.delay);
	}

	// ----- Goodbye ----- //
	endwin();
	return 0;
}

/* --------------------------------------------------
---------------------- FONCTIONS --------------------
-------------------------------------------------- */

void set_col(struct Column* column, int rows){
	column->speed		= rand()%5 + 10;
	column->tick		= 0;
	column->index		= 0;
	column->length 		= rand()%(int)(0.8*rows) + 3;
	column->padding		= rand()%(int)(0.5*rows) + 3;
	column->is_blank 	= (rand()%10 < 6) ? true : false;
}

void set_all_cols(struct Column* column, int cols, int rows){
	for(size_t i = 0; i < cols; i++){
		set_col(&column[i], rows);
	}
}

void move_cols(struct Column* column, struct Matrix* matrix, int cols, int rows){
	// why the fuck didnt you set var to c????????
	// This function should be cleaned up and broken into smaller bits.
	// Unlce Bob would bitch slap you for this.
	for(size_t i = 0; i < cols; i++){
		
		column[i].tick++;

		if(column[i].tick == column[i].speed){
			column[i].index++;
			column[i].tick = 0;

			// Add a new random val to top of column
			// Second value doesn't really matter as long as it's a valid ascii character.
			(matrix + i*rows + 0)->val = (column[i].is_blank || column[i].index <= column[i].padding) ? 0 : (rand()%(charset_len)) + 1;
			mvprintw(0, i, "%s", charset[(matrix + i*rows + 0)->val]);
			
			// Move the column down one position
			for(size_t r = rows; r > 1; r--){
				
				bool current_blank = ((matrix + i*rows + r-1)->val == 0) ? true : false;
				bool above_blank = ((matrix + i*rows + r-2)->val == 0) ? true : false;
				
				if(current_blank && !(above_blank)){
					// Adds a new random character one below the falling string, then prints
					(matrix + i*rows + r-1)->val = (rand()%(charset_len)) + 1;
					attron(COLOR_PAIR(1));
					mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);

					attron(COLOR_PAIR(2));
					mvprintw(r-2, i, "%s", charset[(matrix + i*rows + r-2)->val]);
				
				// Sets last character of string to " "
				} else if(!(current_blank) && above_blank){
					(matrix + i*rows + r-1)->val = 0;
					mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);
				
				// Changes some of the non-blank values to a new value
				// for some added randomness.
				} else if(!(current_blank)){
					if(rand()%10 == 0){
						(matrix + i*rows + r-1)->val = (rand()%(charset_len)) + 1;
						attron(COLOR_PAIR(2));
						mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);
					}
				}
				
				// Prevents head from staying head color when it reaches the bottom
				if(r == rows){
					attron(COLOR_PAIR(2));
					mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);
				}
			}

			// Once we reach the end, reshuffle the column with new values
			if(column[i].index == column[i].padding + column[i].length){
				set_col(&column[i], rows);
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
