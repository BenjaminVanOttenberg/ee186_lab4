/*
 * music_player.h
 *
 *  Created on: Nov 20, 2025
 *      Author: benotter
 */

#ifndef INC_MUSIC_PLAYER_H_
#define INC_MUSIC_PLAYER_H_

#include <stdint.h>
#include "main.h"


void music_player_init();

void music_player_update_frequency(uint16_t adc_copy);

uint16_t music_player_get_dac_output();

#endif /* INC_MUSIC_PLAYER_H_ */
