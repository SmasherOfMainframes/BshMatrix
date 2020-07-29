#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

/* HOW DOES THIS GARBAGE WORK???
 *
 * gr8 question lol
 *
 */

// Mr. Worldwides

// Data struct for each column
struct Column{
	int speed;		// How many ticks until a letter appears
	int tick;		// Keeps track of time for each column
	int index;		// How far along length the column is
	int length;		// Determines the length of the "droplet" string
	bool is_blank;	// Is the "droplet" string blank or regular?
};

void set_col(struct Column* col, int rows){
	col->speed	= rand()%4 + 1;
	col->tick	= 0;
	col->index	= 0;
	col->length = rand()%rows + 1;
	col->is_blank = (rand()%10 < 4) ? true : false;
}

int main(int argc, char* argv[]){
	// Set rand seed
	srand(time(0));

	// Sets COLS and ROWS to command line arguments. 
	// strtol(string, endpointer, base);
	const int COLS = strtol(argv[1], NULL, 10);
	const int ROWS = strtol(argv[2], NULL, 10);
	
	// Array of Column structs, used to get column-specific data
	struct Column columns[COLS];
	
	// THE MATRIX
	int thematrix[COLS][ROWS];
	for(size_t i = 0; i < ROWS; i++){
		for(size_t j = 0; j < COLS; j++){
			thematrix[j][i] = 32;
		}
	}

	// Cycle through the first matrix and set all the droplets.
	for(size_t i = 0; i < COLS; i++){
		set_col(&columns[i], ROWS);
	}
	
	FILE * f;
	f = fopen("the_matrix", "w");
	for(size_t i = 0; i < COLS; i++){
		fprintf(f, "Col %d, Speed %d, Tick %d, Index %d, Length %d, Isblank %d\n", (int)i, columns[i].speed, columns[i].tick, columns[i].index, columns[i].length, columns[i].is_blank);
	}

	return 0;
}

