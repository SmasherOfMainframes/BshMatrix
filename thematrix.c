#include <stdio.h>
#include <stdlib.h>		// strtol
#include <unistd.h>		
#include <time.h>		// srand(time(0))
#include <stdbool.h>	// BOOL
#include <string.h>		// strcmp

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

// Los colores
static const char RED[] 	= "\e[31m";
static const char BLACK[] 	= "\e[30m";
static const char GREEN[] 	= "\e[32m";
static const char YELLOW[] 	= "\e[33m";
static const char BLUE[] 	= "\e[34m";
static const char MAGENTA[] = "\e[35m";
static const char CYAN[] 	= "\e[36m";
static const char WHITE[] 	= "\e[97m";

// Data struct for each column
struct Column{
	int speed;		// How many ticks until a letter appears
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

// For debugging
FILE * f;

/* --------------------------------------------------
---------------------- PROTOTYPES -------------------
-------------------------------------------------- */

void set_col(struct Column* col, int rows);

void set_all_cols(struct Column* column, int cols, int rows);

void write_col(struct Column* column, int i);

void write_all_cols(struct Column* columns, int cols);

void print_matrix(struct Matrix* matrix, int cols, int rows);

void move_cols(struct Column* column, struct Matrix* matrix, int cols, int rows);

void randomizer(struct Matrix* matrix, int cols, int rows);

/* --------------------------------------------------
------------------------- MAIN ----------------------
-------------------------------------------------- */

int main(int argc, char* argv[]){
	f = fopen("the_matrix", "w");
	fclose(f);

	// Set rand seed
	srand(time(0));

	// Sets COLS and ROWS to command line arguments. 
	// strtol(string, endpointer, base);
	const int COLS = strtol(argv[1], NULL, 10);
	const int ROWS = strtol(argv[2], NULL, 10);
	
	// Array of Column structs, used to get column-specific data
	struct Column columns[COLS];
	
	// THE MATRIX
	struct Matrix thematrix[COLS][ROWS];
	// Set the matrix to all " "
	for(size_t i = 0; i < ROWS; i++){
		for(size_t j = 0; j < COLS; j++){
			thematrix[j][i].val = 32; // 32 = " "
			strcpy(thematrix[j][i].col, CYAN);
		}

	}
	
	// Initialize the starting values of each column.
	set_all_cols(columns, COLS, ROWS);

	// write_all_cols(columns, COLS);		// For debugging

	// ------ MAIN LOOP -------- //

	while(1){
		system("clear");
		print_matrix(thematrix[0], COLS, ROWS);	
		move_cols(columns, thematrix[0], COLS, ROWS);
		randomizer(thematrix[0], COLS, ROWS);
		// SLOWEST : 150000
		// FASTEST : 60000
		usleep(60000);
	}

	return 0;
}

/* --------------------------------------------------
---------------------- FUNK SHUNS -------------------
-------------------------------------------------- */

void set_col(struct Column* column, int rows){
	column->speed		= rand()%3 + 1;
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

void write_col(struct Column* column, int i){
	fprintf(f, "Col %d, Speed %d, Tick %d, Index %d, Length %d, Isblank %d\n", 
			(int)i, column[i].speed, column[i].tick, column[i].index, column[i].length, column[i].is_blank);
}

void write_all_cols(struct Column* columns, int cols){
	f = fopen("the_matrix", "a");
	for(size_t i = 0; i < cols; i++){
		write_col(columns, i);
	}
	fclose(f);
}

void move_cols(struct Column* column, struct Matrix* matrix, int cols, int rows){
	for(size_t i = 0; i < cols; i++){
		
		column[i].tick++;

		if(column[i].tick == column[i].speed){
			column[i].index++;
			column[i].tick = 0;

			// Add a new random val to top of column
			// Second value doesn't really matter as long as it's a valid ascii character.
			(matrix + i*rows + 0)->val = (column[i].is_blank || column[i].index <= column[i].padding) ? 32 : 33;

			// Move the column down one position
			for(size_t r = rows; r > 1; r--){
				
				bool below_blank = ((matrix + i*rows + r)->val == 32) ? true : false;
				bool current_blank = ((matrix + i*rows + r-1)->val == 32) ? true : false;
				bool above_blank = ((matrix + i*rows + r-2)->val == 32) ? true : false;
				
				// Adds a new random character one below the falling string
				if(current_blank && !(above_blank)){
					(matrix + i*rows + r-1)->val = (rand()%(127-33))+33;
					// Sets new head color and previous head to body color
					strcpy((matrix + i*rows + r-1)->col, WHITE);
					strcpy((matrix + i*rows + r-2)->col, CYAN);
				// Removes last character of string
				} else if(!(current_blank) && above_blank){
					(matrix + i*rows + r-1)->val = 32;
				}
				// Prevents head from staying head color when it reaches the bottom
				if(r == rows){
					strcpy((matrix + i*rows + r-1)->col, CYAN);
				}
			}

			// Once we reach the end, reshuffle the column with new values
			if(column[i].index == column[i].padding+column[i].length){
				set_col(&column[i], rows);
			}
		} 
	}
}

/*
0 4 8
1 5 9
2 6 10
3 7 11
*/
void print_matrix(struct Matrix* matrix, int cols, int rows){
	for(size_t r = 0; r < rows; r++){
		for(size_t c = 0; c < cols; c++){
			printf("%s%c", (matrix + c*rows + r)->col, (matrix + c*rows + r)->val);
		}
		printf("\n");
	}
}

void randomizer(struct Matrix* matrix, int cols, int rows){
	for(size_t i = 0; i < rand()%(cols*rows/2); i++){
		int x = rand()%(cols+1);
		int y = rand()%(rows+1);
		if((matrix + y*rows + x)->val != 32){
			(matrix + y*rows + x)->val = (rand()%(127-33))+33;
		}
	}	
}