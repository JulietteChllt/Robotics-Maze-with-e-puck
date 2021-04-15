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
#include "controle.h"
#include "sensors/proximity.h"


#define NB_SENSORS 8
#define SENSORFRONT1 	0
#define SENSORFRONT2	7
#define SENSORLEFT		5
#define SENSORRIGHT		2
#define THRESHOLD_CLOSE_OBSTACLE 130

static BSEMAPHORE_DECL(open_camera_sem, TRUE);



static THD_WORKING_AREA(waControle, 256); //changer taille??
static THD_FUNCTION(Controle, arg){
	//completer

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	uint8_t i=0;
	int sensor_val=0;
	int ambient_light=0;
	while(1){
		for(i=0;i<NB_SENSORS;i++){
			sensor_val=get_calibrated_prox(i);
			ambient_light = get_ambient_light(i);
			chprintf((BaseSequentialStream *) &SDU1, "valeur senseur %d = %d\n",i, sensor_val); //ENLEVER LES PRINT QUAND PROJET FONCTIONNE
		}


		//chBSemSignal(&open_camera_sem); //a appeller quand on a detecté un croisement
		chThdSleepMilliseconds(10); //ou utiliser chThdSleepUntilWindowed(time, time + MS2ST(10));
	}

}
void controle_start(void){
	chThdCreateStatic(waControle, sizeof(waControle), NORMALPRIO, Controle, NULL);
}

void wait_semaphore_ready(void){
	chBSemWait(&open_camera_sem);
}

int calibrate_ambient_light(uint8_t sensor1){
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
	return (get_free_space(SENSORFRONT1)+get_free_space(SENSORFRONT2))/2;
}

uint8_t get_free_space_left(void){
	return get_free_space(SENSORLEFT);
}

uint8_t get_free_space_right(void){
	return get_free_space(SENSORRIGHT);
}
