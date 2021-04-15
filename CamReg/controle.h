/*
 * controle.h
 *
 *  Created on: 14 avr. 2021
 *      Author: balth
 */

#ifndef CONTROLE_H_
#define CONTROLE_H_

void controle_start(void);
void wait_semaphore_ready(void);
int calibrate_ambient_light(uint8_t sensor1);
uint8_t get_free_space(uint8_t sensor1);
uint8_t get_free_space_front(void);
uint8_t get_free_space_left(void);
uint8_t get_free_space_right(void);


#endif /* CONTROLE_H_ */
