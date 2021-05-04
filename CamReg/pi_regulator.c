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
#define K_P 		  		8
#define K_I			  		5
#define QUARTER_TURN  		310 // corresponds to 250 steps
#define SMALL_STEP_FORWARD	350 // about 4 cm

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	systime_t time;
	int32_t pos_left_motor=0;
	int32_t pos_right_motor=0;
	uint8_t wall_position=0;


	while(1){
		time = chVTGetSystemTime();
		// if one direction possible, go in that direction while following the right wall
		//chprintf((BaseSequentialStream *) &SDU1, " directions possibles =%d direction changed = %d\n",get_possible_directions(),get_direction_changed());

		if(get_possible_directions()==1){
			follow_wall(SENSORRIGHT);
			if(get_direction_changed()==0){

				//chprintf((BaseSequentialStream *) &SDU1, "dans follow right wall avec ref = %d\n",get_reference_right());

			}
			else{
				//reset motor position
				right_motor_set_pos(0);
				left_motor_set_pos(0);

				if(get_free_space_left()==1){
					// turn 90° counterclockwise

					turn_counterclockwise();
				}

				else if(get_free_space_right()==1){
					//turn 90° clockwise

					turn_clockwise();
				}

				//small step forward
				move_forward_smallstep();


				//redo reference
				//do_new_reference();
			}
		}
		else if (get_possible_directions()==2){
			//reset motor psoition
			right_motor_set_pos(0);
			left_motor_set_pos(0);
			wall_position = get_wall_position();
			switch(wall_position){ //probleme dans case left --> on reste bloqué dedans
			case FRONT:
				//open camera

				break;
			case LEFT:{
				// turn towards the wall and open camera
				turn_counterclockwise();
			}
			break;
			case RIGHT:{
				// turn towards the wall and open camera
				turn_clockwise();

			}
			break;
			}
			semaphore_ready();

			switch(get_color()){
			case CODE_BLUE:{
				//TURN RIGHT
				//ATTENTION DES FOIS IL FAUDRSIT TOURNER UN DEMI TOUR COMPLET
				switch(wall_position){
				case FRONT:{
					turn_clockwise();
				}
				break;
				case LEFT:{
					turn_clockwise();
					turn_clockwise();
				}
				break;
				case RIGHT:{
					turn_counterclockwise();
				}
				break;
				}
			}
			break;
			case CODE_RED:{
				//TURN LEFT
				switch(wall_position){
				case FRONT:{
					turn_counterclockwise();
				}
				break;
				case LEFT:{
					turn_clockwise();
				}
				break;
				case RIGHT:{
					turn_counterclockwise();
					turn_counterclockwise();
				}
				break;
				}
			}
<<<<<<< HEAD
			//semaphore_ready();
=======
			}


>>>>>>> balt
		}
		//apres avoir pris une image il faut qu'on recupere color et qu on tourne dans la direction indiquée
		//peut etre utiliser un autre semaphore, ou autre moyen equivalent


		else if(get_possible_directions()==0){
			semaphore_ready();
			if(get_color()==CODE_GREEN){
				//turn on leds and stop motor
			}
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
	/*else if(sensor==SENSORLEFT){
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
	}*/
	//if(i==10)
	//chprintf((BaseSequentialStream *) &SDU1, "valeur senseur = %d, ref= %d err= %d. sum_err = %d, commande = %d\n", get_calibrated_prox(SENSORLEFT),get_reference(), err, sum_err, command);

	sum_err += err;
	if(sum_err > (WORKING_SPEED)/K_I)
		sum_err = WORKING_SPEED/K_I;
	else if(sum_err< -(WORKING_SPEED)/K_I)
		sum_err = -WORKING_SPEED/K_I;

	//i++;
}

void turn_clockwise(void){

	left_motor_set_pos(0);
	right_motor_set_pos(0);

	uint8_t done_left=0, done_right=0;

	left_motor_set_speed(WORKING_SPEED);
	right_motor_set_speed(-WORKING_SPEED);

	while(done_left+done_right!=2){
		if(right_motor_get_pos()<=(-QUARTER_TURN)){
			right_motor_set_speed(0);
			done_right=1;
		}

		if(left_motor_get_pos()>=QUARTER_TURN){
			left_motor_set_speed(0);
			done_left=1;
		}
	}
	done_right=0,done_left=0;
}

void turn_counterclockwise(void){

	left_motor_set_pos(0);
	right_motor_set_pos(0);

	uint8_t done_left=0, done_right=0;

	left_motor_set_speed(-WORKING_SPEED);
	right_motor_set_speed(WORKING_SPEED);

	while(done_left+done_right!=2){
		if(right_motor_get_pos()>=(QUARTER_TURN)){
			right_motor_set_speed(0);
			done_right=1;
		}

		if(left_motor_get_pos()<=-QUARTER_TURN){
			left_motor_set_speed(0);
			done_left=1;
		}
	}
	done_right=0,done_left=0;
}

void move_forward_smallstep(void){

	uint8_t done_left=0, done_right=0;


	right_motor_set_pos(0);
	left_motor_set_pos(0);
	// move forward
	left_motor_set_speed(WORKING_SPEED);
	right_motor_set_speed(WORKING_SPEED);
	while(done_left+done_right!=2){
		if(left_motor_get_pos()>=SMALL_STEP_FORWARD){
			left_motor_set_speed(0);
			done_left=1;
		}

		if(right_motor_get_pos()>=SMALL_STEP_FORWARD){
			right_motor_set_speed(0);
			done_right=1;
		}
		//chprintf((BaseSequentialStream *) &SDU1, "dans while avec done = %d", done_right+done_left);
	}
	done_right=0, done_left=0;
}

