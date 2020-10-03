#include <stdlib.h>		// strtol
#include <unistd.h>		// usleep()
#include <time.h>		// srand(time(0))
#include <stdbool.h>	// BOOL
#include <string.h>		// strcmp
#include <locale.h>		// for setting locale
#include <ncursesw/ncurses.h>

/* TODO:
 * 0. Shouldn't need to hold values, should be able to read them from screen, should help memory usage
 * 1. Add sys args for...
 		Mode (rain mode -mr, matrix mode (normal) -mmn, matrix mode (japanese char) -mmj
		Head/tail color			-h COL -t COL
 		Fullness				-f (0% - 100%)
 		Speed					-s (0 - 1000000)
 		Random value changing	-r (0% - 100%)
 * 2. Apply Clean Code (TM) to the move_col functions
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

// CHARACTERS
// This is the characterset to be printed, the first entry MUST be " ".
// The second dimension size must be able to accomadate biggest unicode 
// symbol (+1 for /0).
// const char charset[][4] = {
// 	" ","0","1","2","3","4","5","6","7","8","9",":",".","=","*",
// 	"+","-","¦","|","_","ｦ","ｱ","ｳ","ｴ","ｵ","ｶ","ｷ","ｹ","ｺ","ｻ",
// 	"ｼ","ｽ","ｾ","ｿ","ﾀ","ﾂ","ﾃ","ﾅ","ﾆ","ﾇ","ﾈ","ﾊ","ﾋ","ﾎ","ﾏ",
// 	"ﾐ","ﾑ","ﾒ","ﾓ","ﾔ","ﾕ","ﾗ","ﾘ","ﾜ"
// };
// const char charset[][4] = {
// 	" ","0","1"
// };
// const char charset[][4] = {
// 	" ","☻","☺"
// };
// const char charset[][4] = {
// " ","⠁","⠂","⠃","⠄","⠅","⠆","⠇","⠈","⠉","⠊","⠋","⠌","⠍","⠎","⠏","⠐","⠑","⠒",
// "⠓","⠔","⠕","⠖","⠗","⠘","⠙","⠚","⠛","⠜","⠝","⠞","⠟","⠠","⠡","⠢","⠣","⠤","⠥",
// "⠦","⠧","⠨","⠩","⠪","⠫","⠬","⠭","⠮","⠯","⠰","⠱","⠲","⠳","⠴","⠵","⠶","⠷","⠸",
// "⠹","⠺","⠻","⠼","⠽","⠾","⠿"
// };
// const int charset_len = sizeof(charset)/sizeof(charset[0]);
char charset[256][4];
int charset_len = 1;

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

/* --------------------------------------------------
---------------------- PROTOTYPES -------------------
-------------------------------------------------- */

void set_col(struct Column* col, int rows);

void set_all_cols(struct Column* column, int cols, int rows);

void move_cols(struct Column* column, struct Matrix* matrix, int cols, int rows);

void set_color(char* head, char* tail);

int is_keypressed();

/* --------------------------------------------------
------------------------- MAIN ----------------------
-------------------------------------------------- */

int main(int argc, char* argv[]){
	// Needed for unicode support
	setlocale(LC_ALL, "en_CA.UTF-8");

	// ncurses init stuff
	initscr();	
	start_color();
	use_default_colors();	// allows transparent background
	curs_set(0);			// turns off cursor
	nodelay(stdscr, TRUE);	// Turns getch() into a non-blocking call, allows key press to quit

	srand(time(0));

	////////////////////////////
	////////////////////////////

	FILE* fp = fopen("charsets", "r");
	char buff_line[256];


	// if(argv[1] != NULL){
	// 	char MODE[] = argv[1];
	// } else {
	// 	char MODE[] = "BRAILLE";
	// }

	charset[0][0] = ' ';
	while(fgets(buff_line, 256, fp) != NULL){
		char buff_name[64];
		int idx = 0;
		while(buff_line[idx] != ' '){
			buff_name[idx] = buff_line[idx];
			idx++;
		}
		buff_name[idx] = '\0';

		idx++;

		if(!strcmp(buff_name, argv[1])){
			// LOOP THROUGH ALL CHARS
			char buff_char[4];
			int i = 0;
			while(buff_line[idx-1] != '\0'){
				if(buff_line[idx] == ' ' || buff_line[idx] == '\0'){
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
			charset_len-=2;	// lol why does this stabilize everything??
			break;
		}
	}



	////////////////////////////
	////////////////////////////

	set_color(argv[2], argv[3]);	// Head color, tail color

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
		usleep(2500);
	}
	// ------ MAIN LOOP -------- //

	// Goodbye
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

void set_color(char* head, char* tail){
	// yanderedev has enetered the chat.
	// Set head color
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

	// Set tail color
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
	// yanderedev has left the chat.
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
