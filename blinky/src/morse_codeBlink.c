/*
 * morse_codeBlink.c
 *
 *  Created on: Feb 12, 2018
 *      Author: derek
 */

#include "NUM_Converter.h"

int morse[5] = {0,0,0,0,0};

void get_Morse(char var){
	switch(var){
		case('A'):
			morse[0] = 0; morse[1] = 1; morse[2] = 0;
			morse[3] = 0; morse[4] = 2;
			break;
		case('B'):
					morse[0] = 1; morse[1] = 0; morse[2] = 0;
					morse[3] = 0; morse[4] = 4;
					break;
		case('C'):
					morse[0] = 1; morse[1] = 0; morse[2] = 1;
					morse[3] = 0; morse[4] = 4;
					break;
		case('D'):
					morse[0] = 1; morse[1] = 0; morse[2] = 0;
					morse[3] = 0; morse[4] = 3;
					break;
		case('E'):
					morse[0] = 0; morse[1] = 0; morse[2] = 0;
					morse[3] = 0; morse[4] = 5;
					break;
		case('F'):
					morse[0] = 0; morse[1] = 0; morse[2] = 1;
					morse[3] = 0; morse[4] = 4;
					break;
		case('0'):
					morse[0] = 1; morse[1] = 1; morse[2] = 1;
					morse[3] = 1; morse[4] = 1;
					break;
		case('1'):
					morse[0] = 0; morse[1] = 1; morse[2] = 1;
					morse[3] = 1; morse[4] = 1;
					break;
		case('2'):
					morse[0] = 0; morse[1] = 0; morse[2] = 1;
					morse[3] = 1; morse[4] = 1;
					break;
		case('3'):
					morse[0] = 0; morse[1] = 0; morse[2] = 0;
					morse[3] = 1; morse[4] = 1;
					break;
		case('4'):
					morse[0] = 0; morse[1] = 0; morse[2] = 0;
					morse[3] = 0; morse[4] = 1;
					break;
		case('5'):
					morse[0] = 0; morse[1] = 0; morse[2] = 0;
					morse[3] = 0; morse[4] = 0;
					break;
		case('6'):
					morse[0] = 1; morse[1] = 0; morse[2] = 0;
					morse[3] = 0; morse[4] = 0;
					break;
		case('7'):
					morse[0] = 1; morse[1] = 1; morse[2] = 0;
					morse[3] = 0; morse[4] = 0;
					break;
		case('8'):
					morse[0] = 1; morse[1] = 1; morse[2] = 1;
					morse[3] = 0; morse[4] = 0;
					break;
		case('9'):
					morse[0] = 1; morse[1] = 1; morse[2] = 1;
					morse[3] = 1; morse[4] = 0;
					break;
		}
    }

/*  init * getMorse(char var) {
			 else if(var =='8'){
				static int morse[5] = {0,0,1,1,1};
				return morse;
			}
			else if(var =='9'){
				static int morse[5] = {0,1,1,1,1};
				return morse;
			}
		} */
