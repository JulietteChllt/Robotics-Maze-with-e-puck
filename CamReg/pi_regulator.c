#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

#define K_P 			120
#define K_I				1
#define MAX_SUM_ERROR 	(MOTOR_SPEED_LIMIT/K_I)
#define ERROR_THRESHOLD			0.1f

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

	chRegSetThreadName(__FUNCTION__);
	(void)arg;

	systime_t time;

	int16_t speed = 0;
	float reference = 10.0;
	float err=0;
	float sum_err = 0;
	//uint16_t wait=0;

	while(1){
		time = chVTGetSystemTime();

		err=get_distance_cm()-reference;
		sum_err+=err;

		//disables the PI regulator if the error is to small
		if(fabs(err) < ERROR_THRESHOLD){
			speed = 0;
		}
		//set a maximum and a minimum for the sum to avoid an uncontrolled growth
		if(sum_err > MAX_SUM_ERROR){
			sum_err = MAX_SUM_ERROR;
		}else if(sum_err < -MAX_SUM_ERROR){
			sum_err = -MAX_SUM_ERROR;
		}
		/*if(wait>=60)
		{
			//chprintf((BaseSequentialStream *) &SDU1, "Up = %f, Ui= %f	\n",kp*err, ki*sum_err);
			wait=0;
		}*/
		speed= (int16_t) K_P*err + K_I*sum_err; //PROBLEME AVEC TERME INTEGRAL!!!!!

		//applies the speed from the PI regulator
		right_motor_set_speed(speed);
		left_motor_set_speed(speed);

		//wait++;

		//100Hz
		chThdSleepUntilWindowed(time, time + MS2ST(10));
	}
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
