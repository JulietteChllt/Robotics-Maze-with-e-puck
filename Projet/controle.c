#include "ch.h"
#include "hal.h"
#include <usbcfg.h>
#include "stdio.h"
#include <main.h>
#include "sensors/proximity.h"
#include "controle.h"

static volatile int variable_reference=0;
static volatile uint8_t direction_changed=0;

/*=================================================================================================================*/
/*================================================ Thread definition ==============================================*/
/*=================================================================================================================*/

static BSEMAPHORE_DECL(open_camera_sem, TRUE);

static THD_WORKING_AREA(waControle, 256);
static THD_FUNCTION(Controle, arg){
	chRegSetThreadName(__FUNCTION__);
	(void)arg;
	uint8_t did_ref=0, prev_front= get_free_space_front(), prev_left= get_free_space_left(), prev_right= get_free_space_right(), new_front= 0, new_left=0, new_right=0;
	int old_ref=0, new_ref=0;

	while(1){
		new_front= get_free_space_front();
		new_left= get_free_space_left();
		new_right= get_free_space_right();

		//Checks whether the direction the robot is suppose to follow has changed
		if(new_front!=prev_front || new_left!=prev_left || new_right!=prev_right)
			direction_changed=1;
		else
			direction_changed=0;

		//Computes the reference to follow the right wall
		while(!did_ref){
			do_new_reference();
			new_ref=variable_reference;
			if(new_ref>= STABILITY_THRESHOLD && abs(new_ref-old_ref)<STABILITY_THRESHOLD)
				did_ref=1;
			else old_ref=new_ref;
		}

		prev_front = new_front;
		prev_left = new_left;
		prev_right = new_right;

		chThdSleepMilliseconds(5);
	}
}

/*=================================================================================================================*/
/*================================================ Helper functions ===============================================*/
/*=================================================================================================================*/

void controle_start(void){
	chThdCreateStatic(waControle, sizeof(waControle), NORMALPRIO, Controle, NULL);
}

void wait_semaphore_ready(void){
	chBSemWait(&open_camera_sem);
}

void semaphore_ready(void){
	chBSemSignal(&open_camera_sem);
}

uint8_t get_free_space(uint8_t sensor1){
	return (get_prox(sensor1)>=THRESHOLD_CLOSE_OBSTACLE)? 0: 1;
}

uint8_t get_free_space_front(void){
	return ((get_prox(SENSORFRONT1)+get_prox(SENSORFRONT2))/2>THRESHOLD_CLOSE_OBSTACLE-30)? 0: 1; //peut etre diminuer threshold front
}

uint8_t get_free_space_left(void){
	return get_free_space(SENSORLEFT);
}

uint8_t get_free_space_right(void){
	return get_free_space(SENSORRIGHT);
}

void do_new_reference(void){
	variable_reference=get_prox(SENSORRIGHT);
}

int get_reference(void){
	return variable_reference;
}

uint8_t get_possible_directions(void){
	return (get_free_space_front()+get_free_space_left()+get_free_space_right());
}

uint8_t get_direction_changed(void){
	return direction_changed;
}

