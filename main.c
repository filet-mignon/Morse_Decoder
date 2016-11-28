/*
 * main.c
 *
 *  Created on: Nov 21, 2016
 *      Author: EE113D
 */

#include <math.h>
#include "L138_LCDK_aic3106_init.h"

#define THRESHOLD 10000
#define framelen 372
#define HIGH 1
#define LOW 0

int counter = 0;
volatile int record;
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

char* state_message;
char* state_0 = "No active task";
char* state_1 = "Recording";
char* state_2 = "Analyzing";

char message[20];
int m_index = 0;
char letter;

int16_t buffer[4];

int count = 0;

interrupt void interrupt4(void) // interrupt service routine
{
	if(record == 1){
		left_sample = input_left_sample();
		temp = left_sample*left_sample;
		//Have not identified the first word yet
		if(flag == LOW){
			avgEn = avgEn + temp - circbuff[counter];
			circbuff[counter] = temp;
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
				if(avgEn > THRESHOLD){
					if(morseCounter >= 0){
						morseCounter++;
					}
					else{ //last entry was a space
						if(morseCounter > -2) //short pause, end of symbol
							let_index++;
						else if(morseCounter > -4) //medium pause, end of letter
						{

						}
						else{
							//long pause, end of word (add space)
							message[m_index] = ' ';
							m_index++;
						}
					}
					//0 is dot, 1 is dash
					if(morseCounter > 2)
					{
						buffer[let_index] = 0;
					}
					if(morseCounter > 3*framelen)
					{
						buffer[let_index] = 1;
					}
					else
					{
						buffer[let_index] = 2;
					}
				}
				let_index++;
				//when avgEn is lower than threshold, check the counter
				//If morseCounter is 1, it is a dot. If morseCounter is 3, it is a dash.

				if(avgEn < THRESHOLD){

				}
				avgEn = 0;
			}
		}
	}

	else{
		counter = 0;
	}

	counter = (count+1)%framelen;
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

	L138_initialise_intr(FS_8000_HZ,ADC_GAIN_24DB,DAC_ATTEN_0DB,LCDK_MIC_INPUT);
	while(1);

}
