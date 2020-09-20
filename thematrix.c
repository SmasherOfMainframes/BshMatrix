#include <stdio.h>
#include <stdlib.h>		// strtol
#include <unistd.h>		// usleep()
#include <time.h>		// srand(time(0))
#include <stdbool.h>	// BOOL
#include <string.h>		// strcmp
#include <locale.h>		// for setting locale
#include <ncursesw/ncurses.h>

/* How does this trash work?
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
 * Matrix. If false, then a random ascii-mapped value is added to the head of the 
 * falling string, or, if INDEX is currently less than the value of PADDING, a " " or 
 * 32 is added.
 * 
 * Padding refers to how many leading zeroes will be added before the regular 
 * characters are added. This prevents long, continuous strings from displaying.
 *
 * Also, whenever INDEX is incremented, starting from the bottom of the Column, values
 * are copied down from the very bottom to the second-from-the-top row. This is what 
 * causes the strings to "fall" down the matrix.
 *
 * This continues until INDEX reaches the value LENGTH. When this happens, the Column
 * struct is reshuffled with new, randomized values.
 *
 * Finally, a random number of non-" " characters are selected to be changed to a new
 * value, to give the falling strings some fun randomness.
 *
 */

/* --------------------------------------------------
-------------------- MR. WORLD WIDE -----------------
-------------------------------------------------- */

// CHARACTERS
// This is the characterset to be printed, the first entry MUST be " ".
// The second dimension size must be able to accomadate biggest unicode 
// symbol (+1 for /0).
const char charset[][4] = {
	" ","0","1","2","3","4","5","6","7","8","9",":",".","=","*",
	"+","-","¦","|","_","ｦ","ｱ","ｳ","ｴ","ｵ","ｶ","ｷ","ｹ","ｺ","ｻ",
	"ｼ","ｽ","ｾ","ｿ","ﾀ","ﾂ","ﾃ","ﾅ","ﾆ","ﾇ","ﾈ","ﾊ","ﾋ","ﾎ","ﾏ",
	"ﾐ","ﾑ","ﾒ","ﾓ","ﾔ","ﾕ","ﾗ","ﾘ","ﾜ"
};
const int charset_len = sizeof(charset)/sizeof(charset[0]);

char HEAD_COL[7];
char TAIL_COL[7];

// Data struct for each column
struct Column{
	unsigned int speed;		// How many ticks until a letter appears
	int tick;		// Keeps track of time for each column
	int index;		// How far along length the column is
	int padding;	// How many leading " " are in the string
	int length;		// Determines the length of the "droplet" string
	bool is_blank;	// Is the "droplet" string blank or regular?
};

// Data struct of The Matrix, which holds value and color data
struct Matrix{
	int val;
	char col[7];
};

/* --------------------------------------------------
---------------------- PROTOTYPES -------------------
-------------------------------------------------- */

void set_col(struct Column* col, int rows);

void set_all_cols(struct Column* column, int cols, int rows);

void move_cols(struct Column* column, struct Matrix* matrix, int cols, int rows);

void set_color(char* head, char* tail);

/* --------------------------------------------------
------------------------- MAIN ----------------------
-------------------------------------------------- */

int main(int argc, char* argv[]){
	setlocale(LC_ALL, "en_CA.UTF-8");

	// ncurses init
	initscr();	
	start_color();
	use_default_colors();	// allows transparent background
	curs_set(0);			// turns off cursor

	// Set rand seed
	srand(time(0));

	// Set colors
	// set_color(argv[3], argv[4]);
	set_color("WHITE", "MAGENTA");

	// Sets COLS and ROWS to command line arguments. 
	// strtol(string, endpointer, base);
	// const int COLS = strtol(argv[1], NULL, 10);
	// const int ROWS = strtol(argv[2], NULL, 10);
	int COLS;
	int ROWS;
	getmaxyx(stdscr, ROWS, COLS);
	// getmaxyx(stdscr, ROWS, COLS);
	// Array of Column structs, used to get column-specific data
	struct Column columns[COLS];
	
	// THE MATRIX
	struct Matrix thematrix[COLS][ROWS];
	// Set the matrix to all " "
	for(size_t i = 0; i < ROWS; i++){
		for(size_t j = 0; j < COLS; j++){
			thematrix[j][i].val = 0;
		}

	}
	
	// Initialize the starting values of each column.
	set_all_cols(columns, COLS, ROWS);

	// ------ MAIN LOOP -------- //

	while(1){
		move_cols(columns, thematrix[0], COLS, ROWS);
		refresh();
		usleep(15000);
	}
	
	endwin();
	return 0;
}

/* --------------------------------------------------
---------------------- FUNK SHUNS -------------------
-------------------------------------------------- */

void set_col(struct Column* column, int rows){
	column->speed		= rand()%3 + 2;
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
	for(size_t i = 0; i < cols; i++){
		
		column[i].tick++;

		if(column[i].tick == column[i].speed){
			column[i].index++;
			column[i].tick = 0;

			// Add a new random val to top of column
			// Second value doesn't really matter as long as it's a valid ascii character.
			(matrix + i*rows + 0)->val = (column[i].is_blank || column[i].index <= column[i].padding) ? 0 : (rand()%(charset_len-1)) + 1;
			mvprintw(0, i, "%s", charset[(matrix + i*rows + 0)->val]);
			
			// Move the column down one position
			for(size_t r = rows; r > 1; r--){
				
				bool current_blank = ((matrix + i*rows + r-1)->val == 0) ? true : false;
				bool above_blank = ((matrix + i*rows + r-2)->val == 0) ? true : false;
				
				if(current_blank && !(above_blank)){
					// Adds a new random character one below the falling string, then prints
					(matrix + i*rows + r-1)->val = (rand()%(charset_len-1)) + 1;
					attron(COLOR_PAIR(1));
					mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);

					attron(COLOR_PAIR(2));
					mvprintw(r-2, i, "%s", charset[(matrix + i*rows + r-2)->val]);

					// Sets new head color to head color and previous head to body color
					// strcpy((matrix + i*rows + r-1)->col, HEAD_COL);
					// strcpy((matrix + i*rows + r-2)->col, TAIL_COL);
				
				// Removes last character of string
				} else if(!(current_blank) && above_blank){
					(matrix + i*rows + r-1)->val = 0;
					mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);
				
				// Changes some of the non-blank values to a new value
				// for some added randomness.
				} else if(!(current_blank)){
					int randint = rand()%10;
					if(randint < 2){
						(matrix + i*rows + r-1)->val = (rand()%(charset_len-1)) + 1;
						attron(COLOR_PAIR(2));
						mvprintw(r-1, i, "%s", charset[(matrix + i*rows + r-1)->val]);
					}
				}
				
				// Prevents head from staying head color when it reaches the bottom
				if(r == rows){
					// strcpy((matrix + i*rows + r-1)->col, TAIL_COL);
					// attron
				}
			}

			// Once we reach the end, reshuffle the column with new values
			if(column[i].index == column[i].padding+column[i].length){
				set_col(&column[i], rows);
			}
		} 
	}
}

void set_color(char* head, char* tail){
	// Set head color
	if(strcmp(head, "WHITE") == 0){
		init_pair(1, 7, -1);
	} else if(strcmp(head, "BLACK") == 0){
		init_pair(1, 0, -1);
	}else if(strcmp(head, "GREEN") == 0){
		init_pair(1, 2, -1);
	}else if(strcmp(head, "YELLOW") == 0){
		init_pair(1, 3, -1);
	}else if(strcmp(head, "BLUE") == 0){
		init_pair(1, 4, -1);
	}else if(strcmp(head, "MAGENTA") == 0){
		init_pair(1, 5, -1);
	}else if(strcmp(head, "CYAN") == 0){
		init_pair(1, 6, -1);
	}else if(strcmp(head, "RED") == 0){
		init_pair(1, 1, -1);
	}

	// Set tail color
	if(strcmp(tail, "WHITE") == 0){
		init_pair(2, 7, -1);
	} else if(strcmp(tail, "BLACK") == 0){
		init_pair(2, 0, -1);
	}else if(strcmp(tail, "GREEN") == 0){
		init_pair(2, 2, -1);
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
