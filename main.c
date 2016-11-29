/*
 * main.c
 *
 *  Created on: Nov 21, 2016
 *      Author: EE113D
 */

#include <math.h>
#include "L138_LCDK_aic3106_init.h"

#define THRESHOLD 200
#define framelen 320
#define HIGH 1
#define LOW 0

int counter = 0;
volatile int record = 0;
volatile int reset = 0;
int k;

int16_t left_sample;

float circbuf[framelen];
float avgEn = 0;
float temp = 0;

int circ_index = 0;
int morseCounter = 0;
int let_index = 0;
int16_t state = LOW;
int16_t flag = LOW;
int16_t test[framelen];

char* state_message;
char* state_0 = "No active task";
char* state_1 = "Recording";
char* state_2 = "Analyzing";

char message[64];
int m_index = 0;
char letter;

int16_t buffer[4];

int count = 0;

char decode(int16_t* morse){
	if(morse[0] == 0){
		if(morse[1] == -1)
			return 'e';
		else if(morse[1] == 0){
			if(morse[2] == -1)
				return 'i';
			else if(morse[2] == 0){
				if(morse[3] == -1)
					return 's';
				else if(morse[3] == 0)
					return 'h';
				else
					return 'v';
			}
			else{
				if(morse[3] == -1)
					return 'u';
				else if (morse[3] == 0)
					return 'f';
				else
					return '?';
			}	
		}
		else{
			if(morse[2] == -1)
				return 'a';
			else if(morse[2] == 0){
				if(morse[3] == -1)
					return 'r';
				else if(morse[3] == 0)
					return 'l';
				else
					return '?';
			}
			else{
				if(morse[3] == -1)
					return 'w';
				if(morse[3] == 0)
					return 'p';
				else
					return 'j';
			}
		}
	}
	else{
		if(morse[1] == -1)
			return 't';
		else if(morse[1] == 0){
			if(morse[2] == -1)
				return 'n';
			if(morse[2] == 0){
				if(morse[3] == -1)
					return 'd';
				else if(morse[3] == 0)
					return 'b';
				else
					return '?';
			}
			else{
				if(morse[3] == -1)
					return 'k';
				else if(morse[3] == 0)
					return 'c';
				else
					return 'y';
			}
		}
		else{
			if(morse[2] == -1){
				return 'm';
			}
			else if(morse[2] == 0){
				if(morse[3] == -1)
					return 'g';
				else if(morse[3] == 0)
					return 'z';
				else
					return 'q';
			}
			else{
				if(morse[3] == -1)
					return 'o';
				else
					return '?';
			}
		}
	}
}

interrupt void interrupt4(void) // interrupt service routine
{
	if(record == 1){
		left_sample = input_left_sample();
		test[counter] = left_sample;
		temp = left_sample*left_sample / 1000000.0;
		//Have not identified the first word yet
		if(flag == LOW){
			avgEn = avgEn + temp - circbuf[counter];
			circbuf[counter] = temp;
			if(avgEn > THRESHOLD){
				flag = HIGH;
				morseCounter = 1;
				avgEn = 0;
			}
		}
		//have found the first high point
		else{
			if(counter < framelen){
				avgEn += temp;
			}
			else{
				counter = 0;
				if(avgEn > THRESHOLD){
					if(morseCounter >= 0){
						morseCounter++;
					}
					else{ //last entry was a pause
						if(morseCounter > -2){ //short pause, end of symbol
						}
						else if(morseCounter > -4){ //medium pause, end of letter
							message[m_index] = decode(buffer);
							int i;
							for(i = 0; i < 4; i++)
								buffer[i] = -1;
							m_index = (m_index+1)%64;
						}
						else{
							//long pause, end of word (add space)
							message[m_index] = ' ';
							m_index = (m_index+1)%64;
						}
						morseCounter = 1;
					}

				}

				//when avgEn is lower than threshold, check the counter
				//If morseCounter is 1, it is a dot. If morseCounter is 3, it is a dash.

				if(avgEn < THRESHOLD){
					if(morseCounter <= 0){ //previously was a pause
						morseCounter--;
					}
					else{ //previous entry was either dot or dash
						if(morseCounter <= 2) //dot
							buffer[let_index] = 0;
						else if(morseCounter > 2) //dash
							buffer[let_index] = 1;
						let_index++;
						morseCounter = -1;

					}

				}
				avgEn = 0;
			}
		}

		counter = (counter+1);
	}

	else{
		counter = 0;
	}

	output_left_sample(0);

}

int main(void)
{
	int i, j;
	state_message = state_0;

	//buffer is for dots and dashes
	for(j = 0; j < 4; j++)
		buffer[j] = 1;

	//circbuf is for the avg energy
	for(j = 0; j < framelen; j++)
		circbuf[j] = 0;

	for(i = 0; i < 20; i++)
		message[i] = ' ';

	L138_initialise_intr(FS_8000_HZ,ADC_GAIN_0DB,DAC_ATTEN_0DB,LCDK_LINE_INPUT);
	while(1){
		if(reset == 1)
		{
			record = 0;
			avgEn = 0;
			counter = 0;
			state_message = state_0;
			flag = LOW;
			reset = 0;
		}
	}

}
