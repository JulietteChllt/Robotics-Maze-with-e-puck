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


static THD_WORKING_AREA(waControle, 256); //changer taille??
static THD_FUNCTION(Controle, arg){
	//completer

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	uint8_t i=0;
	//int sensor_val=0;
	int old_ref_right=0, new_ref_right=0,old_ref_left=0, new_ref_left=0;
	uint8_t did_ref=0;
	while(1){

		while(!did_ref){
			do_new_reference();
			new_ref_right=variable_reference_right;
			new_ref_left=variable_reference_left;
			if((new_ref_right>= STABILITY_THRESHOLD && abs(new_ref_right-old_ref_right)<STABILITY_THRESHOLD) ||(new_ref_left>= STABILITY_THRESHOLD && abs(new_ref_left-old_ref_left)<STABILITY_THRESHOLD))
				did_ref=1;
			else {
				old_ref_right=new_ref_right;
				old_ref_left=new_ref_left;
			}
		}


		//chBSemSignal(&open_camera_sem); //a appeller quand on a detecté un croisement
		chThdSleepMilliseconds(5); //ou utiliser chThdSleepUntilWindowed(time, time + MS2ST(10));
	}
}

void controle_start(void){
	chThdCreateStatic(waControle, sizeof(waControle), NORMALPRIO, Controle, NULL);
}

void wait_semaphore_ready(void){
	chBSemWait(&open_camera_sem);
}

int calibrate_ambient_light(uint8_t sensor1){ //penser a utiliser get_prox pour la calibration et eliminer lumiere ambiante
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
}

uint8_t get_free_space(uint8_t sensor1){
	return (calibrate_ambient_light(sensor1)>THRESHOLD_CLOSE_OBSTACLE)? 1: 0;
}

uint8_t get_free_space_front(void){
	return ((calibrate_ambient_light(SENSORFRONT1)+calibrate_ambient_light(SENSORFRONT2))/2>THRESHOLD_CLOSE_OBSTACLE)? 1: 0;;
}

uint8_t get_free_space_left(void){
	return get_free_space(SENSORLEFT);
}

uint8_t get_free_space_right(void){
	return get_free_space(SENSORRIGHT);
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

uint8_t do_next_action(void){
	uint8_t possible_direction = get_possible_directions();
	if(possible_direction==0){
		//check that the wall is green and activate led
	}
	else if(possible_direction==1){

	}
	else if (possible_direction==2){
		// look at the wall to know where to go
		if(get_free_space_front()==0){
			//open cam
			}
		else if (get_free_space_left()==0){
		// turn to the left and open cam
			}
		else if(get_free_space_right()==0){
		//turn to the right and open cam
		}
	}
}

