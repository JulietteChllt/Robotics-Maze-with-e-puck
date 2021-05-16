#ifndef MOTOR_REGULATOR_H
#define MOTOR_REGULATOR_H

#define K_P							5
#define WORKING_SPEED 				300
#define QUARTER_TURN 				310
#define SMALL_STEP_FORWARD 			500
#define SMALL_STEP_BACKWARD 		300

//Creates the motor regulator thread
void motor_regulator_start(void);

//Makes the robot follow the wall on it's right
void follow_wall(void);

//Turns the robot 90° clockwise
void turn_clockwise(void);

//Turns the robot 90° counterclockwise
void turn_counterclockwise(void);

//Makes the robot take a small step forward, the robot is stopped if it gets to close to a wall
void move_forward_smallstep(void);

/*Makes the robot take a small step backward, this improves the color perception of the robot when opening the camera
 * to chose the next direction to take or to check if the end of the labyrinth was reached */
void move_backward_smallstep(void);

#endif /* PI_REGULATOR_H */
