#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define ASCII_START 65
#define ASCII_END 91
#define SPACE 32

int main(int argc, char *argv[]){

	if (argc != 2){
		fprintf(stderr, "%s: usage keygen 'numbers'\n", __FILE__);
		exit(1);
	}
	
	srand(time(NULL));
	
	int key_len = atoi(argv[1])+ 1;
	char* s = malloc(sizeof(char) * key_len);
	memset(s, '\0', sizeof(char) * key_len);

	for (int i = 0; i < key_len - 1; i++){
		s[i] = (rand() % (ASCII_END + 1 - ASCII_START) + ASCII_START);
		if (s[i] == ASCII_END) {
			s[i] = SPACE;
		}
	}
	
	s[key_len - 1] = '\0';
	printf("%s\n", s);
	
	free(s);
}
