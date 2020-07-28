#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* HOW DOES THIS GARBAGE WORK???
 * There is one matrix holding $(cols) arrays. These are the matrix
 * droplets. One droplet for each column.
 *
 * As the droplet descends, once the last character is on screen, the
 * array is transfered to a second matrix, where it will be allocated
 * on heap. At the same time, the copy in the first matrix will be
 * reshuffled and sent back to the top.
 *
 * When a droplet is created in the first matrix, it has a length, speed,
 * offset and index. 
 * 	SPEED: How many ticks until the droplet descends one position.
 * 	LENGTH: The length of the array of characters.
 * 	OFFSET: How many leading blank spaces are in the array.
 * 	INDEX: Where the droplet is currently positioned on screen.
 *
 * FUNCTIONS:
 * 	set_droplet : creates a new droplet with randomized data.
 * 	move_droplet: moves droplet from matrix 1 to matrix 2.
 *	blank_droplet: 3/10 columns should be blank, so this should be
 *		called before a new droplet is created to add in some blank spots.
 *
 *
 *
 */

struct Droplet{
	int length;
	int speed;
	int offset;
	int size;		// offset + length
	int index;
	int tick;
	int* arr_ptr;
};

void set_droplet(struct Droplet* droplet_ptr, int rows){
	droplet_ptr->index	= 0;
	droplet_ptr->length = rows - rand()%(rows/2);
	droplet_ptr->speed 	= rand()%4 + 1;
	droplet_ptr->offset = rand()%(int)(rows*0.5) + 1;
	droplet_ptr->size	= droplet_ptr->offset + droplet_ptr->length;
	droplet_ptr->arr_ptr= (int*)malloc((droplet_ptr->length+droplet_ptr->offset)*sizeof(int));

	int len = droplet_ptr->length;	
	int off = droplet_ptr->offset;
	
	// Assign random int to matrix
	for(int i = 0; i < off; i++){
		droplet_ptr->arr_ptr[i] = 32;
	}

	int blank = rand()%10;
	for(int i = off; i < (len + off); i++){	
		if(blank < 3){
			droplet_ptr->arr_ptr[i] = 32;
		}
		droplet_ptr->arr_ptr[i] = (rand()%(127-33))+32;
	}
}

int main(int argc, char* argv[]){
	
	// These are temporary, they should be replaced by argv 1 and 2
	// later, which will be "$(tput cols)" and "$(tput lines)".
	int cols = 78;
	int rows = 44;
	
	// The first matrix, holds droplets whose final element 
	// is not currently displayed
	struct Droplet matrix1[cols];
	
	// The matrix which is actually displayed
	int matrixD[cols][rows];
	for(size_t i = 0; i < rows; i++){
		for(size_t j = 0; j < cols; j++){
			matrixD[j][i] = 32;
		}
	}

	// Cycle through the first matrix and set all the droplets.
	for(size_t i = 0; i < cols; i++){
		printf("Droplet %d\n", i);
		set_droplet(&matrix1[i], rows);
	}
	
	// print shit
	for(int i = 0; i < cols; i++){
		printf("%d: speed :%d, length :%d, offset: %d\n", i, matrix1[i].speed, matrix1[i].length, matrix1[i].offset);

		int size = matrix1[i].length + matrix1[i].offset;
		printf("SIZE: %d\n", size);
		for(int j = 0; j < size; j++){
			printf("%c ", matrix1[i].arr_ptr[j]);
		}
		printf("\n\n");
	}
	
	printf("NEXT\n\n");	
	system("clear");

	// main loop	
	while(1){
		// matrixD[COL][ROW]
		for(size_t i = 0; i < cols; i++){
				
			if(matrix1[i].tick == matrix1[i].speed){
				matrix1[i].tick = 0;
				matrix1[i].index += 1;
				
				for(size_t j = 0; j < matrix1[i].index; j++){
					int ind = matrix1[i].index;
					matrixD[i][j] = matrix1[i].arr_ptr[ind-j];
							//matrix1[i].index+47-j;
							//matrix1[i].arr_ptr[j];
				}
			}
			matrix1[i].tick += 1;
		}
		system("clear");
		for(size_t i = 0; i < rows; i++){
			for(size_t j = 0; j < cols; j++){
				printf("%c", matrixD[j][i]);
			}
			printf("\n");
		}
		system("sleep 0.4");
	}
	
	/*
	system("truncate -s 0 the_matrix");
		
	system("clear");
	for(int j = 0; j < 20; j++){
		fout = fopen("the_matrix", "a");
		for(int i = 0; i < cols; i++){
			char c = (char)matrix1[i].arr_ptr[j];
			fprintf(fout, "%c", c);
		}
		fprintf(fout, "\n");
		fclose(fout);
		system("clear");
		system("tac the_matrix");
		system("sleep 0.2");
	}
	*/

	return 0;
}

