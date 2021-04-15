#include "ch.h"
#include "hal.h"
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>


#include <main.h>
#include <motors.h>
#include <pi_regulator.h>
#include <process_image.h>

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    float reference = 10.0;
    float err=0;
    float sum_err = 0;
    float prev_err=0;
    uint16_t kp=500; //determiner kp ATTENTION PEUT ETRE 16BIT??
    uint8_t ki=4;
    int16_t command=0;
    uint16_t wait=0;

    while(1){
        time = chVTGetSystemTime();

        /*
		*	To complete
		*/
        err=get_distance_cm()-reference;
        sum_err += err;
        if(sum_err > MOTOR_SPEED_LIMIT/ki)
        	sum_err = MOTOR_SPEED_LIMIT/ki;
        if(wait>=5)
        {
        	//chprintf((BaseSequentialStream *) &SDU1, "Up = %f, Ui= %f	\n",kp*err, ki*sum_err);
        	chprintf((BaseSequentialStream *) &SDU1, "distance cm = %f, erreur = %f, somm err = %f \n",get_distance_cm(), err, sum_err);
        	wait=0;

        }

        command= (int16_t) kp*err + ki*sum_err; //PROBLEME AVEC TERME INTEGRAL!!!!!
       /* if(command<MOTOR_SPEED_LIMIT)
        	speed=command;
        else speed = 0; //vitesse de sécurité assez lente
        */
        //applies the speed from the PI regulator
		right_motor_set_speed(command);
		left_motor_set_speed(command);

		prev_err=err;
		wait++;

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
