/*
 * main.c
 *
 *  Created on: Nov 21, 2016
 *      Author: EE113D
 */



#include <math.h>
#include "L138_LCDK_aic3106_init.h"

#define THRESHOLD 10000
#define BUFLEN 4
#define HIGH 1
#define LOW 0

int counter = 0;
volatile int record;
int k;

int16_t left_sample;

int16_t buffer[BUFLEN];
int buf_index = 0;
int16_t state = LOW;

char* state_message;
char* state_0 = "No active task";
char* state_1 = "Recording";
char* state_2 = "Analyzing";

char message[20];
int m_index = 0;
char l3etter;

char decode(int16_t* morse){
	if(morse[1] == 0){
		if(morse[2] == -1)
			return 'e';
		else if(morse[2] == 0){
			if(morse[3] == -1)
				return 'i';
			else if(morse[3] == 0){
				if(morse[4] == -1)
					return 's';
				else if(morse[4] == 0)
					return 'h';
				else
					return 'v';
			}
			else{
				if(morse[4] == -1)
					return 'u';
				else if (morse[4] == 0)
					return 'f';
				else
					return '?';
			}	
		}
		else{
			if(morse[3] == -1)
				return 'a';
			else if(morse[3] == 0){
				if(morse[4] == -1)
					return 'r';
				else if(morse[4] == 0)
					return 'l';
				else
					return '?';
			}
			else{
				if(morse[4] == -1)
					return 'w';
				if(morse[4] == 0)
					return 'p';
				else
					return 'j';
			}
		}
	}
	else{
		if(morse[2] == -1)
			return 't';
		else if(morse[2] == 0){
			if(morse[3] == -1)
				return 'n';
			if(morse[3] == 0){
				if(morse[4] == -1)
					return 'd';
				else if(morse[4] == 0)
					return 'b';
				else
					return '?';
			}
			else{
				if(morse[4] == -1)
					return 'k';
				else if(morse[4] == 0)
					return 'c';
				else
					return 'y';
			}
		}
		else{
			if(morse[3] == -1){
				return 'm';
			}
			else if(morse[3] == 0){
				if(morse[4] == -1)
					return 'g';
				else if(morse[4] == 0)
					return 'z';
				else
					return 'q';
			}
			else{
				if(morse[4] == -1)
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
		if(left_sample > 10000){
			//LOW to HIGH transition
			if(state == LOW){
				state = HIGH;
				if(counter > 1500){
					//Add a space
					message[m_index] = ' ';
					m_index++;
					buf_index = -1;
					//reset buffer
					for(k = 0; k < BUFLEN; k++)
						buffer[k] = 0;
				}
				else if(counter > 500){
					//Add a letter
					letter = 'x';
					message[m_index] = letter;
					m_index++;
				}
				else{
					buf_index++;
				}

				counter = 1;
			}
			//HIGH TO HIGH
			else{
				counter++;
			}
		}
		else if (left_sample < 10000){
			//LOW to HIGH transition
			if(state == LOW){
				state = HIGH;
				if(counter > 1500){
					//Add a space
				}
				else if(counter > 500){
					//Add a letter
				}
				else{
					buf_index++;
				}
				buf_index++;
				counter = 1;
			}
			//HIGH TO HIGH
			else{
				counter++;
			}

		}
		else{
			//HIGH TO LOW transition
			if(state == HIGH){
				//Decide whether last thing was dot or dash
				state = LOW;
				if(counter < 400){ //Register a dot
					buffer[buf_index] = 0;
				}
				else{ //Register a dash
					buffer[buf_index] = 1;
				}
				counter = 1;
			}
			//LOW TO LOW
			else{
				counter++;
			}
		}
//			state_message = state_1;
//			counter++;
//			if(counter < 500)
//				left_sample = 0;
//			else if(counter < 500+SAMPLELEN){
//				sample[counter-500] = input_left_sample();
//			}else{
//				record = 0;
//				counter = -1;
//				left_sample = 0;
//				state_message = state_0;
//			}
//			output_left_sample(left_sample);


	}

	output_left_sample(0);
}

int main(void)
{
	int i, j;
	state_message = state_0;


	for(j = 0; j < 4; j++)
		buffer[j] = 1;

	for(i = 0; i < 20; i++)
		message[i] = ' ';


	L138_initialise_intr(FS_8000_HZ,ADC_GAIN_24DB,DAC_ATTEN_0DB,LCDK_MIC_INPUT);
	while(1);

}
