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

#define WORKING_SPEED 		300
#define K_P 		  		10
#define K_I			  		3
#define QUARTER_TURN  		250 // corresponds to 250 steps
#define SMALL_STEP_FORWARD	350 // about 4 cm

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	systime_t time;

	int32_t pos_left_motor=0;
	int32_t pos_right_motor=0;


	while(1){
		time = chVTGetSystemTime();

		// if one direction possible, go in that direction while following the right wall
		if(get_possible_directions()==1){
			if(get_direction_changed()==0){
				follow_wall(SENSORRIGHT);
			}
			else{
				pos_left_motor=left_motor_get_pos();
				pos_right_motor=right_motor_get_pos();
				if(get_free_space_left()==1){
					// turn 90° counterclockwise
					// A REVOIR POSE PEUT ETRE PROBLEME
					while(left_motor_get_pos()!=pos_left_motor-QUARTER_TURN && right_motor_get_pos()!=pos_right_motor+QUARTER_TURN){
						left_motor_set_speed(-WORKING_SPEED);
						right_motor_set_speed(WORKING_SPEED);
					}
				}
				else if(get_free_space_right()==1){
					//turn 90° clockwise
					while(left_motor_get_pos()!=pos_left_motor+QUARTER_TURN && right_motor_get_pos()!=pos_right_motor-QUARTER_TURN){
						left_motor_set_speed(WORKING_SPEED);
						right_motor_set_speed(-WORKING_SPEED);
					}
				}
				pos_left_motor=left_motor_get_pos();
				pos_right_motor=right_motor_get_pos();
				// move forward
				while(left_motor_get_pos()!=pos_left_motor+SMALL_STEP_FORWARD && right_motor_get_pos()!=pos_right_motor+SMALL_STEP_FORWARD){
					left_motor_set_speed(WORKING_SPEED);
					right_motor_set_speed(WORKING_SPEED);
				}
				//redo reference
				do_new_reference();
			}
		}
		else if (get_possible_directions()==2){
			pos_left_motor=left_motor_get_pos();
			pos_right_motor=right_motor_get_pos();
			switch(get_wall_position()){
			case FRONT:
				//open camera
				break;
			case LEFT:{
				// turn towards the wall and open camera
				while(left_motor_get_pos()!=pos_left_motor-QUARTER_TURN && right_motor_get_pos()!=pos_right_motor+QUARTER_TURN){
					left_motor_set_speed(-WORKING_SPEED);
					right_motor_set_speed(WORKING_SPEED);
				}
			}
			break;
			case RIGHT:{
				// turn towards the wall and open camera
				while(left_motor_get_pos()!=pos_left_motor+QUARTER_TURN && right_motor_get_pos()!=pos_right_motor-QUARTER_TURN){
					left_motor_set_speed(WORKING_SPEED);
					right_motor_set_speed(-WORKING_SPEED);
				}
			}
			break;
			}
		}

		//follow_wall(SENSORLEFT);

		/*switch(get_possible_directions()){
		case 0:
			break;
		case 1:

			break;
		case 2:
			break;
		//default :
		}*/

		//100Hz
		chThdSleepUntilWindowed(time, time + MS2ST(10));
	}
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

void follow_wall(uint8_t sensor){
	int16_t err=0, sum_err=0;
	int16_t command=0;
	if(sensor==SENSORRIGHT){
		err = (int16_t) get_calibrated_prox(sensor) - get_reference_right();
		command = abs(K_P*err + K_I*sum_err); //ajouter K_I
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
		err = (int16_t) get_calibrated_prox(sensor) - get_reference_left();
		command = abs(K_P*err + K_I*sum_err); //ajouter K_I
		if(err<0){ //on est trop eloigné du mur de gauche --> ajouter un peu de vitesse au moteur de droite
			right_motor_set_speed(WORKING_SPEED + command);
			left_motor_set_speed(WORKING_SPEED);
		}
		else{
			right_motor_set_speed(WORKING_SPEED);
			left_motor_set_speed(WORKING_SPEED + command);
		}
	}
	//if(i==10)
	//chprintf((BaseSequentialStream *) &SDU1, "valeur senseur = %d, ref= %d err= %d. sum_err = %d, commande = %d\n", get_calibrated_prox(SENSORLEFT),get_reference(), err, sum_err, command);

	sum_err += err;
	if(sum_err > (WORKING_SPEED)/K_I)
		sum_err = WORKING_SPEED/K_I;
	else if(sum_err< -(WORKING_SPEED)/K_I)
		sum_err = -WORKING_SPEED/K_I;

	//i++;
}
