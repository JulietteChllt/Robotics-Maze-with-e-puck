#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include "sensors/proximity.h"
#include <main.h>
#include <motors.h>
#include <leds.h>
#include <process_image.h>
#include <controle.h>
#include "motor_regulator.h"

/*=================================================================================================================*/
/*================================================ Thread definition ==============================================*/
/*=================================================================================================================*/

static THD_WORKING_AREA(waMotorRegulator, 256);
static THD_FUNCTION(MotorRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	systime_t time;

	uint8_t dir_left_possible = 0;
	uint8_t dir_front_possible = 0;
	uint8_t dir_right_possible = 0;
	uint8_t nb_possible_direction = 0;
	uint8_t end=0;

	while(1){
		time = chVTGetSystemTime();

		dir_right_possible = get_free_space_right();
		dir_front_possible = get_free_space_front();
		dir_left_possible = get_free_space_left();
		nb_possible_direction = dir_right_possible +dir_front_possible+dir_left_possible;

		/*if the robot can't go anywhere, it checks whether it reached it's final destination : the green wall
		 * if its the case the body LED turns green
		 * else the LEDs turn red
		 */
		if(nb_possible_direction == 0 && !end){
			move_backward_smallstep();
			semaphore_ready();
			move_forward_smallstep();
			if(get_color()==CODE_GREEN){
				set_rgb_led(LED2,0,100,0);
				set_rgb_led(LED4,0,100,0);
				set_rgb_led(LED6,0,100,0);
				set_rgb_led(LED8,0,100,0);
				end=1;
			}
			else{
				set_rgb_led(LED2,0,100,0);
				set_rgb_led(LED4,0,100,0);
				set_rgb_led(LED6,0,100,0);
				set_rgb_led(LED8,0,100,0);
			}
		}

		/* if the robot can only go in 1 direction, there are 3 cases :
		 * - the robot can only go forward : in that case he should keep following the wall
		 * - the robot can only go left : the robot should turn counterclockwise and redo the reference to follow the wall
		 * - the robot can only go right : same as above
		 * Remark : the small steps forward allow the robot to be in the right position to either turn or simply do the reference
		 */
		if(nb_possible_direction==1){

			if(get_free_space_front()==1 && get_direction_changed()==0){
				follow_wall();
			}
			else{
				right_motor_set_speed(0);
				left_motor_set_speed(0);

				if(get_free_space_left()){
					move_forward_smallstep();
					turn_counterclockwise();
					move_forward_smallstep();
					do_new_reference();
				}
				else if(get_free_space_right()){
					turn_clockwise();
					move_forward_smallstep();
					do_new_reference();
				}
			}

		}

		/* if the robot can go in 2 directions, there are 3 cases again :
		 * - if the robot can go left or right : it should check the color of the wall in front and do a quarter turn in the right direction
		 * - if the robot can go in front or right : it should check again that there is no wall in front
		 * 	(this is to avoid the case where the wall would be just a little further) and
		 * 	 if it is indeed free : it should check the color of the wall on the left and go to the right if the wall is blue and to the front if the wall is red
		 * 	 if not : it should just turn right and follow the wall again
		 * - if the robot can only go front or left : this case is really similar to the previous one only the reference to the right wall should be redone if,
		 *   after the turn, the robot sees a wall on its right
		 * Remark : the small step backward allow the robot to see the color of the wall better
		 */
		else if(nb_possible_direction==2){
			if(dir_front_possible==0){
				move_backward_smallstep();
				semaphore_ready();
				move_forward_smallstep();
				switch(get_direction()){
				case CODE_RED :
					turn_counterclockwise();
					break;
				case CODE_BLUE :
					turn_clockwise();
					break;
				}
				move_forward_smallstep();
				do_new_reference();
			}

			else if(dir_left_possible==0){
				if(get_free_space_front()==1){
					turn_counterclockwise();
					move_backward_smallstep();
					semaphore_ready();
					move_forward_smallstep();
					switch(get_direction()){
					case CODE_RED:
						turn_clockwise();
						break;
					case CODE_BLUE:
						turn_clockwise();
						turn_clockwise();
						break;
					}
				}
				else {
					turn_clockwise();
					move_forward_smallstep();
				}
				move_forward_smallstep();
				if(!get_free_space_right())
					do_new_reference();

			}
			else if(dir_right_possible==0){
				move_forward_smallstep();
				if(get_free_space_front()==1){
					turn_clockwise();
					move_backward_smallstep();
					semaphore_ready();
					switch(get_direction()){
					case CODE_RED :
						turn_counterclockwise();
						break;
					case CODE_BLUE :
						turn_counterclockwise();
						turn_counterclockwise();
						break;
					}
				}
				else{
					turn_counterclockwise();
					move_forward_smallstep();
					//move_forward_smallstep();
				}
				move_forward_smallstep();
				if(!get_free_space_right())
					do_new_reference();
			}
		}

		//100Hz
		chThdSleepUntilWindowed(time, time + MS2ST(1));
	}
}

/*=================================================================================================================*/
/*================================================ Helper functions ===============================================*/
/*=================================================================================================================*/

void motor_regulator_start(void){
	chThdCreateStatic(waMotorRegulator, sizeof(waMotorRegulator), NORMALPRIO, MotorRegulator, NULL);
}

void follow_wall(void){
	int16_t err=0;
	int16_t command=0;

	err = (int16_t) get_prox(SENSORRIGHT) - get_reference();
	command = abs(K_P*err);
	if(err<-40){//faire prev error
		right_motor_set_speed(0);
		left_motor_set_speed(0);
		move_forward_smallstep();
	}
	// if the robot is too far from the right wall, it should increase it's left motor speed proportionately
	if(err<0){
		right_motor_set_speed(WORKING_SPEED);
		left_motor_set_speed(WORKING_SPEED + command);
	}
	// if the robot is too close to the right wall, it should increase it's right motor speed proportionately
	else if(err>0){
		right_motor_set_speed(WORKING_SPEED + command);
		left_motor_set_speed(WORKING_SPEED);
	}
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
}

void move_forward_smallstep(void){

	uint8_t done_left=0, done_right=0;

	right_motor_set_pos(0);
	left_motor_set_pos(0);
	// move forward
	left_motor_set_speed(WORKING_SPEED);
	right_motor_set_speed(WORKING_SPEED);
	while((done_left+done_right!=2)){
		if(left_motor_get_pos()>=SMALL_STEP_FORWARD){

			left_motor_set_speed(0);
			done_left=1;
		}

		if(right_motor_get_pos()>=SMALL_STEP_FORWARD){
			right_motor_set_speed(0);
			done_right=1;
		}
		if((get_prox(SENSORFRONT1)+get_prox(SENSORFRONT2))/2>THRESHOLD_WALL_IN_FRONT){
			done_left=1;
			done_right=1;
			left_motor_set_speed(0);
			right_motor_set_speed(0);
		}
	}
}

void move_backward_smallstep(void){

	uint8_t done_left=0, done_right=0;
	right_motor_set_pos(0);
	left_motor_set_pos(0);
	// move forward
	left_motor_set_speed(-WORKING_SPEED);
	right_motor_set_speed(-WORKING_SPEED);
	while(done_left+done_right!=2){
		if(left_motor_get_pos()<=-SMALL_STEP_BACKWARD){
			left_motor_set_speed(0);
			done_left=1;
		}
		if(right_motor_get_pos()<=-SMALL_STEP_BACKWARD){
			right_motor_set_speed(0);
			done_right=1;
		}
	}
}

