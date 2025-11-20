/*
 * music_player.c
 *
 *  Created on: Nov 20, 2025
 *      Author: benotter
 */

#include <stdint.h>
#include "adc.h"
#include "dac.h"
#include "tim.h"


#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>



#define DAC_MAX 4095   // 12-bit DAC
#define SINE_TABLE_LEN 256

#define F_MIN 100
#define F_MAX 500


typedef struct {
	int *freqs;
	char **notes;
	int count;
} Musical_Scale;

int blues_freqs[] = {
	110, 131, 147, 156, 165, 196,
	220, 262, 294, 311, 330, 392,
	440, 523, 587, 622, 659, 784,
};

char *blues_notes[] = {
	"A", "C", "D", "D#", "E", "G",
	"A", "C", "D", "D#", "E", "G",
	"A", "C", "D", "D#", "E", "G"
};

Musical_Scale A_Minor_Blues_Scale = {
		.freqs = blues_freqs,
		.notes = blues_notes,
		.count = sizeof(blues_freqs) / sizeof(int)
};


/*
 * This function defines a lookup sine table from 0 to DAC_MAX, all above 0
 */
uint16_t* sine_table_init(void) {
	uint16_t *arr = (uint16_t *)malloc(SINE_TABLE_LEN * sizeof(uint16_t));
	if (!arr) return NULL;
	for (int i = 0; i < SINE_TABLE_LEN; i++) {
		float angle = 2.0f * M_PI * (float)i / (float)SINE_TABLE_LEN;
		arr[i] = (uint16_t)((sin(angle) + 1.0f) * DAC_MAX / 2);
	}
	return arr;
}

uint16_t* square_table_init(void) {
	uint16_t *arr = (uint16_t *)malloc(SINE_TABLE_LEN * sizeof(uint16_t));
	if (!arr) return NULL;
	for (int i = 0; i < SINE_TABLE_LEN/2; i++) arr[i] = 1;
	for (int i = SINE_TABLE_LEN/2; i < SINE_TABLE_LEN; i++) arr[i] = 0;
	return arr;
}

uint16_t* saw_table_init(void) {
	uint16_t *arr = (uint16_t *)malloc(SINE_TABLE_LEN * sizeof(uint16_t));
	if (!arr) return NULL;
	for (int i = 0; i < SINE_TABLE_LEN; i++) {
			float step = (float)i / (float)SINE_TABLE_LEN;
			arr[i] = step;
		}
	return arr;
}



void set_note_freq(int freq_hz) {
    uint32_t timer_clk = HAL_RCC_GetPCLK1Freq(); // for APB1 timers on STM32L4
    uint32_t prescaler = htim3.Init.Prescaler + 1;
    uint32_t timer_tick = timer_clk / prescaler;

    uint32_t arr = timer_tick / (freq_hz * SINE_TABLE_LEN) - 1;
    __HAL_TIM_SET_AUTORELOAD(&htim3, arr);
}

int snap_freq_to_note(int f_in, Musical_Scale* scale) {
	int best = scale->freqs[0];
	int best_diff = fabsf(f_in - best);
	int best_note_i = 0;

	for (int i = 1; i < scale->count; i++) {
		float diff = fabsf(f_in - scale->freqs[i]);
		if (diff < best_diff) {
			best = scale->freqs[i];
			best_diff = diff;
			best_note_i = i;
		}
	}

	// printf("Converted %dHz to %d (%s)\r\n", f_in, best,  scale->notes[best_note_i]);
	return best;

}

volatile int sine_cycle = 0;
volatile uint16_t* sine_table_arr = NULL;

void music_player_init() {
	sine_table_arr = sine_table_init();
}

void music_player_update_frequency(uint16_t adc_copy) {
	int scale_freq = F_MIN + adc_copy*(F_MAX-F_MIN)/DAC_MAX;
	int note = snap_freq_to_note(scale_freq, &A_Minor_Blues_Scale);
	set_note_freq(note);
}


uint16_t music_player_get_dac_output() {
	sine_cycle = (sine_cycle + 1) % SINE_TABLE_LEN; // so it wraps back to 0
	uint16_t output_dac = sine_table_arr[sine_cycle];
	return output_dac;
}

