/* bin2inc.c */
#include <stdio.h>

int main(int argc, char *argv[]) {
	int data;
	int i;

	FILE *fp = fopen(argv[1], "rb");
	if(fp == NULL){
		printf("File is not found.\r\n");
		return 0;
	}
	data = fgetc(fp);
	if ( data == EOF ) {
		fclose(fp);
		return 1;
	}
	printf("0x%02X", data);
	i=0;
	for(;;) {
		data = fgetc(fp);
		if ( data == EOF ) {
			printf("\n");
			fclose(fp);
			return 1;
		}
		else {
			printf(",");
			i++;
			if ( i == 16 ) {
				printf("\n");
				i = 0;
			}
			printf("0x%02X", data);
		}
	}
}