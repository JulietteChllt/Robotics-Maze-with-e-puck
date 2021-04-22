#ifndef PI_REGULATOR_H
#define PI_REGULATOR_H

//start the PI regulator thread
void pi_regulator_start(void);
void follow_wall(uint8_t sensor);

#endif /* PI_REGULATOR_H */
