#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

/* HOW DOES THIS GARBAGE WORK???
 *
 * gr8 question lol
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
	droplet_ptr->tick	= 0;
	droplet_ptr->offset = rand()%(int)(rows*1.5) + 1;
	droplet_ptr->size	= droplet_ptr->offset + droplet_ptr->length;
	droplet_ptr->arr_ptr= (int*)malloc((droplet_ptr->length+droplet_ptr->offset)*sizeof(int));

	// Set front " " offset
	for(int i = 0; i < droplet_ptr->offset; i++){
		droplet_ptr->arr_ptr[i] = 32;
	}
	
	// Set following digits
	for(int i = droplet_ptr->offset; i < droplet_ptr->size; i++){	
		droplet_ptr->arr_ptr[i] = (rand()%(127-33))+33;
	}
}

int main(int argc, char* argv[]){
	// Set rand seed
	srand(time(0));

	// These are temporary, they should be replaced by argv 1 and 2
	// later, which will be "$(tput cols)" and "$(tput lines)".
	int cols = 60;
	int rows = 40;
	
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
		printf("Droplet %d\n", (int)i);
		set_droplet(&matrix1[i], rows);
	}
	
	// print shit
	
	FILE * f;
	f = fopen("the_matrix", "w");
	for(int i = 0; i < cols; i++){
		fprintf(f, "%d: speed :%d, length :%d, offset: %d\n", i, matrix1[i].speed, matrix1[i].length, matrix1[i].offset);

		int size = matrix1[i].length + matrix1[i].offset;
		fprintf(f, "SIZE: %d\n", size);
		for(int j = 0; j < size; j++){
			fprintf(f, "%c ", matrix1[i].arr_ptr[j]);
		}
		fprintf(f, "\n\n");
	}
	fclose(f);
	
	system("clear");
	// main loop	
	while(1){
		// matrixD[COL][ROW]
		for(size_t i = 0; i < cols; i++){
			if(matrix1[i].index < matrix1[i].size-1){	
				if(matrix1[i].tick == matrix1[i].speed){
					matrix1[i].tick = 0;
					matrix1[i].index += 1;
				
					for(size_t j = 0; j < matrix1[i].index; j++){
						int ind = matrix1[i].index;
						matrixD[i][j] = matrix1[i].arr_ptr[ind-j];
					}
				}
				matrix1[i].tick += 1;
			} else {
				free(matrix1[i].arr_ptr);
				set_droplet(&matrix1[i], rows);
			}
		}
		system("clear");
		// Border top
		printf("+");
		for(size_t i = 0; i < cols; i++){
			printf("-");
		}
		printf("+\n");

		for(size_t i = 0; i < rows; i++){
			printf("|");
			for(size_t j = 0; j < cols; j++){
				printf("%c", matrixD[j][i]);
			}
			printf("|\n");
		}

		printf("+");
		for(size_t i = 0; i < cols; i++){
			printf("-");
		}
		printf("+\n\n");
		
		/*
		for(size_t i = 0; i < cols; i++){
			printf("col %d, tick %d, index %d, size %d\n", (int)i, matrix1[i].tick, matrix1[i].index, matrix1[i].size);
		}
		*/
		system("sleep 0.1");
	}

	return 0;
}

