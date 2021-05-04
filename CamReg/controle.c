/*
 * controle.c
 *
 *  Created on: 14 avr. 2021
 *      Author: balth
 */
#include "ch.h"
#include "hal.h"
#include <chprintf.h>
#include <usbcfg.h>
#include "stdio.h"
#include <main.h>
#include "sensors/proximity.h"
#include "controle.h"

static BSEMAPHORE_DECL(open_camera_sem, TRUE);

volatile static int variable_reference_right=0;
volatile static int variable_reference_left=0;
volatile static uint8_t available_dir_changed=0;


static THD_WORKING_AREA(waControle, 256); //changer taille??
static THD_FUNCTION(Controle, arg){
	//completer

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	uint8_t i=0;
	uint8_t prev_front=0, prev_left=0, prev_right =0;
	uint8_t new_front=0, new_left=0, new_right=0;



	//int sensor_val=0;
	int old_ref_right=0, new_ref_right=0,old_ref_left=0, new_ref_left=0;
	uint8_t did_ref=0;
	while(1){

		new_front= get_free_space_front();
		new_left= get_free_space_left();
		new_right= get_free_space_right();

		if(get_possible_directions()==1 && (new_front!=prev_front || new_right!= prev_right || new_left!=prev_left)){
			available_dir_changed = 1;
			//update prev_front/right/left
			prev_front = new_front;
			prev_right = new_right;
			prev_left = new_left; //verifier qu'on n'ecrase pas les anciennes valeurs de maniere incorrecte
		}
		else{
			available_dir_changed =0;
		}

		while(!did_ref){
			do_new_reference();
			new_ref_right=variable_reference_right;
			new_ref_left=variable_reference_left;
			if((new_ref_right> STABILITY_THRESHOLD && abs(new_ref_right-old_ref_right)<STABILITY_THRESHOLD)){ //j'ai enleve la conditon sur la gauche pour l instant
				did_ref=1;
				//chprintf((BaseSequentialStream *) &SDU1, "nouvelle ref droite = %d", new_ref_right);
			}

			else {
				old_ref_right=new_ref_right;
				old_ref_left=new_ref_left;
			}
		}


		chThdSleepMilliseconds(500); //ou utiliser chThdSleepUntilWindowed(time, time + MS2ST(10));
	}
}

void controle_start(void){
	chThdCreateStatic(waControle, sizeof(waControle), NORMALPRIO, Controle, NULL);
}

void wait_semaphore_ready(void){
	chBSemWait(&open_camera_sem);
}

void semaphore_ready(void){
	chBSemSignal(&open_camera_sem);
}

/*int calibrate_ambient_light(uint8_t sensor1){ //penser a utiliser get_prox pour la calibration et eliminer lumiere ambiante
	int calibration_factor=0;
	uint16_t avg_light=0;
	uint16_t avg_sensor=0;
	for(int i = 0; i<NB_SENSORS; i++){
		avg_light+=get_ambient_light(i);
		avg_sensor += get_calibrated_prox(i);
	}
	avg_light/=NB_SENSORS;
	avg_sensor/=NB_SENSORS;
	calibration_factor = (get_ambient_light(sensor1)/avg_light - 1)*avg_sensor ;
	return get_calibrated_prox(sensor1)+calibration_factor;
}*/

uint8_t get_free_space(uint8_t sensor1){
	return (get_calibrated_prox(sensor1)>THRESHOLD_CLOSE_OBSTACLE)? 0: 1;
}

uint8_t get_free_space_front(void){
	return ((get_calibrated_prox(SENSORFRONT1)+get_calibrated_prox(SENSORFRONT2))/2>THRESHOLD_CLOSE_OBSTACLE)? 0: 1;
}

uint8_t get_free_space_left(void){
	return get_free_space(SENSORLEFT);
}

uint8_t get_free_space_right(void){
	return get_free_space(SENSORRIGHT);
}

uint8_t get_direction_changed(void){
	return available_dir_changed;
}

void do_new_reference(void){ //faire appel a cette fonction en debut de programme puis a chauqe fois qu'on fait un quart de tour
	variable_reference_right=get_calibrated_prox(SENSORRIGHT);
	variable_reference_left=get_calibrated_prox(SENSORLEFT);
}

int get_reference_right(void){
	return variable_reference_right;
}

int get_reference_left(void){
	return variable_reference_left;
}

uint8_t get_possible_directions(void){
	return (get_free_space_front()+get_free_space_left()+get_free_space_right());
}

uint8_t get_wall_position(void){
	if(get_free_space_left()==0)
		return LEFT;
	else if(get_free_space_front()==0)
		return FRONT;
	else if (get_free_space_right()==0)
		return RIGHT;
	else return -1;
}


