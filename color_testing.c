#include <stdio.h>
#include <stdlib.h>		// strtol
#include <unistd.h>		
#include <time.h>		// srand(time(0))
#include <stdbool.h>

#define REDD "\e[31m"

static const char RED[] 	= "\e[31m";
static const char BLACK[] 	= "\e[30m";
static const char GREEN[] 	= "\e[32m";
static const char YELLOW[] 	= "\e[33m";
static const char BLUE[] 	= "\e[34m";
static const char MAGENTA[] = "\e[35m";
static const char CYAN[] 	= "\e[36m";
static const char WHITE[] 	= "\e[97m";

struct Test{
	int val;
	char col[7];
};

int main(){

	// int arr[] = {40, 41, 42, 43, 45};

	printf("%s%c\n", REDD, 50);


	struct Test test[4];

	for(size_t i = 0; i < 4; i++){

		test[i].val = rand()%10+33;
		for(size_t j = 0; j < 7; j++){
			test[i].col[j] = RED[j];
		}
		printf("%s%c\n", test[i].col, test[i].val);

	}




	return 0;
}