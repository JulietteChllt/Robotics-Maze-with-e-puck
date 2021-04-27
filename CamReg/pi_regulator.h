#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

#define MOTOR_ONE_TURN	1000
//start the PI regulator thread
void pi_regulator_start(void);
void follow_wall(uint8_t sensor);

#endif /* PI_REGULATOR_H */
