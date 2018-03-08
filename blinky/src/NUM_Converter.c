
#include "NUM_Converter.h"


char hex_num[4] = {'0','0','0','0'};
void NUM_Converter(int n){
	int i = 0;
	int remainder = 0;

	while(n != 0){
		remainder = n % 16;
		n = n/16;
		if(remainder < 10){
			//add 48 to convert decimal to ascii b/c we're saving to a char array
			hex_num[i++] = 48 + remainder;
		}
		else{
			//add 55 to 10 to represent where A starts in hex -> A is 65 in hex
			hex_num[i++] = 55 + remainder;
		}
	}
}

