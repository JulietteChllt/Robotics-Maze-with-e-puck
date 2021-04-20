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

#define WORKING_SPEED 200

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t err=0, sum_err=0;
    uint8_t kp=20, ki=2;
    int16_t command=0;
    while(1){
        time = chVTGetSystemTime();

        /*
		*	To complete
		*/
        err = (int16_t) get_calibrated_prox(SENSORRIGHT) - get_reference();
        command = abs(kp*err); //ajouter ki



        if(err<0){ //on est trop eloigné du mur de droite --> ajouter un peu de vitesse au moteur de gauche
        	right_motor_set_speed(WORKING_SPEED);
        	left_motor_set_speed(WORKING_SPEED + command);
        }
        else{
        	right_motor_set_speed(WORKING_SPEED + command);
        	left_motor_set_speed(WORKING_SPEED);
        }

        chprintf((BaseSequentialStream *) &SDU1, "valeur senseur droit = %d, ref= %d err= %d\n", get_calibrated_prox(SENSORRIGHT),get_reference(), err);

        /*sum_err += err;
        if(sum_err > MOTOR_SPEED_LIMIT/ki)
        	sum_err = MOTOR_SPEED_LIMIT/ki;*/


        //applies the speed from the PI regulator



        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(10));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}
