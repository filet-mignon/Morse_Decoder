/*
 * main.c
 *
 *  Created on: Nov 21, 2016
 *      Author: EE113D
 */

#include <math.h>
#include "L138_LCDK_aic3106_init.h"

#define THRESHOLD 32000
#define BUFLEN 4
#define framelen 960
#define HIGH 1
#define LOW 0
#define k 0.9
#define SYMLEN 100

int counter = 0;
int morseCounter = 0;
int16_t flag = LOW;
int trigger = 0;

volatile int record = 0;
volatile int reset = 0;

int16_t left_sample;

float circbuf[framelen];
char message[64];
int16_t buffer[BUFLEN];
int16_t sym_buf[SYMLEN];

float avgEn;
float temp = 0;

int circ_index = 0;
int let_index = 0;
int m_index = 0;
int s_index = 0;

int16_t test[framelen];

char* state_message;
char* state_0 = "No active task";
char* state_1 = "Recording";
char* state_2 = "Analyzing";

char letter;
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
	else if (morse[0] == -1)
		return ' ';
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
		state_message = state_1;
		left_sample = input_left_sample();
		test[counter] = left_sample;
		temp = left_sample*left_sample / 10000000.0;
		avgEn = avgEn + temp - circbuf[counter];
		circbuf[counter] = temp;
		//Have not identified the first word yet
		if(flag == LOW){
			if(avgEn > THRESHOLD){
				flag = HIGH;
				counter = 0;
				sym_buf[s_index] = 1;
				s_index++;
				//let_index++;
			}
		}
		//have found the first high point
		else{
			if (counter == 0){
				if(avgEn > k*THRESHOLD){
					//buffer[let_index]++;
					sym_buf[s_index] = 1;
					trigger = 0;
				}
				else{
					sym_buf[s_index] = 0;
					trigger++;
					if(trigger > 9){
						record = 0;
					}
					if(trigger > 6){
						int i;
						for(i = 0; i < BUFLEN; i++){
							buffer[i] = -1;
						}
						for(i = 0; i < SYMLEN; i++)
							sym_buf[i] = -1;
						s_index = -1;
						let_index = 0;
					}
					if(trigger > 2){
						int i;
						for(i = 0; i < SYMLEN; i++)
						{
							if(sym_buf[i] == 1)
								buffer[let_index]++;
							else
								let_index = (let_index+1)%4;
							//sym_buf[i] = -1;
						}
						message[m_index] = decode(buffer);
						for(i = 0; i < BUFLEN; i++){
							buffer[i] = -1;
						}
						for(i = 0; i < SYMLEN; i++)
							sym_buf[i] = -1;
						s_index = -1;
						let_index = 0;
						if(message[m_index] != ' ')
							trigger = 0;
						else if (message[m_index -1] == ' ')
							m_index--;
						m_index++;
					}
				}
				s_index = (s_index+1)%SYMLEN;
			}
		}
		counter = (counter+1)%framelen;

	}


	output_left_sample(0);

}

int main(void)
{
	int i, j;
	avgEn = 0;
	state_message = state_0;

	//buffer is for dots and dashes
	for(j = 0; j < BUFLEN; j++)
		buffer[j] = -1;

	//circbuf is for the avg energy
	for(j = 0; j < framelen; j++)
		circbuf[j] = 0;

	for(j = 0; j < 64; j++)
			sym_buf[j] = -1;

	for(i = 0; i < 64; i++)
		message[i] = ' ';

	L138_initialise_intr(FS_8000_HZ,ADC_GAIN_24DB,DAC_ATTEN_0DB,LCDK_MIC_INPUT);
	while(1){
		if(reset == 1)
		{
			record = 0;
			avgEn = 0;
			counter = 0;
			state_message = state_0;
			trigger = 0;
			flag = LOW;
			reset = 0;
			circ_index = 0;
			s_index = 0;
			for(j = 0; j < framelen; j++)
					circbuf[j] = 0;
			let_index = 0;
			m_index = 0;
			for(i = 0; i < 64; i++)
					message[i] = ' ';
			for(j = 0; j < 64; j++)
					sym_buf[j] = -1;
		}
		if(m_index > SYMLEN)
			reset = 1;
	}

}
