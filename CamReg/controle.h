/*
 * controle.h
 *
 *  Created on: 14 avr. 2021
 *      Author: balth
 */

#ifndef CONTROLE_H_
#define CONTROLE_H_

#define NB_SENSORS 		8
#define SENSORFRONT1 	0
#define SENSORFRONT2	7
#define SENSORLEFT		5
#define SENSORRIGHT		2
#define THRESHOLD_CLOSE_OBSTACLE 130
#define STABILITY_THRESHOLD 20
#define FRONT 0
#define LEFT 1
#define RIGHT 2

void controle_start(void);
void wait_semaphore_ready(void);
//int calibrate_ambient_light(uint8_t sensor1);
uint8_t get_free_space(uint8_t sensor1);
uint8_t get_free_space_front(void);
uint8_t get_free_space_left(void);
uint8_t get_free_space_right(void);
uint8_t get_direction_changed(void);
void do_new_reference(void);
int get_reference_right(void);
int get_reference_left(void);
uint8_t get_possible_directions(void);
uint8_t get_wall_position(void);

#endif /* CONTROLE_H_ */
