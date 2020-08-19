#include <stdio.h>
#include <stdlib.h>		// strtol
#include <unistd.h>		
#include <time.h>		// srand(time(0))
#include <stdbool.h>	// BOOL
#include <string.h>		// strcmp

int main(){

	char kata[] = "\u30A1";
	char temp[4];
	memcpy(temp, &kata[0], 4);
	printf("%s\n", temp);


	const char arr[5][4] = {"0", "1", " ", "ｦ", "ｱ"};

	for(int i = 0; i < 5; i++){
		printf("%s ", arr[i]);
	}

	printf("\n%ld\n", sizeof(arr)/sizeof(arr[0]));

	return 0;
}