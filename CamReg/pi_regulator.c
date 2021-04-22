#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include "sensors/proximity.h"
#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>
#include <controle.h>

#define WORKING_SPEED 300
#define K_P 		  10
#define K_I			  3

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	systime_t time;


	while(1){
		time = chVTGetSystemTime();
		switch(get_possible_directions()){
		case 0:
			break;
		case 1:
			follow_wall(SENSORLEFT);
			break;
		case 2:
			break;
		//default :
		}

		//100Hz
		chThdSleepUntilWindowed(time, time + MS2ST(10));
	}
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

void follow_wall(uint8_t sensor){
	int16_t err=0, sum_err=0;
	//uint8_t i=0;
	int16_t command=0;
	err = (int16_t) get_calibrated_prox(sensor) - get_reference();
	command = abs(K_P*err + K_I*sum_err); //ajouter K_I
	if(sensor==SENSORRIGHT){
		if(err<0){ //on est trop eloigné du mur de droite --> ajouter un peu de vitesse au moteur de gauche
			right_motor_set_speed(WORKING_SPEED);
			left_motor_set_speed(WORKING_SPEED + command);
		}
		else{
			right_motor_set_speed(WORKING_SPEED + command);
			left_motor_set_speed(WORKING_SPEED);
		}
	}
	else if(sensor==SENSORLEFT){
		if(err>=0){ //on est trop eloigné du mur de droite --> ajouter un peu de vitesse au moteur de gauche
			right_motor_set_speed(WORKING_SPEED);
			left_motor_set_speed(WORKING_SPEED + command);
		}
		else{
			right_motor_set_speed(WORKING_SPEED + command);
			left_motor_set_speed(WORKING_SPEED);
		}
	}
	//if(i==10)
	//chprintf((BaseSequentialStream *) &SDU1, "valeur senseur droit = %d, ref= %d err= %d. sum_err = %d, commande = %d\n", get_calibrated_prox(SENSORRIGHT),get_reference(), err, sum_err, command);

	sum_err += err;
	if(sum_err > (WORKING_SPEED)/K_I)
		sum_err = WORKING_SPEED/K_I;
	else if(sum_err< -(WORKING_SPEED)/K_I)
		sum_err = -WORKING_SPEED/K_I;

	//i++;
}
