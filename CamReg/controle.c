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


#define MAX_SENSOR 7

static BSEMAPHORE_DECL(open_camera_sem, TRUE);



static THD_WORKING_AREA(waControle, 256); //changer taille??
static THD_FUNCTION(Controle, arg){
	//completer

	uint8_t i=0;
	int sensor_val=0;

	for(i;i<MAX_SENSOR;i++){
		sensor_val=get_calibrated_prox(i);
		chprintf((BaseSequentialStream *) &SDU1, "valeur senseur %d = %d",i, sensor_val);
	}


	chBSemSignal(&open_camera_sem); //a appeller quand on a detecté un croisement
	chThdSleepMilliseconds(1000); //ou utiliser chThdSleepUntilWindowed(time, time + MS2ST(10));
}



void controle_start(void){
	chThdCreateStatic(waControle, sizeof(waControle), NORMALPRIO, Controle, NULL);
}

void wait_semaphore_ready(void){
	chBSemWait(&open_camera_sem);
}

